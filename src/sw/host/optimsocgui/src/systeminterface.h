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

#ifndef SYSTEMINTERFACE_H
#define SYSTEMINTERFACE_H

#include <QThread>
#include <QMap>
#include <QQueue>
#include <QTimer>
#include <QMutex>

#include "traceevents.h"

#include "liboptimsochost.h"

class SystemInterfaceWorker;

/**
 * System Interface: interact with the the OpTiMSoC system
 *
 * This class is implemented as singleton, use instance() to get an
 * instance of it.
 */
class SystemInterface : public QObject
{
    Q_OBJECT

    friend class SystemInterfaceWorker;

public:
    enum ConnectionStatus {
        Connected,
        Connecting,
        Disconnected,
        Disconnecting
    };

    enum SystemStatus {
        Unknown,
        Ready,
        Running,
        Stalled
    };

    virtual ~SystemInterface();

    static SystemInterface* instance();

    // callbacks from liboptimsochost
    static void instrTraceCallback(struct optimsoc_ctx *incoming_ctx,
                                   int core_id, uint32_t timestamp,
                                   uint32_t pc, int count);
    static void softwareTraceCallback(uint32_t core_id, uint32_t timestamp,
                                      uint16_t id, uint32_t value);
    static void logCallback(struct optimsoc_log_ctx *ctx,
                            int priority, const char *file,
                            int line, const char *fn,
                            const char *format, va_list args);

    bool configured();
    SystemInterface::ConnectionStatus connectionStatus() { return m_connectionStatus; }

    SoftwareTraceEventDistributor *softwareTraceEventDistributor() { return &m_softwareTraceDistributor; }

public slots:
    void configure(optimsoc_backend_id type, QMap<QString, QString> options);
    void connectToSystem();
    void disconnectFromSystem();
    void writeToMemory(unsigned int memoryId, QByteArray data,
                       unsigned int baseAddress = 0);
    void resetSystem();
    void startCpus();
    void stallCpus();

    void softwareTraceTimer();

signals:
    void connectionStatusChanged(SystemInterface::ConnectionStatus oldStatus,
                                 SystemInterface::ConnectionStatus newStatus);
    void systemStatusChanged(SystemInterface::SystemStatus oldStatus,
                             SystemInterface:: SystemStatus newStatus);
    void systemDiscovered(int systemId);
    void memoryWriteFinished(unsigned int memoryId, bool success);
    void instructionTraceReceived(int core_id, unsigned int timestamp,
                                  unsigned int pc, int count);
    void softwareTraceReceived(unsigned int core_id, unsigned int timestamp,
                               unsigned int id, unsigned int value);
    void logMsgReceived(int priority, QString file, int line, QString fn,
                        QString msg);

private:
    Q_DISABLE_COPY(SystemInterface)

    static SystemInterface *s_instance;
    struct optimsoc_ctx *m_octx;
    QMutex m_octx_mutex;
    ConnectionStatus m_connectionStatus;
    SystemStatus m_systemStatus;
    QQueue<SoftwareTraceEvent*> m_softwareTraceQueue;
    QTimer m_softwareTraceTimer;
    QMutex m_softwareTraceMutex;
    SoftwareTraceEventDistributor m_softwareTraceDistributor;
    QThread m_workerThread;
    SystemInterfaceWorker *m_worker;

    SystemInterface(QObject *parent = 0);

    void emitInstructionTraceReceived(int core_id, uint32_t timestamp,
                                      uint32_t pc, int count);
    void emitSoftwareTraceReceived(uint32_t core_id, uint32_t timestamp,
                                   uint16_t id, uint32_t value);
    void emitLogMsgReceived(int priority, QString file, int line, QString fn,
                            QString msg);

private slots:
    void workerConnectionStatusChanged(SystemInterface::ConnectionStatus status);
    void workerSystemStatusChanged(SystemInterface::SystemStatus status);
};

#endif // SYSTEMINTERFACE_H
