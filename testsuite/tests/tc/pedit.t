#!/bin/sh

source lib/generic.sh

DEV="$(rand_dev)"
ts_ip "$0" "Add $DEV dummy interface" link add dev $DEV type dummy
ts_ip "$0" "Enable $DEV" link set $DEV up
ts_tc "pedit" "Add ingress qdisc" qdisc add dev $DEV ingress


do_pedit() {
	ts_tc "pedit" "Drop ingress qdisc" \
		qdisc del dev $DEV ingress
	ts_tc "pedit" "Add ingress qdisc" \
		qdisc add dev $DEV ingress
	ts_tc "pedit" "Add pedit action $*" \
		filter add dev $DEV parent ffff: \
		u32 match u32 0 0 \
		action pedit munge $@
	ts_tc "pedit" "Show ingress filters" \
		filter show dev $DEV parent ffff:
}

do_pedit offset 12 u32 set 0x12345678
test_on "key #0  at 12: val 12345678 mask 00000000"
do_pedit offset 12 u16 set 0x1234
test_on "key #0  at 12: val 12340000 mask 0000ffff"
do_pedit offset 14 u16 set 0x1234
test_on "key #0  at 12: val 00001234 mask ffff0000"
do_pedit offset 12 u8 set 0x23
test_on "key #0  at 12: val 23000000 mask 00ffffff"
do_pedit offset 13 u8 set 0x23
test_on "key #0  at 12: val 00230000 mask ff00ffff"
do_pedit offset 14 u8 set 0x23
test_on "key #0  at 12: val 00002300 mask ffff00ff"
do_pedit offset 15 u8 set 0x23
test_on "key #0  at 12: val 00000023 mask ffffff00"

do_pedit offset 13 u8 invert
test_on "key #0  at 12: val 00ff0000 mask ffffffff"
do_pedit offset 13 u8 clear
test_on "key #0  at 12: val 00000000 mask ff00ffff"
do_pedit offset 13 u8 preserve
test_on "key #0  at 12: val 00000000 mask ffffffff"

# the following set of tests has been auto-generated by running this little
# shell script:
#
# do_it() {
#	echo "do_pedit $@"
#	tc qd del dev veth0 ingress >/dev/null 2>&1
#	tc qd add dev veth0 ingress >/dev/null 2>&1
#	tc filter add dev veth0 parent ffff: u32 \
#		match u32 0 0 \
#		action pedit munge $@ >/dev/null 2>&1
#	tc filter show dev veth0 parent ffff: | \
#		sed -n 's/^[\t ]*\(key #0.*\)/test_on "\1"/p'
# }
#
# do_it_all() { # (field, val1 [, val2, ...])
#	local field=$1
#	shift
#	for val in $@; do
#		do_it ip $field set $val
#	done
#	for i in preserve invert clear; do
#		do_it ip $field $i
#	done
# }
#
# do_it_all ihl 0x04 0x40
# do_it_all src 1.2.3.4
# do_it_all dst 1.2.3.4
# do_it_all tos 0x1 0x10
# do_it_all protocol 0x23
# do_it_all nofrag 0x23 0xf4
# do_it_all firstfrag 0x03 0xfa
# do_it_all ce 0x23 0x04 0xf3
# do_it_all df 0x23 0x04 0xf3
# do_it_all mf 0x23 0x04 0xf3
# do_it_all dport 0x1234
# do_it_all sport 0x1234
# do_it_all icmp_type 0x23
# do_it_all icmp_code 0x23

