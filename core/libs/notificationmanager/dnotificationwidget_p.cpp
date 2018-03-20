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

#include <QAction>
#include <QEvent>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QShowEvent>
#include <QTimeLine>
#include <QToolButton>
#include <QStyle>

// KDE includes

#include <klocalizedstring.h>

namespace Digikam
{

DNotificationWidget::Private::Private(DNotificationWidget* const q_ptr)
    : QObject(q_ptr)
{
    q           = q_ptr;
    content     = 0;
    iconLabel   = 0;
    textLabel   = 0;
    closeButton = 0;
    timeLine    = 0;
    messageType = DNotificationWidget::Information;
    wordWrap    = false;
}

DNotificationWidget::Private::~Private()
{
}

void DNotificationWidget::Private::init()
{
    q->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

    timeLine = new QTimeLine(500, q);

    connect(timeLine, SIGNAL(valueChanged(qreal)),
            this, SLOT(slotTimeLineChanged(qreal)));

    connect(timeLine, SIGNAL(finished()),
            this, SLOT(slotTimeLineFinished()));

    content = new QFrame(q);
    content->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    wordWrap = false;

    iconLabel = new QLabel(content);
    iconLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    iconLabel->hide();

    textLabel = new QLabel(content);
    textLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    textLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);

    connect(textLabel, &QLabel::linkActivated, q,
            &DNotificationWidget::linkActivated);

    connect(textLabel, &QLabel::linkHovered, q,
            &DNotificationWidget::linkHovered);

    QAction* const closeAction = new QAction(q);
    closeAction->setText(i18n("&Close"));
    closeAction->setToolTip(i18n("Close message"));
    closeAction->setIcon(q->style()->standardIcon(QStyle::SP_DialogCloseButton));

    connect(closeAction, &QAction::triggered, q,
            &DNotificationWidget::animatedHide);

    closeButton = new QToolButton(content);
    closeButton->setAutoRaise(true);
    closeButton->setDefaultAction(closeAction);

    q->setMessageType(DNotificationWidget::Information);
}

void DNotificationWidget::Private::createLayout()
{
    delete content->layout();

    content->resize(q->size());

    qDeleteAll(buttons);
    buttons.clear();

    Q_FOREACH (QAction* const action, q->actions())
    {
        QToolButton* const button = new QToolButton(content);
        button->setDefaultAction(action);
        button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        buttons.append(button);
    }

    // AutoRaise reduces visual clutter, but we don't want to turn it on if
    // there are other buttons, otherwise the close button will look different
    // from the others.
    closeButton->setAutoRaise(buttons.isEmpty());

    if (wordWrap)
    {
        QGridLayout* const layout = new QGridLayout(content);
        // Set alignment to make sure icon does not move down if text wraps
        layout->addWidget(iconLabel, 0, 0, 1, 1, Qt::AlignHCenter | Qt::AlignTop);
        layout->addWidget(textLabel, 0, 1);

        QHBoxLayout* const buttonLayout = new QHBoxLayout;
        buttonLayout->addStretch();

        Q_FOREACH (QToolButton* const button, buttons)
        {
            // For some reason, calling show() is necessary if wordwrap is true,
            // otherwise the buttons do not show up. It is not needed if
            // wordwrap is false.
            button->show();
            buttonLayout->addWidget(button);
        }

        buttonLayout->addWidget(closeButton);
        layout->addItem(buttonLayout, 1, 0, 1, 2);
    }
    else
    {
        QHBoxLayout* const layout = new QHBoxLayout(content);
        layout->addWidget(iconLabel);
        layout->addWidget(textLabel);

        Q_FOREACH (QToolButton* const button, buttons)
        {
            layout->addWidget(button);
        }

        layout->addWidget(closeButton);
    };

    if (q->isVisible())
    {
        q->setFixedHeight(content->sizeHint().height());
    }

    q->updateGeometry();
}

void DNotificationWidget::Private::updateLayout()
{
    if (content->layout())
    {
        createLayout();
    }
}

void DNotificationWidget::Private::updateSnapShot()
{
    // Attention: updateSnapShot calls QWidget::render(), which causes the whole
    // window layouts to be activated. Calling this method from resizeEvent()
    // can lead to infinite recursion, see:
    // https://bugs.kde.org/show_bug.cgi?id=311336
    contentSnapShot = QPixmap(content->size() * q->devicePixelRatio());
    contentSnapShot.setDevicePixelRatio(q->devicePixelRatio());
    contentSnapShot.fill(Qt::transparent);
    content->render(&contentSnapShot, QPoint(), QRegion(), QWidget::DrawChildren);
}

void DNotificationWidget::Private::slotTimeLineChanged(qreal value)
{
    q->setFixedHeight(qMin(value * 2, qreal(1.0)) * content->height());
    q->update();
}

void DNotificationWidget::Private::slotTimeLineFinished()
{
    if (timeLine->direction() == QTimeLine::Forward)
    {
        // Show
        // We set the whole geometry here, because it may be wrong if a
        // DNotificationWidget is shown right when the toplevel window is created.
        content->setGeometry(0, 0, q->width(), bestContentHeight());

        // notify about finished animation
        emit q->showAnimationFinished();
    }
    else
    {
        // hide and notify about finished animation
        q->hide();
        emit q->hideAnimationFinished();
    }
}

int DNotificationWidget::Private::bestContentHeight() const
{
    int height = content->heightForWidth(q->width());

    if (height == -1)
    {
        height = content->sizeHint().height();
    }

    return height;
}

} // namespace Digikam
