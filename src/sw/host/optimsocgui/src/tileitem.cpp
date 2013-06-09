/**
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
 * Driver for the simple message passing hardware.
 *
 * (c) 2012-2013 by the author(s)
 *
 * Author(s):
 *    Philipp Wagner, philipp.wagner@tum.de
 */

#include "tileitem.h"

#include <QDebug>
#include <QRectF>
#include <QPointF>

#include "noclinkitem.h"
#include "tile.h"

TileItem::TileItem(Tile* tile, QGraphicsItem *parent) :
    ComponentItem(parent),
    m_tile(tile),
    m_localLink(0)
{
    m_outline = QPolygonF(QRectF(0, 0, DefaultWidth, DefaultHeight));
    setPolygon(m_outline);

    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
}

QPointF TileItem::anchorPointForLink(NocLinkItem *link) const
{
    Q_UNUSED(link);
    return QPointF(boundingRect().width(), boundingRect().height());
}

QVariant TileItem::itemChange(GraphicsItemChange change,
                              const QVariant &value)
{
    if (change == QGraphicsItem::ItemPositionChange) {
        if (m_localLink) {
            m_localLink->updatePosition();
        }
    }

    return value;
}
