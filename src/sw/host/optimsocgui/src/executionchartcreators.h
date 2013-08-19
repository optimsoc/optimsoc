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

#ifndef EXECUTIONCHARTCREATORS_H
#define EXECUTIONCHARTCREATORS_H

// Forward declarations
class ExecutionChartElement;
class ExecutionChartSection;

#include <QGraphicsScene>
#include <QMap>

class ExecutionChartElementCreator : public QObject
{
    Q_OBJECT

public:
    ExecutionChartElementCreator(QObject *parent, QGraphicsScene *scene);
    virtual void addTrace(unsigned int timestamp,
                          unsigned int id,
                          unsigned int value) = 0;
    void rescale(double newscale, double oldscale);
    virtual void expand(int maximum) {}
protected:
    QList<ExecutionChartElement*> m_elements;
    QGraphicsScene *m_scene;
    double m_scale;
};

class ExecutionChartSectionCreator : public ExecutionChartElementCreator
{
    Q_OBJECT

public:
    ExecutionChartSectionCreator(QObject *parent, QGraphicsScene *scene,
                                 unsigned int baseline, unsigned int height);
    virtual void addTrace(unsigned int timestamp, unsigned int id,
                          unsigned int value);
    virtual void expand(int maximum);
private:
    void createSection(unsigned int from, unsigned int to, int id,
                       QString text);

    unsigned int m_currentSectionDefinition;
    QMap<unsigned int,QString> m_sectionNames;
    bool m_inSection;
    unsigned int m_currentSection;
    unsigned int m_currentSectionStart;

    unsigned int m_baseline;
    unsigned int m_height;
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
    ExecutionChartEventCreator(QObject *parent, QGraphicsScene *scene,
                               unsigned int baseline, unsigned int height,
                               unsigned int width, QString textFormat,
                               unsigned int color = 0xff000000);
    bool appendEmpty();
    bool appendDec(bool actualTime);
    bool appendDecSigned(bool actualTime);
    bool appendHex(bool actualTime);
    bool appendFloat(bool actualTime);
    bool appendChar(bool actualTime);
    bool appendString(bool actualTime);

    virtual void addTrace(unsigned int timestamp, unsigned int id,
                          unsigned int value);
private:
    unsigned int m_baseline;
    unsigned int m_height;
    unsigned int m_width;
    QString m_textFormat;

    QVector<Event*> m_eventSequence;
    QVector<Event*>::const_iterator m_eventSequenceIterator;
    bool m_acceptAppends;
    Event* m_eventActualTime;
    unsigned int m_timestamp;
    QColor m_color;
};

#endif // EXECUTIONCHARTCREATORS_H
