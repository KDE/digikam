/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-07-03
 * Description : A widget to provide feedback or propose opportunistic interactions
 *
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (c) 2011      by Aurelien Gateau <agateau at kde dot org>
 * Copyright (c) 2014      by Dominik Haumann <dhaumann at kde dot org>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "dnotificationwidget_p.h"

// Qt includes

#include <QEvent>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QShowEvent>
#include <QStyle>

namespace Digikam
{

DNotificationWidget::DNotificationWidget(QWidget* parent)
    : QFrame(parent),
      d(new Private(this))
{
    d->init();
}

DNotificationWidget::DNotificationWidget(const QString& text, QWidget* parent)
    : QFrame(parent),
      d(new Private(this))
{
    d->init();
    setText(text);
}

DNotificationWidget::~DNotificationWidget()
{
    delete d;
}

QString DNotificationWidget::text() const
{
    return d->textLabel->text();
}

void DNotificationWidget::setText(const QString& text)
{
    d->textLabel->setText(text);
    updateGeometry();
}

DNotificationWidget::MessageType DNotificationWidget::messageType() const
{
    return d->messageType;
}

static QColor darkShade(QColor c)
{
    qreal contrast = 0.7; // taken from kcolorscheme for the dark shade
    qreal darkAmount;

    if (c.lightnessF() < 0.006)
    {
        /* too dark */
        darkAmount = 0.02 + 0.40 * contrast;
    }
    else if (c.lightnessF() > 0.93)
    {
        /* too bright */
        darkAmount = -0.06 - 0.60 * contrast;
    }
    else
    {
        darkAmount = (-c.lightnessF()) * (0.55 + contrast * 0.35);
    }

    qreal v = c.lightnessF() + darkAmount;
    v       = v > 0.0 ? (v < 1.0 ? v : 1.0) : 0.0;
    c.setHsvF(c.hslHueF(), c.hslSaturationF(), v);
    return c;
}

void DNotificationWidget::setMessageType(DNotificationWidget::MessageType type)
{
    d->messageType = type;
    QColor bg0, bg1, bg2, border, fg;

    switch (type)
    {
        case Positive:
            bg1.setRgb(0, 110,  40); // values taken from kcolorscheme.cpp (Positive)
            break;
        case Information:
            bg1 = palette().highlight().color();
            break;
        case Warning:
            bg1.setRgb(176, 128, 0); // values taken from kcolorscheme.cpp (Neutral)
            break;
        case Error:
            bg1.setRgb(191, 3, 3); // values taken from kcolorscheme.cpp (Negative)
            break;
    }

    // Colors
    fg     = palette().highlightedText().color();
    bg0    = bg1.lighter(110);
    bg2    = bg1.darker(110);
    border = darkShade(bg1);

    d->content->setStyleSheet(
        QString::fromLatin1(".QFrame {"
                              "background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,"
                              "    stop: 0 %1,"
                              "    stop: 0.1 %2,"
                              "    stop: 1.0 %3);"
                              "border-radius: 5px;"
                              "border: 1px solid %4;"
                              "margin: %5px;"
                              "}"
                              ".QLabel { color: %6; }"
                             )
        .arg(bg0.name())
        .arg(bg1.name())
        .arg(bg2.name())
        .arg(border.name())
        // DefaultFrameWidth returns the size of the external margin + border width. We know our border is 1px, so we subtract this from the frame normal QStyle FrameWidth to get our margin
        .arg(style()->pixelMetric(QStyle::PM_DefaultFrameWidth, 0, this) - 1)
        .arg(fg.name())
    );
}

QSize DNotificationWidget::sizeHint() const
{
    ensurePolished();
    return d->content->sizeHint();
}

QSize DNotificationWidget::minimumSizeHint() const
{
    ensurePolished();
    return d->content->minimumSizeHint();
}

bool DNotificationWidget::event(QEvent* event)
{
    if (event->type() == QEvent::Polish && !d->content->layout())
    {
        d->createLayout();
    }

    return QFrame::event(event);
}

void DNotificationWidget::resizeEvent(QResizeEvent* event)
{
    QFrame::resizeEvent(event);

    if (d->timeLine->state() == QTimeLine::NotRunning)
    {
        d->content->resize(width(), d->bestContentHeight());
    }
}

int DNotificationWidget::heightForWidth(int width) const
{
    ensurePolished();
    return d->content->heightForWidth(width);
}

void DNotificationWidget::paintEvent(QPaintEvent* event)
{
    QFrame::paintEvent(event);

    if (d->timeLine->state() == QTimeLine::Running)
    {
        QPainter painter(this);
        painter.setOpacity(d->timeLine->currentValue() * d->timeLine->currentValue());
        painter.drawPixmap(0, 0, d->contentSnapShot);
    }
}

bool DNotificationWidget::wordWrap() const
{
    return d->wordWrap;
}

void DNotificationWidget::setWordWrap(bool wordWrap)
{
    d->wordWrap = wordWrap;
    d->textLabel->setWordWrap(wordWrap);
    QSizePolicy policy = sizePolicy();
    policy.setHeightForWidth(wordWrap);
    setSizePolicy(policy);
    d->updateLayout();

    // Without this, when user does wordWrap -> !wordWrap -> wordWrap, a minimum
    // height is set, causing the widget to be too high.
    // Mostly visible in test programs.
    if (wordWrap)
    {
        setMinimumHeight(0);
    }
}

bool DNotificationWidget::isCloseButtonVisible() const
{
    return d->closeButton->isVisible();
}

void DNotificationWidget::setCloseButtonVisible(bool show)
{
    d->closeButton->setVisible(show);
    updateGeometry();
}

void DNotificationWidget::addAction(QAction* action)
{
    QFrame::addAction(action);
    d->updateLayout();
}

void DNotificationWidget::removeAction(QAction* action)
{
    QFrame::removeAction(action);
    d->updateLayout();
}

void DNotificationWidget::animatedShow()
{
    if (!style()->styleHint(QStyle::SH_Widget_Animate, 0, this))
    {
        show();
        emit showAnimationFinished();
        return;
    }

    if (isVisible())
    {
        return;
    }

    QFrame::show();
    setFixedHeight(0);
    int wantedHeight = d->bestContentHeight();
    d->content->setGeometry(0, -wantedHeight, width(), wantedHeight);
    d->updateSnapShot();
    d->timeLine->setDirection(QTimeLine::Forward);

    if (d->timeLine->state() == QTimeLine::NotRunning)
    {
        d->timeLine->start();
    }
}

void DNotificationWidget::animatedHide()
{
    if (!style()->styleHint(QStyle::SH_Widget_Animate, 0, this))
    {
        hide();
        emit hideAnimationFinished();
        return;
    }

    if (!isVisible())
    {
        return;
    }

    d->content->move(0, -d->content->height());
    d->updateSnapShot();
    d->timeLine->setDirection(QTimeLine::Backward);

    if (d->timeLine->state() == QTimeLine::NotRunning)
    {
        d->timeLine->start();
    }
}

bool DNotificationWidget::isHideAnimationRunning() const
{
    return (d->timeLine->direction() == QTimeLine::Backward) &&
           (d->timeLine->state()     == QTimeLine::Running);
}

bool DNotificationWidget::isShowAnimationRunning() const
{
    return (d->timeLine->direction() == QTimeLine::Forward) &&
           (d->timeLine->state()     == QTimeLine::Running);
}

QIcon DNotificationWidget::icon() const
{
    return d->icon;
}

void DNotificationWidget::setIcon(const QIcon &icon)
{
    d->icon = icon;

    if (d->icon.isNull())
    {
        d->iconLabel->hide();
    }
    else
    {
        const int size = style()->pixelMetric(QStyle::PM_ToolBarIconSize);
        d->iconLabel->setPixmap(d->icon.pixmap(size));
        d->iconLabel->show();
    }
}

} // namespace Digikam
