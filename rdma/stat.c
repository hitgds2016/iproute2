// SPDX-License-Identifier: GPL-2.0 OR Linux-OpenIB
/*
 * rdma.c	RDMA tool
 * Authors:     Mark Zhang <markz@mellanox.com>
 */

#include "rdma.h"
#include "res.h"
#include <inttypes.h>

static int stat_help(struct rd *rd)
{
	pr_out("Usage: %s [ OPTIONS ] statistic { COMMAND | help }\n", rd->filename);
	pr_out("       %s statistic OBJECT show\n", rd->filename);
	pr_out("       %s statistic OBJECT show link [ DEV/PORT_INDEX ] [ FILTER-NAME FILTER-VALUE ]\n", rd->filename);
	pr_out("Examples:\n");
	pr_out("       %s statistic qp show\n", rd->filename);
	pr_out("       %s statistic qp show link mlx5_2/1\n", rd->filename);

	return 0;
}

static int res_get_hwcounters(struct rd *rd, struct nlattr *hwc_table, bool print)
{
	struct nlattr *nla_entry;
	const char *nm;
	uint64_t v;
	int err;

	mnl_attr_for_each_nested(nla_entry, hwc_table) {
		struct nlattr *hw_line[RDMA_NLDEV_ATTR_MAX] = {};

		err = mnl_attr_parse_nested(nla_entry, rd_attr_cb, hw_line);
		if (err != MNL_CB_OK)
			return -EINVAL;

		if (!hw_line[RDMA_NLDEV_ATTR_STAT_HWCOUNTER_ENTRY_NAME] ||
		    !hw_line[RDMA_NLDEV_ATTR_STAT_HWCOUNTER_ENTRY_VALUE]) {
			return -EINVAL;
		}

		if (!print)
			continue;

		nm = mnl_attr_get_str(hw_line[RDMA_NLDEV_ATTR_STAT_HWCOUNTER_ENTRY_NAME]);
		v = mnl_attr_get_u64(hw_line[RDMA_NLDEV_ATTR_STAT_HWCOUNTER_ENTRY_VALUE]);
		if (rd->pretty_output && !rd->json_output)
			newline_indent(rd);
		res_print_uint(rd, nm, v, hw_line[RDMA_NLDEV_ATTR_STAT_HWCOUNTER_ENTRY_NAME]);
	}

	return MNL_CB_OK;
}

static int res_counter_line(struct rd *rd, const char *name, int index,
		       struct nlattr **nla_line)
{
	uint32_t cntn, port = 0, pid = 0, qpn;
	struct nlattr *hwc_table, *qp_table;
	struct nlattr *nla_entry;
	const char *comm = NULL;
	bool isfirst;
	int err;

	if (nla_line[RDMA_NLDEV_ATTR_PORT_INDEX])
		port = mnl_attr_get_u32(nla_line[RDMA_NLDEV_ATTR_PORT_INDEX]);

	hwc_table = nla_line[RDMA_NLDEV_ATTR_STAT_HWCOUNTERS];
	qp_table = nla_line[RDMA_NLDEV_ATTR_RES_QP];
	if (!hwc_table || !qp_table ||
	    !nla_line[RDMA_NLDEV_ATTR_STAT_COUNTER_ID])
		return MNL_CB_ERROR;

	cntn = mnl_attr_get_u32(nla_line[RDMA_NLDEV_ATTR_STAT_COUNTER_ID]);
	if (rd_is_filtered_attr(rd, "cntn", cntn,
				nla_line[RDMA_NLDEV_ATTR_STAT_COUNTER_ID]))
		return MNL_CB_OK;

	if (nla_line[RDMA_NLDEV_ATTR_RES_PID]) {
		pid = mnl_attr_get_u32(nla_line[RDMA_NLDEV_ATTR_RES_PID]);
		comm = get_task_name(pid);
	}
	if (rd_is_filtered_attr(rd, "pid", pid,
				nla_line[RDMA_NLDEV_ATTR_RES_PID]))
		return MNL_CB_OK;

	if (nla_line[RDMA_NLDEV_ATTR_RES_KERN_NAME])
		comm = (char *)mnl_attr_get_str(
			nla_line[RDMA_NLDEV_ATTR_RES_KERN_NAME]);

	mnl_attr_for_each_nested(nla_entry, qp_table) {
		struct nlattr *qp_line[RDMA_NLDEV_ATTR_MAX] = {};

		err = mnl_attr_parse_nested(nla_entry, rd_attr_cb, qp_line);
		if (err != MNL_CB_OK)
			return -EINVAL;

		if (!qp_line[RDMA_NLDEV_ATTR_RES_LQPN])
			return -EINVAL;

		qpn = mnl_attr_get_u32(qp_line[RDMA_NLDEV_ATTR_RES_LQPN]);
		if (rd_is_filtered_attr(rd, "lqpn", qpn,
					qp_line[RDMA_NLDEV_ATTR_RES_LQPN]))
			return MNL_CB_OK;
	}

	err = res_get_hwcounters(rd, hwc_table, false);
	if (err != MNL_CB_OK)
		return err;

	if (rd->json_output) {
		jsonw_string_field(rd->jw, "ifname", name);
		if (port)
			jsonw_uint_field(rd->jw, "port", port);
		jsonw_uint_field(rd->jw, "cntn", cntn);
	} else {
		if (port)
			pr_out("link %s/%u cntn %u ", name, port, cntn);
		else
			pr_out("dev %s cntn %u ", name, cntn);
	}

