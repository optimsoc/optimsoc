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

#include "memory.h"

#include "systeminterface.h"
#include "util.h"
//#include "writememorydialog.h"

#include <QDomNode>
#include <QMenu>
#include <QDebug>
#include <QWidget>
#include <QLabel>
#include <QGridLayout>

Memory::Memory(const QDomNode &memoryNode, OptimsocSystem* system,
               OptimsocSystemElement *parent) :
    OptimsocSystemElement(system, parent), m_contextMenu(NULL),
    m_propertiesWidget(NULL)
{
    QDomNamedNodeMap attributes = memoryNode.attributes();
    setId(attributes.namedItem("id").nodeValue());
    m_memoryId = attributes.namedItem("memoryid").nodeValue().toInt();

    unsigned int size = attributes.namedItem("size").nodeValue().toUInt();
    QString sizeunit = attributes.namedItem("sizeunit").nodeValue();
    if (sizeunit == "G" || sizeunit == "g") {
        m_byteSize = size * 1024 * 1024 * 1024;
    } else if (sizeunit == "M" || sizeunit == "m") {
        m_byteSize = size * 1024 * 1024;
    } else if (sizeunit == "K" || sizeunit == "k") {
        m_byteSize = size * 1024;
    } else {
        m_byteSize = size;
    }
}

Memory::~Memory()
{
    delete m_contextMenu;
}

/**
 * Get the size of the memory in bytes
 *
 * @return memory size [bytes]
 */
unsigned int Memory::size()
{
    return m_byteSize;
}

/**
 * Initialize this memory
 *
 * This method is asynchronous. The return code is delivered through the
 * memoryWriteFinished() signal.
 */
void Memory::write(QByteArray data, unsigned int baseAddress)
{
    SystemInterface* sysif = SystemInterface::instance();
    connect(sysif, SIGNAL(memoryWriteFinished(unsigned int, bool)),
            this, SLOT(emitMemoryWriteFinished(unsigned int, bool)));
    sysif->writeToMemory(m_memoryId, data, baseAddress);
}

void Memory::emitMemoryWriteFinished(unsigned int memoryId, bool success)
{
    if (memoryId != m_memoryId) {
        return;
    }

    SystemInterface* sysif = SystemInterface::instance();
    disconnect(sysif, SIGNAL(memoryWriteFinished(unsigned int, bool)),
               this, SLOT(emitMemoryWriteFinished(unsigned int, bool)));

    emit writeFinished(success);
}

QMenu* Memory::contextMenu()
{
    /*if (!m_contextMenu) {
        m_contextMenu = new QMenu();
        QAction* writeMemoryAction = new QAction("Write memory", m_contextMenu);
        connect(writeMemoryAction, SIGNAL(triggered()),
                this, SLOT(showWriteMemoryDialog()));
        m_contextMenu->addAction(writeMemoryAction);
    }*/

    return m_contextMenu;
}

void Memory::showWriteMemoryDialog()
{
    /*WriteMemoryDialog* diag = new WriteMemoryDialog(this);
    diag->setModal(true);
    diag->exec();
    delete diag;*/
}

QString Memory::name()
{
    return QString("memory %1").arg(m_memoryId);
}

QWidget* Memory::propertiesWidget()
{
    if (m_propertiesWidget) {
        return m_propertiesWidget;
    }

    m_propertiesWidget = new QWidget();
    QGridLayout *layout = new QGridLayout(m_propertiesWidget);
    m_propertiesWidget->setLayout(layout);
    QLabel *lblComputetileidDesc = new QLabel("Memory identifier", m_propertiesWidget);
    QLabel *lblComputetileidContent = new QLabel(QString::number(m_memoryId), m_propertiesWidget);
    layout->addWidget(lblComputetileidDesc, 0, 0);
    layout->addWidget(lblComputetileidContent, 0, 1);
    QLabel *lblSizeDesc = new QLabel("Memory size", m_propertiesWidget);
    QLabel *lblSizeContent = new QLabel(Util::formatBytesHumanReadable(m_byteSize));
    layout->addWidget(lblSizeDesc, 1, 0);
    layout->addWidget(lblSizeContent, 1, 1);
    return m_propertiesWidget;
}
