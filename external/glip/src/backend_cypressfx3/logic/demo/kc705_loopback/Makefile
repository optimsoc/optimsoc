
all: vivado-project

vivado-project-32: vivado-project

vivado-project-16:
	vivado -mode batch -source vivado_16.tcl

vivado-project:
	vivado -mode batch -source vivado_32.tcl

clean: clean-vivado 

clean-vivado: 
	rm -rf vivado
	rm -f vivado_pid*
	rm -f vivado.jou vivado.log
	rm -f *.backup.jou
	rm -f *.backup.log
	rm -rf .Xil
	rm -f ipc_tcp_xilinx_srv.log ipc_tcp_xilinx.log
	rm -rf kc705_loopback_16
	rm -rf kc705_loopback_32

