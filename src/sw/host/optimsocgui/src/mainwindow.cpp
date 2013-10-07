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
 */

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QDebug>

#include "aboutdialog.h"
#include "configuredialog.h"
#include "optimsocsystem.h"
#include "optimsocsystemfactory.h"
#include "executionchart.h"
#include "writememorydialog.h"

#include "plotspectrogram.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    m_ui(new Ui::MainWindow),
    m_hardwareInterfaceThread(new QThread),
    m_statusBarConnectionStat(new QLabel)
{
    // init UI
    m_ui->setupUi(this);

    m_system = NULL;

    statusBar()->addPermanentWidget(m_statusBarConnectionStat);

    // create hardware interface thread
    m_hwif = HardwareInterface::instance();
    connect(m_hardwareInterfaceThread, SIGNAL(started()),
            m_hwif, SLOT(threadStarted()));
    connect(m_hardwareInterfaceThread, SIGNAL(finished()),
            m_hwif, SLOT(threadFinished()));
    m_hwif->moveToThread(m_hardwareInterfaceThread);
    m_hardwareInterfaceThread->start();

    connect(m_ui->actionQuit, SIGNAL(triggered()), this, SLOT(close()));
    connect(m_ui->actionAbout, SIGNAL(triggered()),
            this, SLOT(showAboutDialog()));

    connect(m_ui->actionInitialize, SIGNAL(triggered()), this, SLOT(initMemories()));

    connect(m_ui->actionConfigure, SIGNAL(triggered()), this, SLOT(configure()));
    connect(m_ui->actionConnect, SIGNAL(triggered()),
            m_hwif, SLOT(connect()));
    connect(m_ui->actionDisconnect, SIGNAL(triggered()),
            m_hwif, SLOT(disconnect()));
    connect(m_hwif, SIGNAL(connectionStatusChanged(HardwareInterface::ConnectionStatus, HardwareInterface::ConnectionStatus)),
            this, SLOT(updateConnectionStatus(HardwareInterface::ConnectionStatus, HardwareInterface::ConnectionStatus)));
    connect(m_hwif, SIGNAL(systemDiscovered(int)),
            this, SLOT(systemDiscovered(int)));
    connect(m_ui->actionStartCpus, SIGNAL(triggered()),
            m_hwif, SLOT(startCpus()));
    connect(m_ui->actionReset, SIGNAL(triggered()),
            m_hwif, SLOT(reset()));

    // initialize connection status widget
    updateConnectionStatus(HardwareInterface::Disconnected, m_hwif->connectionStatus());

    // instruction trace display
    m_traceModel = new QStandardItemModel(0, 4, this);

    m_traceModel->setHeaderData(0, Qt::Horizontal, QObject::tr("Core ID"));
    m_traceModel->setHeaderData(1, Qt::Horizontal, QObject::tr("Timestamp"));
    m_traceModel->setHeaderData(2, Qt::Horizontal, QObject::tr("$PC"));
    m_traceModel->setHeaderData(3, Qt::Horizontal, QObject::tr("#"));

    m_ui->instructionTraceTableView->setModel(m_traceModel);
    connect(m_hwif, SIGNAL(instructionTraceReceived(int, unsigned int, unsigned int, int)),
            this, SLOT(addTraceToModel(int, unsigned int, unsigned int, int)));

    // XXX: Disable instruction trace dock until it contains data
    m_ui->instructionTraceDockWidget->hide();

    connect(m_hwif->softwareTraceEventDistributor(), SIGNAL(softwareTraceEvent(struct SoftwareTraceEvent)),
            this, SLOT(addSoftwareTraceToStdout(struct SoftwareTraceEvent)));

    // software trace display
    m_swTraceModel = new QStandardItemModel(0, 4, this);

    m_swTraceModel->setHeaderData(0, Qt::Horizontal, QObject::tr("Core ID"));
    m_swTraceModel->setHeaderData(1, Qt::Horizontal, QObject::tr("Timestamp"));
    m_swTraceModel->setHeaderData(2, Qt::Horizontal, QObject::tr("Message"));
    m_swTraceModel->setHeaderData(3, Qt::Horizontal, QObject::tr("Value"));

    m_ui->softwareTraceTableView->setModel(m_swTraceModel);
    connect(m_hwif->softwareTraceEventDistributor(), SIGNAL(softwareTraceEvent(SoftwareTraceEvent)),
            this, SLOT(addSoftwareTraceToModel(SoftwareTraceEvent)));

    // Connect to Execution Trace
    connect(m_hwif->softwareTraceEventDistributor(), SIGNAL(softwareTraceEvent(SoftwareTraceEvent)),
            m_ui->widgetExecutionTrace, SLOT(addTraceEvent(SoftwareTraceEvent)));

    connect(m_hwif, SIGNAL(systemDiscovered(int)),
            m_ui->widgetExecutionTrace, SLOT(systemDiscovered(int)));

    PlotSpectrogram *heatmap = new PlotSpectrogram(this);
    heatmap->hide();
    connect(m_hwif->softwareTraceEventDistributor(), SIGNAL(softwareTraceEvent(SoftwareTraceEvent)),
            heatmap, SLOT(softwareTraceEvent(SoftwareTraceEvent)));

    configure();
}

