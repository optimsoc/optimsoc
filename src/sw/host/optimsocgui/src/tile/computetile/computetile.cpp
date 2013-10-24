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

#include "computetile.h"

#include <QDebug>
#include <QDomNode>
#include <QWidget>
#include <QLabel>
#include <QGridLayout>

#include "memory.h"
#include "optimsocsystem.h"

/**
 * Create a new ComputeTile based on its XML description
 *
 * @param tileNode the XML description of the tile
 * @param parent
 */
ComputeTile::ComputeTile(const QDomNode &tileNode, OptimsocSystem *system,
                         OptimsocSystemElement *parent) :
    Tile(system, parent), m_propertiesWidget(NULL)
{
    QDomNamedNodeMap attributes = tileNode.attributes();
    setId(attributes.namedItem("id").nodeValue());
    m_computeTileId = attributes.namedItem("computetileid").nodeValue().toInt();
    m_type = attributes.namedItem("type").nodeValue();

    // read all memories
    if (tileNode.isElement()) {
        QDomNodeList memories = tileNode.toElement().elementsByTagNameNS(OptimsocSystem::SYSDESC_NS, "memory");
        for (int i = 0; i < memories.length(); i++) {
            new Memory(memories.item(i), optimsocSystem(), this);
        }
    }
}

ComputeTile::~ComputeTile()
{
    delete m_propertiesWidget;
}

int ComputeTile::computeTileId()
{
    return m_computeTileId;
}

QString ComputeTile::type()
{
    return m_type;
}

QString ComputeTile::name()
{
    return QString("Compute Tile %1").arg(m_computeTileId);
}

QWidget* ComputeTile::propertiesWidget()
{
    if (m_propertiesWidget) {
        return m_propertiesWidget;
    }

    m_propertiesWidget = new QWidget();
    QGridLayout *layout = new QGridLayout(m_propertiesWidget);
    m_propertiesWidget->setLayout(layout);
    QLabel *lblComputetileidDesc = new QLabel("Compute tile identifier", m_propertiesWidget);
    QLabel *lblComputetileidContent = new QLabel(QString::number(m_computeTileId), m_propertiesWidget);
    layout->addWidget(lblComputetileidDesc, 0, 0);
    layout->addWidget(lblComputetileidContent, 0, 1);

    QLabel *lblTypeDesc = new QLabel("CPU tile type", m_propertiesWidget);
    QLabel *lblTypeContent = new QLabel(m_type, m_propertiesWidget);
    layout->addWidget(lblTypeDesc, 1, 0);
    layout->addWidget(lblTypeContent, 1, 1);
    return m_propertiesWidget;
}
