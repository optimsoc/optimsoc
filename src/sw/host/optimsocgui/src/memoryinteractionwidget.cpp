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

#include "memoryinteractionwidget.h"
#include "ui_memoryinteractionwidget.h"

#include <QByteArray>
#include <QDebug>
#include <QFileDialog>
#include <QList>
#include <QSettings>
#include <QTimer>

#include "memory.h"
#include "memorytablemodel.h"
#include "optimsocsystemfactory.h"
#include "systeminterface.h"
#include "util.h"

MemoryInteractionWidget::MemoryInteractionWidget(QWidget *parent) :
    QWidget(parent),
    m_ui(new Ui::MemoryInteractionWidget),
    m_sysif(SystemInterface::instance()),
    m_memmodel(NULL)
{
    m_ui->setupUi(this);

    connect(m_ui->btnChooseFile, SIGNAL(clicked()),
            this, SLOT(showFileDialog()));
    connect(m_ui->editMemoryFile, SIGNAL(editingFinished()),
            this, SLOT(recalculateFileSize()));
    connect(OptimsocSystemFactory::instance(),
            SIGNAL(currentSystemChanged(OptimsocSystem*, OptimsocSystem*)),
            this,
            SLOT(populateMemoryList()));
    connect(m_ui->btnWriteMemory, SIGNAL(clicked()),
            this, SLOT(writeMemory()));

    m_ui->lblMessages->clear();

    readSettings();

    // ensure initial UI state
    recalculateFileSize();
}

MemoryInteractionWidget::~MemoryInteractionWidget()
{
    writeSettings();

    delete m_ui;
}

void MemoryInteractionWidget::readSettings()
{
    QSettings settings;
    settings.beginGroup("MemoryInteractionWidget");

    m_ui->editMemoryFile->setText(settings.value("editMemoryFileText").toString());
    m_ui->sbStartAddress->setValue(settings.value("sbStartAddressValue").toInt());

    settings.endGroup();
}

void MemoryInteractionWidget::writeSettings()
{
    QSettings settings;
    settings.beginGroup("MemoryInteractionWidget");

    settings.setValue("editMemoryFileText", m_ui->editMemoryFile->text());
    settings.setValue("sbStartAddressValue", m_ui->sbStartAddress->value());

    settings.endGroup();
}

void MemoryInteractionWidget::showFileDialog()
{
    QString filename = m_ui->editMemoryFile->text();
    QFileDialog fileDialog(this);
    fileDialog.setFileMode(QFileDialog::ExistingFile);
    fileDialog.setWindowTitle(tr("Choose a memory dump file"));
    fileDialog.setNameFilter(tr("Memory Dumps(*.bin);;All Files(*.*)"));
    fileDialog.selectFile(filename);
    if (!fileDialog.exec() || fileDialog.selectedFiles().count() < 1) {
        return;
    }
    filename = fileDialog.selectedFiles().at(0);

    m_ui->editMemoryFile->setText(filename);
    recalculateFileSize();
}

/**
 * Refresh the label displaying the size of the selected memory file
 */
void MemoryInteractionWidget::recalculateFileSize()
{
    QString filename = m_ui->editMemoryFile->text();
    QFileInfo fileinfo(filename);
    if (!fileinfo.exists()) {
        return;
    }

    m_ui->lblFileSize->setText(Util::formatBytesHumanReadable(fileinfo.size()));
}

/**
 * Populate the list of memories after the system information has been read
 */
void MemoryInteractionWidget::populateMemoryList()
{
    OptimsocSystem *system = OptimsocSystemFactory::currentSystem();
    if (!system) {
        return;
    }

    if (!m_memmodel) {
        m_memmodel = new MemoryTableModel(system, this);
    } else {
        m_memmodel->setOptimsocSystem(system);
    }

    m_ui->tvMemories->setModel(m_memmodel);
}

/**
 * Write the file to all selected memory tiles
 *
 * @see writeNextMemory()
 */
