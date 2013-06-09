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

#ifndef EXECUTIONCHARTPLOTS_H
#define EXECUTIONCHARTPLOTS_H

#include <QObject>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsLineItem>
#include <QList>

// Forward declaration
class ExecutionChartElementCreator;

/**
  * Abstract prototype for plotting classes
  */
class ExecutionChartPlot : public QObject
{
    Q_OBJECT

public:
    ExecutionChartPlot(QObject *parent, QGraphicsView *gv,QGraphicsScene *scene,unsigned int baseline,unsigned int height);
    virtual void addSoftwareTrace(unsigned int timestamp, unsigned int id, unsigned int value) = 0;
protected:
    int m_baseline;
    int m_height;
    QGraphicsView *m_graphicsView;
    QGraphicsScene *m_scene;
};

class ExecutionChartPlotScale : public ExecutionChartPlot
{
    Q_OBJECT

public:
    ExecutionChartPlotScale(QObject *parent, QGraphicsView *gv,
                            QGraphicsScene *scene, unsigned int baseline,
                            unsigned int height);
    void addSoftwareTrace(unsigned int timestamp, unsigned int id, unsigned int value);

public slots:
    void rescale(double newscale,double oldscale);
    void expand(int maximum);
private:
    QGraphicsLineItem *m_line;
    int m_currentMaximum;
    double m_currentScale;
};

class ExecutionChartPlotCore : public ExecutionChartPlot
{
    Q_OBJECT

public:
    ExecutionChartPlotCore(QObject *parent, QGraphicsView *gv,
                            QGraphicsScene *scene, unsigned int baseline, unsigned int height);
    void addSoftwareTrace(unsigned int timestamp, unsigned int id, unsigned int value);
    void readEventsFromFile(QString filename);
    void readEventsFromPath(QString path);
public slots:
    void rescale(double newscale,double oldscale);
    void expand(int maximum);
private:
    QList<ExecutionChartElementCreator*> m_creators;
    QMap<unsigned int,QVector<ExecutionChartElementCreator*> > m_traceCreators;
};

#endif // EXECUTIONCHARTPLOTS_H
