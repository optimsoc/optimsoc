#include "executionchartevents.h"

#include <QBrush>
#include <QPen>

ExecutionChartSingularEvent::ExecutionChartSingularEvent(int baseline,
                                                         int height,
                                                         int timestamp,
                                                         QString text,
                                                         QColor color,
                                                         QColor bordercolor,
                                                         int width)
    : QGraphicsRectItem(), m_baseline(baseline), m_height(height),
      m_timestamp(timestamp), m_text(text), m_color(color),
      m_borderColor(bordercolor), m_width(width)
{
    setRect(timestamp,baseline,width,height);
    setToolTip(text);
    setBrush(QBrush(color));
    setPen(QPen(bordercolor));
}

ExecutionChartSingularEvent::~ExecutionChartSingularEvent()
{

}
