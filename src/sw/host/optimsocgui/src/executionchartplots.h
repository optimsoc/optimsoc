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
