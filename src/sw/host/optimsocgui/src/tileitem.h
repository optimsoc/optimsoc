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

#ifndef TILEITEM_H
#define TILEITEM_H

#include <QSize>

#include "componentitem.h"

class NocLinkItem;
class Tile;

class TileItem : public ComponentItem
{
    Q_OBJECT
public:
    TileItem(Tile* tile, QGraphicsItem *parent = 0);


    NocLinkItem* localLink() const { return m_localLink; }
    void setLocalLink(NocLinkItem* link) { m_localLink = link; }

    QPointF anchorPointForLink(NocLinkItem* link) const;

    static const int DefaultWidth = 100;
    static const int DefaultHeight = 100;

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);
    Tile* m_tile;

private:
    QPolygonF m_outline;
    NocLinkItem* m_localLink;
};

#endif // TILEITEM_H