void MemoryInteractionWidget::writeMemory()
{
    if (m_memmodel->selectedMemories().count() == 0) {
        return;
    }
    disableUiForUpload(true);


    m_writeCurrentBaseAddress = m_ui->sbStartAddress->value();
    m_writeCurrentSelectedMemories = m_memmodel->selectedMemories();

    // write data from file
    QFile file(m_ui->editMemoryFile->text());
    if (!file.open(QIODevice::ReadOnly)) {
        m_ui->lblMessages->setText("<font color=red>Unable to open memory "
                                   "file!</font>");
        disableUiForUpload(false);
        return;
    }
    QByteArray data = file.readAll();
    if (data.isEmpty()) {
        m_ui->lblMessages->setText("<font color=red>Unable to read data from "
                                   "file!</font>");
        disableUiForUpload(false);
        return;
    }
    m_writeCurrentData = data;

    // write to first selected memory
    Memory* mem = m_writeCurrentSelectedMemories.at(0);
    connect(mem, SIGNAL(writeFinished(bool)),
            this, SLOT(writeNextMemory(bool)));
    m_writeCurrentMemoryIdx = 0;
    QString msg = QString("Writing to %1 ...").arg(mem->name());
    m_ui->lblMessages->setText(msg);
    mem->write(m_writeCurrentData, m_writeCurrentBaseAddress);
}

/**
 * Enable/Disable parts of the UI while writing data to a memory
 *
 * @param disable disable UI? (true = disable, false = enable)
 */
void MemoryInteractionWidget::disableUiForUpload(bool disable)
{
    m_ui->tvMemories->setEnabled(!disable);
    m_ui->btnChooseFile->setEnabled(!disable);
    m_ui->editMemoryFile->setEnabled(!disable);
    m_ui->sbStartAddress->setEnabled(!disable);
    m_ui->btnWriteMemory->setEnabled(!disable);
}

/**
 * Write to the next selected memory
 *
 * This method is called in reaction to a writeFinished() signal from the the
 * previously written memory. The result status is checked and the next memory
 * in the selected list is written (if any).
 *
 * @see writeMemory()
 *
 * @param prevSuccessful was the previous memory write successful?
 */
void MemoryInteractionWidget::writeNextMemory(bool prevSuccessful)
{
    disconnect(m_writeCurrentSelectedMemories.at(m_writeCurrentMemoryIdx),
               SIGNAL(writeFinished(bool)),
               this,
               SLOT(writeNextMemory(bool)));

    if (!prevSuccessful) {
        QString msg = QString("<font color=red>Unable to write to %1.</font>")
                        .arg(m_writeCurrentSelectedMemories.at(m_writeCurrentMemoryIdx)->name());
        m_ui->lblMessages->setText(msg);
        disableUiForUpload(false);
        return;
    }

    if (m_writeCurrentMemoryIdx == m_writeCurrentSelectedMemories.count() - 1) {
        // all memories have been written, we're done!
        m_writeCurrentMemoryIdx = 0;
        m_writeCurrentSelectedMemories.clear();
        m_writeCurrentData.clear();
        m_writeCurrentBaseAddress = 0;

        m_ui->lblMessages->setText("<font color=green>All memories have "
                                   "successfully been written.</font>");
        QTimer::singleShot(5000, this, SLOT(clearMessageLabel()));
        disableUiForUpload(false);
        return;
    }

    // write to next selected memory
    Memory* mem = m_writeCurrentSelectedMemories.at(++m_writeCurrentMemoryIdx);
    connect(mem, SIGNAL(writeFinished(bool)),
            this, SLOT(writeNextMemory(bool)));
    QString msg = QString("Writing to %1 ...").arg(mem->name());
    m_ui->lblMessages->setText(msg);
    mem->write(m_writeCurrentData, m_writeCurrentBaseAddress);
}

void MemoryInteractionWidget::clearMessageLabel()
{
    m_ui->lblMessages->clear();
}
