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

#ifndef SYSTEMINTERFACEWORKER_H
#define SYSTEMINTERFACEWORKER_H

#include <QMutex>
#include <QObject>

#include "liboptimsochost.h"
#include "systeminterface.h"

class SystemInterfaceWorker : public QObject
{
    Q_OBJECT

    friend class SystemInterface;

public:
    explicit SystemInterfaceWorker(struct optimsoc_ctx** ctx, QMutex& ctx_mutex);

signals:
    void connectionStatusChanged(SystemInterface::ConnectionStatus newStatus);
    void systemStatusChanged(SystemInterface::SystemStatus newStatus);

private:
    QMutex& m_octx_mutex;
    struct optimsoc_ctx** m_octx;

private slots:
    void connectToSystem();
    void disconnectFromSystem();
    void resetSystem();
    void startCpus();
    void stallCpus(bool doStall = true);
    void writeToMemory(unsigned int memoryId, QByteArray data,
                       unsigned int baseAddress = 0);
};

#endif // SYSTEMINTERFACEWORKER_H
