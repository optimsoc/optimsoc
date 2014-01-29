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

#include "executionchartcreators.h"

#include <QToolTip>

#include "executionchartelements.h"

#include "assert.h"

ExecutionChartElementCreator::ExecutionChartElementCreator(ExecutionChartPlotCore *plot)
    : m_plot(plot)
{

}

ExecutionChartSectionCreator::ExecutionChartSectionCreator(ExecutionChartPlotCore *plot)
    : ExecutionChartElementCreator(plot), m_inSection(false), m_currentSection(0), m_activeSection(-1), m_handleExceptions(false), m_inException(false)
{

}

QMap<unsigned int, QString> ExecutionChartSectionCreator::m_globalSectionNames;

unsigned int ExecutionChartSectionCreator::addTrace(SoftwareTraceEvent *event)
{
    QChar c;
    switch(event->id) {
    case 0x1:
        createSection(event->timestamp, event->timestamp, -1, "Exited");
    case 0x20:
        m_currentSectionDefinition = event->value;
        break;
    case 0x21:
        c = event->value & 0xff;
        if (m_sectionNames.find(m_currentSectionDefinition) == m_sectionNames.end()) {
            m_sectionNames[m_currentSectionDefinition] = c;
        } else {
            m_sectionNames[m_currentSectionDefinition] += c;
        }
        break;
    case 0x22:
        m_activeSection = (int) event->value;

        if (!m_handleExceptions || !m_inException) {
            if (m_sectionNames.find(m_activeSection) != m_sectionNames.end()) {
                createSection(event->timestamp, event->timestamp, m_activeSection, m_sectionNames[m_activeSection]);
            } else if (m_globalSectionNames.find(m_activeSection) != m_globalSectionNames.end()) {
                createSection(event->timestamp, event->timestamp, m_activeSection, m_globalSectionNames[m_activeSection]);
            }
        }
        break;
    case 0x23:
        // Activate exception handling for the sections
        m_handleExceptions = true;
    case 0x24:
        // Return from exception
        if (m_handleExceptions && m_inException) {
            if (m_sectionNames.find(m_activeSection) != m_sectionNames.end()) {
                createSection(event->timestamp, event->timestamp, m_activeSection, m_sectionNames[m_activeSection]);
            } else if (m_globalSectionNames.find(m_activeSection) != m_globalSectionNames.end()) {
                createSection(event->timestamp, event->timestamp, m_activeSection, m_globalSectionNames[m_activeSection]);
            }
        }
        m_inException = false;
        break;
    case 0x25:
        c = event->value & 0xff;
        if (m_globalSectionNames.find(m_currentSectionDefinition) == m_sectionNames.end()) {
            m_globalSectionNames[m_currentSectionDefinition] = c;
        } else {
            m_globalSectionNames[m_currentSectionDefinition] += c;
        }
        break;
    case 0x31:
        m_inException = true;
        createSection(event->timestamp, event->timestamp, -1, "System start");
        break;
    case 0x32:
        m_inException = true;
        createSection(event->timestamp, event->timestamp, -1, "Bus fault");
        break;
    case 0x33:
        m_inException = true;
        createSection(event->timestamp, event->timestamp, -1, "Data page fault");
        break;
    case 0x34:
        m_inException = true;
        createSection(event->timestamp, event->timestamp, -1, "Insn page fault");
        break;
    case 0x35:
        m_inException = true;
        createSection(event->timestamp, event->timestamp, -1, "Timer exception");
        break;
    case 0x36:
        m_inException = true;
        createSection(event->timestamp, event->timestamp, -1, "Alignment exception");
        break;
    case 0x37:
        m_inException = true;
        createSection(event->timestamp, event->timestamp, -1, "Illegal instruction");
        break;
    case 0x38:
        m_inException = true;
        createSection(event->timestamp, event->timestamp, -1, "Interrupt");
        break;
    case 0x39:
        m_inException = true;
        createSection(event->timestamp, event->timestamp, -1, "Data TLB miss");
        break;
    case 0x3a:
        m_inException = true;
        createSection(event->timestamp, event->timestamp, -1, "Instruction TLB miss");
        break;
    case 0x3b:
        m_inException = true;
        createSection(event->timestamp, event->timestamp, -1, "Range exception");
        break;
    case 0x3c:
        m_inException = true;
        createSection(event->timestamp, event->timestamp, -1, "Syscall");
        break;
    default:
        break;
    }

    return event->timestamp;
}

void ExecutionChartSectionCreator::createSection(unsigned int from,
                                                 unsigned int to,
                                                 int id,
                                                 QString text)
{
    updateExtend(from);
    ExecutionChartSection *sect;
    sect = new ExecutionChartSection(from, to, id, text);
    m_elements.push_back(sect);
    m_currentSection = sect;
}

void ExecutionChartSectionCreator::updateExtend(unsigned int extend)
{
    if (m_currentSection) {
        m_currentSection->updateExtend(extend);
    }
}

