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

#include "memorytablemodel.h"

#include <QLabel>
#include <QList>
#include <QDebug>

#include "memory.h"
#include "optimsocsystem.h"
#include "util.h"

MemoryTableModel::MemoryTableModel(OptimsocSystem *system, QObject *parent) :
    m_system(system), QAbstractTableModel(parent)
{
    if (m_system) {
        m_memories = m_system->memories();
    }
}

void MemoryTableModel::setOptimsocSystem(OptimsocSystem *system)
{
    if (!system) {
        return;
    }
    m_system = system;
    m_memories = m_system->memories();
}

bool MemoryTableModel::setData(const QModelIndex& index, const QVariant& value,
                               int role)
{
    if (role == Qt::CheckStateRole) {
        m_selectedMemories.insert(index.row(), value.toBool());
        return true;
    }
    return false;
}

QVariant MemoryTableModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= m_memories.count()) {
        return QVariant();
    }

    if (role == Qt::DisplayRole) {
        if (index.column() == 0) {
            return m_memories.at(index.row())->name();
        } else if (index.column() == 1) {
            return Util::formatBytesHumanReadable(m_memories.at(index.row())->size());
        }
    } else if (role == Qt::UserRole) {
        //return QVariant::fromValue((void*)m_memories.at(index.row()));
        return QVariant();
    } else if (role == Qt::CheckStateRole) {
        if (index.column() != 0) {
            return QVariant();
        }
        if (!m_selectedMemories.contains(index.row())) {
            return Qt::Unchecked; // default
        }
        return m_selectedMemories.value(index.row()) ? Qt::Checked : Qt::Unchecked;
    } else {
        return QVariant();
    }
}

Qt::ItemFlags MemoryTableModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return 0;
    }

    Qt::ItemFlags flags;
    flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    if (index.column() == 0) {
        flags |= Qt::ItemIsUserCheckable;
    }
    return flags;
}

int MemoryTableModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return m_memories.count();
}

int	MemoryTableModel::columnCount(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return 2;
}

QVariant MemoryTableModel::headerData(int section, Qt::Orientation orientation,
                                      int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole) {
        return QVariant();
    }

    switch (section) {
    case 0:
        return "Memory";
    case 1:
        return "Size";
    default:
        return QString();
    }
}

QList<Memory*> MemoryTableModel::selectedMemories()
{
    QList<Memory*> selectedMemories;

    QMapIterator<int, bool> i(m_selectedMemories);
    while (i.hasNext()) {
        i.next();
        if (i.value() == true) {
            selectedMemories << m_memories.at(i.key());
        }
    }
    return selectedMemories;
}
