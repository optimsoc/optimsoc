/*
 * This file is part of OpTiMSoC-GUI.
 *
 * OpTiMSoC-GUI is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * OpTiMSoC-GUI is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with OpTiMSoC. If not, see <http://www.gnu.org/licenses/>.
 *
 * =================================================================
 *
 * (c) 2012-2013 by the author(s)
 *
 * Author(s):
 *    Philipp Wagner, philipp.wagner@tum.de
 *    Stefan Wallentowitz, stefan.wallentowitz@tum.de
 */

#include "hardwareinterface.h"

#include <QDebug>
#include <QMetaType>
#include <stdint.h>
#include <cstring>

HardwareInterface* HardwareInterface::s_instance = 0;

void HardwareInterface::instrTraceCallback(struct optimsoc_ctx *incoming_ctx,
                                           int core_id, uint32_t timestamp,
                                           uint32_t pc, int count)
{
    instance()->emitInstructionTraceReceived(core_id, timestamp, pc, count);
}

void HardwareInterface::softwareTraceCallback(uint32_t core_id,
                                              uint32_t timestamp,
                                              uint16_t id,
                                              uint32_t value)
{
    instance()->emitSoftwareTraceReceived(core_id, timestamp, id, value);
}

HardwareInterface::HardwareInterface(QObject *parent)
    : QObject(parent), m_octx(NULL), m_connectionStatus(Disconnected)
{
    qRegisterMetaType<ConnectionStatus>("HardwareInterface::ConnectionStatus");
}

HardwareInterface* HardwareInterface::instance()
{
    if (!HardwareInterface::s_instance) {
        HardwareInterface::s_instance = new HardwareInterface();
    }
    return HardwareInterface::s_instance;
}

HardwareInterface::~HardwareInterface()
{
    if (m_connectionStatus != Disconnected) {
        optimsoc_disconnect(m_octx);
    }
    optimsoc_free(m_octx);
}

void HardwareInterface::configure(optimsoc_backend_id backend,
                                  QMap<QString,QString> options)
{
    if (m_octx) {
        if (m_connectionStatus != Disconnected) {
            disconnect();
        }

        optimsoc_free(m_octx);
    }

    // The option array and the contained strings are only valid during the
    // initialization call. They are free'd below
    int optcount = 0;
    struct optimsoc_backend_option * optionarray;
    optionarray = new struct optimsoc_backend_option[options.size()];

    for (QMap<QString,QString>::const_iterator it = options.begin(); it != options.end(); ++it, ++optcount) {
        optionarray[optcount].name = strdup(it.key().toAscii().data());
        optionarray[optcount].value = strdup(it.value().toAscii().data());
    }

    optimsoc_new(&m_octx, (optimsoc_backend_id) backend, options.size(), optionarray);

    // Delete all strings
    for (optcount=0;optcount<options.size();++optcount) {
        delete(optionarray[optcount].name);
        delete(optionarray[optcount].value);
    }
    // Delete container
    delete(optionarray);
}

bool HardwareInterface::configured() {
    return (m_octx!=NULL);
}

/**
 * Connect to target hardware and discover system
 */
void HardwareInterface::connect()
{
    ConnectionStatus oldConnectionStatus = m_connectionStatus;
    m_connectionStatus = Connecting;
    emit connectionStatusChanged(oldConnectionStatus, m_connectionStatus);
    oldConnectionStatus = m_connectionStatus;

    int rv = optimsoc_connect(m_octx);
    if (rv != 0) {
        m_connectionStatus = Disconnected;
        emit connectionStatusChanged(oldConnectionStatus, m_connectionStatus);
        return;
    }

    rv = optimsoc_discover_system(m_octx);
    if (rv == 0) {
        m_connectionStatus = Connected;

        emit systemDiscovered(optimsoc_get_sysid(m_octx));
    } else {
        m_connectionStatus = Disconnected;
    }

    emit connectionStatusChanged(oldConnectionStatus, m_connectionStatus);

    // register callback function for instruction traces
    // XXX: This is currently disabled since it generates too much overhead.
    //      Find solution with better performance.
    /*optimsoc_itm_register_callback(m_octx,
                                   &HardwareInterface::instrTraceCallback);*/

    // register callback function for software traces
    optimsoc_stm_register_callback(m_octx,
                                   &HardwareInterface::softwareTraceCallback);
    return;
}

void HardwareInterface::disconnect()
{
    if (m_connectionStatus != Connected) {
        qWarning("Not connected to system!");
        return;
    }

    ConnectionStatus oldConnectionStatus = m_connectionStatus;
    m_connectionStatus = Disconnecting;
    emit connectionStatusChanged(oldConnectionStatus, m_connectionStatus);
    oldConnectionStatus = m_connectionStatus;

    int rv = optimsoc_disconnect(m_octx);
    if (rv == 0) {
        m_connectionStatus = Disconnected;
    } else {
        m_connectionStatus = Connected;
    }

    emit connectionStatusChanged(oldConnectionStatus, m_connectionStatus);
}

void HardwareInterface::initMemory(int memoryTileId, QByteArray data)
{
    if (m_connectionStatus != Connected) {
        qWarning("Not connected to system!");
        return;
    }

    int rv = optimsoc_mem_write(m_octx, 3, 0x00,
                                (const uint8_t*)(data.constData()),
                                data.length());
    emit memoryWriteFinished(rv == 0);
}

void HardwareInterface::reset()
{
    if (m_connectionStatus != Connected) {
        qWarning("Not connected to system!");
        return;
    }

    int rv = optimsoc_reset(m_octx);
    if (rv < 0) {
        qWarning("Unable to reset system");
    }
}

void HardwareInterface::startCpus()
{
    if (m_connectionStatus != Connected) {
        qWarning("Not connected to system!");
        return;
    }

    int rv = optimsoc_cpu_start(m_octx);
    if (rv < 0) {
        qWarning("Unable to start CPUs");
    }
}

void HardwareInterface::emitInstructionTraceReceived(int core_id,
                                                     uint32_t timestamp,
                                                     uint32_t pc, int count)
{
    static int cnt;
    if (cnt++ > 10000)
        return;
    emit instructionTraceReceived(core_id, timestamp, pc, count);
}


void HardwareInterface::emitSoftwareTraceReceived(uint32_t core_id,
                                                  uint32_t timestamp,
                                                  uint16_t id,
                                                  uint32_t value)

{
    emit softwareTraceReceived(core_id, timestamp, id, value);
}
