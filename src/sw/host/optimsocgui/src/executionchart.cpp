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
    m_ui(new Ui::ExecutionChart)
{
    // Set up user interface
    m_ui->setupUi(this);

    // Associate our scene with graphics view
    m_ui->graphicsView->setScene(&m_scene);

    // Create basic plots: scale
    m_plotScale = new ExecutionChartPlotScale(this,m_ui->graphicsView,&m_scene,0,15);
    connect(this,SIGNAL(expand(int)),m_plotScale,SLOT(expand(int)));
    connect(this,SIGNAL(expand(int)),this,SLOT(onExpand(int)));

    m_ui->graphicsViewLabels->setScene(&m_sceneLabels);

    // Connect signals to slots
    connect(this,SIGNAL(rescale(double,double)),
            this,SLOT(updateZoomLabel(double,double)));
    connect(this,SIGNAL(rescale(double,double)),
            this,SLOT(onRescale(double,double)));
    connect(this,SIGNAL(rescale(double,double)),
            m_plotScale,SLOT(rescale(double,double)));

    rescale(100,1);
    unsigned int initial = 10000;
    expand(initial);
    m_currentMaximum = initial;
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

    m_sceneLabels.clear();

    QGraphicsTextItem *timelabel = new QGraphicsTextItem("Time");
    m_sceneLabels.addItem(timelabel);
    timelabel->setPos(0,0);

    QList<Tile*> tiles = system->tiles();
    for (int i=0; i<tiles.size(); ++i) {
        QGraphicsTextItem *corelabel = new QGraphicsTextItem(QString("Core %1").arg(i));
        m_sceneLabels.addItem(corelabel);
        corelabel->setPos(0,(i+1)*50);

        ExecutionChartPlotCore *plot;
        plot = new ExecutionChartPlotCore(this, m_ui->graphicsView, &m_scene,
                                          50*(i+1), 20);
        m_plotCores.push_back(plot);
        connect(this, SIGNAL(rescale(double, double)), plot,
                SLOT(rescale(double, double)));
        connect(this,SIGNAL(expand(int)), plot, SLOT(expand(int)));
    }

    rescale(m_currentScale,m_currentScale);
}

const QPair<double,QString> ExecutionChart::mapScaleToString(double x)
{
    static bool initialized = false;
    static QMap<int,QString> expmap;
    if (!initialized) {
        expmap[1] = "ns";
        expmap[2] = "us";
        expmap[3] = "ms";
        expmap[4] = "s";
    }

    int exp = 1;

    while ((x/1000>=1) && (exp<expmap.size())) {
        x /=1000;
        exp++;
    }

    return QPair<double,QString>(x,expmap[exp]);
}

void ExecutionChart::addTraceEvent(unsigned int core_id, unsigned int timestamp,
                                   unsigned int id, unsigned int value)
{
    if (core_id < m_plotCores.size()) {
        m_plotCores[core_id]->addSoftwareTrace(timestamp,id,value);
    }

//    m_currentMaximum = m_scene.width(); That hangs, no idea why
    m_currentMaximum = timestamp;
    expand(m_currentMaximum);
}

void ExecutionChart::on_zoomInButton_clicked()
{
    // Always round to the next power of 10
    int thispower = ceil(log10(m_currentScale));
    if (thispower-log10(m_currentScale)>0.99) {
        thispower = thispower - 1;
    }
    rescale(exp10(thispower-1),m_currentScale);
}

void ExecutionChart::on_zoomOutButton_clicked()
{
    // Always round to the next power of 10
    int thispower = floor(log10(m_currentScale));
    if (log10(m_currentScale)-thispower>0.99) {
        thispower = thispower + 1;
    }
    rescale(exp10(thispower+1),m_currentScale);
}

void ExecutionChart::on_zoomOriginalButton_clicked()
{
    // The 10 is somewhat arbitrary for the moment
    double width = m_ui->graphicsView->width()-10;
    rescale(m_currentMaximum/width,m_currentScale);
}


void ExecutionChart::onRescale(double newscale, double oldscale)
{
    m_currentScale = newscale;
    m_ui->graphicsView->scale(oldscale/newscale,1);
}

void ExecutionChart::onExpand(int maximum)
{
    // Set the displayed rectangle
    m_ui->graphicsView->setSceneRect(0,0,maximum,50);

    // Always autoscroll
    m_ui->graphicsView->centerOn(maximum,0);
}

void ExecutionChart::updateZoomLabel(double newscale,double oldscale)
{
    QPair<double,QString> display = mapScaleToString(newscale);
    m_ui->zoomLabel->setText(QString("%1 %2/px").arg(display.first).arg(display.second));
}

void ExecutionChart::resizeEvent(QResizeEvent *event)
{

}
