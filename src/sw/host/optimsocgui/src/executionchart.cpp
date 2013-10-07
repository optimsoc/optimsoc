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

#include "executionchart.h"
#include "ui_executionchart.h"

#include "optimsocsystem.h"
#include "optimsocsystemfactory.h"

#include <QGraphicsTextItem>
#include <QGraphicsRectItem>
#include <QGraphicsLineItem>
#include <QMessageBox>
#include <QPair>

#include <cmath>

ExecutionChart::ExecutionChart(QWidget *parent) :
    QWidget(parent),
    m_ui(new Ui::ExecutionChart), m_currentMaximum(0), m_autoscroll(true), m_zoomFactor(10.0), m_slideFactor(0.5)
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
}

ExecutionChart::~ExecutionChart()
{
    delete m_ui;
}

void ExecutionChart::systemDiscovered(int id)
{
    OptimsocSystem *system = OptimsocSystemFactory::createSystemFromId(id);
    if (!system) {
        qWarning("No system description is available; unable to create execution chart.");
        return;
    }


    QList<Tile*> tiles = system->tiles();
    unsigned int row = 0;
    for (int i=0; i<tiles.size(); ++i) {
        QCPAxisRect *coreaxis = new QCPAxisRect(m_ui->widget_plot);
        ExecutionChartPlotCore *plot = new ExecutionChartPlotCore(coreaxis->axis(QCPAxis::atBottom), coreaxis->axis(QCPAxis::atLeft), "core");
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
    }

    if (update_extend > 0) {
        ExecutionChartPlotCore *coreplot;
        foreach(coreplot, m_plotCores) {
            coreplot->updateExtend(update_extend);
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
