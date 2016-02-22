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

#include "nocrouter.h"

#include "nocrouteritem.h"

NocRouter::NocRouter(int routerId, QObject *parent) :
    QObject(parent),
    m_routerId(routerId)
{
    for (int i=0; i<5; i++) {
        m_links[i] = 0;
    }
}

NocLink* NocRouter::link(NocRouterPort port) const
{
    if (static_cast<int>(port) > 4) {
        qFatal("Link does not exist!");
        return 0;
    }
    return m_links[(int)port];
}

void NocRouter::setLink(NocRouterPort port, NocLink *link)
{
    m_links[(int)port] = link;
}

void NocRouter::setTile(Tile* tile)
{
    m_tile = tile;
}

Tile* NocRouter::tile() const
{
    return m_tile;
}

NocRouterItem* NocRouter::componentItem()
{
    return new NocRouterItem(this);
}
