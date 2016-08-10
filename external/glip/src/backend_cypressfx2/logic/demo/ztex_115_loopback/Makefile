
clean: clean-synplify

bitgen: rev_1/pr_1/ztex_115_loopback.ncd
	bitgen -w rev_1/pr_1/ztex_115_loopback.ncd

configure: bitgen
	FWLoader -c -rf -uf rev_1/pr_1/ztex_115_loopback.bit

measure:
	glip_loopback_measure -b cypressfx2 -ousb_vid=0x221a,usb_pid=0x0100

clean-synplify:
	rm -rdf rev_?
	rm -rf _xmsgs
	rm -f stdout.log synlog.tcl xilinx_device_details.xml
	rm -f proj_1.prd
	rm -f loopback_synplify.prd
	rm -f run_vivado.tcl
	rm -f synplify.prj

