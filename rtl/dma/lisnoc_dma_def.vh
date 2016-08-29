`include "lisnoc_def.vh"

`define FLIT_WIDTH 34

// Type of flit
// The coding is chosen, so that
// type[0] signals that this is the first flit of a packet
// type[1] signals that this is the last flit of a packet

`define FLIT_TYPE_MSB (`FLIT_WIDTH - 1)
`define FLIT_TYPE_WIDTH 2
`define FLIT_TYPE_LSB (`FLIT_TYPE_MSB - `FLIT_TYPE_WIDTH + 1)

// This is the flit content size
`define FLIT_CONTENT_WIDTH 32
`define FLIT_CONTENT_MSB   31
`define FLIT_CONTENT_LSB    0

// The following fields are only valid for header flits
`define FLIT_DEST_WIDTH 5
// destination address field of header flit
`define FLIT_DEST_MSB `FLIT_CONTENT_MSB
`define FLIT_DEST_LSB `FLIT_DEST_MSB - `FLIT_DEST_WIDTH + 1

// packet type field  of header flit
`define PACKET_CLASS_MSB (`FLIT_DEST_LSB - 1)
`define PACKET_CLASS_WIDTH 3
`define PACKET_CLASS_LSB (`PACKET_CLASS_MSB - `PACKET_CLASS_WIDTH + 1)

`define PACKET_CLASS_DMA 3'b010

// source address field  of header flit
`define SOURCE_MSB 23
`define SOURCE_WIDTH 5
`define SOURCE_LSB 19

// packet id field  of header flit
`define PACKET_ID_MSB   18
`define PACKET_ID_WIDTH 4
`define PACKET_ID_LSB   15

`define PACKET_TYPE_MSB   14
`define PACKET_TYPE_WIDTH 2
`define PACKET_TYPE_LSB   13

`define PACKET_TYPE_L2R_REQ  2'b00
`define PACKET_TYPE_R2L_REQ  2'b01
`define PACKET_TYPE_L2R_RESP 2'b10
`define PACKET_TYPE_R2L_RESP 2'b11

`define PACKET_REQ_LAST 12
`define PACKET_RESP_LAST 12

`define SIZE_MSB 31
`define SIZE_WIDTH 32
`define SIZE_LSB   0

`define DMA_REQUEST_WIDTH 103

`define DMA_REQFIELD_LADDR_WIDTH 32
`define DMA_REQFIELD_SIZE_WIDTH  32
`define DMA_REQFIELD_RTILE_WIDTH  5
`define DMA_REQFIELD_RADDR_WIDTH 32

`define DMA_REQFIELD_LADDR_MSB 102
`define DMA_REQFIELD_LADDR_LSB 70
`define DMA_REQFIELD_SIZE_MSB  69
`define DMA_REQFIELD_SIZE_LSB  38
`define DMA_REQFIELD_RTILE_MSB 37
`define DMA_REQFIELD_RTILE_LSB 33
`define DMA_REQFIELD_RADDR_MSB 32
`define DMA_REQFIELD_RADDR_LSB 1
`define DMA_REQFIELD_DIR       0

`define DMA_REQUEST_INVALID 1'b0
`define DMA_REQUEST_VALID   1'b1

`define DMA_REQUEST_L2R 1'b0
`define DMA_REQUEST_R2L 1'b1

`define DMA_REQMASK_WIDTH 5
`define DMA_REQMASK_LADDR 0
`define DMA_REQMASK_SIZE  1
`define DMA_REQMASK_RTILE 2
`define DMA_REQMASK_RADDR 3
`define DMA_REQMASK_DIR   4

`define DMA_RESPFIELD_SIZE_WIDTH 14

