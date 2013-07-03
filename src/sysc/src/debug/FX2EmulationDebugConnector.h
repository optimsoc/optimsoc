/*
 * This file is part of OpTiMSoC.
 *
 * OpTiMSoC is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * OpTiMSoC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with OpTiMSoC. If not, see <http://www.gnu.org/licenses/>.
 *
 * ============================================================================
 *
 * (c) 2012-2013 by the author(s)
 *
 * Author(s):
 *    Stefan Wallentowitz, stefan.wallentowitz@tum.de
 */

#ifndef FX2EMULATIONDEBUGCONNECTOR_H_
#define FX2EMULATIONDEBUGCONNECTOR_H_

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <queue>
#include <systemc.h>


/**
 * A SystemC module emulating the USB interface as used by the ZTEX boards
 *
 * This module has the same interface as the EZ-USB FX2 chip used on the
 * ZTEX 1.15 boards to communicate over USB 2.0 with the host, but instead of
 * using a physical USB connection a TCP connection is established.
 * The liboptimsochost library can then be used to talk with a
 * system simulation running in e.g. ModelSim as if it was using the real
 * USB connection.
 *
 * The TCP connection uses port 23000 for trace data and port 23001 for control
 * information (simulating the physical rest pin).
 *
 * @todo Change fx2 clock to be generated by the module
 */
SC_MODULE(FX2EmulationDebugConnector)
{
    sc_in<bool> fx2_ifclk;
    sc_out<bool> fx2_flaga;
    sc_out<bool> fx2_flagb;
    sc_out<bool> fx2_flagc;
    sc_out<bool> fx2_flagd;
    sc_in<bool> fx2_sloe;
    sc_in<bool> fx2_slrd;
    sc_in<bool> fx2_slwr;
    sc_in<bool> fx2_pktend;
    sc_in<sc_bv<2> > fx2_fifoadr;
    sc_inout<sc_lv<16> > fx2_fd;
    sc_out<bool> reset;

    bool in_reset;
    std::queue<uint16_t> input_queue;
    std::queue<uint16_t> output_queue;

    SC_CTOR(FX2EmulationDebugConnector)
    {
        SC_THREAD(handler);
        SC_THREAD(ctrlhandler);
        SC_METHOD(interface);
        sensitive << fx2_ifclk.posedge_event();
        SC_METHOD(drivefd);
        sensitive << fx2_sloe;
        in_reset = true;
    }

    void drivefd();
    void interface();
    void handler();
    void ctrlhandler();
};

#endif