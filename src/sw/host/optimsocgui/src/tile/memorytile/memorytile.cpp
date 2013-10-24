/* Copyright (c) 2013 by the author(s)
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

#include "memorytile.h"

#include <QDebug>
#include <QDomNode>
#include <QWidget>
#include <QLabel>
#include <QGridLayout>

#include "memory.h"
#include "optimsocsystem.h"

/**
 * Create a new MemoryTile based on its XML description
 *
 * @param tileNode the XML description of the tile
 * @param parent
 */
MemoryTile::MemoryTile(const QDomNode &tileNode, OptimsocSystem *system,
                       OptimsocSystemElement *parent) :
    Tile(system, parent), m_propertiesWidget(NULL)
{
    QDomNamedNodeMap attributes = tileNode.attributes();
    setId(attributes.namedItem("id").nodeValue());
    m_memoryTileId = attributes.namedItem("memorytileid").nodeValue().toInt();

    // read all memories
    if (tileNode.isElement()) {
        QDomNodeList memories = tileNode.toElement().elementsByTagNameNS(OptimsocSystem::SYSDESC_NS, "memory");
        for (int i = 0; i < memories.length(); i++) {
            new Memory(memories.item(i), optimsocSystem(), this);
        }
    }
}

MemoryTile::~MemoryTile()
{
    delete m_propertiesWidget;
}

int MemoryTile::memoryTileId()
{
    return m_memoryTileId;
}

QString MemoryTile::name()
{
    return QString("Memory Tile %1").arg(m_memoryTileId);
}

QWidget* MemoryTile::propertiesWidget()
{
    if (m_propertiesWidget) {
        return m_propertiesWidget;
    }

    m_propertiesWidget = new QWidget();
    QGridLayout *layout = new QGridLayout(m_propertiesWidget);
    m_propertiesWidget->setLayout(layout);
    QLabel *lblMemorytileidDesc = new QLabel("Memory tile identifier", m_propertiesWidget);
    QLabel *lblMemorytileidContent = new QLabel(QString::number(m_memoryTileId), m_propertiesWidget);
    layout->addWidget(lblMemorytileidDesc, 0, 0);
    layout->addWidget(lblMemorytileidContent, 0, 1);

    return m_propertiesWidget;
}
