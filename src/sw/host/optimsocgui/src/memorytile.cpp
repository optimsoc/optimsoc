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

#include "memorytile.h"

#include "memorytileitem.h"
#include "hardwareinterface.h"

MemoryTile::MemoryTile(int memoryTileId, QObject *parent) :
    Tile(parent),
    m_memoryTileId(memoryTileId)
{
}

TileItem* MemoryTile::componentItem()
{
    return new MemoryTileItem(this);
}

/**
 * Initialize the memory of this tile.
 *
 * This method is asynchronous. The return code is delivered through the
 * memoryWriteFinished() signal.
 */
bool MemoryTile::initMemory(QByteArray data)
{
    HardwareInterface* hwif = HardwareInterface::instance();
    connect(hwif, SIGNAL(memoryWriteFinished(bool)),
            this, SIGNAL(memoryWriteFinished(bool)));
    QMetaObject::invokeMethod(hwif, "initMemory", Q_ARG(int, m_memoryTileId),
                              Q_ARG(QByteArray, data));
}
