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

#ifndef OPTIMSOCSYSTEM_H
#define OPTIMSOCSYSTEM_H

#include <QObject>
#include <QList>

#include "tile.h"
#include "noc.h"

class OptimsocSystem : public QObject
{
Q_OBJECT
public:
    explicit OptimsocSystem(QObject *parent = 0);
    virtual ~OptimsocSystem();

    void addTile(Tile *tile);
    void setNoc(Noc *noc);
    Noc* noc();
    QList<Tile*> tiles();

signals:

public slots:

private:
    QList<Tile*> m_tiles;
    Noc *m_noc;
};

#endif // OPTIMSOCSYSTEM_H
