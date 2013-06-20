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

#include "executionchartplots.h"

#include "executionchart.h"
#include "executionchartcreators.h"

#include <QMessageBox>
#include <cmath>

#include <QPair>
#include <QList>
#include <QGraphicsItem>
#include <QGraphicsSimpleTextItem>

#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QProcessEnvironment>

ExecutionChartPlot::ExecutionChartPlot(QObject *parent, QGraphicsView *gv,
                                       QGraphicsScene *scene, unsigned int baseline, unsigned int height)
    : QObject(parent), m_graphicsView(gv), m_scene(scene), m_baseline(baseline), m_height(height)
{

}

ExecutionChartPlotScale::ExecutionChartPlotScale(QObject *parent,
                                                 QGraphicsView *gv,
                                                 QGraphicsScene *scene, unsigned int baseline, unsigned int height)
    : ExecutionChartPlot(parent,gv,scene,baseline,height)
{
    m_currentMaximum = 0;
    m_line = new QGraphicsLineItem();
    m_scene->addItem(m_line);
}

void ExecutionChartPlotScale::rescale(double newscale, double oldscale)
{
    m_currentScale = newscale;
    QList<QGraphicsItem*> children = m_line->childItems();
    for (QList<QGraphicsItem*>::const_iterator it=children.begin();it!=children.end();++it) {
        delete(*it);
    }

    int max = m_currentMaximum;
    m_currentMaximum = 0;
    expand(max);
}

void ExecutionChartPlotScale::expand(int maximum)
{
    // The basic line is just modified to match the maximum size
    m_line->setLine(0,m_baseline+2,maximum,m_baseline+2);

    // Get the 10s logarithm of this scale
    double scalelog10 = log10(m_currentScale);

    int lowerscalelog10 = floor(scalelog10);
    int upperscalelog10 = ceil(scalelog10);

    double stepsminor, stepsmajor;

    // Zoom level is a power of ten
    if (fabs(scalelog10-lowerscalelog10)<0.01) {
        stepsminor = 25*m_currentScale;
        stepsmajor = 100*m_currentScale;
    } else {
        double difftolog10 = upperscalelog10-scalelog10;
        if (difftolog10>0.8) {
            stepsminor = 2.5*exp10(upperscalelog10);
            stepsmajor = stepsminor*4;
        } else if (difftolog10>0.6) {
            stepsminor = 5*exp10(upperscalelog10);
            stepsmajor = stepsminor*4;
        } else if (difftolog10>0.2) {
            stepsminor = 12.5*exp10(upperscalelog10);
            stepsmajor = stepsminor*4;
        } else if (difftolog10>0.04) {
            stepsminor = 25*exp10(upperscalelog10);
            stepsmajor = stepsminor*4;
        } else {
            stepsminor = 50*exp10(upperscalelog10);
            stepsmajor = stepsminor*4;
        }
    }

    double i;

    // Generate ticks
    i = ceil(m_currentMaximum/stepsminor)*stepsminor;
    while (i <= maximum) {
        qreal x0, x1, y0, y1;
        double mod = fmod(i,stepsmajor);
        bool modnearzero = (mod<0.01);
        bool modnearstep = ((stepsmajor-mod)<0.01);
        bool ismajor = (modnearzero || modnearstep);
        x0 = x1 = i;
        y0 = 2;
        if (ismajor) {
            y1 = 12;
        } else {
            y1 = 6;
        }
        new QGraphicsLineItem(x0,y0,x1,y1,m_line);

        if (ismajor) {
            QPair<double,QString> scaled = ExecutionChart::mapScaleToString(i);
            QString scaledString = QString("%1 %2").arg(scaled.first).arg(scaled.second);
            QGraphicsSimpleTextItem *text = new QGraphicsSimpleTextItem(scaledString,m_line);
            QFont font = text->font();
            font.setPixelSize(10);
            text->setFont(font);

            QTransform textTransform;
            text->setPos(i+2*m_currentScale,9);
            textTransform.scale(m_currentScale,1);
            text->setTransformOriginPoint(i,10);
            text->setTransform(textTransform);
        }

        i=i+stepsminor;
    }
    m_currentMaximum = maximum;
}

void ExecutionChartPlotScale::addSoftwareTrace(unsigned int timestamp,
                                               unsigned int id,
                                               unsigned int value)
{

}

