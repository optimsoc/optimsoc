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

#include "systemview.h"
#include "ui_systemview.h"

#include <QMessageBox>
#include <QItemSelection>
#include <QModelIndex>
#include <QModelIndexList>
#include <QDebug>

#include "systeminterface.h"
#include "optimsocsystem.h"
#include "optimsocsystemfactory.h"
#include "optimsocsystemmodel.h"

SystemView::SystemView(QWidget *parent) :
    QWidget(parent),
    m_ui(new Ui::SystemView),
    m_sysif(SystemInterface::instance()),
    m_sysmodel(NULL)
{
    m_ui->setupUi(this);

    connect(m_sysif, SIGNAL(systemDiscovered(int)),
            this, SLOT(systemDiscovered(int)));
}

SystemView::~SystemView()
{
    delete m_ui;
}

/**
 * The system discovery has been finished and a new system has been found
 *
 * @param systemId unique ID of the discovered system
 */
void SystemView::systemDiscovered(int systemId)
{
    // The allocated system is free'd in the destructor of the MainWindow to
    // ensure it being deleted only when the application closes.
    OptimsocSystem *system = OptimsocSystemFactory::createSystemFromId(systemId);
    if (!system) {
        //QMetaObject::invokeMethod(m_hwif, "disconnect");
        /*QMessageBox::warning(this, "System not in database",
                             QString("The discovered system with ID %1 is not "
                                     "in the device database.").arg(systemId));*/
        return;
    }
    OptimsocSystem *currentSystem = OptimsocSystemFactory::currentSystem();
    if (currentSystem) {
        delete currentSystem;
    }
    OptimsocSystemFactory::setCurrentSystem(system);
    m_system = system;

    // initialize the hierarchical system view
    if (!m_sysmodel) {
        m_sysmodel = new OptimsocSystemModel(system, this);
    } else {
        m_sysmodel->setOptimsocSystem(system);
    }
    m_ui->tvOptimsocSystem->setModel(m_sysmodel);
    connect(m_ui->tvOptimsocSystem->selectionModel(),
            SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            this,
            SLOT(treeViewSelectionChanged(QModelIndex,QModelIndex)));

    // initialize system graphical overview widget
    m_ui->systemOverview->setSystem(m_system);
    connect(m_ui->systemOverview, SIGNAL(elementClicked(QString)),
            this, SLOT(selectElementInHierarchicalView(QString)));

}

/**
 * A different item in the hierarchical element view was selected (slot)
 *
 * This updates the properties widget to reflect the currently selected
 * element.
 *
 * @param selected
 * @param deselected
 */
void SystemView::treeViewSelectionChanged(const QModelIndex &selected,
                                          const QModelIndex &deselected)
{
    OptimsocSystemElement *oldElement = static_cast<OptimsocSystemElement*>(deselected.internalPointer());
    OptimsocSystemElement *element = static_cast<OptimsocSystemElement*>(selected.internalPointer());

    // remove deselected widget (if any)
    if (oldElement) {
        QWidget* oldPropertiesWidget = oldElement->propertiesWidget();
        if (oldPropertiesWidget) {
            oldPropertiesWidget->hide();
            m_ui->gbSystemProperties->layout()->removeWidget(oldPropertiesWidget);
        }
    }

    // insert new widget
    QWidget* propertiesWidget = element->propertiesWidget();
    if (propertiesWidget) {
        propertiesWidget->show();
        m_ui->gbSystemProperties->layout()->addWidget(propertiesWidget);
    }
}

void SystemView::selectElementInHierarchicalView(QString id)
{
    OptimsocSystemElement* element = m_system->elementById(id);
    if (!element) {
        return;
    }
    QPersistentModelIndex selectedIndex = QPersistentModelIndex(m_sysmodel->indexForElement(element));
    m_ui->tvOptimsocSystem->setCurrentIndex(selectedIndex);
    m_ui->tvOptimsocSystem->scrollTo(selectedIndex, QTreeView::EnsureVisible);
}
