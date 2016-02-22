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
#include "externaltile.h"
#include "memory.h"
#include "memorytile.h"
#include "optimsocsystemfactory.h"
#include "xsltproc.h"

#include <QDomDocument>
#include <QFile>
#include <QFileInfo>
#include <QUrl>
#include <QtXmlPatterns/QXmlSchema>
#include <QtXmlPatterns/QXmlSchemaValidator>
#include <QtDebug>

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
 * A couple things are done in order to extract the correct system description
 * out of the XML file:
 *
 * -# The input XML file @p xmlDescFile is validated against the XML Schema
 *    for the OpTiMSoC System Description XML, optimsoc-sysdesc.xsd
 * -# The XML file is converted into the generic system description form,
 *    replacing short forms like the meshnoc elements with the generic form,
 *    e.g. genericnoc.
 *    At the same time the graphic showing the system is auto-generated if
 *    requested by the author (layout/\@autogen = 'true').
 * -# The SVG system overview graphic is extracted and the SystemOverviewWidget
 *    is initialized with it.
 * -# Information about the system, e.g. the tile configuration, etc. is
 *    extracted.
 *
 * @param xmlDescFile the file name of the input XML file
 * @return parsing successful?
 */
bool OptimsocSystem::parseXmlDescription(QString xmlDescFile)
{
    QDomDocument doc;

    // get the system description directory (it contains the XSLT and
    // XML Schema files)
    QString systemDescriptionsDir = OptimsocSystemFactory::getSysdescDir();
    if (systemDescriptionsDir.isNull()) {
        qWarning("No system description directory found. Unable to create system.");
        return false;
    }

    // check if the XML file exists
    if (!QFile::exists(xmlDescFile)) {
        qWarning() << "The XML system description file '" << xmlDescFile << "' does not exist.";
        return false;
    }

    // validate the system description XML against the XML Schema
    // We do this at every loading since all following operations require a
    // XML document following the specifications layed out in the schema. They
    // might fail in obscure ways if the XML file is wrong in some way.
    QString schemaFile = QString("%1/util/optimsoc-system.xsd").arg(systemDescriptionsDir);
    QXmlSchema schema;
    schema.load(QUrl::fromLocalFile(schemaFile));
    if (!schema.isValid()) {
        qWarning() << "The XML Schema " << schemaFile << "is not valid";
        return false;
    }
    QXmlSchemaValidator schemaValidator(schema);
    if (!schemaValidator.validate(QUrl::fromLocalFile(xmlDescFile))) {
        qWarning() << "The instance " << xmlDescFile << " does not validate "
                   << "against the schema " << schemaFile;
        return false;
    }

    // convert the input XML file to a generic system description
    // We don't use the XSLT engine from Qt's XmlPatterns module since it does
    // support only a very limited subset of XSLT (2.0) and does not allow
    // writing reliable stylesheets which work across different XSLT processors.
    // Instead we use libxslt, which is encapsulated inside the XsltProc class.
    QString genericXslt = QString("%1/util/convert-to-generic.xsl").arg(systemDescriptionsDir);
    XsltProc xsltproc(xmlDescFile, genericXslt);
    if (!xsltproc.transform()) {
        qWarning("Unable to convert input XML to generic description");
        return false;
    }

    // read generic description into DOM
    QString errorMsg;
    int errorLine, errorColumn;
    if (!doc.setContent(xsltproc.outputDocument(), true, &errorMsg, &errorLine,
                        &errorColumn)) {
        qWarning("Unable to put XML system description into DOM: "
                 "%s in line %d, column %d.", errorMsg.toLatin1().data(),
                 errorLine, errorColumn);
        return false;
    }

    // extract the layout SVG
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
    } else {
        // the SVG is inline in the system description XML
        QByteArray svgData("<?xml version='1.0' encoding='UTF-8'?>\n");
        QTextStream svgDataStream(&svgData, QIODevice::Append);
        svgDataStream.setCodec("UTF-8");
        layout.firstChild().save(svgDataStream, 2, QDomNode::EncodingFromTextStream);
        m_layoutSvg = svgData;
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

/**
 * Get the SVG graphic showing the system layout
 *
 * @return the complete (and valid) XML document describing the SVG
 */
QByteArray OptimsocSystem::layoutSvg()
{
    return m_layoutSvg;
}

/**
 * Register an ID for a system element in the element registry
 *
 * @param id      the ID to register
 * @param element the element with the ID
 * @return
 */
bool OptimsocSystem::registerId(QString id, OptimsocSystemElement *element)
{
    // XXX: Check if the element is already registered and un-register it first!
    m_elementIdLookup.insert(id, element);
    return true;
}
