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

#ifndef EXECUTIONCHARTCREATORS_H
#define EXECUTIONCHARTCREATORS_H

// Forward declarations
class ExecutionChartElement;
class ExecutionChartSection;

#include <QGraphicsScene>
#include <QMap>

#include "qcustomplot.h"

#include "traceevents.h"
#include "executionchartplots.h"
#include "executionchartelements.h"

class ExecutionChartElementCreator : public QObject
{
    Q_OBJECT

public:
    ExecutionChartElementCreator(ExecutionChartPlotCore *plot);
    /**
      * Add trace event
      *
      * @return Updated maximum extend
      */
    virtual unsigned int addTrace(SoftwareTraceEvent *event) = 0;
    virtual void updateExtend(unsigned int extend) {}
    virtual void draw(QCPPainter *painter) = 0;
    virtual double selectTest(const QPointF &pos, bool onlySelectable, QVariant *details) const = 0;
    virtual void selectEvent(QMouseEvent *event, bool additive, const QVariant &details, bool *selectionStateChanged) = 0;
protected:
    QList<ExecutionChartElement*> m_elements;
    ExecutionChartPlotCore *m_plot;
};

Q_DECLARE_METATYPE(ExecutionChartElementCreator*)

class ExecutionChartSectionCreator : public ExecutionChartElementCreator
{
    Q_OBJECT

public:
    ExecutionChartSectionCreator(ExecutionChartPlotCore *plot);
    virtual unsigned int addTrace(SoftwareTraceEvent *event);
    virtual void updateExtend(unsigned int extend);
    virtual void draw(QCPPainter *painter);
    virtual double selectTest(const QPointF &pos, bool onlySelectable, QVariant *details) const;
    virtual void selectEvent(QMouseEvent *event, bool additive, const QVariant &details, bool *selectionStateChanged);

private:
    void createSection(unsigned int from, unsigned int to, int id,
                       QString text);

    unsigned int m_currentSectionDefinition;
    QMap<unsigned int,QString> m_sectionNames;
    bool m_inSection;

    ExecutionChartSection* m_currentSection;
    ExecutionChartSection* m_currentSelection;

};

class ExecutionChartEventCreator : public ExecutionChartElementCreator
{
    Q_OBJECT

    class Event
    {
    public:
        virtual bool occur(unsigned int value) = 0;
        virtual QString format(QString input) = 0;
    };

    class EventEmpty : public Event
    {
    public:
        bool occur(unsigned int value) {
            return true;
        }
        QString format(QString input) {
            return input;
        }
    };

    class EventDec : public Event
    {
    public:
        bool occur(unsigned int value) {
            m_value = value;
            return true;
        }
        QString format(QString input) {
            return input.arg(m_value);
        }
    private:
        unsigned int m_value;
    };

    class EventDecSigned : public Event
    {
    public:
        bool occur(unsigned int value) {
            m_value = int(value);
            return true;
        }
        QString format(QString input) {
            return input.arg(m_value);
        }
    private:
        int m_value;
    };
    class EventHex : public Event
    {
    public:
        bool occur(unsigned int value) {
            m_value = value;
            return true;
        }
        QString format(QString input) {
            return input.arg(m_value,0,16);
        }
    private:
        unsigned int m_value;
    };
    class EventFloat : public Event
    {
    public:
        bool occur(unsigned int value) {
            m_value = *(reinterpret_cast<float*>(&value));
            return true;
        }
        QString format(QString input) {
            return input.arg(m_value);
        }
    private:
        float m_value;
    };
    class EventChar : public Event
    {
    public:
        bool occur(unsigned int value) {
            m_value = char(value);
            return true;
        }
        QString format(QString input) {
            return input.arg(m_value);
        }
    private:
        QChar m_value;
    };
    class EventString : public Event
    {
    public:
        bool occur(unsigned int value) {
            if (value != 0) {
                m_value += char(value);
                return false;
            } else {
                return true;
            }
        }
        QString format(QString input) {
            QString ret = input.arg(m_value);
            m_value.clear();
            return ret;
        }
    private:
        QString m_value;
    };

public:
    ExecutionChartEventCreator(ExecutionChartPlotCore *plot, unsigned int width, QString textFormat, QColor color);
    virtual double selectTest(const QPointF &pos, bool onlySelectable, QVariant *details) const;
    virtual void selectEvent(QMouseEvent *event, bool additive, const QVariant &details, bool *selectionStateChanged);
    virtual void draw(QCPPainter *painter);
    bool appendEmpty();
    bool appendDec(bool actualTime);
    bool appendDecSigned(bool actualTime);
    bool appendHex(bool actualTime);
    bool appendFloat(bool actualTime);
    bool appendChar(bool actualTime);
    bool appendString(bool actualTime);

    virtual unsigned int addTrace(SoftwareTraceEvent *event);
private:
    unsigned int m_width;
    QString m_textFormat;

    QVector<Event*> m_eventSequence;
    QVector<Event*>::const_iterator m_eventSequenceIterator;
    bool m_acceptAppends;
    Event* m_eventActualTime;
    unsigned int m_timestamp;
    QColor m_color;

    ExecutionChartEvent *m_currentSelection;
};

#endif // EXECUTIONCHARTCREATORS_H
