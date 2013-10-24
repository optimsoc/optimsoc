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

#ifndef COMPUTETILE_H
#define COMPUTETILE_H

#include "tile.h"

class QDomNode;
class QWidget;

class ComputeTile : public Tile
{
    Q_OBJECT
public:
    explicit ComputeTile(const QDomNode &tileNode, OptimsocSystem *system,
                         OptimsocSystemElement *parent = 0);
    ~ComputeTile();
    QString tileTypeName() const { return "compute tile"; }
    int computeTileId();
    QString type();
    QString name();
    QWidget* propertiesWidget();

private:
    int m_computeTileId;
    QString m_type;
    QWidget *m_propertiesWidget;

};

#endif // COMPUTETILE_H
