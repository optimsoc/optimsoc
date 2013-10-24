/* Copyright (c) 2012-2013 by the author(s)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * Author(s):
 *   Philipp Wagner <philipp.wagner@tum.de>
 */

#include "optimsocsystem.h"

#include "computetile.h"
#include "memorytile.h"
#include "externaltile.h"
#include "memory.h"

#include <QDomDocument>
#include <QFile>
#include <QUrl>
#include <QDebug>

/**
 * Namespace of the OpTiMSoC system description XML
 */
const QString OptimsocSystem::SYSDESC_NS = "http://www.optimsoc.org/xmlns/optimsoc-system";

/**
 * Constructor: Create a new instance
 *
 * @param xmlDescFile the XML file describing the system
 * @param parent
 */
OptimsocSystem::OptimsocSystem(QString xmlDescFile, QObject *parent) :
    QObject(parent),
    m_noc(0)
{
    parseXmlDescription(xmlDescFile);
}

OptimsocSystem::~OptimsocSystem()
{
    delete m_noc;

    qDeleteAll(m_tiles);
    m_tiles.clear();
}

/**
 * Parse the XML system description file and initialize this object with it
 *
 * @param xmlDescFile
 * @return
 */
bool OptimsocSystem::parseXmlDescription(QString xmlDescFile)
{
    QDomDocument doc;

    QFile file(xmlDescFile);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning("Unable to read XML system description file.");
        return false;
    }
    QString errorMsg;
    int errorLine, errorColumn;
    if (!doc.setContent(&file, true, &errorMsg, &errorLine, &errorColumn)) {
        file.close();
        qWarning("Unable to put XML system description into DOM: "
                 "%s in line %d, column %d.", errorMsg.toLatin1().data(),
                 errorLine, errorColumn);
        return false;
    }
    file.close();

    // layout SVG
    QDomNode layout = doc.elementsByTagNameNS(SYSDESC_NS, "layout").item(0);
    if (layout.attributes().contains("src")) {
        // possibly resolve a relative @src
        QUrl baseUrl = QUrl::fromLocalFile(xmlDescFile);
        QUrl srcUrl(layout.attributes().namedItem("src").nodeValue());
        QUrl svgUrl = baseUrl.resolved(srcUrl);

        if (!svgUrl.isLocalFile()) {
            qWarning() << "URL to SVG file is not local and will not be used!";
        } else {
            QFile svgFile(svgUrl.toLocalFile());
            if (!svgFile.open(QIODevice::ReadOnly)) {
                qWarning() << "Unable to open SVG file" << svgUrl.toLocalFile() << "for system layout";
            } else {
                m_layoutSvg = svgFile.readAll();
            }
        }
    }

    // read all tiles
    QDomNode tilesRoot = doc.elementsByTagNameNS(SYSDESC_NS, "tiles").item(0);
    QDomNodeList tiles = tilesRoot.childNodes();
    for (int i = 0; i < tiles.length(); i++) {
        if (tiles.item(i).namespaceURI() != OptimsocSystem::SYSDESC_NS) {
            // just skip tile elements that are not in our namespace
            continue;
        }

        Tile* tile;
        QString localName = tiles.item(i).localName();
        if (localName == "computetile") {
            tile = new ComputeTile(tiles.item(i), this);
            m_tiles.append(tile);
        }/* else if (localName == "memorytile") {
            tile = new MemoryTile(tiles.item(i), this);
            m_tiles.append(tile);
        } else if (tiles.item(i).localName() == "externaltile") {
            tile = new ExternalTile(tiles.item(i), this);
            m_tiles.insert(id, tile);
        }*/ else {
            qWarning() << "Ignoring unknown tile type" << tiles.item(i).localName();
        }
    }

    return true;
}

void OptimsocSystem::setNoc(Noc *noc)
{
    if (m_noc) {
        delete m_noc;
    }

    m_noc = noc;
}

Noc* OptimsocSystem::noc()
{
    return m_noc;
}

/**
 * Get a list of all tiles in the system
 *
 * @return
 */
QList<OptimsocSystemElement*> OptimsocSystem::tiles()
{
    return m_tiles;
}

/**
 * Get all memories in the system.
 *
 * This list contains all memories, including distributed memory inside compute
 * tiles and memory inside memory tiles.
 *
 * @return
 */
QList<Memory*> OptimsocSystem::memories()
{
    QList<Memory*> memories;
    foreach (OptimsocSystemElement* el, m_tiles) {
        appendMemoryChildrenToList(el, memories);
    }
    return memories;
}

void OptimsocSystem::appendMemoryChildrenToList(OptimsocSystemElement *element,
                                                QList<Memory*>& list)
{
    foreach (OptimsocSystemElement *child, element->children()) {
        Memory* mem = dynamic_cast<Memory*>(child);
        if (mem) {
            list.append(mem);
        }
    }
}

/**
 * Get the element with a given ID
 *
 * @param id
 * @return
 */
OptimsocSystemElement* OptimsocSystem::elementById(const QString id)
{
    if (!m_elementIdLookup.contains(id)) {
        return NULL;
    }
    return m_elementIdLookup.value(id);
}

QByteArray OptimsocSystem::layoutSvg()
{
    return m_layoutSvg;
}

bool OptimsocSystem::registerId(QString id, OptimsocSystemElement *element)
{
    // XXX: Check if the element is already registered and un-register it first!
    m_elementIdLookup.insert(id, element);
    return true;
}