	res_print_uint(rd, "pid", pid, nla_line[RDMA_NLDEV_ATTR_RES_PID]);
	print_comm(rd, comm, nla_line);

	res_get_hwcounters(rd, hwc_table, true);

	isfirst = true;
	mnl_attr_for_each_nested(nla_entry, qp_table) {
		struct nlattr *qp_line[RDMA_NLDEV_ATTR_MAX] = {};

		if (isfirst && !rd->json_output)
			pr_out("\n    LQPN: <");

		err = mnl_attr_parse_nested(nla_entry, rd_attr_cb, qp_line);
		if (err != MNL_CB_OK)
			return -EINVAL;

		if (!qp_line[RDMA_NLDEV_ATTR_RES_LQPN])
			return -EINVAL;

		qpn = mnl_attr_get_u32(qp_line[RDMA_NLDEV_ATTR_RES_LQPN]);
		if (rd->json_output) {
			jsonw_uint_field(rd->jw, "lqpn", qpn);
		} else {
			if (isfirst)
				pr_out("%d", qpn);
			else
				pr_out(", %d", qpn);
		}
		isfirst = false;
	}

	if (!rd->json_output)
		pr_out(">\n");
	return MNL_CB_OK;
}

static int stat_qp_show_parse_cb(const struct nlmsghdr *nlh, void *data)
{
	struct nlattr *tb[RDMA_NLDEV_ATTR_MAX] = {};
	struct nlattr *nla_table, *nla_entry;
	struct rd *rd = data;
	const char *name;
	uint32_t idx;
	int ret;

	mnl_attr_parse(nlh, 0, rd_attr_cb, tb);
	if (!tb[RDMA_NLDEV_ATTR_DEV_INDEX] || !tb[RDMA_NLDEV_ATTR_DEV_NAME] ||
	    !tb[RDMA_NLDEV_ATTR_STAT_COUNTER])
		return MNL_CB_ERROR;

	name = mnl_attr_get_str(tb[RDMA_NLDEV_ATTR_DEV_NAME]);
	idx = mnl_attr_get_u32(tb[RDMA_NLDEV_ATTR_DEV_INDEX]);
	nla_table = tb[RDMA_NLDEV_ATTR_STAT_COUNTER];

	mnl_attr_for_each_nested(nla_entry, nla_table) {
		struct nlattr *nla_line[RDMA_NLDEV_ATTR_MAX] = {};

		ret = mnl_attr_parse_nested(nla_entry, rd_attr_cb, nla_line);
		if (ret != MNL_CB_OK)
			break;

		ret = res_counter_line(rd, name, idx, nla_line);
		if (ret != MNL_CB_OK)
			break;
	}

	return ret;
}

static const struct filters stat_valid_filters[MAX_NUMBER_OF_FILTERS] = {
	{ .name = "cntn", .is_number = true },
	{ .name = "lqpn", .is_number = true },
	{ .name = "pid", .is_number = true },
};

static int stat_qp_show_one_link(struct rd *rd)
{
	int flags = NLM_F_REQUEST | NLM_F_ACK | NLM_F_DUMP;
	uint32_t seq;
	int ret;

	if (!rd->port_idx)
		return 0;

	ret = rd_build_filter(rd, stat_valid_filters);
	if (ret)
		return ret;

	rd_prepare_msg(rd, RDMA_NLDEV_CMD_STAT_GET, &seq, flags);
	mnl_attr_put_u32(rd->nlh, RDMA_NLDEV_ATTR_DEV_INDEX, rd->dev_idx);
	mnl_attr_put_u32(rd->nlh, RDMA_NLDEV_ATTR_PORT_INDEX, rd->port_idx);
	mnl_attr_put_u32(rd->nlh, RDMA_NLDEV_ATTR_STAT_RES, RDMA_NLDEV_ATTR_RES_QP);
	ret = rd_send_msg(rd);
	if (ret)
		return ret;

	if (rd->json_output)
		jsonw_start_object(rd->jw);
	ret = rd_recv_msg(rd, stat_qp_show_parse_cb, rd, seq);
	if (rd->json_output)
		jsonw_end_object(rd->jw);

	return ret;
}

static int stat_qp_show_link(struct rd *rd)
{
	return rd_exec_link(rd, stat_qp_show_one_link, false);
}

static int stat_qp_show(struct rd *rd)
{
	const struct rd_cmd cmds[] = {
		{ NULL,		stat_qp_show_link },
		{ "link",	stat_qp_show_link },
		{ "help",	stat_help },
		{ 0 }
	};

	return rd_exec_cmd(rd, cmds, "parameter");
}

static int stat_qp(struct rd *rd)
{
	const struct rd_cmd cmds[] =  {
		{ NULL,		stat_qp_show },
		{ "show",	stat_qp_show },
		{ "list",	stat_qp_show },
		{ "help",	stat_help },
		{ 0 }
	};

	return rd_exec_cmd(rd, cmds, "parameter");
}

int cmd_stat(struct rd *rd)
{
	const struct rd_cmd cmds[] =  {
		{ NULL,		stat_help },
		{ "help",	stat_help },
		{ "qp",		stat_qp },
		{ 0 }
	};

	return rd_exec_cmd(rd, cmds, "statistic command");
}