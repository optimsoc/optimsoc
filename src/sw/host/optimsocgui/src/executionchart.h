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

#ifndef EXECUTIONCHART_H
#define EXECUTIONCHART_H

#include <QWidget>
#include <QTimer>
#include <QMap>

#include "executionchartplots.h"
#include "traceevents.h"

class SystemInterface;
class OptimsocSystem;

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
    void rangeChanged(QCPRange oldrange, QCPRange newrange);
    void replot();

private slots:
    void on_zoomInButton_clicked();
    void on_zoomOutButton_clicked();
    void on_zoomOriginalButton_clicked();
    void on_leftButton_clicked();
    void on_rightButton_clicked();

    void systemChanged(OptimsocSystem* oldSystem, OptimsocSystem* newSystem);
    void addTraceEvent(struct SoftwareTraceEvent);

private:
    Ui::ExecutionChart *m_ui;
    QVector<ExecutionChartPlotCore*> m_plotCores;
    unsigned int m_currentMaximum;
    bool m_autoscroll;
    double m_zoomFactor;
    double m_slideFactor;
    SystemInterface *m_sysif;
    QTimer m_plotTimer;

signals:
    void rangeChange(QCPRange newrange);

};

#endif // EXECUTIONCHART_H
