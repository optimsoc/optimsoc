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

#include "plotspectrogram.h"
#include "ui_plotspectrogram.h"

#include "systeminterface.h"
#include <assert.h>

PlotSpectrogram::PlotSpectrogram(QWidget *parent) :
    QDockWidget(parent),
    m_ui(new Ui::PlotSpectrogram),
    m_sysif(SystemInterface::instance())
{
    m_ui->setupUi(this);

    connect(m_sysif->softwareTraceEventDistributor(),
            SIGNAL(softwareTraceEvent(struct SoftwareTraceEvent)),
            this,
            SLOT(softwareTraceEvent(struct SoftwareTraceEvent)));
}

PlotSpectrogram::~PlotSpectrogram()
{
    delete m_ui;
}

void PlotSpectrogram::softwareTraceEvent(struct SoftwareTraceEvent event)
{
    QRectF r;
    QCPAxisRect *axes;
    QCPItemPixmap *pixmapitem;
    QPixmap *pixmap;

    switch(event.id) {
    case 0x1100:
        m_x = event.value;
        break;
    case 0x1101:
        m_y = event.value;

        m_cmap = new QwtLinearColorMap();
        m_cmap->setColorInterval(QColor(0,0,255), QColor(255,0,0));

        m_ui->plot->addGraph();
        m_ui->plot->xAxis->setRange(0, m_x);
        m_ui->plot->yAxis->setRange(0, m_y);
        m_ui->plot->xAxis->grid()->setVisible(false);
        m_ui->plot->yAxis->grid()->setVisible(false);

        m_image = new QImage(m_x, m_y, QImage::Format_RGB32);
        for (int x = 0; x < m_x; ++x) {
            for (int y = 0; y < m_y; ++y) {
                m_image->setPixel(x, y, 0xff0000ff);
            }
        }

        m_ui->plot->axisRect()->setBackground(QPixmap::fromImage(*m_image), true, Qt::IgnoreAspectRatio);

        m_ui->plot->replot();

        // Finally show the window
        this->show();

        m_timer.start(20);
        connect(&m_timer, SIGNAL(timeout()), m_ui->plot, SLOT(replot()));

        break;
    case 0x1102:
        assert(event.value<m_x);
        if (m_coreCurrentX.size() <= event.core_id) {
            m_coreCurrentX.resize(event.core_id + 1);
        }
        m_coreCurrentX[event.core_id] = event.value;
        break;
    case 0x1103:
        assert(event.value<m_y);
        if (m_coreCurrentY.size() <= event.core_id) {
            m_coreCurrentY.resize(event.core_id + 1);
        }
        m_coreCurrentY[event.core_id] = event.value;
        break;
    case 0x1104:
        float value = *((float*)&event.value);
        if (value > 10) {
            value = 10;
        }
        QRgb color = m_cmap->rgb(QwtInterval(0,10), value);

        m_image->setPixel(m_coreCurrentX[event.core_id], m_coreCurrentY[event.core_id], color);
        m_ui->plot->axisRect()->setBackground(QPixmap::fromImage(*m_image));
//        m_ui->plot->replot();
        break;
    }
}
