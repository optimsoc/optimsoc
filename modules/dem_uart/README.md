# Device Emulation Module - UART

This module can be connected to a bus and behaves like a standard UART
device. Instead of driving RX/TX lines it instead sends the packet
over the debug interconnect to the host. For more details see the
[specification].

[specification]: doc/specification.md
