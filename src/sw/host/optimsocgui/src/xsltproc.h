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

#ifndef XSLTPROC_H
#define XSLTPROC_H

#include <QString>
#include <QByteArray>

/**
 * XSLT Processor
 *
 * This class encapsulates the libxslt XSLT processor for use inside a Qt
 * application. It tries to be as simple as possible, just doing
 * transformations, not much else.
 */
class XsltProc
{
public:
    XsltProc();
    XsltProc(const QString xmlFile, const QString xslFile);
    virtual ~XsltProc();

    void setXmlFile(const QString fileName);
    void setXslFile(const QString fileName);
    bool transform();
    QByteArray outputDocument();

private:
    QString m_xmlFileName;
    QString m_xslFileName;
    QByteArray m_outputDocument;
    char* m_outputDocumentChar;
};

#endif // XSLTPROC_H
