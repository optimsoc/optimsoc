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

#ifndef PLOTSPECTROGRAM_H
#define PLOTSPECTROGRAM_H

#include <QDockWidget>
#include <QVector>

#include <QImage>
#include <QTimer>

#include <qwt/qwt_color_map.h>

#include "traceevents.h"

class SystemInterface;

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
    void softwareTraceEvent(struct SoftwareTraceEvent event);

private:
    Ui::PlotSpectrogram *m_ui;
    unsigned int m_x;
    unsigned int m_y;

    QTimer m_timer;

    QwtLinearColorMap *m_cmap;

    SystemInterface *m_sysif;

    QImage *m_image;
    QVector<unsigned int> m_coreCurrentX;
    QVector<unsigned int> m_coreCurrentY;
};

#endif // PLOTSPECTROGRAM_H
