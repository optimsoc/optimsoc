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

#ifndef NOCLINKITEM_H
#define NOCLINKITEM_H

#include "componentitem.h"

QT_BEGIN_NAMESPACE
class QGraphicsPolygonItem;
class QGraphicsLineItem;
class QRectF;
class QGraphicsSceneMouseEvent;
class QPainterPath;
class QPointF;
QT_END_NAMESPACE

class NocLinkItem : public QGraphicsLineItem
{
public:
    NocLinkItem(ComponentItem *startItem, ComponentItem *endItem,
                QGraphicsItem *parent = 0);

    ComponentItem *startItem() const
        { return m_startItem; }
    ComponentItem *endItem() const
        { return m_endItem; }

    void updatePosition();

private:
    ComponentItem *m_startItem;
    ComponentItem *m_endItem;
};

#endif // NOCLINKITEM_H
