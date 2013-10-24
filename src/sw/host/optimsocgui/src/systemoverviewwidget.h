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

#ifndef SYSTEMOVERVIEWWIDGET_H
#define SYSTEMOVERVIEWWIDGET_H

#include <QWidget>
#include <QtWebKit/QWebView>

#include "optimsocsystem.h"

class QDomElement;
class SystemOverviewJsApi;

class SystemOverviewWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SystemOverviewWidget(QWidget *parent = 0);

    void setSystem(OptimsocSystem *system);

signals:
    void elementClicked(QString id);

public slots:
    void handleItemClicked(QString idref);

private:
    OptimsocSystem *m_optimsocSystem;
    QWebView* m_webView;
    SystemOverviewJsApi *m_jsapi;

    void setupUi();

    /**
     * Modify all DOMElements with @idref to contain an onclick handler
     *
     * This recursive method goes through all DOMElements and inserts an
     * onclick handler if the element contains an @optimsoc:idref attribute.
     *
     * @param el
     */
    void modifyDomElementForOnclick(QDomElement el);

    /**
     * Load the SVG depicting the system into the QWebView used for display
     *
     * The image is also modified to allow the interaction with the system to
     * happen.
     */
    void loadSvgIntoWebView();

};

#endif // SYSTEMOVERVIEWWIDGET_H
