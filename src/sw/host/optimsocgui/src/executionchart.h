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
#include <QTimer>
#include <QMap>

#include "executionchartplots.h"

#include "traceevents.h"

namespace Ui {
class ExecutionChart;
}

class ExecutionChart : public QWidget
{
    Q_OBJECT

public:
    explicit ExecutionChart(QWidget *parent = 0);
    ~ExecutionChart();

public slots:
    void addTraceEvent(struct SoftwareTraceEvent);
    void systemDiscovered(int id);
    void rangeChanged(QCPRange oldrange, QCPRange newrange);
    void replot();

private slots:
    void on_zoomInButton_clicked();
    void on_zoomOutButton_clicked();
    void on_zoomOriginalButton_clicked();

    void on_leftButton_clicked();

    void on_rightButton_clicked();

private:
    Ui::ExecutionChart *m_ui;

    QVector<ExecutionChartPlotCore*> m_plotCores;
    unsigned int m_currentMaximum;
    bool m_autoscroll;

    double m_zoomFactor;
    double m_slideFactor;

    QTimer m_plotTimer;

signals:
    void rangeChange(QCPRange newrange);

};

#endif // EXECUTIONCHART_H
