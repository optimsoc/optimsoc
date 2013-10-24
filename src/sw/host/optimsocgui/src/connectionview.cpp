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

#include "connectionview.h"
#include "ui_connectionview.h"

#include <QDebug>
#include <QSettings>

ConnectionView::ConnectionView(QWidget *parent) :
    QWidget(parent),
    m_ui(new Ui::ConnectionView)
{
    m_ui->setupUi(this);

    // connection parameters UI
    connect(m_ui->rbTypeHardware, SIGNAL(toggled(bool)),
            this, SLOT(updateUiAfterConParameterChange()));
    connect(m_ui->rbTypeSimulation, SIGNAL(toggled(bool)),
            this, SLOT(updateUiAfterConParameterChange()));
    connect(m_ui->rbMethodHwUsb, SIGNAL(toggled(bool)),
            this, SLOT(updateUiAfterConParameterChange()));
    connect(m_ui->rbMethodSimDbgnocTcp, SIGNAL(toggled(bool)),
            this, SLOT(updateUiAfterConParameterChange()));
    connect(m_ui->rbMethodSimSimtcp, SIGNAL(toggled(bool)),
            this, SLOT(updateUiAfterConParameterChange()));

    m_sysif = SystemInterface::instance();
    connect(m_sysif, SIGNAL(connectionStatusChanged(SystemInterface::ConnectionStatus, SystemInterface::ConnectionStatus)),
            this, SLOT(connectionStatusChanged(SystemInterface::ConnectionStatus,SystemInterface::ConnectionStatus)));

    // actions
    connect(m_ui->btnConnect, SIGNAL(pressed()), this, SLOT(doConnect()));
    connect(m_ui->btnDisconnect, SIGNAL(pressed()), this, SLOT(doDisconnect()));

    // read all settings to restore the widget states from the last run
    readSettings();

    // ensure correct initial UI state
    updateUiAfterConParameterChange();
    connectionStatusChanged(SystemInterface::Disconnected,
                            m_sysif->connectionStatus());
}

ConnectionView::~ConnectionView()
{
    writeSettings();

    delete m_ui;
}

/**
 * Read all settings and restore the widget state with it
 */
void ConnectionView::readSettings()
{
    QSettings settings;
    settings.beginGroup("ConnectionView");

    m_ui->rbTypeHardware->setChecked(settings.value("rbTypeHardwareChecked", true).toBool());
    m_ui->rbTypeSimulation->setChecked(settings.value("rbTypeSimulationChecked", false).toBool());
    m_ui->rbMethodSimDbgnocTcp->setChecked(settings.value("rbMethodSimDbgnocTcpChecked", true).toBool());
    m_ui->rbMethodSimSimtcp->setChecked(settings.value("rbMethodSimSimtcpChecked", false).toBool());
    m_ui->rbMethodHwUsb->setChecked(settings.value("rbMethodHwUsbChecked", true).toBool());
    m_ui->editSettingsDbgnocTcpHost->setText(settings.value("editSettingsDbgnocTcpHostText", "localhost").toString());
    m_ui->editSettingsDbgnocTcpPort->setText(settings.value("editSettingsDbgnocTcpPortText", "23000").toString());
    m_ui->editSettingsSimtcpHost->setText(settings.value("editSettingsSimtcpHostText", "localhost").toString());
    m_ui->editSettingsSimtcpPort->setText(settings.value("editSettingsSimtcpPortText", "22000").toString());

    settings.endGroup();
}

/**
 * Write all relevant widget state to the settings to be able to restore it
 */
void ConnectionView::writeSettings()
{
    QSettings settings;
    settings.beginGroup("ConnectionView");

    settings.setValue("rbTypeHardwareChecked",
                      m_ui->rbTypeHardware->isChecked());
    settings.setValue("rbTypeSimulationChecked",
                      m_ui->rbTypeSimulation->isChecked());
    settings.setValue("rbMethodSimDbgnocTcpChecked",
                      m_ui->rbMethodSimDbgnocTcp->isChecked());
    settings.setValue("rbMethodSimSimtcpChecked",
                      m_ui->rbMethodSimSimtcp->isChecked());
    settings.setValue("rbMethodHwUsbChecked",
                      m_ui->rbMethodHwUsb->isChecked());
    settings.setValue("editSettingsDbgnocTcpHostText",
                      m_ui->editSettingsDbgnocTcpHost->text());
    settings.setValue("editSettingsDbgnocTcpPortText",
                      m_ui->editSettingsDbgnocTcpPort->text());
    settings.setValue("editSettingsSimtcpHostText",
                      m_ui->editSettingsSimtcpHost->text());
    settings.setValue("editSettingsSimtcpPortText",
                      m_ui->editSettingsSimtcpPort->text());

    settings.endGroup();
}

