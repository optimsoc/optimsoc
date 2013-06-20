/**
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
 * Driver for the simple message passing hardware.
 *
 * (c) 2013 by the author(s)
 *
 * Author(s):
 *    Stefan Wallentowitz, stefan.wallentowitz@tum.de
 */

#ifndef EXECUTIONCHARTELEMENTS_H
#define EXECUTIONCHARTELEMENTS_H

#include <QGraphicsItem>

class ExecutionChartElement : public QObject
{
    Q_OBJECT

public:
    ExecutionChartElement(QObject *parent, double scale)
        : QObject(parent), m_scale(scale) {}
    virtual QGraphicsItem *getItem() = 0;
    virtual void rescale(double newscale, double oldscale) = 0;

protected:
    double m_scale;
};

class ExecutionChartSection : public ExecutionChartElement
{
    Q_OBJECT
public:
    ExecutionChartSection(QObject *parent, double scale,
                          unsigned int baseline, unsigned int height,
                          unsigned int from, unsigned int to,
                          int id, QString text);
    virtual QGraphicsItem *getItem();
    virtual void rescale(double newscale, double oldscale);
    virtual void expand(int maximum);

private:
    static const QPair<QColor,QColor> colorMap(int id);

    unsigned int m_baseline;
    unsigned int m_height;

    unsigned int m_from;
    unsigned int m_to;
    unsigned int m_id;
    QString m_text;

    QGraphicsRectItem *m_item;
};

class ExecutionChartEvent : public ExecutionChartElement
{
    Q_OBJECT

public:
    ExecutionChartEvent(QObject *parent, double scale,
                        unsigned int baseline, unsigned int height,
                        unsigned int timestamp, unsigned int width,
                        QString text, QColor color);
    virtual QGraphicsItem *getItem();
    virtual void rescale(double newscale, double oldscale);

private:
    QGraphicsRectItem *m_item;

    unsigned int m_baseline;
    unsigned int m_height;
    unsigned int m_timestamp;
    unsigned int m_width;

    QString m_text;
    QColor m_color;
};

#endif // EXECUTIONCHARTELEMENTS_H
