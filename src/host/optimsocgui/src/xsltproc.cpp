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

#include "xsltproc.h"

#include <libxml/xmlmemory.h>
#include <libxslt/xslt.h>
#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>

XsltProc::XsltProc()
{
}

XsltProc::XsltProc(const QString xmlFile, const QString xslFile)
    : m_xmlFileName(xmlFile), m_xslFileName(xslFile)
{
}

XsltProc::~XsltProc()
{
    // The QByteArray m_outputDocument does not take ownership of this data
    // so we need to manually free it (see XsltProc::transform())
    delete m_outputDocumentChar;
}

/**
 * Set the input XML file
 *
 * @param fileName
 */
void XsltProc::setXmlFile(const QString fileName)
{
    m_xmlFileName = fileName;
}

/**
 * Set the XSL file describing the transformation to apply
 *
 * @param fileName
 */
void XsltProc::setXslFile(const QString fileName)
{
    m_xslFileName = fileName;
}

/**
 * Transform the input document using the stylesheet to a output document
 *
 * @return transformation successful?
 */
bool XsltProc::transform()
{
    xsltStylesheetPtr stylesheet = NULL;
    xmlDocPtr inDoc, outDoc;

    // parse stylesheet
    stylesheet = xsltParseStylesheetFile(reinterpret_cast<const xmlChar *>(m_xslFileName.toUtf8().constData()));

    // parse XML document
    // XXX: Verify that UTF-8 encoded filenames is what this function expects.
    //      The API docs are rather silent on this topic.
    inDoc = xmlParseFile(m_xmlFileName.toUtf8().constData());

    // transform
    outDoc = xsltApplyStylesheet(stylesheet, inDoc, NULL);

    int outDocCharLen;
    xmlDocDumpMemoryEnc(outDoc,
                        reinterpret_cast<xmlChar**>(&m_outputDocumentChar),
                        &outDocCharLen, "UTF8");

    // This creates a new QByteArray referencing the output data memory as
    // allocated by libxml2. QByteArray does not take ownership of this data,
    // so we need to manually delete it in the destructor.
    // Note that QByteArray are implicitly shared, so if we pass
    // m_outputDocument by value to other methods, the data is not copied
    // unless it is modified. Also, m_outputDocumentChar is never modified
    // outside of this class if we only pass out m_outputDocument by value.
    m_outputDocument = QByteArray::fromRawData(m_outputDocumentChar,
                                               outDocCharLen);

    // free resources
    xsltFreeStylesheet(stylesheet);
    xmlFreeDoc(outDoc);
    xmlFreeDoc(inDoc);
    xsltCleanupGlobals();
    xmlCleanupParser();

    return true;
}

/**
 * Get the output document, i.e. the transformed document
 *
 * @return the UTF-8 encoded output data
 */
QByteArray XsltProc::outputDocument()
{
    return m_outputDocument;
}
