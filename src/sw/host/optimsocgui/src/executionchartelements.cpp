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

#include "executionchartelements.h"

#include <QGraphicsItem>
#include <QPair>
#include <QBrush>
#include <QPen>

ExecutionChartSection::ExecutionChartSection(unsigned int from, unsigned int to,
                                             int id, QString text)
    : ExecutionChartElement(), m_from(from), m_to(to), m_id(id), m_text(text)
{
    m_colors = colorMap(m_id);
}

void ExecutionChartSection::getRange(unsigned int &from, unsigned int &to)
{
    from = m_from;
    to = m_to;
}

void ExecutionChartSection::getColors(QColor &pen, QColor &brush)
{
    brush = m_colors.first;
    pen = m_colors.second;
}

void ExecutionChartSection::updateExtend(unsigned int to)
{
    m_to = to;
}

const QPair<QColor,QColor> ExecutionChartSection::colorMap(int id)
{
    static bool initialized = false;
    static QMap<unsigned int,QPair<QColor,QColor> > map;

    if (!initialized) {
        // From http://www.colorcombos.com/ (Combo 430)
        map[0] = QPair<QColor,QColor>(QColor(0xFFDD1E2F),Qt::black);
        map[1] = QPair<QColor,QColor>(QColor(0xFFEBB035),Qt::black);
        map[2] = QPair<QColor,QColor>(QColor(0xFF06A2CB),Qt::black);
        map[3] = QPair<QColor,QColor>(QColor(0xFF218559),Qt::black);
        map[4] = QPair<QColor,QColor>(QColor(0xFFD0C6B1),Qt::black);
        map[5] = QPair<QColor,QColor>(QColor(0xFF192823),Qt::black);
        // Combo 428
        map[6] = QPair<QColor,QColor>(QColor(0xFF74A6BD),Qt::black);
        map[7] = QPair<QColor,QColor>(QColor(0xFF7195A3),Qt::black);
        map[8] = QPair<QColor,QColor>(QColor(0xFFD4E7ED),Qt::black);
        map[9] = QPair<QColor,QColor>(QColor(0xFFEB8540),Qt::black);
        map[10] = QPair<QColor,QColor>(QColor(0xFFB06A3B),Qt::black);
        map[11] = QPair<QColor,QColor>(QColor(0xFFAB988B),Qt::black);
    }

    if (id == -1) {
        return QPair<QColor,QColor>(QColor("black"),QColor("black"));
    } else if (map.find(id)!=map.end()) {
        return map[id];
    } else {
        return QPair<QColor,QColor>(QColor("white"),QColor("black"));
    }
}

ExecutionChartEvent::ExecutionChartEvent(unsigned int timestamp, unsigned int width,
                                         QString text, QColor color)
    : ExecutionChartElement(), m_timestamp(timestamp), m_width(width), m_color(color)
{
    m_text = text+QString("\ntimestamp: %1 ns").arg(m_timestamp);
    m_text = m_text.replace("\\n","\n");
}
