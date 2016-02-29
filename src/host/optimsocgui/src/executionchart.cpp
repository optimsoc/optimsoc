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

#include "executionchart.h"
#include "ui_executionchart.h"

#include "optimsocsystem.h"
#include "optimsocsystemelement.h"
#include "optimsocsystemfactory.h"
#include "systeminterface.h"

#include <QGraphicsTextItem>
#include <QGraphicsRectItem>
#include <QGraphicsLineItem>
#include <QMessageBox>
#include <QPair>

#include <cmath>

ExecutionChart::ExecutionChart(QWidget *parent) :
    QWidget(parent),
    m_ui(new Ui::ExecutionChart), m_currentMaximum(0), m_autoscroll(true),
    m_zoomFactor(10.0), m_slideFactor(0.5), m_sysif(SystemInterface::instance())
{
    // Set up user interface
    m_ui->setupUi(this);

    m_ui->horizontalSpacer->changeSize(qApp->style()->pixelMetric(QStyle::PM_ScrollBarExtent) +
                                       qApp->style()->pixelMetric(QStyle::PM_ScrollView_ScrollBarSpacing), 0);

    // Clear scale plot
    m_ui->widget_plot_scale->plotLayout()->clear();

    // Create the axes for scale plot
    QCPAxisRect *scale_axis = new QCPAxisRect(m_ui->widget_plot_scale);
    scale_axis->axis(QCPAxis::atTop)->setVisible(true);
    scale_axis->removeAxis(scale_axis->axis(QCPAxis::atBottom));
    scale_axis->removeAxis(scale_axis->axis(QCPAxis::atRight));
    scale_axis->removeAxis(scale_axis->axis(QCPAxis::atLeft));

    // Add axis to scale plot
    m_ui->widget_plot_scale->plotLayout()->addElement(0, 0, scale_axis);

    // Clear main plot
    m_ui->widget_plot->plotLayout()->clear();

    // Set interactivity
    m_ui->widget_plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);

    // Zoom only along horizontal
    scale_axis->setRangeZoom(Qt::Horizontal);
    scale_axis->setRangeDrag(Qt::Horizontal);

    // Connect the range changes from the plot to the scale plot
    connect(this, SIGNAL(rangeChange(QCPRange)), scale_axis->axis(QCPAxis::atTop), SLOT(setRange(QCPRange)));

    // Create margin group
    QCPMarginGroup *margingrp = new QCPMarginGroup(m_ui->widget_plot);
    scale_axis->setMarginGroup(QCP::msLeft | QCP::msRight, margingrp);

    connect(&m_plotTimer, SIGNAL(timeout()), this, SLOT(replot()));
    m_plotTimer.start(20);

    connect(OptimsocSystemFactory::instance(),
            SIGNAL(currentSystemChanged(OptimsocSystem*, OptimsocSystem*)),
            this,
            SLOT(systemChanged(OptimsocSystem*, OptimsocSystem*)));
    connect(m_sysif->softwareTraceEventDistributor(),
            SIGNAL(softwareTraceEvent(struct SoftwareTraceEvent)),
            this,
            SLOT(addTraceEvent(struct SoftwareTraceEvent)));
}

ExecutionChart::~ExecutionChart()
{
    delete m_ui;
}

void ExecutionChart::systemChanged(OptimsocSystem* oldSystem,
                                   OptimsocSystem* newSystem)
{
    Q_UNUSED(oldSystem);

    QList<OptimsocSystemElement*> tiles = newSystem->tiles();
    unsigned int row = 0;
    for (int i=0; i<tiles.size(); ++i) {
        QCPAxisRect *coreaxis = new QCPAxisRect(m_ui->widget_plot);
        ExecutionChartPlotCore *plot = new ExecutionChartPlotCore(coreaxis->axis(QCPAxis::atBottom), coreaxis->axis(QCPAxis::atLeft), QString("Core %1").arg(i));
        m_ui->widget_plot->plotLayout()->addElement(row++, 0, coreaxis);
        coreaxis->setRangeZoom(Qt::Horizontal);
        coreaxis->setRangeDrag(Qt::Horizontal);

        connect(coreaxis->axis(QCPAxis::atBottom), SIGNAL(rangeChanged(QCPRange,QCPRange)), this, SLOT(rangeChanged(QCPRange,QCPRange)));
        connect(this, SIGNAL(rangeChange(QCPRange)), coreaxis->axis(QCPAxis::atBottom), SLOT(setRange(QCPRange)));

        coreaxis->setMarginGroup(QCP::msLeft | QCP::msRight, m_ui->widget_plot_scale->axisRect(0)->marginGroup(QCP::msLeft));
        coreaxis->setMinimumSize(10,50);
        coreaxis->setMaximumSize(100000,50);

        m_plotCores.resize(m_plotCores.size()+1);
        m_plotCores[i] = plot;

        QCPAxisRect *loadaxis = new QCPAxisRect(m_ui->widget_plot);
        ExecutionChartPlotLoad *plotload = new ExecutionChartPlotLoad(loadaxis->axis(QCPAxis::atBottom), loadaxis->axis(QCPAxis::atLeft));
        m_ui->widget_plot->plotLayout()->addElement(row++, 0, loadaxis);
        loadaxis->setRangeZoom(Qt::Horizontal);
        loadaxis->setRangeDrag(Qt::Horizontal);

        connect(loadaxis->axis(QCPAxis::atBottom), SIGNAL(rangeChanged(QCPRange,QCPRange)), this, SLOT(rangeChanged(QCPRange,QCPRange)));
        connect(this, SIGNAL(rangeChange(QCPRange)), loadaxis->axis(QCPAxis::atBottom), SLOT(setRange(QCPRange)));

        loadaxis->setMarginGroup(QCP::msLeft | QCP::msRight, m_ui->widget_plot_scale->axisRect(0)->marginGroup(QCP::msLeft));
        loadaxis->setMinimumSize(10,50);
        loadaxis->setMaximumSize(100000,50);

        m_plotLoads.resize(m_plotLoads.size()+1);
        m_plotLoads[i] = plotload;
    }
}

