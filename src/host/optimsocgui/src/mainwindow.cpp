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

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QLabel>
#include <QSettings>
#include <QStandardItemModel>

#include "aboutdialog.h"
#include "optimsocsystemfactory.h"

#include "plotspectrogram.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    m_ui(new Ui::MainWindow),
    m_statusBarConnectionStat(new QLabel),
    m_statusBarSystemStat(new QLabel),
    m_sysif(SystemInterface::instance())
{
    // init UI
    m_ui->setupUi(this);

    statusBar()->addPermanentWidget(m_statusBarConnectionStat);
    statusBar()->addPermanentWidget(m_statusBarSystemStat);

    connect(m_ui->actionQuit, SIGNAL(triggered()), this, SLOT(close()));
    connect(m_ui->actionAbout, SIGNAL(triggered()),
            this, SLOT(showAboutDialog()));

    connect(m_sysif,
            SIGNAL(connectionStatusChanged(SystemInterface::ConnectionStatus, SystemInterface::ConnectionStatus)),
            this,
            SLOT(showConnectionStatus(SystemInterface::ConnectionStatus, SystemInterface::ConnectionStatus)));

    connect(m_sysif,
            SIGNAL(systemStatusChanged(SystemInterface::SystemStatus, SystemInterface::SystemStatus)),
            this,
            SLOT(showSystemStatus(SystemInterface::SystemStatus, SystemInterface::SystemStatus)));

    connect(m_ui->actionStartCpus, SIGNAL(triggered()),
            m_sysif, SLOT(startCpus()));
    connect(m_ui->actionStallCpus, SIGNAL(triggered()),
            m_sysif, SLOT(stallCpus()));
    connect(m_ui->actionResetSystem, SIGNAL(triggered()),
            m_sysif, SLOT(resetSystem()));

    connect(m_ui->btnToggleLogViewer, SIGNAL(toggled(bool)),
            this, SLOT(toggleLogViewer(bool)));
    connect(m_ui->logViewer, SIGNAL(unseenLogMsgs(uint,uint)),
            this, SLOT(updateUnseenLogMsgsInButton(uint,uint)));

    // initialize connection status widget
    showConnectionStatus(SystemInterface::Disconnected,
                         m_sysif->connectionStatus());

    readSettings();

    // ensure initial state
    updateUnseenLogMsgsInButton(0, 0);
}

MainWindow::~MainWindow()
{
    delete m_sysif;

    delete m_statusBarConnectionStat;
    delete m_statusBarSystemStat;
    delete m_ui;

    // The system is allocated in SystemView. We delete it here to ensure it is
    // deleted when closing the application.
    OptimsocSystem *sys = OptimsocSystemFactory::currentSystem();
    if (sys) {
        delete sys;
    }
}

/**
 * The window received a close event
 *
 * This implementation writes all settings otherwise does not change the event
 * handling.
 *
 * @see QMainWindow::closeEvent()
 *
 * @param event
 */
void MainWindow::closeEvent(QCloseEvent *event)
{
    writeSettings();
    QMainWindow::closeEvent(event);
}

/**
 * Restore the UI state based on the settings
 */
void MainWindow::readSettings()
{
    QSettings settings;
    settings.beginGroup("MainWindow");

    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("windowState").toByteArray());

    m_ui->splitterBottomSection->restoreState(settings.value("splitterBottomSectionState").toByteArray());
    m_ui->logViewer->setVisible(settings.value("logViewerVisible").toBool());
    m_ui->btnToggleLogViewer->setChecked(settings.value("logViewerVisible").toBool());

    settings.endGroup();
}

/**
 * Write the UI state to the settings to restore it on the next program run
 */
void MainWindow::writeSettings()
{
    QSettings settings;
    settings.beginGroup("MainWindow");

    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());

    settings.setValue("splitterBottomSectionState",
                      m_ui->splitterBottomSection->saveState());
    settings.setValue("logViewerVisible", m_ui->logViewer->isVisible());

    settings.endGroup();
}

void MainWindow::showAboutDialog()
{
    AboutDialog *aboutDialog = new AboutDialog(this);
    aboutDialog->setModal(true);
    aboutDialog->exec();
}

/**
 * Update the UI after the connection status changed
 *
 * @param oldStatus
 * @param newStatus
 */
