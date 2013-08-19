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

#include "meshnoclayout.h"

#include "meshnoc.h"
#include "nocrouteritem.h"
#include "tile.h"
#include "tileitem.h"
#include "noclinkitem.h"

MeshNocLayout::MeshNocLayout(MeshNoc* meshNoc, QObject *parent) :
    NocLayout(parent),
    m_meshNoc(meshNoc)
{
}

void MeshNocLayout::initScene(QGraphicsScene *scene)
{
    int xDistance = TileItem::DefaultWidth * 2;
    int yDistance = TileItem::DefaultHeight * 2;

    QList<NocRouterItem*> nocRouterItems;
    for (int y = 0; y < m_meshNoc->sizeY(); y++) {
        for (int x = 0; x < m_meshNoc->sizeX(); x++) {
            // tiles
            Tile* tile = m_meshNoc->router(x, y)->tile();
            TileItem* tileItem = tile->componentItem();
            tileItem->setPos(x*xDistance, y*yDistance);
            scene->addItem(tileItem);

            // NoC router
            NocRouterItem* router = m_meshNoc->router(x, y)->componentItem();
            nocRouterItems.append(router);
            router->setPos(x*xDistance + tileItem->boundingRect().width() + 20,
                           y*yDistance + tileItem->boundingRect().height() + 20);
            scene->addItem(router);

            // NoC links
            // local (tile to router)
            NocLinkItem* tileToRouter = new NocLinkItem(router, tileItem);
            router->setLink(NocRouter::Local, tileToRouter);
            tileItem->setLocalLink(tileToRouter);
            scene->addItem(tileToRouter);
            tileToRouter->updatePosition();

            // west and north (router to router)
            if (x != 0) {
                // west
                NocRouterItem* eastRouter = nocRouterItems.at(x-1+y*m_meshNoc->sizeY());
                NocLinkItem* westLink = new NocLinkItem(eastRouter, router);
                eastRouter->setLink(NocRouter::East, westLink);
                router->setLink(NocRouter::West, westLink);
                scene->addItem(westLink);
                westLink->updatePosition();
            }

            if (y != 0) {
                // north
                NocRouterItem* northRouter = nocRouterItems.at(x+(y-1)*m_meshNoc->sizeY());
                NocLinkItem* northLink = new NocLinkItem(northRouter, router);
                northRouter->setLink(NocRouter::South, northLink);
                router->setLink(NocRouter::North, northLink);
                scene->addItem(northLink);
                northLink->updatePosition();
            }
        }
    }
}