do_pedit ip ihl set 0x04
test_on "key #0  at 0: val 04000000 mask f0ffffff"
do_pedit ip ihl set 0x40
test_on "key #0  at 0: val 00000000 mask f0ffffff"
do_pedit ip ihl preserve
test_on "key #0  at 0: val 00000000 mask ffffffff"
do_pedit ip ihl invert
test_on "key #0  at 0: val 0f000000 mask ffffffff"
do_pedit ip ihl clear
test_on "key #0  at 0: val 00000000 mask f0ffffff"
do_pedit ip src set 1.2.3.4
test_on "key #0  at 12: val 01020304 mask 00000000"
do_pedit ip src preserve
test_on "key #0  at 12: val 00000000 mask ffffffff"
do_pedit ip src invert
test_on "key #0  at 12: val ffffffff mask ffffffff"
do_pedit ip src clear
test_on "key #0  at 12: val 00000000 mask 00000000"
do_pedit ip dst set 1.2.3.4
test_on "key #0  at 16: val 01020304 mask 00000000"
do_pedit ip dst preserve
test_on "key #0  at 16: val 00000000 mask ffffffff"
do_pedit ip dst invert
test_on "key #0  at 16: val ffffffff mask ffffffff"
do_pedit ip dst clear
test_on "key #0  at 16: val 00000000 mask 00000000"
do_pedit ip tos set 0x1
test_on "key #0  at 0: val 00010000 mask ff00ffff"
do_pedit ip tos set 0x10
test_on "key #0  at 0: val 00100000 mask ff00ffff"
do_pedit ip tos preserve
test_on "key #0  at 0: val 00000000 mask ffffffff"
do_pedit ip tos invert
test_on "key #0  at 0: val 00ff0000 mask ffffffff"
do_pedit ip tos clear
test_on "key #0  at 0: val 00000000 mask ff00ffff"
do_pedit ip protocol set 0x23
test_on "key #0  at 8: val 00230000 mask ff00ffff"
do_pedit ip protocol preserve
test_on "key #0  at 8: val 00000000 mask ffffffff"
do_pedit ip protocol invert
test_on "key #0  at 8: val 00ff0000 mask ffffffff"
do_pedit ip protocol clear
test_on "key #0  at 8: val 00000000 mask ff00ffff"
do_pedit ip nofrag set 0x23
test_on "key #0  at 4: val 00002300 mask ffffc0ff"
do_pedit ip nofrag set 0xf4
test_on "key #0  at 4: val 00003400 mask ffffc0ff"
do_pedit ip nofrag preserve
test_on "key #0  at 4: val 00000000 mask ffffffff"
do_pedit ip nofrag invert
test_on "key #0  at 4: val 00003f00 mask ffffffff"
do_pedit ip nofrag clear
test_on "key #0  at 4: val 00000000 mask ffffc0ff"
do_pedit ip firstfrag set 0x03
test_on "key #0  at 4: val 00000300 mask ffffe0ff"
do_pedit ip firstfrag set 0xfa
test_on "key #0  at 4: val 00001a00 mask ffffe0ff"
do_pedit ip firstfrag preserve
test_on "key #0  at 4: val 00000000 mask ffffffff"
do_pedit ip firstfrag invert
test_on "key #0  at 4: val 00001f00 mask ffffffff"
do_pedit ip firstfrag clear
test_on "key #0  at 4: val 00000000 mask ffffe0ff"
do_pedit ip ce set 0x23
test_on "key #0  at 4: val 00000000 mask ffff7fff"
do_pedit ip ce set 0x04
test_on "key #0  at 4: val 00000000 mask ffff7fff"
do_pedit ip ce set 0xf3
test_on "key #0  at 4: val 00008000 mask ffff7fff"
do_pedit ip ce preserve
test_on "key #0  at 4: val 00000000 mask ffffffff"
do_pedit ip ce invert
test_on "key #0  at 4: val 00008000 mask ffffffff"
do_pedit ip ce clear
test_on "key #0  at 4: val 00000000 mask ffff7fff"
do_pedit ip df set 0x23
test_on "key #0  at 4: val 00000000 mask ffffbfff"
do_pedit ip df set 0x04
test_on "key #0  at 4: val 00000000 mask ffffbfff"
do_pedit ip df set 0xf3
test_on "key #0  at 4: val 00004000 mask ffffbfff"
do_pedit ip df preserve
test_on "key #0  at 4: val 00000000 mask ffffffff"
do_pedit ip df invert
test_on "key #0  at 4: val 00004000 mask ffffffff"
do_pedit ip df clear
test_on "key #0  at 4: val 00000000 mask ffffbfff"
do_pedit ip mf set 0x23
test_on "key #0  at 4: val 00002000 mask ffffdfff"
do_pedit ip mf set 0x04
test_on "key #0  at 4: val 00000000 mask ffffdfff"
do_pedit ip mf set 0xf3
test_on "key #0  at 4: val 00002000 mask ffffdfff"
do_pedit ip mf preserve
test_on "key #0  at 4: val 00000000 mask ffffffff"
do_pedit ip mf invert
test_on "key #0  at 4: val 00002000 mask ffffffff"
do_pedit ip mf clear
test_on "key #0  at 4: val 00000000 mask ffffdfff"
do_pedit ip dport set 0x1234
test_on "key #0  at 20: val 00001234 mask ffff0000"
do_pedit ip dport preserve
test_on "key #0  at 20: val 00000000 mask ffffffff"
do_pedit ip dport invert
test_on "key #0  at 20: val 0000ffff mask ffffffff"
do_pedit ip dport clear
test_on "key #0  at 20: val 00000000 mask ffff0000"
do_pedit ip sport set 0x1234
test_on "key #0  at 20: val 12340000 mask 0000ffff"
do_pedit ip sport preserve
test_on "key #0  at 20: val 00000000 mask ffffffff"
do_pedit ip sport invert
test_on "key #0  at 20: val ffff0000 mask ffffffff"
do_pedit ip sport clear
test_on "key #0  at 20: val 00000000 mask 0000ffff"
do_pedit ip icmp_type set 0x23
test_on "key #0  at 20: val 23000000 mask 00ffffff"
do_pedit ip icmp_type preserve
test_on "key #0  at 20: val 00000000 mask ffffffff"
do_pedit ip icmp_type invert
test_on "key #0  at 20: val ff000000 mask ffffffff"
do_pedit ip icmp_type clear
test_on "key #0  at 20: val 00000000 mask 00ffffff"
do_pedit ip icmp_code set 0x23
test_on "key #0  at 20: val 23000000 mask 00ffffff"
do_pedit ip icmp_code preserve
test_on "key #0  at 20: val 00000000 mask ffffffff"
do_pedit ip icmp_code invert
test_on "key #0  at 20: val ff000000 mask ffffffff"
do_pedit ip icmp_code clear
test_on "key #0  at 20: val 00000000 mask 00ffffff"