void MainWindow::showConnectionStatus(SystemInterface::ConnectionStatus oldStatus,
                                      SystemInterface::ConnectionStatus newStatus)
{
    Q_UNUSED(oldStatus);

    switch (newStatus) {
    case SystemInterface::Connected:
        m_statusBarConnectionStat->setText(tr("Connected"));

        m_ui->actionConfigure->setEnabled(true);
        m_ui->actionConnect->setEnabled(false);
        m_ui->actionDisconnect->setEnabled(true);

        m_ui->actionResetSystem->setEnabled(true);
        break;
    case SystemInterface::Connecting:
        m_statusBarConnectionStat->setText(tr("Connecting ..."));

        m_ui->actionConfigure->setEnabled(false);
        m_ui->actionConnect->setEnabled(false);
        m_ui->actionDisconnect->setEnabled(false);

        m_ui->actionResetSystem->setEnabled(false);
        break;
    case SystemInterface::Disconnected:
        m_statusBarConnectionStat->setText(tr("Disconnected"));

        m_ui->actionConfigure->setEnabled(true);
        m_ui->actionConnect->setEnabled(true);
        m_ui->actionDisconnect->setEnabled(false);

        m_ui->actionResetSystem->setEnabled(false);

        m_ui->actionStartCpus->setEnabled(false);
        m_ui->actionStallCpus->setEnabled(false);
        break;
    case SystemInterface::Disconnecting:
        m_statusBarConnectionStat->setText(tr("Disconnecting ..."));

        m_ui->actionConfigure->setEnabled(false);
        m_ui->actionConnect->setEnabled(false);
        m_ui->actionDisconnect->setEnabled(false);

        m_ui->actionResetSystem->setEnabled(false);

        m_ui->actionStartCpus->setEnabled(false);
        m_ui->actionStallCpus->setEnabled(false);
        break;
    default:
        m_statusBarConnectionStat->setText(tr("Unknown"));
    }

    // always show connection tab, but the others only if the we're connected
    if (newStatus != SystemInterface::Connected) {
        m_ui->taskTabWidget->setTabEnabled(1, false);
        m_ui->taskTabWidget->setTabEnabled(2, false);
    } else {
        m_ui->taskTabWidget->setTabEnabled(1, true);
        m_ui->taskTabWidget->setTabEnabled(2, true);
    }
}

/**
 * Update the UI after the system status changed
 *
 * @param oldStatus
 * @param newStatus
 */
void MainWindow::showSystemStatus(SystemInterface::SystemStatus oldStatus,
                                  SystemInterface::SystemStatus newStatus)
{
    Q_UNUSED(oldStatus);

    switch (newStatus) {
    case SystemInterface::Ready:
        m_statusBarSystemStat->setText("Ready");

        m_ui->actionStartCpus->setEnabled(true);
        m_ui->actionStallCpus->setEnabled(false);
        break;
    case SystemInterface::Running:
        m_statusBarSystemStat->setText("Running");

        m_ui->actionStartCpus->setEnabled(false);
        m_ui->actionStallCpus->setEnabled(true);
        break;
    case SystemInterface::Stalled:
        m_statusBarSystemStat->setText("Stalled");

        m_ui->actionStartCpus->setEnabled(true);
        m_ui->actionStallCpus->setEnabled(false);
        break;
    case SystemInterface::Unknown:
    default:
        m_statusBarSystemStat->setText("Unknown");

        m_ui->actionStartCpus->setEnabled(true);
        m_ui->actionStallCpus->setEnabled(false);
        break;
    }
}

/**
 * Show/hide the log viewer at the bottom
 *
 * @param showLogViewer should the log viewer be visible?
 */
void MainWindow::toggleLogViewer(bool showLogViewer)
{
    m_ui->logViewer->setVisible(showLogViewer);
}

/**
 * Update the label of the "toggle log viewer" button to display new messages
 *
 * @param m_unseenInfo
 * @param m_unseenImportant
 */
void MainWindow::updateUnseenLogMsgsInButton(unsigned int m_unseenInfo,
                                             unsigned int m_unseenImportant)
{
    if (m_unseenImportant > 0) {
        // XXX: Use system color definitions to work with different themes
        m_ui->btnToggleLogViewer->setStyleSheet("color: red");
    } else {
        m_ui->btnToggleLogViewer->setStyleSheet("");
    }

    unsigned int cnt = m_unseenImportant + m_unseenInfo;
    QString label;
    if (cnt == 0) {
        label = QString("Log Messages");
    } else {
        label = QString("Log Messages (%1 unseen)").arg(cnt);
    }
    m_ui->btnToggleLogViewer->setText(label);
}
