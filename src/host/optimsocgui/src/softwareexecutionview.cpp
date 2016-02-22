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

#include "softwareexecutionview.h"
#include "ui_softwareexecutionview.h"

#include <QStandardItemModel>

#include "plotspectrogram.h"
#include "systeminterface.h"

SoftwareExecutionView::SoftwareExecutionView(QWidget *parent) :
    QWidget(parent),
    m_ui(new Ui::SoftwareExecutionView),
    m_sysif(SystemInterface::instance())
{
    m_ui->setupUi(this);

    // system console: STM printf's
    connect(m_sysif->softwareTraceEventDistributor(),
            SIGNAL(softwareTraceEvent(struct SoftwareTraceEvent)),
            this,
            SLOT(addSoftwareTraceToStdout(struct SoftwareTraceEvent)));

    // software trace (STM) table
    m_swTraceModel = new QStandardItemModel(0, 4, this);
    m_swTraceModel->setHeaderData(0, Qt::Horizontal, "Core ID");
    m_swTraceModel->setHeaderData(1, Qt::Horizontal, "Timestamp");
    m_swTraceModel->setHeaderData(2, Qt::Horizontal, "Message");
    m_swTraceModel->setHeaderData(3, Qt::Horizontal, "Value");
    m_ui->softwareTraceTableView->setModel(m_swTraceModel);

    connect(m_sysif->softwareTraceEventDistributor(),
            SIGNAL(softwareTraceEvent(struct SoftwareTraceEvent)),
            this,
            SLOT(addSoftwareTraceToModel(struct SoftwareTraceEvent)));

    PlotSpectrogram *heatmap = new PlotSpectrogram(this);
    heatmap->hide();
}

SoftwareExecutionView::~SoftwareExecutionView()
{
    delete m_ui;
}

/**
 * Insert a STM event into the model associated with the table view
 *
 * @param event
 */
void SoftwareExecutionView::addSoftwareTraceToModel(struct SoftwareTraceEvent event)
{
    QList<QStandardItem*> items;

    QStandardItem* coreIdItem = new QStandardItem(QString("%1").arg(event.core_id));
    items.append(coreIdItem);

    QStandardItem* timestampItem = new QStandardItem(QString("%1").arg(event.timestamp));
    items.append(timestampItem);

    QStandardItem* idItem = new QStandardItem(QString("0x%1").arg(event.id, 0, 16));
    items.append(idItem);

    QStandardItem* valueItem = new QStandardItem(QString("0x%1").arg(event.value, 0, 16));
    items.append(valueItem);

    m_swTraceModel->appendRow(items);
}

/**
 * Print all STM printf() events to the system console view
 *
 * @param event
 */
void SoftwareExecutionView::addSoftwareTraceToStdout(struct SoftwareTraceEvent event)
{
    if (event.id == 0x4) {
        QChar character(event.value);

        if (m_stdoutcollector.size() <= event.core_id) {
            m_stdoutcollector.resize(event.core_id+1);
        }

        if (m_stdoutcollector[event.core_id].length() == 0) {
            m_stdoutcollector[event.core_id] = QString("[%1, %2 ns] %3").arg(event.core_id).arg(event.timestamp).arg(character);
        } else if (character != '\n') {
            m_stdoutcollector[event.core_id] += character;
        }

        if (character == '\n') {
            m_ui->stdoutTextEdit->appendPlainText(m_stdoutcollector[event.core_id]);
            m_stdoutcollector[event.core_id] = "";
        }
    }
}