double ExecutionChartSectionCreator::selectTest(const QPointF &pos, bool onlySelectable, QVariant *details) const
{
    double x, y;
    m_plot->pixelsToCoords(pos, x, y);

    if ((y < 0.25) || (y > 0.75)) {
        return -1.0;
    }

    ExecutionChartElement *elem;
    ExecutionChartSection *section;
    foreach (elem, m_elements) {
        section = dynamic_cast<ExecutionChartSection*>(elem);
        unsigned int from, to;
        section->getRange(from, to);
        if ((x > from) && (x < to)) {
            *details = QVariant::fromValue<ExecutionChartSection*>(section);
            return 0.0;
        }
    }

    return -1.0;
}

void ExecutionChartSectionCreator::selectEvent(QMouseEvent *event, bool additive, const QVariant &details, bool *selectionStateChanged)
{
    ExecutionChartSection* section = details.value<ExecutionChartSection*>();
    QToolTip::showText(event->globalPos(), section->text(), 0);
}

void ExecutionChartSectionCreator::draw(QCPPainter *painter)
{
    ExecutionChartElement *elem;
    ExecutionChartSection *section;
    foreach (elem, m_elements) {
        section = dynamic_cast<ExecutionChartSection*>(elem);
        double x1, x2, y1, y2;

        unsigned int from, to;
        QColor brush, pen;

        section->getRange(from, to);
        section->getColors(pen, brush);

        m_plot->coordsToPixels(from, 0.25, x1, y1);
        m_plot->coordsToPixels(to, 0.75, x2, y2);

        painter->setBrush(QBrush(brush));
        painter->setPen(QPen(pen));

        painter->drawRect(x1, y1, x2-x1, y2-y1);
    }
}


ExecutionChartEventCreator::ExecutionChartEventCreator(ExecutionChartPlotCore *plot, unsigned int width, QString textFormat, QColor color)
    : ExecutionChartElementCreator(plot), m_width(width), m_acceptAppends(true),
      m_eventActualTime(NULL), m_textFormat(textFormat), m_color(color)
{
}

unsigned int ExecutionChartEventCreator::addTrace(SoftwareTraceEvent *event)
{
    if (m_eventSequence.size()==0) {
        return 0;
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

    bool rv = e->occur(event->value);
    if (rv) {
        // If the event is complete: advance
        m_eventSequenceIterator++;
    }

    // Assign timestamp if matches
    if (e == m_eventActualTime) {
        m_timestamp = event->timestamp;
    }

    if (m_eventSequenceIterator == m_eventSequence.end()) {
        // This was the last in the sequence
        // Generate string to display
        QString text = m_textFormat;
        foreach(e, m_eventSequence) {
            text = e->format(text);
        }

        // Draw actual event
        ExecutionChartEvent *event = new ExecutionChartEvent(m_timestamp, m_width,
                                                             text, m_color);

        m_elements.append(event);

        // Reset to the begin of the sequence
        m_eventSequenceIterator = m_eventSequence.begin();
    }

    return event->timestamp;
}

double ExecutionChartEventCreator::selectTest(const QPointF &pos, bool onlySelectable, QVariant *details) const
{
    double x, y;
    m_plot->pixelsToCoords(pos, x, y);

    if ((y < 0.1) || (y > 0.9)) {
        return -1.0;
    }

    ExecutionChartElement *elem;
    ExecutionChartEvent *event;
    foreach (elem, m_elements) {
        event = dynamic_cast<ExecutionChartEvent*>(elem);

        double ex, ey;
        m_plot->coordsToPixels(event->timestamp(), 0, ex, ey);
        if ((pos.x() >= ex - 2) && (pos.x() <= ex + event->width() + 2)) {
            *details = QVariant::fromValue<ExecutionChartEvent*>(event);
            return 0.0;
        }
    }

    return -1.0;
}

void ExecutionChartEventCreator::selectEvent(QMouseEvent *event, bool additive, const QVariant &details, bool *selectionStateChanged)
{
    ExecutionChartEvent* ev = details.value<ExecutionChartEvent*>();
    QToolTip::showText(event->globalPos(), ev->text(), 0);
}

void ExecutionChartEventCreator::draw(QCPPainter *painter)
{
    ExecutionChartElement *elem;
    ExecutionChartEvent *event;
    foreach (elem, m_elements) {
        event = dynamic_cast<ExecutionChartEvent*>(elem);

        unsigned int time = event->timestamp();
        QColor brush = event->color();
        unsigned int width = event->width();

        double x1, x2, y1, y2;

        m_plot->coordsToPixels(time, 0.1, x1, y1);
        m_plot->coordsToPixels(time, 0.9, x2, y2);

        x2 = x1 + width;

        painter->setBrush(brush);
        painter->setPen(Qt::NoPen);

        painter->drawRect(x1, y1, x2-x1, y2-y1);
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
