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

#include "noclinkitem.h"

#include <QGraphicsLineItem>
#include <QDebug>

NocLinkItem::NocLinkItem(ComponentItem *startItem, ComponentItem *endItem,
                         QGraphicsItem *parent)
    : QGraphicsLineItem(parent),
      m_startItem(startItem), m_endItem(endItem)
{
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setPen(QPen(Qt::black, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
}

void NocLinkItem::updatePosition()
{
    if (!m_startItem || !m_endItem)
        return;

    QLineF line(mapFromItem(m_startItem, m_startItem->anchorPointForLink(this)),
                mapFromItem(m_endItem, m_endItem->anchorPointForLink(this)));
    setLine(line);
}