ExecutionChartPlotCore::ExecutionChartPlotCore(QObject *parent,
                                               QGraphicsView *gv,
                                               QGraphicsScene *scene, unsigned int baseline, unsigned int height)
    : ExecutionChartPlot(parent,gv,scene,baseline,height)
{
    ExecutionChartSectionCreator *section =
            new ExecutionChartSectionCreator(this, scene, m_baseline+2, m_height-4);
    m_traceCreators[0x1].push_back(section);
    m_traceCreators[0x20].push_back(section);
    m_traceCreators[0x21].push_back(section);
    m_traceCreators[0x22].push_back(section);
    m_traceCreators[0x23].push_back(section);
    m_creators.append(section);

    ExecutionChartEventCreator *decevent =
            new ExecutionChartEventCreator(this, scene, m_baseline, m_height, 3, "dec: %1");
    decevent->appendDec(true);
    m_traceCreators[0x30].push_back(decevent);
    m_creators.append(decevent);

    ExecutionChartEventCreator *decsignedevent =
            new ExecutionChartEventCreator(this, scene, m_baseline, m_height, 3, "signed: %1");
    decsignedevent->appendDecSigned(true);
    m_traceCreators[0x31].push_back(decsignedevent);
    m_creators.append(decsignedevent);

    ExecutionChartEventCreator *hexevent =
            new ExecutionChartEventCreator(this, scene, m_baseline, m_height, 3, "hex: %1");
    hexevent->appendHex(true);
    m_traceCreators[0x32].push_back(hexevent);
    m_creators.append(hexevent);

    ExecutionChartEventCreator *floatevent =
            new ExecutionChartEventCreator(this, scene, m_baseline, m_height, 3, "float: %1");
    floatevent->appendFloat(true);
    m_traceCreators[0x33].push_back(floatevent);
    m_creators.append(floatevent);

    ExecutionChartEventCreator *charevent =
            new ExecutionChartEventCreator(this, scene, m_baseline, m_height, 3, "char: %1");
    charevent->appendChar(true);
    m_traceCreators[0x34].push_back(charevent);
    m_creators.append(charevent);

    ExecutionChartEventCreator *stringevent =
            new ExecutionChartEventCreator(this, scene, m_baseline, m_height, 3, "string: %1");
    stringevent->appendString(true);
    m_traceCreators[0x35].push_back(stringevent);
    m_creators.append(stringevent);

    QString combined = "dec: %1\nsigned: %2\nhex: %3\nfloat: %4\nchar: %5\nstring: %6";
    ExecutionChartEventCreator *combinedevent =
            new ExecutionChartEventCreator(this, scene, m_baseline, m_height, 3, combined);
    combinedevent->appendDec(true);
    combinedevent->appendDecSigned(false);
    combinedevent->appendHex(false);
    combinedevent->appendFloat(false);
    combinedevent->appendChar(false);
    combinedevent->appendString(false);
    m_traceCreators[0x36].push_back(combinedevent);
    m_creators.append(combinedevent);

    QString path = QProcessEnvironment::systemEnvironment().value("OPTIMSOC");
    path = path + "/src/sw/host/optimsocgui/events.d";

    readEventsFromPath(path);
}

void ExecutionChartPlotCore::readEventsFromPath(QString path)
{
    QDir dir(path);
    QStringList flist = dir.entryList();

    for (QStringList::iterator it = flist.begin(); it != flist.end(); ++it ) {
        if ((*it != ".") && (*it != "..") && (it->right(1) != "~" ))
            readEventsFromFile(path+"/"+*it);
    }
}

void ExecutionChartPlotCore::readEventsFromFile(QString filename)
{
    QFile file(filename);

    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }

    QTextStream stream(&file);
    while (!stream.atEnd()) {
        QString line = stream.readLine();

        if (line[0] == '#')
            continue;

        int i = line.indexOf(":");
        if (i<0)
            continue;

        QString id = line.left(i);
        line = line.mid(i+1);

        i = line.indexOf(":");
        if (i<0)
            continue;

        QString format = line.left(i);
        line = line.mid(i+1);

        i = line.indexOf(":");
        if (i<0)
            continue;

        QString swidth = line.left(i);
        line = line.mid(i+1);

        i = line.indexOf(":");
        if (i<0)
            continue;

        QString scolor = line.left(i);
        QString text = line.mid(i+1);

        // Convert index to uint
        unsigned int index;
        i = id.indexOf("x");
        bool ok;
        if (i>=0) {
            index = id.mid(i+1).toUInt(&ok,16);
        } else {
            index = id.toUInt(&ok);
        }
        if (!ok)
            continue;

        // Convert width to uint
        unsigned int width = swidth.toUInt(&ok);
        if (!ok)
            width = 3;

        // Convert color
        unsigned int color = 0xff000000 | scolor.toUInt(&ok);
        if (!ok)
            color = 0xff000000;

        ExecutionChartEventCreator *event =
                new ExecutionChartEventCreator(this, m_scene, m_baseline, m_height, width, text, color);

        if (format.length() == 0) {
            event->appendEmpty();
        }

        for (i = 0; i < format.length(); ++i) {
            if (format[i] == 'u') {
                event->appendDec(i==0);
            } else if (format[i] == 'd') {
                event->appendDecSigned(i==0);
            } else if (format[i] == 'x') {
                event->appendHex(i==0);
            } else if (format[i] == 'f') {
                event->appendFloat(i==0);
            } else if (format[i] == 'c') {
                event->appendChar(i==0);
            } else if (format[i] == 's') {
                event->appendString(i==0);
            }
        }

        m_traceCreators[index].push_back(event);
        m_creators.append(event);

    }

}

void ExecutionChartPlotCore::addSoftwareTrace(unsigned int timestamp,
                                              unsigned int id,
                                              unsigned int value)
{
    if (m_traceCreators.find(id) != m_traceCreators.end()) {
        QVector<ExecutionChartElementCreator*>::const_iterator it;
        for (it=m_traceCreators[id].begin();it!=m_traceCreators[id].end();++it) {
            (*it)->addTrace(timestamp, id, value);
        }
    }
}

void ExecutionChartPlotCore::rescale(double newscale, double oldscale)
{
    ExecutionChartElementCreator *e;
    foreach(e,m_creators) {
        e->rescale(newscale,oldscale);
    }
}

void ExecutionChartPlotCore::expand(int maximum)
{
    ExecutionChartElementCreator *e;
    foreach(e,m_creators) {
        e->expand(maximum);
    }
}
