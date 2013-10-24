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

#include "systemoverviewwidget.h"

#include "optimsocsystemfactory.h"
#include "optimsocsystem.h"
#include "systemoverviewjsapi.h"
#include "optimsocsystemelement.h"

#include <QDebug>
#include <QVBoxLayout>
#include <QDomDocument>
#include <QDomNode>
#include <QDomElement>
#include <QMenu>

SystemOverviewWidget::SystemOverviewWidget(QWidget *parent) :
    QWidget(parent)
{
    setupUi();
}

void SystemOverviewWidget::setSystem(OptimsocSystem *system)
{
    m_optimsocSystem = system;
    loadSvgIntoWebView();
}

void SystemOverviewWidget::setupUi()
{
    m_webView = new QWebView(this);

    // disable the WebKit context menu, we're providing our own one
    m_webView->setContextMenuPolicy(Qt::NoContextMenu);

    // JS API, providing a global window.optimsoc object for all documents
    m_jsapi = new SystemOverviewJsApi(this);
    m_jsapi->setWebView(m_webView);
    connect(m_jsapi, SIGNAL(itemClicked(QString)),
            this, SLOT(handleItemClicked(QString)));

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(m_webView);
    setLayout(layout);
}

void SystemOverviewWidget::loadSvgIntoWebView()
{
    // parse SVG into DOM
    QDomDocument doc;
    QString errorMsg;
    int errorLine, errorColumn;
    if (!doc.setContent(m_optimsocSystem->layoutSvg(), true, &errorMsg,
                        &errorLine, &errorColumn)) {
        qWarning("Unable to load SVG document into DOM: "
                 "%s in line %d, column %d.", errorMsg.toLatin1().data(),
                 errorLine, errorColumn);
    }

    // recursively go through all elements and insert onclick handler
    modifyDomElementForOnclick(doc.documentElement());

    // load into QWebView (unfortunately it has to parse it again)
    m_webView->setContent(doc.toByteArray(), "image/svg+xml");
}

void SystemOverviewWidget::modifyDomElementForOnclick(QDomElement el)
{
    if (el.hasAttributeNS(OptimsocSystem::SYSDESC_NS, "idref")) {
        QString idref = el.attributeNS(OptimsocSystem::SYSDESC_NS, "idref");
        // XXX: Sanitize idref!
        el.setAttribute("onclick", "optimsoc.handleIdClick('"+idref+"')");
    }

    QDomNode n = el.firstChild();
    while (!n.isNull()) {
        if (n.isElement()) {
            QDomElement e = n.toElement();
            modifyDomElementForOnclick(e);
        }
        n = n.nextSibling();
    }
}

void SystemOverviewWidget::handleItemClicked(QString idref)
{
    OptimsocSystemElement *element = m_optimsocSystem->elementById(idref);

    if (element) {
        QMenu* contextMenu = element->contextMenu();
        if (contextMenu) {
            contextMenu->exec(QCursor::pos());
        }
    }

    emit elementClicked(idref);
}
