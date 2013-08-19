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

#ifndef WRITEMEMORYDIALOG_H
#define WRITEMEMORYDIALOG_H

#include <QDialog>

namespace Ui {
class WriteMemoryDialog;
}

class MemoryTile;

class WriteMemoryDialog : public QDialog
{
    Q_OBJECT

public:
    explicit WriteMemoryDialog(MemoryTile* memoryTile, QWidget *parent = 0);
    ~WriteMemoryDialog();

public slots:
    void showFilePicker();
    void initMemory();
    void memoryWriteFinished(bool success);

private:
    Ui::WriteMemoryDialog *m_ui;
    MemoryTile* m_memoryTile;

    void disableUiForUpload(bool disable);
};

#endif // WRITEMEMORYDIALOG_H
