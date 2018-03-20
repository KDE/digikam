/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-10-05
 * Description : a presentation tool.
 *
 * Copyright (C) 2008 by Valerio Fuoglio <valerio dot fuoglio at gmail dot com>
 *
 * Partially based on Renchi Raju's ToolBar class.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "presentationctrlwidget.h"

// Qt includes

#include <QToolButton>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QPixmap>
#include <QApplication>
#include <QIcon>

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

PresentationCtrlWidget::PresentationCtrlWidget(QWidget* const parent)
    : QWidget(parent)
{
    setupUi(this);
    m_playButton->setCheckable(true);
    m_slideLabel->setPixmap(QIcon::fromTheme(QString::fromLatin1("view-presentation")).pixmap(64, 64));

    m_prevButton->setText(QString());
    m_nextButton->setText(QString());
    m_playButton->setText(QString());
    m_stopButton->setText(QString());

    m_prevButton->setIcon(QIcon::fromTheme(QString::fromLatin1("media-skip-backward")));
    m_nextButton->setIcon(QIcon::fromTheme(QString::fromLatin1("media-skip-forward")));
    m_playButton->setIcon(QIcon::fromTheme(QString::fromLatin1("media-playback-start")));
    m_stopButton->setIcon(QIcon::fromTheme(QString::fromLatin1("media-playback-stop")));

    m_canHide = true;

    connect(m_playButton, SIGNAL(toggled(bool)),
            this, SLOT(slotPlayButtonToggled()));

    connect(m_nextButton, SIGNAL(clicked()),
            this, SLOT(slotNexPrevClicked()));

    connect(m_prevButton, SIGNAL(clicked()),
            this, SLOT(slotNexPrevClicked()));

    connect(m_nextButton, SIGNAL(clicked()),
            this, SIGNAL(signalNext()));

    connect(m_prevButton, SIGNAL(clicked()),
            this, SIGNAL(signalPrev()));

    connect(m_stopButton, SIGNAL(clicked()),
            this, SIGNAL(signalClose()));

    slotPlayButtonToggled();
}

PresentationCtrlWidget::~PresentationCtrlWidget()
{
}

bool PresentationCtrlWidget::canHide() const
{
    return m_canHide;
}

bool PresentationCtrlWidget::isPaused() const
{
    return m_playButton->isChecked();
}

void PresentationCtrlWidget::setPaused(bool val)
{
    if (val == isPaused())
        return;

    m_playButton->setChecked(val);

    slotPlayButtonToggled();
}

void PresentationCtrlWidget::setEnabledPlay(bool val)
{
    m_playButton->setEnabled(val);
}

void PresentationCtrlWidget::setEnabledNext(bool val)
{
    m_nextButton->setEnabled(val);
}

void PresentationCtrlWidget::setEnabledPrev(bool val)
{
    m_prevButton->setEnabled(val);
}

void PresentationCtrlWidget::slotPlayButtonToggled()
{
    if (m_playButton->isChecked())
    {
        m_canHide = false;
        m_playButton->setIcon(QIcon::fromTheme(QString::fromLatin1("media-playback-start")).pixmap(22));
        emit signalPause();
    }
    else
    {
        m_canHide = true;
        m_playButton->setIcon(QIcon::fromTheme(QString::fromLatin1("media-playback-pause")).pixmap(22));
        emit signalPlay();
    }
}

void PresentationCtrlWidget::slotNexPrevClicked()
{
    if (!m_playButton->isChecked())
    {
        m_playButton->setChecked(true);
        m_canHide = false;
        m_playButton->setIcon(QIcon::fromTheme(QString::fromLatin1("media-playback-start")).pixmap(22));
        emit signalPause();
    }
}

void PresentationCtrlWidget::keyPressEvent(QKeyEvent* event)
{
    switch (event->key())
    {
        case(Qt::Key_Space):
        {
            if (m_playButton->isEnabled())
                m_playButton->animateClick();

            break;
        }

        case(Qt::Key_PageUp):
        {
            if (m_prevButton->isEnabled())
                m_prevButton->animateClick();

            break;
        }

        case(Qt::Key_PageDown):
        {
            if (m_nextButton->isEnabled())
                m_nextButton->animateClick();

            break;
        }

        case(Qt::Key_Escape):
        {
            if (m_stopButton->isEnabled())
                m_stopButton->animateClick();

            break;
        }

        default:
            break;
    }

    event->accept();
}

} // namespace Digikam
