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

#include "systemoverviewjsapi.h"

#include <QDebug>
#include <QWebView>
#include <QWebFrame>

SystemOverviewJsApi::SystemOverviewJsApi(QObject *parent)
    : QObject(parent)
{

}

void SystemOverviewJsApi::setWebView(QWebView *view)
{
    QWebPage *page = view->page();
    m_frame = page->mainFrame();

    attachObject();
    connect(m_frame, SIGNAL(javaScriptWindowObjectCleared()),
            this, SLOT(attachObject()));
}

void SystemOverviewJsApi::attachObject()
{
    m_frame->addToJavaScriptWindowObject("optimsoc", this);
}

void SystemOverviewJsApi::handleIdClick(const QString &idref)
{
    emit itemClicked(idref);
}
