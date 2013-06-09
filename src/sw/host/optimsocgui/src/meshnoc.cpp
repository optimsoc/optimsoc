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

#include "meshnoc.h"

#include "meshnoclayout.h"

MeshNoc::MeshNoc(int sizeX, int sizeY, QObject *parent) :
    Noc(parent),
    m_sizeX(sizeX),
    m_sizeY(sizeY)
{
}

int MeshNoc::sizeX()
{
    return m_sizeX;
}

int MeshNoc::sizeY()
{
    return m_sizeY;
}

NocRouter* MeshNoc::router(int x, int y)
{
    return m_routers.value(x + m_sizeY*y);
}

void MeshNoc::setRouter(int x, int y, NocRouter *router)
{
    m_routers.insert(x + m_sizeY*y, router);
}

MeshNocLayout* MeshNoc::layout()
{
    return new MeshNocLayout(this, this);
}
