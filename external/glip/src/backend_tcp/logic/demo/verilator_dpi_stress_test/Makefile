
VERILATOR ?=verilator

srcs_verilog = tb_verilator_dpi_stress_test.v \
           ../../dpi/glip_tcp_toplevel.sv \
           ../../../../common/logic/interface/glip_channel.sv \
           ../../../../common/logic/stress_test/io_stress_test.v \
           ../../../../common/logic/stress_test/stress_test_lfsr.v
		   

srcs_cxx = main.cpp \
           ../../dpi/glip_tcp_dpi.cpp \
           ../../dpi/GlipTcp.cpp

flags = -CFLAGS "-I../../../dpi -std=c++11 -g" -LDFLAGS "-pthread" --exe --trace

all: demo

demo: obj_dir/Vtb_verilator_dpi_stress_test.mk
	make -C obj_dir -f Vtb_verilator_dpi_stress_test.mk
	
obj_dir/Vtb_verilator_dpi_stress_test.mk: $(srcs_verilog) $(srcs_cxx)
	$(VERILATOR) --cc $(srcs_verilog) $(srcs_cxx) $(flags) -o ../demo
	touch obj_dir/Vtb_verilator_dpi_stress_test.mk
	
clean:
	rm -rf demo obj_dir
