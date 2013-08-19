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

#ifndef EXECUTIONCHART_H
#define EXECUTIONCHART_H

#include <QWidget>
#include <QGraphicsScene>
#include <QMap>

#include "executionchartplots.h"

namespace Ui {
class ExecutionChart;
}

class ExecutionChart : public QWidget
{
    Q_OBJECT

public:
    explicit ExecutionChart(QWidget *parent = 0);
    ~ExecutionChart();

    static const QPair<double,QString> mapScaleToString(double x);

public slots:
    void addTraceEvent(unsigned int core_id, unsigned int timestamp,
                       unsigned int id, unsigned int value);
    void systemDiscovered(int id);

private slots:
    void on_zoomInButton_clicked();
    void on_zoomOutButton_clicked();
    void updateZoomLabel(double newscale,double oldscale);
    void onRescale(double newscale,double oldscale);
    void onExpand(int maximum);

    void on_zoomOriginalButton_clicked();

private:
    void resizeEvent(QResizeEvent *event);

    Ui::ExecutionChart *m_ui;
    QGraphicsScene m_scene;
    QGraphicsScene m_sceneLabels;

    ExecutionChartPlotScale *m_plotScale;
    QVector<ExecutionChartPlotCore*> m_plotCores;

    double m_currentScale;
    int m_currentMaximum;

signals:
    void rescale(double newscale,double oldscale);
    void expand(int maximum);
};

#endif // EXECUTIONCHART_H
