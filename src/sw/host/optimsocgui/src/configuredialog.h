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

#ifndef CONFIGUREDIALOG_H
#define CONFIGUREDIALOG_H

#include <QDialog>
#include <QFormLayout>
#include <QMap>

#include "hardwareinterface.h" // For backend type

namespace Ui {
class ConfigureDialog;
}

class ConfigureDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit ConfigureDialog(QWidget *parent = 0);
    ~ConfigureDialog();

    optimsoc_backend_id getBackend();
    QMap<QString,QString> getOptions();

private:
    Ui::ConfigureDialog *m_ui;
    enum DbgNocConnection { DBGNOC_USB=0, DBGNOC_TCP=1 };

};

#endif // CONFIGUREDIALOG_H
