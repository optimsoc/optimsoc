/* Copyright (c) 2012-2013 by the author(s)
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
 *   Philipp Wagner <philipp.wagner@tum.de>
 */

#include "systeminterface.h"

#include "systeminterfaceworker.h"

#include <QDebug>
#include <QMetaType>
#include <stdint.h>
#include <cstring>
#include <stdarg.h>

SystemInterface* SystemInterface::s_instance = 0;

/**
 * Constructor: setup object
 *
 * @param parent
 */
SystemInterface::SystemInterface(QObject *parent)
    : QObject(parent), m_octx(NULL), m_connectionStatus(Disconnected),
      m_systemStatus(Unknown)
{
    qRegisterMetaType<ConnectionStatus>("SystemInterface::ConnectionStatus");
    qRegisterMetaType<SystemStatus>("SystemInterface::SystemStatus");
    qRegisterMetaType<SoftwareTraceEvent>("SoftwareTraceEvent");
    qRegisterMetaType<optimsoc_backend_id>("optimsoc_backend_id");

    // start worker thread
    m_worker = new SystemInterfaceWorker(&m_octx, m_octx_mutex);

    connect(m_worker, SIGNAL(connectionStatusChanged(SystemInterface::ConnectionStatus)),
            this, SLOT(workerConnectionStatusChanged(SystemInterface::ConnectionStatus)));
    connect(m_worker, SIGNAL(systemStatusChanged(SystemInterface::SystemStatus)),
            this, SLOT(workerSystemStatusChanged(SystemInterface::SystemStatus)));

    m_worker->moveToThread(&m_workerThread);
    m_workerThread.start();
}

SystemInterface::~SystemInterface()
{
    // shutdown worker thread
    m_workerThread.quit();
    m_workerThread.wait();

    // disconnect from system and free library context
    if (m_connectionStatus != Disconnected) {
        optimsoc_disconnect(m_octx);
    }
    optimsoc_free(m_octx);
}

/**
 * Get the only instance of this class
 *
 * @return
 */
SystemInterface* SystemInterface::instance()
{
    if (!SystemInterface::s_instance) {
        SystemInterface::s_instance = new SystemInterface();
    }
    return SystemInterface::s_instance;
}

void SystemInterface::instrTraceCallback(struct optimsoc_ctx *incoming_ctx,
                                         int core_id, uint32_t timestamp,
                                         uint32_t pc, int count)
{
    instance()->emitInstructionTraceReceived(core_id, timestamp, pc, count);
}

void SystemInterface::softwareTraceCallback(uint32_t core_id,
                                            uint32_t timestamp,
                                            uint16_t id,
                                            uint32_t value)
{
    SoftwareTraceEvent *event = new SoftwareTraceEvent;
    event->core_id   = core_id;
    event->timestamp = timestamp;
    event->id        = id;
    event->value     = value;

    instance()->m_softwareTraceMutex.lock();
    instance()->m_softwareTraceQueue.push_back(event);
    instance()->m_softwareTraceMutex.unlock();
}

void SystemInterface::logCallback(struct optimsoc_log_ctx *ctx,
                                  int priority, const char *file,
                                  int line, const char *fn,
                                  const char *format, va_list args)
{
    QString strFile = QString::fromUtf8(file); // XXX: is it really UTF8?
    QString strFn = QString::fromAscii(fn);

    // put message into QString
    // C string allocation code adapted from the printf(3) man page,
    // Linux Programmer's Manual version 3.46
    // The code for glibc < 2.1 is removed (glibc 2.1 was released in 1999)
    int n;
    int size = 100;
    char *cMsg, *cMsgNew;

    if ((cMsg = static_cast<char*>(malloc(size))) == NULL) {
        return;
    }

    while (1) {
        // Try to print in the allocated space.
        n = vsnprintf(cMsg, size, format, args);

        // If that worked, return the string.
        if (n > -1 && n < size) {
            break;
        }

        // Else try again with more space.
        size = n+1;

        if ((cMsgNew = static_cast<char*>(realloc(cMsg, size))) == NULL) {
            free(cMsg);
            return;
        } else {
            cMsg = cMsgNew;
        }
    }

    QString strMsg = QString::fromUtf8(cMsg);

    instance()->emitLogMsgReceived(priority, strFile, line, strFn, strMsg);
}

/**
 * The connection status changed in the worker
 *
 * @param status the new connection status
 */
void SystemInterface::workerConnectionStatusChanged(ConnectionStatus status)
{
    ConnectionStatus oldStatus = m_connectionStatus;
    if (oldStatus == status) {
        return;
    }

    emit connectionStatusChanged(oldStatus, status);
    m_connectionStatus = status;

    // discovery is run in the worker thread, we only need to get the system ID
    if (status == Connected) {
        QMutexLocker octx_mutex_locker(&m_octx_mutex);
        int systemId = optimsoc_get_sysid(m_octx);
        emit systemDiscovered(systemId);
    }
}

/**
 * The system status changed in the worker
 *
 * @param status the new system status
 */
