
clean: clean-vivado clean-synplify

clean-vivado: 
	rm -rf loopback
	rm -f vivado_pid*
	rm -f vivado.jou vivado.log
	rm -f *.backup.jou
	rm -f *.backup.log
	rm -rf .Xil
	rm -f ipc_tcp_xilinx_srv.log ipc_tcp_xilinx.log

clean-synplify:
	rm -rf rev_?
	rm -rf _xmsgs
	rm -f stdout.log synlog.tcl xilinx_device_details.xml
	rm -f proj_1.prd
	rm -f loopback_synplify.prd
	rm -f run_vivado.tcl
	rm -f synplify.prj

