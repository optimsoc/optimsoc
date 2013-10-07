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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include <QLabel>
#include <QStandardItemModel>

#include "hardwareinterface.h"

class OptimsocSystem;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *m_ui;

    HardwareInterface *m_hwif;
    QThread *m_hardwareInterfaceThread;
    QLabel *m_statusBarConnectionStat;
    OptimsocSystem *m_system;
    QStandardItemModel *m_traceModel;
    QStandardItemModel *m_swTraceModel;

    QVector<QString> m_stdoutcollector;

public slots:
    void showAboutDialog();
    void updateConnectionStatus(HardwareInterface::ConnectionStatus oldStatus,
                                HardwareInterface::ConnectionStatus newStatus);
    void systemDiscovered(int systemId);
    void addTraceToModel(int core_id, unsigned int timestamp,
                         unsigned int pc, int count);
    void addSoftwareTraceToModel(struct SoftwareTraceEvent);
    void addSoftwareTraceToStdout(struct SoftwareTraceEvent event);
    void configure();

    void initMemories();

};

#endif // MAINWINDOW_H
