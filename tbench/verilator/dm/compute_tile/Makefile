all: tb_compute_tile tb_compute_tile-vcd

SOURCES=main.cpp

main.o: main.cpp obj_dir/Vtb_compute_tile.a
	g++ -Wall -I $(SYSTEMC)/include -I $(VERILATOR_ROOT)/include -I $(VERILATOR_ROOT)/include/vltstd -I $(OPTIMSOC_SYSC)/TraceMonitor/ -o main.o -c main.cpp

main-vcd.o: main.cpp obj_dir/Vtb_compute_tile.a
	g++ -Wall -I $(SYSTEMC)/include -I $(VERILATOR_ROOT)/include -I $(VERILATOR_ROOT)/include/vltstd -I $(OPTIMSOC_SYSC)/TraceMonitor/ -DVCD_TRACE -o main-vcd.o -c main.cpp

TraceMonitor.o: $(OPTIMSOC_SYSC)/TraceMonitor/TraceMonitor.cpp $(OPTIMSOC_SYSC)/TraceMonitor/TraceMonitor.h
	g++ -Wall -I $(SYSTEMC)/include -o TraceMonitor.o -c $(OPTIMSOC_SYSC)/TraceMonitor/TraceMonitor.cpp

obj_dir/Vtb_compute_tile.a:
	verilator -sc -Wno-width -Wno-unoptflat -f tb_compute_tile.vc -trace
	make -C obj_dir -f Vtb_compute_tile.mk

verilated.o:
	g++ -o verilated.o -c $(VERILATOR_ROOT)/include/verilated.cpp

verilated_vcd_c.o:
	g++ -o verilated_vcd_c.o -c $(VERILATOR_ROOT)/include/verilated_vcd_c.cpp

tb_compute_tile: obj_dir/Vtb_compute_tile.a main.o verilated.o TraceMonitor.o verilated_vcd_c.o
	g++ -o tb_compute_tile -pthread -Wl,--start-group $(SYSTEMC)/lib-linux/libsystemc.a obj_dir/Vtb_compute_tile__ALL.a verilated.o main.o TraceMonitor.o verilated_vcd_c.o -Wl,--end-group

tb_compute_tile-vcd: obj_dir/Vtb_compute_tile.a main-vcd.o verilated.o verilated_vcd_c.o TraceMonitor.o
	g++ -o tb_compute_tile-vcd -pthread -Wl,--start-group $(SYSTEMC)/lib-linux/libsystemc.a obj_dir/Vtb_compute_tile__ALL.a verilated.o main-vcd.o TraceMonitor.o verilated_vcd_c.o -Wl,--end-group

clean:
	rm -rf obj_dir *.o tb_compute_tile
