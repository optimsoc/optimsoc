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
 *   Philipp Wagner <philipp.wagner@tum.de>
 */

#include "logwidget.h"
#include "ui_logwidget.h"

#include <QShowEvent>
#include <QScrollBar>

#include "systeminterface.h"

// from sys/syslog.h
#define	LOG_ERR		3	/* error conditions */
#define	LOG_WARNING	4	/* warning conditions */
#define	LOG_NOTICE	5	/* normal but significant condition */

LogWidget::LogWidget(QWidget *parent) :
    QWidget(parent),
    m_ui(new Ui::LogWidget),
    m_sysif(SystemInterface::instance()),
    m_unseenImportant(0),
    m_unseenInfo(0)
{
    m_ui->setupUi(this);

    connect(m_sysif, SIGNAL(logMsgReceived(int,QString,int,QString,QString)),
            this, SLOT(writeMsgToLog(int,QString,int,QString,QString)));

}

LogWidget::~LogWidget()
{
    delete m_ui;
}

/**
 * Append a log message to the display widget (slot)
 *
 * @param priority
 * @param file
 * @param line
 * @param fn
 * @param msg
 */
void LogWidget::writeMsgToLog(int priority, QString file, int line, QString fn,
                              QString msg)
{
    QString fMessage;
    QString txtColor;
    if (priority == LOG_ERR) {
        txtColor = "red";
    } else {
        txtColor = "black";
    }
    fMessage = QString("<div style='color:%1'>%2</div>").arg(txtColor, msg);

    m_ui->textBrowserLog->append(fMessage);

    // update number of unseen messages and inform others about it
    if (!isVisible()) {
        if (priority == LOG_ERR) {
            m_unseenImportant++;
        } else {
            m_unseenInfo++;
        }
        emit unseenLogMsgs(m_unseenInfo, m_unseenImportant);
    }
}

/**
 * Event called if the widget is shown
 *
 * @see QWidget::showEvent()
 * @param event
 */
void LogWidget::showEvent(QShowEvent *event)
{
    // Reset the number of unseen events, but only if the widget is actively
    // shown, not if it is just made visible by the window system (e.g. shown
    // after being minimized).
    if (!event->spontaneous()) {
        m_unseenImportant = 0;
        m_unseenInfo = 0;
        emit unseenLogMsgs(m_unseenInfo, m_unseenImportant);

        // scroll to the bottom of the textbox to show the latest messages
        QScrollBar *sb = m_ui->textBrowserLog->verticalScrollBar();
        sb->setSliderPosition(sb->maximum());
    }

    QWidget::showEvent(event);
}
