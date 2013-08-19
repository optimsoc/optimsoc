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

#include "memorytileitem.h"

#include <QDebug>
#include "writememorydialog.h"
#include "memorytile.h"

MemoryTileItem::MemoryTileItem(MemoryTile* memoryTile, QGraphicsItem *parent) :
    TileItem(memoryTile, parent)
{
    // memory tiles are blue
    setBrush(QBrush(QColor(152, 198, 234)));
    setPen(QPen(QColor(123, 159, 188)));

    m_contextMenu = new QMenu();
    QAction* writeMemAction = m_contextMenu->addAction("Write memory");

    connect(writeMemAction, SIGNAL(triggered()), this, SLOT(writeMemory()));
}

MemoryTileItem::~MemoryTileItem()
{
    delete m_contextMenu;
}

void MemoryTileItem::writeMemory()
{
    WriteMemoryDialog* diag = new WriteMemoryDialog(dynamic_cast<MemoryTile*>(m_tile));
    diag->setModal(true);
    diag->exec();
    delete diag;
}
