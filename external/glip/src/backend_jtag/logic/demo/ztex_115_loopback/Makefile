SYNPLIFY ?= synplify

synplify: loopback_synplify.prj
	$(SYNPLIFY) -batch loopback_synplify.prj -impl rev_1

bitgen: rev_1/pr_1/ztex_115_loopback.bit

flash: rev_1/pr_1/ztex_115_loopback.bit
	FWLoader -v 0x221a 0x100 -f -uf $<

clean: clean-synplify

clean-synplify:
	rm -rf rev_?
	rm -rf _xmsgs
	rm -f stdout.log synlog.tcl xilinx_device_details.xml
	rm -f proj_1.prd
	rm -f loopback_synplify.prd
	rm -f synplify.prj
	rm -f ipc_tcp_xilinx.log  ipc_tcp_xilinx_srv.log

%.bit: %.ncd
	bitgen -w $*.ncd
 
