package measurepkg;

// All forward declarations
typedef class packet;
typedef class flit;
typedef class packetid;
typedef class sysconfig;
typedef class trafficdesc;
typedef class poisson;
typedef class networkacc;
typedef class measurement;
typedef class environment;
typedef class trafficgen;
typedef class sink;

// Include the sources
`include "packet.sv"
`include "sysconfig.sv"
`include "poisson.sv"
`include "networkacc.sv"
`include "measurement.sv"
`include "environment.sv"
`include "trafficgen.sv"
`include "sink.sv"

endpackage // measure
