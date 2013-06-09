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

#ifndef MESHNOC_H
#define MESHNOC_H

#include <QMap>

#include "noc.h"
#include "nocrouter.h"
#include "meshnoclayout.h"

class MeshNoc : public Noc
{
Q_OBJECT
public:
    explicit MeshNoc(int sizeX, int sizeY, QObject *parent = 0);
    void setRouter(int x, int y, NocRouter *router);
    NocRouter* router(int x, int y);
    int sizeX();
    int sizeY();
    MeshNocLayout *layout();

signals:

public slots:

protected:
    int m_sizeX, m_sizeY;
    QMap<int, NocRouter*> m_routers;

};

#endif // MESHNOC_H
