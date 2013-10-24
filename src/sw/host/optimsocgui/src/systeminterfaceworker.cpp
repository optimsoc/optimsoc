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
 *   Philipp Wagner <philipp.wagner@tum.de>
 */

#include "systeminterfaceworker.h"

/**
 * Create a new SystemInterfaceWorker object
 *
 * Note that QObject's |parent| member is always 0, because otherwise
 * this class cannot be moved into another thread (which is the main reason
 * for this class). See the documentation for QObject::moveToThread() for
 * details.
 *
 * @param octx        liboptimsochost library context
 * @param octx_mutex  mutex protecting octx
 */
SystemInterfaceWorker::SystemInterfaceWorker(struct optimsoc_ctx** octx,
                                             QMutex& octx_mutex)
    : m_octx(octx), m_octx_mutex(octx_mutex), QObject(0)
{
}

void SystemInterfaceWorker::connectToSystem()
{
    QMutexLocker octx_mutex_locker(&m_octx_mutex);

    emit connectionStatusChanged(SystemInterface::Connecting);

    int rv = optimsoc_connect(*m_octx);
    if (rv != 0) {
        emit connectionStatusChanged(SystemInterface::Disconnected);
        return;
    }

    rv = optimsoc_discover_system(*m_octx);
    if (rv == 0) {
        emit systemStatusChanged(SystemInterface::Ready);
        emit connectionStatusChanged(SystemInterface::Connected);
    } else {
        emit connectionStatusChanged(SystemInterface::Disconnected);
    }

    // register callback function for instruction traces
    // XXX: This is currently disabled since it generates too much overhead.
    //      Find solution with better performance.
    /*optimsoc_itm_register_callback(m_octx,
                                   &SystemInterface::instrTraceCallback);*/

    // register callback function for software traces
    optimsoc_stm_register_callback(*m_octx,
                                   &SystemInterface::softwareTraceCallback);
}

void SystemInterfaceWorker::disconnectFromSystem()
{
    QMutexLocker octx_mutex_locker(&m_octx_mutex);
    emit connectionStatusChanged(SystemInterface::Disconnecting);

    int rv = optimsoc_disconnect(*m_octx);
    if (rv == 0) {
        emit systemStatusChanged(SystemInterface::Unknown);
        emit connectionStatusChanged(SystemInterface::Disconnected);
    } else {
        emit connectionStatusChanged(SystemInterface::Connected);
    }
}

void SystemInterfaceWorker::resetSystem()
{
    QMutexLocker octx_mutex_locker(&m_octx_mutex);

    int rv = optimsoc_reset(*m_octx);
    if (rv < 0) {
        qWarning("Unable to reset system");
        return;
    }

    emit systemStatusChanged(SystemInterface::Ready);
}


/**
 * Start all CPUs in the connected system
 */
void SystemInterfaceWorker::startCpus()
{
    QMutexLocker octx_mutex_locker(&m_octx_mutex);

    int rv = optimsoc_cpu_start(*m_octx);
    if (rv < 0) {
        qWarning("Unable to start CPUs");
        return;
    }

    emit systemStatusChanged(SystemInterface::Running);
}

/**
 * Stall (halt, pause) all CPUs in the connected system
 *
 * @param doStall stall the CPUs (true) or unstall them (false)
 */
void SystemInterfaceWorker::stallCpus(bool doStall)
{
    QMutexLocker octx_mutex_locker(&m_octx_mutex);

    int rv = optimsoc_cpu_stall(*m_octx, doStall);
    if (rv < 0) {
        qWarning("Unable to stall CPUs");
        return;
    }

    if (doStall) {
        emit systemStatusChanged(SystemInterface::Stalled);
    } else {
        emit systemStatusChanged(SystemInterface::Running);
    }
}

void SystemInterfaceWorker::writeToMemory(unsigned int memoryId,
                                          QByteArray data,
                                          unsigned int baseAddress)
{
    QMutexLocker octx_mutex_locker(&m_octx_mutex);

    int rv = optimsoc_mem_write(*m_octx, memoryId, baseAddress,
                                reinterpret_cast<const uint8_t*>(data.constData()),
                                data.length());
    emit SystemInterface::instance()->memoryWriteFinished(memoryId, rv == 0);
}
