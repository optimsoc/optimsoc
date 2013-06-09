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

#ifndef NOCROUTERITEM_H
#define NOCROUTERITEM_H

#include "componentitem.h"
#include "nocrouter.h"

class NocLinkItem;

class NocRouterItem : public ComponentItem
{
public:
    NocRouterItem(NocRouter* router, QGraphicsItem *parent = 0);

    NocLinkItem* link(NocRouter::NocRouterPort port) const;
    void setLink(NocRouter::NocRouterPort port, NocLinkItem* link);

    QPointF anchorPointForLink(NocLinkItem* link) const;

signals:

public slots:

protected:
    NocRouter* m_nocRouter;

    QPolygonF m_outline;
    NocLinkItem* m_links[5];

    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);

};

#endif // NOCROUTERITEM_H
