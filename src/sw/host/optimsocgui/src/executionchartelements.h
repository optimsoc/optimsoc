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
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 */

#ifndef EXECUTIONCHARTELEMENTS_H
#define EXECUTIONCHARTELEMENTS_H

#include <QGraphicsItem>

class ExecutionChartElement : public QObject
{
    Q_OBJECT

public:
    ExecutionChartElement() {}
protected:
    double m_scale;
};

class ExecutionChartSection : public ExecutionChartElement
{
    Q_OBJECT
public:
    ExecutionChartSection(unsigned int from, unsigned int to,
                          int id, QString text);
    void getRange(unsigned int &from, unsigned int &to);
    void getColors(QColor &pen, QColor &brush);
    QString text() { return m_text; }
    void updateExtend(unsigned int to);
private:
    static const QPair<QColor,QColor> colorMap(int id);

    unsigned int m_from;
    unsigned int m_to;
    unsigned int m_id;
    QString m_text;

    QPair<QColor,QColor> m_colors;
};

Q_DECLARE_METATYPE(ExecutionChartSection*)

class ExecutionChartEvent : public ExecutionChartElement
{
    Q_OBJECT

public:
    ExecutionChartEvent(unsigned int timestamp, unsigned int width,
                        QString text, QColor color);
    unsigned int timestamp() { return m_timestamp; }
    unsigned int width() { return m_width; }
    QString text() { return m_text; }
    QColor color() { return m_color; }

private:
    QGraphicsRectItem *m_item;

    unsigned int m_timestamp;
    unsigned int m_width;

    QString m_text;
    QColor m_color;
};

Q_DECLARE_METATYPE(ExecutionChartEvent*)

#endif // EXECUTIONCHARTELEMENTS_H
