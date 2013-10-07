/*
 * This file is part of OpTiMSoC-GUI.
 *
 * OpTiMSoC-GUI is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * OpTiMSoC-GUI is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with OpTiMSoC. If not, see <http://www.gnu.org/licenses/>.
 *
 * =================================================================
 *
 * (c) 2013 by the author(s)
 *
 * Author(s):
 *    Stefan Wallentowitz, stefan.wallentowitz@tum.de
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
