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

#ifndef SYSTEMVIEW_H
#define SYSTEMVIEW_H

#include <QWidget>

class SystemInterface;
class OptimsocSystem;
class OptimsocSystemModel;
class QModelIndex;

namespace Ui {
class SystemView;
}

class SystemView : public QWidget
{
    Q_OBJECT

public:
    explicit SystemView(QWidget *parent = 0);
    ~SystemView();

public slots:
    void systemDiscovered(int systemId);
    void treeViewSelectionChanged(const QModelIndex &selected,
                                  const QModelIndex &deselected);
    void selectElementInHierarchicalView(QString id);

private:
    Ui::SystemView *m_ui;
    SystemInterface *m_sysif;
    OptimsocSystem *m_system;
    OptimsocSystemModel *m_sysmodel;
};

#endif // SYSTEMVIEW_H