void SystemInterface::workerSystemStatusChanged(SystemStatus status)
{
    SystemStatus oldStatus = m_systemStatus;
    if (oldStatus == status) {
        return;
    }

    emit systemStatusChanged(oldStatus, status);
    m_systemStatus = status;
}

/**
 * Configure the system interface
 *
 * @param backend
 * @param options
 */
void SystemInterface::configure(optimsoc_backend_id backend,
                                QMap<QString, QString> options)
{
    QMutexLocker octx_mutex_locker(&m_octx_mutex);

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

    for (QMap<QString,QString>::const_iterator it = options.begin();
         it != options.end(); ++it, ++optcount) {

        optionarray[optcount].name = strdup(it.key().toAscii().data());
        optionarray[optcount].value = strdup(it.value().toAscii().data());
    }

    optimsoc_new(&m_octx, (optimsoc_backend_id) backend, options.size(),
                 optionarray);
    optimsoc_set_log_fn(m_octx, &SystemInterface::logCallback);

    // Delete all strings
    for (optcount=0; optcount<options.size(); ++optcount) {
        delete(optionarray[optcount].name);
        delete(optionarray[optcount].value);
    }
    // Delete container
    delete(optionarray);
}

/**
 * Has the system interface already been configured?
 *
 * @see configure()
 *
 * @return
 */
bool SystemInterface::configured()
{
    return (m_octx != NULL);
}

/**
 * Connect to an OpTiMSoC system
 */
void SystemInterface::connectToSystem()
{
    QMetaObject::invokeMethod(m_worker, "connectToSystem");

    connect(&m_softwareTraceTimer, SIGNAL(timeout()),
            this, SLOT(softwareTraceTimer()));
    m_softwareTraceTimer.start(20);
}

/**
 * Disconnect from an OpTiMSoC system
 */
void SystemInterface::disconnectFromSystem()
{
    if (m_connectionStatus != Connected) {
        qWarning("Not connected to system!");
        return;
    }

    QMetaObject::invokeMethod(m_worker, "disconnectFromSystem");
}

/**
 * Write data to a memory
 *
 * The data is written to the memory @p memoryId, starting at address
 * @p baseAddress. After the write is completed, the signal
 * memoryWriteFinished() is emitted, containing the result status of the
 * operation.
 *
 * @param memoryId
 * @param data
 * @param baseAddress
 */
void SystemInterface::writeToMemory(unsigned int memoryId, QByteArray data,
                                    unsigned int baseAddress)
{
    if (m_connectionStatus != Connected) {
        qWarning("Not connected to system!");
        return;
    }

    QMetaObject::invokeMethod(m_worker, "writeToMemory",
                              Q_ARG(unsigned int, memoryId),
                              Q_ARG(QByteArray, data),
                              Q_ARG(unsigned int, baseAddress));
}

/**
 * Reset the connected system
 */
void SystemInterface::resetSystem()
{
    if (m_connectionStatus != Connected) {
        qWarning("Not connected to system!");
        return;
    }

    QMetaObject::invokeMethod(m_worker, "resetSystem");
}

/**
 * (Re-)start all CPUs in the connected system
 *
 * If the CPUs are not yet started the CPUs are initially started.
 * If the CPUs are stalled they are un-stalled.
 */
void SystemInterface::startCpus()
{
    if (m_connectionStatus != Connected) {
        qWarning("Not connected to system!");
        return;
    }

    if (m_systemStatus == Stalled) {
        QMetaObject::invokeMethod(m_worker, "stallCpus", Q_ARG(bool, false));
    } else {
        QMetaObject::invokeMethod(m_worker, "startCpus");
    }
}

/**
 * Stall (halt, pause) all CPUs in the connected system
 */
void SystemInterface::stallCpus()
{
    if (m_connectionStatus != Connected) {
        qWarning("Not connected to system!");
        return;
    }

    QMetaObject::invokeMethod(m_worker, "stallCpus", Q_ARG(bool, true));
}

void SystemInterface::emitInstructionTraceReceived(int core_id,
                                                   uint32_t timestamp,
                                                   uint32_t pc, int count)
{
    static int cnt;
    if (cnt++ > 10000)
        return;
    emit instructionTraceReceived(core_id, timestamp, pc, count);
}

void SystemInterface::emitSoftwareTraceReceived(uint32_t core_id,
                                                uint32_t timestamp,
                                                uint16_t id,
                                                uint32_t value)

{
    emit softwareTraceReceived(core_id, timestamp, id, value);
}

void SystemInterface::softwareTraceTimer()
{
    m_softwareTraceMutex.lock();

    m_softwareTraceDistributor.emitEvents(m_softwareTraceQueue);

    m_softwareTraceQueue.clear();
    m_softwareTraceMutex.unlock();
}

void SystemInterface::emitLogMsgReceived(int priority, QString file,
                                           int line, QString fn, QString msg)
{
    emit logMsgReceived(priority, file, line, fn, msg);
}
