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

#include "writememorydialog.h"
#include "ui_writememorydialog.h"

#include <QFileDialog>

#include "hardwareinterface.h"
#include "tile.h"

WriteMemoryDialog::WriteMemoryDialog(OptimsocSystem* system, QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::WriteMemoryDialog),
    m_system(system)
{
    m_ui->setupUi(this);
    disableUiForUpload(false);

    connect(m_ui->selectFileButton, SIGNAL(clicked()),
            this, SLOT(showFilePicker()));
    connect(m_ui->initMemoryButton, SIGNAL(clicked()),
            this, SLOT(initMemory()));

    m_ui->listWidget->clear();

    if (!system) {
        QListWidgetItem *item = new QListWidgetItem(QString("No system connected"));
        m_ui->listWidget->addItem(item);
        m_ui->initMemoryButton->setEnabled(false);
    } else {
        QList<Tile*> tiles = system->tiles();
        for (QList<Tile*>::iterator it = tiles.begin(); it != tiles.end(); ++it) {
            Tile *tile = (*it);
            if (tile->hasMemory()) {
                QListWidgetItem *listitem = new QListWidgetItem(QString("Tile %1").arg(tile->tileId()));
                m_ui->listWidget->addItem(listitem);
                m_listitems[tile] = listitem;
               listitem->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
               listitem->setCheckState(Qt::Checked);
            }
        }
    }
}

WriteMemoryDialog::~WriteMemoryDialog()
{
    delete m_ui;
}

void WriteMemoryDialog::showFilePicker()
{
    QString fileName;

    QFileDialog fileDialog(this);
    fileDialog.setFileMode(QFileDialog::ExistingFile);
    fileDialog.setWindowTitle(tr("Choose a memory dump file"));
    fileDialog.setNameFilter(tr("Memory Dumps (*.bin);;All Files (*.*)"));
    if (!fileDialog.exec()) {
        return;
    }

    fileName = fileDialog.selectedFiles().at(0);
    m_ui->fileNameEdit->setText(fileName);
}

void WriteMemoryDialog::initMemory()
{
    //disableUiForUpload(true);

    QFile file(m_ui->fileNameEdit->text());
    if (!file.open(QIODevice::ReadOnly)) {
        m_ui->msgLabel->setText(tr("<font color=red>Unable to open data file!"
                                   "</font>"));
        disableUiForUpload(false);
        return;
    }

    QByteArray data = file.readAll();
    if (data.isEmpty()) {
        m_ui->msgLabel->setText(tr("<font color=red>Unable to read data from "
                                   "file!</font>"));
        disableUiForUpload(false);
        return;
    }

    /*    connect(m_memoryTile, SIGNAL(memoryWriteFinished(bool)),
                this, SLOT(memoryWriteFinished(bool)));
          m_memoryTile->initMemory(data);*/
    for (QMap<Tile*, QListWidgetItem*>::iterator it = m_listitems.begin();
         it != m_listitems.end(); ++it) {
        Tile *tile = it.key();
        QListWidgetItem *item = it.value();
        if (item->checkState() == Qt::Checked) {
            tile->initMemory(data);
        }
    }
}

void WriteMemoryDialog::disableUiForUpload(bool disable)
{
    m_ui->fileNameEdit->setEnabled(!disable);
    m_ui->initMemoryButton->setEnabled(!disable);
    if (disable) {
        m_ui->progressBar->show();
        m_ui->msgLabel->hide();
    } else {
        m_ui->progressBar->hide();
        m_ui->msgLabel->show();
    }
}

void WriteMemoryDialog::memoryWriteFinished(bool success)
{
    disableUiForUpload(false);
    if (success) {
        m_ui->msgLabel->setText(tr("<font color=green>Memory succssfully "
                                   "initialized</font>"));
    } else {
        m_ui->msgLabel->setText(tr("<font color=red>Unable to initialize "
                                   "memory</font>"));
    }
}
