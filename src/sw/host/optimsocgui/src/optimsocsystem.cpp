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

#include "optimsocsystem.h"

OptimsocSystem::OptimsocSystem(QObject *parent) :
    QObject(parent),
    m_noc(0)
{

}

OptimsocSystem::~OptimsocSystem()
{
    delete m_noc;

    qDeleteAll(m_tiles);
    m_tiles.clear();
}

/**
 * Add a tile to the optimsoc system
 *
 * The pointer ownership is transferred to this class.
 */
void OptimsocSystem::addTile(Tile *tile)
{
    m_tiles.append(tile);
}

void OptimsocSystem::setNoc(Noc *noc)
{
    if (m_noc) {
        delete m_noc;
    }

    m_noc = noc;
}

Noc* OptimsocSystem::noc()
{
    return m_noc;
}

QList<Tile*> OptimsocSystem::tiles()
{
    return m_tiles;
}
