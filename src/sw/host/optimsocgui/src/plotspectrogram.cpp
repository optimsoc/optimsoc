#include "plotspectrogram.h"
#include "ui_plotspectrogram.h"

#include <assert.h>

PlotSpectrogram::PlotSpectrogram(QWidget *parent) :
    QDockWidget(parent),
    m_ui(new Ui::PlotSpectrogram)
{
    m_ui->setupUi(this);
}

PlotSpectrogram::~PlotSpectrogram()
{
    delete m_ui;
}

void PlotSpectrogram::softwareTraceEvent(SoftwareTraceEvent event)
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
