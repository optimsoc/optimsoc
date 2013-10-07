#ifndef PLOTSPECTROGRAM_H
#define PLOTSPECTROGRAM_H

#include <QDockWidget>
#include <QVector>

#include <QImage>
#include <QTimer>

#include <qwt/qwt_color_map.h>

#include "traceevents.h"

namespace Ui {
class PlotSpectrogram;
}

class PlotSpectrogram : public QDockWidget
{
    Q_OBJECT
    
public:
    explicit PlotSpectrogram(QWidget *parent = 0);
    ~PlotSpectrogram();

public slots:
    void softwareTraceEvent(SoftwareTraceEvent);

private:
    Ui::PlotSpectrogram *m_ui;
    unsigned int m_x;
    unsigned int m_y;

    QTimer m_timer;

    QwtLinearColorMap *m_cmap;

    QImage *m_image;
    QVector<unsigned int> m_coreCurrentX;
    QVector<unsigned int> m_coreCurrentY;
};

#endif // PLOTSPECTROGRAM_H