void ExecutionChart::replot()
{
    m_ui->widget_plot->replot();
    m_ui->widget_plot_scale->replot();
}

void ExecutionChart::rangeChanged(QCPRange oldrange, QCPRange newrange)
{
    if (newrange.lower < 0) {
        if (oldrange.upper - oldrange.lower == newrange.upper - newrange.lower) {
            // If we _move_ don't accidentally zoom
            newrange.lower = 0;
            newrange.upper = oldrange.upper - oldrange.lower;
        } else {
            // If we zoom, just spare out the negative part
            newrange.lower = 0;
        }
    }

    // If the maximum value is visible we autoscroll
    m_autoscroll = m_currentMaximum < newrange.upper;

    emit rangeChange(newrange);
}

void ExecutionChart::addTraceEvent(SoftwareTraceEvent event)
{
    unsigned int update_extend = 0;
    unsigned int tmp_extend;
    if (event.core_id < m_plotCores.size()) {
        tmp_extend = m_plotCores[event.core_id]->addSoftwareTrace(&event);
        if (tmp_extend > update_extend) {
            update_extend = tmp_extend;
        }
        tmp_extend = m_plotLoads[event.core_id]->addSoftwareTrace(&event);
        if (tmp_extend > update_extend) {
            update_extend = tmp_extend;
        }
    }

    if (update_extend > 0) {
        ExecutionChartPlotCore *coreplot;
        foreach(coreplot, m_plotCores) {
            coreplot->updateExtend(update_extend);
        }

        ExecutionChartPlotLoad *loadplot;
        foreach(loadplot, m_plotLoads) {
            loadplot->updateExtend(update_extend);
        }

        if (m_autoscroll) {
            QCPRange range = m_ui->widget_plot_scale->axisRect(0)->axis(QCPAxis::atTop)->range();
            // We only scroll if the new extend would not be visible
            if (update_extend > range.upper) {
                double move = update_extend - range.upper;
                range.lower = range.lower + move;
                range.upper = range.upper + move;
                emit rangeChange(range);
            }
        }

        m_currentMaximum = update_extend;
    }
}

void ExecutionChart::on_zoomInButton_clicked()
{
    QCPRange range = m_ui->widget_plot_scale->axisRect(0)->axis(QCPAxis::atTop)->range();
    double mid = (range.upper + range.lower) / 2;
    range.lower = mid - (mid - range.lower) / m_zoomFactor;
    range.upper = mid + (range.upper - mid) / m_zoomFactor;
    emit rangeChange(range);
}

void ExecutionChart::on_zoomOutButton_clicked()
{
    QCPRange range = m_ui->widget_plot_scale->axisRect(0)->axis(QCPAxis::atTop)->range();
    double mid = (range.upper + range.lower) / 2;
    range.lower = mid - (mid - range.lower) * m_zoomFactor;
    range.upper = mid + (range.upper - mid) * m_zoomFactor;
    emit rangeChange(range);
}

void ExecutionChart::on_zoomOriginalButton_clicked()
{
    QCPRange range;
    range.lower = 0;
    range.upper = m_currentMaximum;

    m_autoscroll = true;

    emit rangeChange(range);
}

void ExecutionChart::on_leftButton_clicked()
{
    QCPRange range = m_ui->widget_plot_scale->axisRect(0)->axis(QCPAxis::atTop)->range();
    double width = range.upper - range.lower;
    double move = width * m_slideFactor;
    range.lower = range.lower - move;
    range.upper = range.upper - move;

    emit rangeChange(range);
}

void ExecutionChart::on_rightButton_clicked()
{
    QCPRange range = m_ui->widget_plot_scale->axisRect(0)->axis(QCPAxis::atTop)->range();
    double width = range.upper - range.lower;
    double move = width * m_slideFactor;
    range.lower = range.lower + move;
    range.upper = range.upper + move;

    emit rangeChange(range);
}
