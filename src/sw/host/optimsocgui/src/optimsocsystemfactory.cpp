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
 *    Stefan Wallentowitz, stefan.wallentowitz@tum.de
 */

#include "optimsocsystemfactory.h"

#include "computetile.h"
#include "memorytile.h"
#include "externaltile.h"
#include "meshnoc.h"

/**
 * Create a OptimsocSystem object for a system with the given \p systemId
 *
 * @todo read the system information from a database instead of hardcoding it
 */
OptimsocSystem* OptimsocSystemFactory::createSystemFromId(int systemId)
{
    OptimsocSystem *system;
    switch(systemId) {
    case 1:
        system = createSystem1();
        break;
    case 2:
        system = createSystem2();
        break;
    case 57005:
        system = createSystem57005();
        break;
    case 0xce75:
        system = createSystem2x2CcccZtex();
        break;
    default:
        qWarning("No system description available for ID %d!", systemId);
        system = NULL;
        break;
    }

    return system;
}

OptimsocSystem *OptimsocSystemFactory::createSystem1()
{
    OptimsocSystem *system = new OptimsocSystem();

    // Tiles: CMEC
    ComputeTile* ct0 = new ComputeTile(0);
    system->addTile(ct0);
    MemoryTile* mt1 = new MemoryTile(1);
    system->addTile(mt1);
    ExternalTile* et2 = new ExternalTile(2);
    system->addTile(et2);
    ComputeTile* ct3 = new ComputeTile(3);
    system->addTile(ct3);

    // NoC: 2x2 mesh
    MeshNoc* noc = new MeshNoc(2, 2);
    NocRouter* r00 = new NocRouter(0);
    r00->setTile(ct0);
    NocRouter* r10 = new NocRouter(1);
    r10->setTile(mt1);
    NocRouter* r01 = new NocRouter(2);
    r01->setTile(et2);
    NocRouter* r11 = new NocRouter(3);
    r11->setTile(ct3);
    noc->setRouter(0, 0, r00);
    noc->setRouter(1, 0, r10);
    noc->setRouter(0, 1, r01);
    noc->setRouter(1, 1, r11);
    system->setNoc(noc);

    return system;
}

OptimsocSystem *OptimsocSystemFactory::createSystem2x2CcccZtex()
{
    OptimsocSystem *system = new OptimsocSystem();

    // Tiles: CCCC
    ComputeTile* ct0 = new ComputeTile(0);
    system->addTile(ct0);
    ComputeTile* ct1 = new ComputeTile(1);
    system->addTile(ct1);
    ComputeTile* ct2 = new ComputeTile(2);
    system->addTile(ct2);
    ComputeTile* ct3 = new ComputeTile(3);
    system->addTile(ct3);

    // NoC: 2x2 mesh
    MeshNoc* noc = new MeshNoc(2, 2);
    NocRouter* r00 = new NocRouter(0);
    r00->setTile(ct0);
    NocRouter* r10 = new NocRouter(1);
    r10->setTile(ct1);
    NocRouter* r01 = new NocRouter(2);
    r01->setTile(ct2);
    NocRouter* r11 = new NocRouter(3);
    r11->setTile(ct3);
    noc->setRouter(0, 0, r00);
    noc->setRouter(1, 0, r10);
    noc->setRouter(0, 1, r01);
    noc->setRouter(1, 1, r11);
    system->setNoc(noc);

    return system;
}

OptimsocSystem *OptimsocSystemFactory::createSystem57005()
{
    OptimsocSystem *system = new OptimsocSystem();

    ComputeTile* ct0 = new ComputeTile(0);
    system->addTile(ct0);

    MeshNoc* noc = new MeshNoc(1, 1);
    NocRouter* r00 = new NocRouter(0);
    r00->setTile(ct0);
    noc->setRouter(0, 0, r00);
    system->setNoc(noc);

    return system;
}

OptimsocSystem *OptimsocSystemFactory::createSystem2()
{
    OptimsocSystem *system = new OptimsocSystem();

    ComputeTile* ct0 = new ComputeTile(0);
    system->addTile(ct0);

    ComputeTile* ct1 = new ComputeTile(1);
    system->addTile(ct1);

    ComputeTile* ct2 = new ComputeTile(2);
    system->addTile(ct2);

    ComputeTile* ct3 = new ComputeTile(3);
    system->addTile(ct3);

    // NoC: 2x2 mesh
    MeshNoc* noc = new MeshNoc(2, 2);
    NocRouter* r00 = new NocRouter(0);
    r00->setTile(ct0);
    NocRouter* r10 = new NocRouter(1);
    r10->setTile(ct1);
    NocRouter* r01 = new NocRouter(2);
    r01->setTile(ct2);
    NocRouter* r11 = new NocRouter(3);
    r11->setTile(ct3);
    noc->setRouter(0, 0, r00);
    noc->setRouter(1, 0, r10);
    noc->setRouter(0, 1, r01);
    noc->setRouter(1, 1, r11);
    system->setNoc(noc);

    return system;
}
