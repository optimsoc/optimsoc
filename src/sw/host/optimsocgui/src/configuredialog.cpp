/**
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
 * Driver for the simple message passing hardware.
 *
 * (c) 2013 by the author(s)
 *
 * Author(s):
 *    Stefan Wallentowitz, stefan.wallentowitz@tum.de
 */

#include "configuredialog.h"
#include "ui_configuredialog.h"

#include <QLabel>

ConfigureDialog::ConfigureDialog(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::ConfigureDialog)
{
    m_ui->setupUi(this);

    connect(m_ui->backendComboBox,SIGNAL(currentIndexChanged(int)),m_ui->optionsStack,SLOT(setCurrentIndex(int)));
    connect(m_ui->dbgnocConnectionComboBox,SIGNAL(currentIndexChanged(int)),m_ui->dbgnocConnectionsStack,SLOT(setCurrentIndex(int)));
}

ConfigureDialog::~ConfigureDialog()
{
    delete m_ui;
}

optimsoc_backend_id ConfigureDialog::getBackend() {
    return (optimsoc_backend_id) m_ui->backendComboBox->currentIndex();
}

QMap<QString,QString> ConfigureDialog::getOptions() {
    QMap<QString,QString> Options;
    switch(getBackend()) {
    case OPTIMSOC_BACKEND_DBGNOC:
        switch(m_ui->dbgnocConnectionComboBox->currentIndex()) {
        case DBGNOC_USB:
            Options["conn"] = "usb";
            break;
        case DBGNOC_TCP:
            Options["conn"] = "tcp";
            Options["host"] = m_ui->dbgnocTCPHostnameLineEdit->text();
            Options["port"] = m_ui->dbgnocTCPPortSpinBox->text();
            break;
        }
        break;
    case OPTIMSOC_BACKEND_SIMTCP:
        Options["host"] = m_ui->simtcpHostnameLineEdit->text();
        Options["port"] = m_ui->simtcpPortLineEdit->text();
        break;
    }
    return Options;
}
