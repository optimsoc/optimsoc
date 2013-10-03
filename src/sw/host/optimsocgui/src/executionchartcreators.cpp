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

#include "executionchartcreators.h"

#include "executionchartelements.h"

ExecutionChartElementCreator::ExecutionChartElementCreator(QObject *parent,
                                                           QGraphicsScene *scene)
    : QObject(parent), m_scene(scene), m_scale(0)
{

}

ExecutionChartSectionCreator::ExecutionChartSectionCreator(QObject *parent,
                                                           QGraphicsScene *scene,
                                                           unsigned int baseline,
                                                           unsigned int height)
    : ExecutionChartElementCreator(parent,scene), m_baseline(baseline), m_height(height),
      m_inSection(false)
{

}

void ExecutionChartSectionCreator::addTrace(unsigned int timestamp,
                                            unsigned int id, unsigned int value)
{
    QChar c;
    switch(id) {
    case 0x1:
        expand(timestamp);
        createSection(timestamp, timestamp, -1, "Exited");
    case 0x20:
        m_currentSectionDefinition = value;
        break;
    case 0x21:
        c = value&0xff;
        if (m_sectionNames.find(m_currentSectionDefinition)==m_sectionNames.end()) {
            m_sectionNames[m_currentSectionDefinition] = c;
        } else {
            m_sectionNames[m_currentSectionDefinition] += c;
        }
        break;
    case 0x22:
        expand(timestamp);
        createSection(timestamp, timestamp, (int) value, m_sectionNames[value]);
        break;
    case 0x31:
        expand(timestamp);
        createSection(timestamp, timestamp, -1, "System start");
    default:
        break;
    }
}

void ExecutionChartSectionCreator::createSection(unsigned int from,
                                                 unsigned int to,
                                                 int id,
                                                 QString text)
{
    ExecutionChartSection *sect;
    sect = new ExecutionChartSection(this, m_scale, m_baseline+2, m_height-4,
                                     from, to, id, text);
    m_elements.push_back(sect);
    m_scene->addItem(sect->item());
}

void ExecutionChartSectionCreator::expand(int maximum)
{
    // If no elements exists
    if (m_elements.length()==0) return;

    // Get last section
    ExecutionChartSection *section;
    section = reinterpret_cast<ExecutionChartSection*>(m_elements.last());

    // Resize section
    section->expand(maximum);
}

void ExecutionChartElementCreator::rescale(double newscale, double oldscale)
{
    ExecutionChartElement *e;
    foreach(e,m_elements) {
        e->rescale(newscale,oldscale);
    }
    m_scale = newscale;
}

ExecutionChartEventCreator::ExecutionChartEventCreator(QObject *parent,
                                                       QGraphicsScene *scene,
                                                       unsigned int baseline,
                                                       unsigned int height,
                                                       unsigned int width,
                                                       QString textFormat,
                                                       unsigned int color)
    : ExecutionChartElementCreator(parent,scene), m_baseline(baseline),
      m_height(height), m_width(width), m_acceptAppends(true),
      m_eventActualTime(NULL), m_textFormat(textFormat), m_color(color)
{
}

void ExecutionChartEventCreator::addTrace(unsigned int timestamp, unsigned int id, unsigned int value)
{
    if (m_eventSequence.size()==0) {
        return;
    }

    if (m_acceptAppends) {
        // Finalize accepting appends, the list is settled now
        // Go to next phase, that is adding trace events
        // Initialize iterator and set to begin
        m_eventSequenceIterator = m_eventSequence.begin();

        // Set timestamp to first if none set
        if (m_eventActualTime == NULL) {
            m_eventActualTime = *m_eventSequenceIterator;
        }

        // Don't accept appends any more
        m_acceptAppends = false;
    }

    // Get current event we are waiting for from iterator
    Event* e = *m_eventSequenceIterator;

    bool rv = e->occur(value);
    if (rv) {
        // If the event is complete: advance
        m_eventSequenceIterator++;
    }

    // Assign timestamp if matches
    if (e == m_eventActualTime) {
        m_timestamp = timestamp;
    }

    if (m_eventSequenceIterator == m_eventSequence.end()) {
        // This was the last in the sequence
        // Generate string to display
        QString text = m_textFormat;
        foreach(e,m_eventSequence) {
            text = e->format(text);
        }

        // Draw actual event
        ExecutionChartEvent *event = new ExecutionChartEvent(this, m_scale, m_baseline,
                                                             m_height, m_timestamp, m_width,
                                                             text, m_color);
        QGraphicsItem *item = event->item();
        m_scene->addItem(item);
        m_elements.append(event);

        // Reset to the begin of the sequence
        m_eventSequenceIterator = m_eventSequence.begin();
    }
}

bool ExecutionChartEventCreator::appendEmpty()
{
    if (!m_acceptAppends)
        return false;

    Event *e = new EventEmpty();

    m_eventSequence.append(e);

    m_eventActualTime = e;

    return true;
}

bool ExecutionChartEventCreator::appendDec(bool actualTime)
{
    if (!m_acceptAppends)
        return false;

    Event *e = new EventDec();

    m_eventSequence.append(e);

    if (actualTime) {
        m_eventActualTime = e;
    }

    return true;
}

bool ExecutionChartEventCreator::appendDecSigned(bool actualTime)
{
    if (!m_acceptAppends)
        return false;

    Event *e = new EventDecSigned();

    m_eventSequence.append(e);

    if (actualTime) {
        m_eventActualTime = e;
    }

    return true;
}

bool ExecutionChartEventCreator::appendHex(bool actualTime)
{
    if (!m_acceptAppends)
        return false;

    Event *e = new EventHex();

    m_eventSequence.append(e);

    if (actualTime) {
        m_eventActualTime = e;
    }

    return true;
}

bool ExecutionChartEventCreator::appendFloat(bool actualTime)
{
    if (!m_acceptAppends)
        return false;

    Event *e = new EventFloat();

    m_eventSequence.append(e);

    if (actualTime) {
        m_eventActualTime = e;
    }

    return true;
}

bool ExecutionChartEventCreator::appendChar(bool actualTime)
{
    if (!m_acceptAppends)
        return false;

    Event *e = new EventChar();

    m_eventSequence.append(e);

    if (actualTime) {
        m_eventActualTime = e;
    }

    return true;
}

bool ExecutionChartEventCreator::appendString(bool actualTime)
{
    if (!m_acceptAppends)
        return false;

    Event *e = new EventString();

    m_eventSequence.append(e);

    if (actualTime) {
        m_eventActualTime = e;
    }

    return true;
}
