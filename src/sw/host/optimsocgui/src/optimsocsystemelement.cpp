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

#include "optimsocsystemelement.h"

#include <QMenu>

#include "optimsocsystem.h"

OptimsocSystemElement::OptimsocSystemElement(OptimsocSystemElement *parent) :
    QObject(0), m_system(NULL), m_parent(parent)
{
    if (parent) {
        parent->m_children.append(this);
    }
}

OptimsocSystemElement::OptimsocSystemElement(OptimsocSystem *system,
                                             OptimsocSystemElement *parent) :
    m_system(system), m_parent(parent)
{
    if (parent) {
        parent->m_children.append(this);
    }
}

OptimsocSystemElement::~OptimsocSystemElement()
{
    qDeleteAll(m_children);
}

/**
 * The context menu for this element
 *
 * The default implementation shows no context menu by returning NULL.
 *
 * @return The context menu or NULL if none is available
 */
QMenu* OptimsocSystemElement::contextMenu()
{
    return NULL;
}

/**
 * Set the ID of this element
 *
 * @see id()
 * @param id
 */
void OptimsocSystemElement::setId(const QString id)
{
    optimsocSystem()->registerId(id, this);
    m_id = id;
}

/**
 * Get the ID of this element
 *
 * @see setId()
 * @return
 */
QString OptimsocSystemElement::id()
{
    return m_id;
}

/**
 * Get the system this element is part of
 *
 * @return
 */
OptimsocSystem* OptimsocSystemElement::optimsocSystem()
{
    return m_system;
}

QList<OptimsocSystemElement*> OptimsocSystemElement::children()
{
    return m_children;
}

OptimsocSystemElement* OptimsocSystemElement::parent()
{
    return m_parent;
}

QString OptimsocSystemElement::name()
{
    return QString();
}

QWidget* OptimsocSystemElement::propertiesWidget()
{
    return NULL;
}
