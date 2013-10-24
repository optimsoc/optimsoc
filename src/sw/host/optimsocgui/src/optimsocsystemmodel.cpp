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

#include "optimsocsystemmodel.h"

#include <QDebug>

#include "optimsocsystem.h"
#include "optimsocsystemelement.h"
#include "optimsocsystemelementtreeitem.h"

OptimsocSystemModel::OptimsocSystemModel(OptimsocSystem *system,
                                         QObject *parent) :
    m_system(system), QAbstractItemModel(parent)
{
    m_rootElement = new OptimsocSystemElementTreeItem("root item");
    m_tileRoot = new OptimsocSystemElementTreeItem("Tiles", m_rootElement);
    if (system) {
        m_tileRoot->setReferencedChildren(system->tiles());
    }
    m_nocRoot = new OptimsocSystemElementTreeItem("NoC", m_rootElement);
}

OptimsocSystemModel::~OptimsocSystemModel()
{
}

void OptimsocSystemModel::setOptimsocSystem(OptimsocSystem *system)
{
    m_system = system;

    m_tileRoot->setReferencedChildren(system->tiles());
}

QModelIndex OptimsocSystemModel::index(int row, int column,
                                       const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    OptimsocSystemElement *parentElement;

    if (!parent.isValid()) {
        parentElement = m_rootElement;
    } else {
        parentElement = static_cast<OptimsocSystemElement*>(parent.internalPointer());
    }

    OptimsocSystemElement *childElement = parentElement->children().at(row);
    if (childElement) {
        return createIndex(row, column, childElement);
    } else {
        return QModelIndex();
    }
}

/**
 * Get the parent of a model index
 *
 * @param index
 * @return
 */
QModelIndex OptimsocSystemModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    OptimsocSystemElement *childElement = static_cast<OptimsocSystemElement*>(index.internalPointer());

    // The following is a bit ugly, since we have to special-case the subtrees
    // set with OptimsocSystemElementTreeItem::setReferencedChildren(). For
    // those trees children() works as expected, but parent() returns 0 when
    // the root of the sub-tree is reached. We need here the root of the
    // complete tree, so we look for those cases and choose the right parent
    // if this happens.
    OptimsocSystemElement *parentElement = childElement->parent();
    if (parentElement == NULL && m_tileRoot->children().contains(childElement)) {
        parentElement = m_tileRoot;
    }

    OptimsocSystemElement *grandparentElement = parentElement->parent();
    if (grandparentElement == NULL && m_tileRoot->children().contains(parentElement)) {
        grandparentElement = m_tileRoot;
    }

    if (parentElement == m_rootElement) {
        // the root element does not have a parent
        return QModelIndex();
    }

    // get the row of the parent item
    int row = 0;
    if (grandparentElement) {
        row = grandparentElement->children().indexOf(parentElement);
    }

    return createIndex(row, 0, parentElement);
}

int OptimsocSystemModel::rowCount(const QModelIndex &parent) const
{
    OptimsocSystemElement *parentElement;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid()) {
        parentElement = m_rootElement;
    } else {
        parentElement = static_cast<OptimsocSystemElement*>(parent.internalPointer());
    }

    return parentElement->children().count();
}

int OptimsocSystemModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 1;
}

QVariant OptimsocSystemModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (role == Qt::DisplayRole) {
        OptimsocSystemElement *element = static_cast<OptimsocSystemElement*>(index.internalPointer());
        return element->name();
    } else if (role == Qt::UserRole) {
        return QVariant::fromValue(index.internalPointer());
    } else {
        return QVariant();
    }
}

Qt::ItemFlags OptimsocSystemModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return 0;
    }

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant OptimsocSystemModel::headerData(int section,
                                         Qt::Orientation orientation,
                                         int role) const
{
    Q_UNUSED(section);

    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        return "OpTiMSoC System Overview";
    }

    return QVariant();
}

/**
 * Get the QModelIndex for a given system element
 *
 * This is useful to select an element, etc.
 *
 * @param element
 * @return
 */
QModelIndex OptimsocSystemModel::indexForElement(OptimsocSystemElement* element)
{
    QModelIndexList list = match(index(0,0), Qt::UserRole,
                                 QVariant::fromValue((void*) element), 1,
                                 Qt::MatchFlags(Qt::MatchRecursive));

    if (list.count() == 0) {
        return QModelIndex();
    } else {
        return list.at(0);
    }
}
