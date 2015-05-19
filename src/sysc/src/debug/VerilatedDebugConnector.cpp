/* Copyright (c) 2013 by the author(s)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 */

#include "VerilatedDebugConnector.h"
#include "VerilatedSTM.h"

#include <cassert>
#include <iomanip>

VerilatedDebugConnector::VerilatedDebugConnector(sc_module_name nm, uint16_t systemid, bool standalone) :
        DebugConnector(nm, systemid), rst_sys("rst_sys"), rst_cpu("rst_cpu"), m_standalone(standalone)
{
}

void VerilatedDebugConnector::start()
{
	// De-assert reset signals
    rst_sys = 0;
    rst_cpu = 0;
}

void VerilatedDebugConnector::stop()
{
	// Assert reset signals
    rst_sys = 1;
    rst_cpu = 1;
}

void VerilatedDebugConnector::resetSystem()
{
	// Simply call stop
	stop();
}

void VerilatedDebugConnector::connection() {
	if (!m_standalone) {
		// In TCP mode simply run standard thread
		DebugConnector::connection();
	} else {
		// In standalone, tell so
		std::cout << "Run in standalone mode" << std::endl;

		// Build stdout vector size by finding largest coreid
		std::vector<DebugModule*>::iterator it;
		for (it = m_debugModules.begin(); it != m_debugModules.end(); ++it) {
			if ((*it)->getType() == DBGTYPE_STM) {
				VerilatedSTM *stm = (VerilatedSTM*) *it;
				if (stm->getCoreId() >= m_standalone_stdout.size()) {
					m_standalone_stdout.resize(stm->getCoreId() + 1);
				}
			}
		}

		// Set events vector size to stdout vector size (number of cores)
		m_standalone_events.resize(m_standalone_stdout.size());

		// Set finished vector size to stdout vector size (number of cores)
		m_standalone_finished.resize(m_standalone_stdout.size(), false);

		// Set the finished counter
		m_standalone_finished_count = 0;

		// Alloc streams and open files for each core
		for (int i = 0; i < m_standalone_stdout.size(); i++) {
			std::ofstream *ofStdout = new std::ofstream;
			std::ofstream *ofEvents = new std::ofstream;
			std::ostringstream fnameStdout;
			std::ostringstream fnameEvents;

			// Generate stdout filename and open
			fnameStdout << "stdout." << std::setfill('0') << std::setw(3) << i;
			ofStdout->open(fnameStdout.str().c_str());
			m_standalone_stdout[i] = ofStdout;

			// Generate events filename and open
			fnameEvents << "events." << std::setfill('0') << std::setw(3) << i;
			ofEvents->open(fnameEvents.str().c_str());
			m_standalone_events[i] = ofEvents;
		}

		// Reset sequence
		rst_sys = 1;
		rst_cpu = 1;
		wait(clk.posedge_event()); // Wait to be sure it is really captured
		wait(clk.posedge_event()); // This is the reset edge
		wait(clk.negedge_event()); // De-assert on negative edge
		rst_sys = 0;
		rst_cpu = 0;
	}
}

bool VerilatedDebugConnector::sendTrace(DebugModule *mod, TracePacket &packet) {
	if (!m_standalone) {
		// If TCP server: call standard method
		DebugConnector::sendTrace(mod, packet);
	} else {
		// Extract STM packet (only known at the moment
		STMTracePacket *stmpacket = (STMTracePacket*) &packet;

		// Check the coreid matches
		assert(stmpacket->coreid < m_standalone_stdout.size());

		if (stmpacket->id == 4) {
			// putc packet, print to file
			*m_standalone_stdout[stmpacket->coreid] << (char) stmpacket->value;
			m_standalone_stdout[stmpacket->coreid]->flush();
		}

		// Write all packets to events file
		*m_standalone_events[stmpacket->coreid] << "[" << stmpacket->timestamp << "] ";
		*m_standalone_events[stmpacket->coreid] << "Event 0x" << std::hex << stmpacket->id;
		*m_standalone_events[stmpacket->coreid] << ": 0x" << stmpacket->value << std::dec << std::endl;

		if ((stmpacket->id == 1) || (stmpacket->id == 10)) {
			m_standalone_finished[stmpacket->coreid] = true;
			m_standalone_finished_count++;

			std::cout << "[" << stmpacket->timestamp << "] ";
			std::cout << "Core " << stmpacket->coreid << " has terminated" << std::endl;

			if (m_standalone_finished_count == m_standalone_finished.size()) {
				std::cout << "[" << stmpacket->timestamp << "] ";
				std::cout << "All cores terminated. Exiting.." << std::endl;
				exit(1);
			}
		}
	}
	return true;
}
