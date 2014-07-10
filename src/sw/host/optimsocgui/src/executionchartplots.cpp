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

#include <QSettings>

ExecutionChartPlot::ExecutionChartPlot(QCPAxis *keyaxis, QCPAxis *valueaxis)
    : QCPAbstractPlottable(keyaxis, valueaxis)
{

}

ExecutionChartPlotCore::ExecutionChartPlotCore(QCPAxis *keyaxis, QCPAxis *valueaxis, QString corename)
    : ExecutionChartPlot(keyaxis, valueaxis), m_currentExtend(0)
{
    ExecutionChartSectionCreator *section =
            new ExecutionChartSectionCreator(this);
    m_traceCreators[0x1].push_back(section);
    m_traceCreators[0x10].push_back(section);
    m_traceCreators[0x20].push_back(section);
    m_traceCreators[0x21].push_back(section);
    m_traceCreators[0x22].push_back(section);
    m_traceCreators[0x23].push_back(section);
    m_traceCreators[0x24].push_back(section);
    m_traceCreators[0x25].push_back(section);
    m_creators.append(section);
    m_creatorLayers.insert(LayerSections, section);

    QString path = QProcessEnvironment::systemEnvironment().value("OPTIMSOC");
    path = path + "/src/sw/host/optimsocgui/events.d";

    readEventsFromPath(path);

    QVector<double> ticks; QVector<QString> ticklabels;
    ticks.append(0.5); ticklabels.append("Exec");
    valueaxis->setAutoTicks(false);
    valueaxis->setAutoTickLabels(false);
    valueaxis->setTickVector(ticks);
    valueaxis->setRange(0.0,1.0);
    valueaxis->setTickVectorLabels(ticklabels);
    valueaxis->setLabel(corename);

    keyaxis->setVisible(false);

}

void ExecutionChartPlotCore::coordsToPixels(double key, double value, double &x, double &y) const
{
    QCPAbstractPlottable::coordsToPixels(key, value, x, y);
}

void ExecutionChartPlotCore::pixelsToCoords(const QPointF &pixelPos, double &key, double &value) const
{
    QCPAbstractPlottable::pixelsToCoords(pixelPos, key, value);
}

void ExecutionChartPlotCore::readEventsFromPath(QString path)
{
    QDir dir(path);
    QStringList flist = dir.entryList();

    for (QStringList::iterator it = flist.begin(); it != flist.end(); ++it ) {
        if ((*it != ".") && (*it != "..") && (it->right(4) == ".ini" ))
            readEventsFromFile(path+"/"+*it);
    }
}

void ExecutionChartPlotCore::readEventsFromFile(QString filename)
{
    QSettings settings(filename, QSettings::IniFormat);

    QStringList ids = settings.childGroups();
    QString id;

    foreach (id, ids) {
        settings.beginGroup(id);
        bool ok;

        unsigned int event_id = id.toUInt(&ok, 0); // C convention
        if (!ok) {
            settings.endGroup();
            continue;
        }

        QVariant width = settings.value("width", 3);
        unsigned int event_width = width.toString().toUInt(&ok, 0);
        if (!ok) {
            event_width = 3;
        }

        QVariant color = settings.value("color", 0);
        unsigned int event_color = color.toString().toUInt(&ok, 0);
        if (!ok) {
            event_color = 0;
        }

        QVariant text = settings.value("text", "");
        QString event_text = text.toString();

        QString format = settings.value("format", "").toString();

        ExecutionChartEventCreator *event =
                new ExecutionChartEventCreator(this, event_width, event_text, event_color);

        if (format.length() == 0) {
            event->appendEmpty();
        }

        for (int i = 0; i < format.length(); ++i) {
            if (format[i] == 'u') {
                event->appendDec(i == 0);
            } else if (format[i] == 'd') {
                event->appendDecSigned(i == 0);
            } else if (format[i] == 'x') {
                event->appendHex(i == 0);
            } else if (format[i] == 'f') {
                event->appendFloat(i == 0);
            } else if (format[i] == 'c') {
                event->appendChar(i == 0);
            } else if (format[i] == 's') {
                event->appendString(i == 0);
            }
        }

        m_traceCreators[event_id].push_back(event);
        m_creators.append(event);
        m_creatorLayers.insert(LayerEvents, event);

        settings.endGroup();
    }

}

