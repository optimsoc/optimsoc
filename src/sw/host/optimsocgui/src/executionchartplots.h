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

#ifndef EXECUTIONCHARTPLOTS_H
#define EXECUTIONCHARTPLOTS_H

#include <QObject>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsLineItem>
#include <QList>

#include "qcustomplot.h"

#include "traceevents.h"

// Forward declaration
class ExecutionChartElementCreator;

/**
  * Abstract prototype for plotting classes
  */
class ExecutionChartPlot : public QCPAbstractPlottable
{
    Q_OBJECT

public:
    ExecutionChartPlot(QCPAxis *keyaxis, QCPAxis *valueaxis);
    virtual unsigned int addSoftwareTrace(SoftwareTraceEvent *event) = 0;
    virtual void updateExtend(unsigned int) = 0;
protected:
};

class ExecutionChartPlotCore : public ExecutionChartPlot
{
    Q_OBJECT

public:
    ExecutionChartPlotCore(QCPAxis *keyaxis, QCPAxis *valueaxis, QString corename);
    void readEventsFromFile(QString filename);
    void readEventsFromPath(QString path);

    virtual double selectTest(const QPointF &pos, bool onlySelectable, QVariant *details) const;
    virtual void draw(QCPPainter *painter);
    virtual void clearData();
    virtual void drawLegendIcon(QCPPainter *painter, const QRectF &rect) const;
    virtual QCPRange getKeyRange(bool &validRange, SignDomain inSignDomain) const;
    virtual QCPRange getValueRange(bool &validRange, SignDomain inSignDomain) const;

    virtual unsigned int addSoftwareTrace(SoftwareTraceEvent *event);

    virtual void updateExtend(unsigned int extend);

    void coordsToPixels(double key, double value, double &x, double &y) const;
    void pixelsToCoords(const QPointF &pixelPos, double &key, double &value) const;

    void selectEvent(QMouseEvent *event, bool additive, const QVariant &details, bool *selectionStateChanged);
private:
    enum LayerPosition {
        LayerSections = 0,
        LayerEvents = 1
    };

    QString m_corename;
    QList<ExecutionChartElementCreator*> m_creators;
    QMultiMap<enum LayerPosition, ExecutionChartElementCreator*> m_creatorLayers;
    QMap<unsigned int,QVector<ExecutionChartElementCreator*> > m_traceCreators;

    unsigned int m_currentExtend;
    ExecutionChartElementCreator *m_currentSelection;
};

#endif // EXECUTIONCHARTPLOTS_H
