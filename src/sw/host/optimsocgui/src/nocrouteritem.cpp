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

#include "nocrouteritem.h"

#include <QDebug>
#include <QRectF>
#include <QPointF>

#include "componentitem.h"
#include "noclinkitem.h"
#include "nocrouter.h"

NocRouterItem::NocRouterItem(NocRouter *router, QGraphicsItem *parent) :
    ComponentItem(parent),
    m_nocRouter(router)
{
    for (int i=0; i<5; i++) {
        m_links[i] = 0;
    }

    m_outline = QPolygonF(QRectF(0, 0, 30, 30));
    setPolygon(m_outline);

    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
}

void NocRouterItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    qDebug() << "Context menu for NocRouterItem!";
}

QPointF NocRouterItem::anchorPointForLink(NocLinkItem *link) const
{
    if (link == m_links[0]) { // north
        return QPointF(boundingRect().width()/2, 0);
    } else if (link == m_links[1]) { // east
        return QPointF(boundingRect().width(), boundingRect().height()/2);
    } else if (link == m_links[2]) { // south
        return QPointF(boundingRect().width()/2, boundingRect().height());
    } else if (link == m_links[3]) { // west
        return QPointF(0, boundingRect().height()/2);
    } else if (link == m_links[4]) { // local
        return QPointF(0, 0);
    } else {
        qFatal("Link not connected to this router.");
        return QPointF();
    }
}

NocLinkItem* NocRouterItem::link(NocRouter::NocRouterPort port) const
{
    if (static_cast<int>(port) > 4) {
        qFatal("Link does not exist!");
        return 0;
    }
    return m_links[(int)port];
}

void NocRouterItem::setLink(NocRouter::NocRouterPort port, NocLinkItem *link)
{
    m_links[(int)port] = link;
}

QVariant NocRouterItem::itemChange(GraphicsItemChange change,
                                   const QVariant &value)
{
    if (change == QGraphicsItem::ItemPositionChange) {
        for (int i=0; i<5; i++) {
            if (m_links[i] != 0) {
                m_links[i]->updatePosition();
            }
        }
    }

    return value;
}