unsigned int ExecutionChartPlotCore::addSoftwareTrace(SoftwareTraceEvent *event)
{
    unsigned int update_extend = 0;
    unsigned int tmp_extend;
    uint16_t id = event->id;
    if (m_traceCreators.find(id) != m_traceCreators.end()) {
        QVector<ExecutionChartElementCreator*>::const_iterator it;
        for (it = m_traceCreators[id].begin(); it != m_traceCreators[id].end(); ++it) {
            tmp_extend = (*it)->addTrace(event);
            if (tmp_extend > update_extend) {
                update_extend = tmp_extend;
            }
        }
    }
    return update_extend;
}

double ExecutionChartPlotCore::selectTest(const QPointF &pos, bool onlySelectable, QVariant *details) const
{
    double select = -1.0;
    double tmp;
    QMap<QString, QVariant> map;
    details->setValue(map);
    enum LayerPosition position;
    ExecutionChartElementCreator *creator;

    QList<enum LayerPosition> layers = m_creatorLayers.keys();

    foreach(position, layers) {
        QList<ExecutionChartElementCreator*> creators = m_creatorLayers.values(position);
        foreach(creator, creators) {
            QVariant subdetails;
            tmp = creator->selectTest(pos, onlySelectable, &subdetails);
            if (tmp >= select) {
                map["subdetails"] = subdetails;
                map["creator"] = QVariant::fromValue<ExecutionChartElementCreator*>(creator);
                select = tmp;
            }
        }
    }

    if (select > -1.0) {
        details->setValue(map);
    }

    return select;
}

void ExecutionChartPlotCore::draw(QCPPainter *painter)
{
    enum LayerPosition position;
    ExecutionChartElementCreator *creator;

    QList<enum LayerPosition> layers = m_creatorLayers.keys();

    foreach(position, layers) {
        QList<ExecutionChartElementCreator*> creators = m_creatorLayers.values(position);
        foreach(creator, creators) {
            creator->draw(painter);
        }
    }
}

void ExecutionChartPlotCore::clearData()
{

}

void ExecutionChartPlotCore::drawLegendIcon(QCPPainter *painter, const QRectF &rect) const
{

}

QCPRange ExecutionChartPlotCore::getKeyRange(bool &validRange, SignDomain inSignDomain) const
{
    return QCPRange(0, m_currentExtend);
}

QCPRange ExecutionChartPlotCore::getValueRange(bool &validRange, SignDomain inSignDomain) const
{
    return QCPRange(0, 1);
}

void ExecutionChartPlotCore::updateExtend(unsigned int extend)
{
    ExecutionChartElementCreator *creator;
    foreach(creator, m_creators) {
        creator->updateExtend(extend);
    }
}

void ExecutionChartPlotCore::selectEvent(QMouseEvent *event, bool additive, const QVariant &details, bool *selectionStateChanged)
{
    QMap<QString, QVariant> map = details.toMap();

    ExecutionChartElementCreator *creator = map["creator"].value<ExecutionChartElementCreator*>();

    creator->selectEvent(event, additive, map["subdetails"], selectionStateChanged);
}

ExecutionChartPlotLoad::ExecutionChartPlotLoad(QCPAxis *keyaxis, QCPAxis *valueaxis)
    : QCPGraph(keyaxis, valueaxis)
{
    keyaxis->setVisible(false);
    valueaxis->setRange(0.0,4.0);
    valueaxis->setAutoTicks(false);
    QVector<double> ticks;
    ticks.append(0.0);
    ticks.append(4.0);
    valueaxis->setTickVector(ticks);
    valueaxis->grid()->setSubGridVisible(true);
    valueaxis->setLabel("Load");
    setPen(QPen(Qt::red));
    setBrush(QBrush(QColor(255,0,0,100)));

    m_current = 0;

    QVector<double> keys; QVector<double> values;
    keys.append(0.0); values.append(0.0);
    addData(keys, values);
}

void ExecutionChartPlotLoad::updateExtend(unsigned int extend)
{
    QVector<double> keys; QVector<double> values;
    keys.append(extend); values.append(m_current);
    addData(keys, values);

}

unsigned int ExecutionChartPlotLoad::addSoftwareTrace(SoftwareTraceEvent *event)
{
    if (event->id == 0x30a) {
        m_current = (double)event->value/16;
        QVector<double> keys; QVector<double> values;
        keys.append(event->timestamp); values.append(m_current);
        addData(keys, values);
    }
    return event->timestamp;
}
