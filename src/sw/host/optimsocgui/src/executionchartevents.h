#ifndef EXECUTIONCHARTEVENTS_H
#define EXECUTIONCHARTEVENTS_H

#include <QGraphicsRectItem>

class ExecutionChartSingularEvent : public QGraphicsRectItem
{
public:
    ExecutionChartSingularEvent(int baseline, int height, int timestamp,
                                QString text, QColor color = Qt::white,
                                QColor bordercolor = Qt::black, int width = 5);
    ~ExecutionChartSingularEvent();

private:
    int m_baseline;
    int m_height;
    int m_timestamp;
    QColor m_color;
    QColor m_borderColor;
    QString m_text;
    int m_width;
};

#endif // EXECUTIONCHARTEVENTS_H
