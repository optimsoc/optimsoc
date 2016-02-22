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

#ifndef MEMORY_H
#define MEMORY_H

#include "optimsocsystemelement.h"

class QDomNode;
class QMenu;
class OptimsocSystem;

class Memory : public OptimsocSystemElement
{
    Q_OBJECT
public:
    explicit Memory(const QDomNode &memoryNode, OptimsocSystem *system,
                    OptimsocSystemElement *parent);
    virtual ~Memory();
    unsigned int size();
    void write(QByteArray data, unsigned int baseAddress = 0);
    QMenu* contextMenu();
    void showWriteMemoryDialog();
    QString name();
    QWidget* propertiesWidget();

signals:
    void writeFinished(bool success);

private:
    unsigned int m_byteSize;
    int m_memoryId;
    QMenu* m_contextMenu;
    QWidget* m_propertiesWidget;

private slots:
    void emitMemoryWriteFinished(unsigned int memoryId, bool success);

};

#endif // MEMORY_H