/**
 * Update the UI to reflect changes made to the connection parameters.
 */
void ConnectionView::updateUiAfterConParameterChange()
{
    ConnectionType conType;
    ConnectionMethod conMethod;

    if (m_ui->rbTypeHardware->isChecked()) {
        conType = Hardware;
    } else if (m_ui->rbTypeSimulation) {
        conType = Simulation;
    }

    if (conType == Hardware) {
        conMethod = DbgnocUsb;
    } else if (conType == Simulation) {
        if (m_ui->rbMethodSimDbgnocTcp->isChecked()) {
            conMethod = DbgnocTcp;
        } else if (m_ui->rbMethodSimSimtcp->isChecked()) {
            conMethod = Simtcp;
        }
    }


    switch (conType) {
    case Hardware:
        m_ui->swConnectionMethod->setCurrentWidget(m_ui->pgMethodHardware);
        break;
    case Simulation:
        m_ui->swConnectionMethod->setCurrentWidget(m_ui->pgMethodSimulation);
        break;
    }

    switch (conMethod) {
    case DbgnocUsb:
        m_ui->swConnectionSettings->setCurrentWidget(m_ui->pgSettingsDbgnocUsb);
        break;
    case DbgnocTcp:
        m_ui->swConnectionSettings->setCurrentWidget(m_ui->pgSettingsDbgnocTcp);
        break;
    case Simtcp:
        m_ui->swConnectionSettings->setCurrentWidget(m_ui->pgSettingsSimtcp);
        break;
    }
}


/**
 * Update UI after the connection parameter changed
 *
 * @param oldStatus
 * @param newStatus
 */
void ConnectionView::connectionStatusChanged(SystemInterface::ConnectionStatus oldStatus,
                                             SystemInterface::ConnectionStatus newStatus)
{
    Q_UNUSED(oldStatus);

    switch (newStatus) {
    case SystemInterface::Connected:
        m_ui->lblConnectionStatus->setText(tr("Connected"));
        break;
    case SystemInterface::Connecting:
        m_ui->lblConnectionStatus->setText(tr("Connecting ..."));
        break;
    case SystemInterface::Disconnected:
        m_ui->lblConnectionStatus->setText(tr("Disconnected"));
        break;
    case SystemInterface::Disconnecting:
        m_ui->lblConnectionStatus->setText(tr("Disconnecting ..."));
        break;
    default:
        m_ui->lblConnectionStatus->setText(tr("Unknown"));
    }

    m_ui->gbParameters->setEnabled(newStatus == SystemInterface::Disconnected);
    m_ui->btnConnect->setEnabled(newStatus == SystemInterface::Disconnected);
    m_ui->btnDisconnect->setEnabled(newStatus == SystemInterface::Connected);
}

void ConnectionView::doConnect()
{
    // extract settings for liboptimsochost from GUI
    optimsoc_backend_id backend;
    QMap<QString, QString> options;
    if (m_ui->rbTypeHardware->isChecked() &&
        m_ui->rbMethodHwUsb->isChecked()) {
        backend = OPTIMSOC_BACKEND_DBGNOC;
        options["conn"] = "usb";
    } else if (m_ui->rbTypeSimulation->isChecked() &&
               m_ui->rbMethodSimDbgnocTcp->isChecked()) {
        backend = OPTIMSOC_BACKEND_DBGNOC;
        options["conn"] = "tcp";
        options["host"] = m_ui->editSettingsDbgnocTcpHost->text();
        options["port"] = m_ui->editSettingsDbgnocTcpPort->text();
    } else if (m_ui->rbTypeSimulation->isChecked() &&
               m_ui->rbMethodSimSimtcp->isChecked()) {
        backend = OPTIMSOC_BACKEND_SIMTCP;
        options["host"] = m_ui->editSettingsSimtcpHost->text();
        options["port"] = m_ui->editSettingsSimtcpPort->text();
    }

    // configure liboptimsochost and connect to system
    m_sysif->configure(backend, options);
    m_sysif->connectToSystem();
}

void ConnectionView::doDisconnect()
{
    m_sysif->disconnectFromSystem();
}