MainWindow::~MainWindow()
{
    m_hardwareInterfaceThread->quit();
    m_hardwareInterfaceThread->wait();

    delete m_hardwareInterfaceThread;
    delete m_hwif;
    delete m_ui;
}

void MainWindow::configure() {
    if (m_hwif->configured()) {
        QMessageBox ask(QMessageBox::Warning,
                        "Already configured",
                        "Already configured. Do you want to reconfigure?",
                        QMessageBox::Yes|QMessageBox::No, this);
        ask.setModal(true);
        ask.exec();
        if (ask.result() == QMessageBox::No) {
            return;
        }

        m_hwif->disconnect();
    }

    ConfigureDialog dialog(this);
    dialog.setModal(true);
    dialog.exec();

    if (dialog.result() == ConfigureDialog::Accepted) {
        m_ui->actionConfigure->setEnabled(true);
        m_ui->actionConnect->setEnabled(true);
        m_ui->actionDisconnect->setEnabled(false);
        m_ui->actionReset->setEnabled(false);
        m_ui->actionStartCpus->setEnabled(false);
        m_ui->menuMemories->setEnabled(false);

        m_hwif->configure(dialog.backend(), dialog.options());
    } else {
        m_ui->actionConfigure->setEnabled(true);
        m_ui->actionConnect->setEnabled(false);
        m_ui->actionDisconnect->setEnabled(false);
        m_ui->actionReset->setEnabled(false);
        m_ui->actionStartCpus->setEnabled(false);
        m_ui->menuMemories->setEnabled(false);
    }
}

void MainWindow::showAboutDialog()
{
    AboutDialog *aboutDialog = new AboutDialog(this);
    aboutDialog->setModal(true);
    aboutDialog->exec();
}

void MainWindow::updateConnectionStatus(HardwareInterface::ConnectionStatus oldStatus,
                                        HardwareInterface::ConnectionStatus newStatus)
{
    switch (newStatus) {
    case HardwareInterface::Connected:
        m_statusBarConnectionStat->setText(tr("Connected"));
        m_ui->actionConfigure->setEnabled(true);
        m_ui->actionConnect->setEnabled(false);
        m_ui->actionDisconnect->setEnabled(true);
        m_ui->actionReset->setEnabled(true);
        m_ui->actionStartCpus->setEnabled(true);
        m_ui->menuMemories->setEnabled(true);
        break;
    case HardwareInterface::Connecting:
        m_statusBarConnectionStat->setText(tr("Connecting ..."));
        break;
    case HardwareInterface::Disconnected:
        m_statusBarConnectionStat->setText(tr("Disconnected"));
        m_ui->actionConfigure->setEnabled(true);
        m_ui->actionConnect->setEnabled(true);
        m_ui->actionDisconnect->setEnabled(false);
        m_ui->actionReset->setEnabled(false);
        m_ui->actionStartCpus->setEnabled(false);
        m_ui->menuMemories->setEnabled(false);
        break;
    case HardwareInterface::Disconnecting:
        m_statusBarConnectionStat->setText(tr("Disconnecting ..."));
        break;
    default:
        m_statusBarConnectionStat->setText(tr("Unknown"));
    }

    if (newStatus != HardwareInterface::Connected) {
        m_ui->disconnectedUiStack->setCurrentIndex(1);
    }
}

void MainWindow::systemDiscovered(int systemId)
{
    m_system = OptimsocSystemFactory::createSystemFromId(systemId);
    if (!m_system) {
        QMetaObject::invokeMethod(m_hwif, "disconnect");
        QMessageBox::warning(this, "System not in database",
                             QString("The discovered system with ID %1 is not "
                                     "in the device database.").arg(systemId));
        return;
    }

    m_ui->systemOverviewWidget->setSystem(m_system);
    m_ui->disconnectedUiStack->setCurrentIndex(0);

}

void MainWindow::addTraceToModel(int core_id, unsigned int timestamp,
                                 unsigned int pc, int count)
{
    QList<QStandardItem*> items;

    QStandardItem* coreIdItem = new QStandardItem(QString("%1").arg(core_id));
    items.append(coreIdItem);

    QStandardItem* timestampItem = new QStandardItem(QString("%1").arg(timestamp));
    items.append(timestampItem);

    QStandardItem* pcItem = new QStandardItem(QString("0x%1").arg(pc, 0, 16));
    items.append(pcItem);

    QStandardItem* countItem = new QStandardItem(QString("%1").arg(count));
    items.append(countItem);


    m_traceModel->appendRow(items);
}

void MainWindow::addSoftwareTraceToModel(struct SoftwareTraceEvent event)
{
    return;
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

void MainWindow::addSoftwareTraceToStdout(SoftwareTraceEvent event)
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

void MainWindow::initMemories()
{
    WriteMemoryDialog* diag = new WriteMemoryDialog(m_system, this);
    diag->setModal(true);
    diag->exec();
    delete diag;
}
