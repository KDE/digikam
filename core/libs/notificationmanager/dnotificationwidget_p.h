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

#ifndef DNOTIFICATION_WIDGET_P_H
#define DNOTIFICATION_WIDGET_P_H

// Qt includes

#include <QFrame>
#include <QObject>
#include <QLabel>
#include <QToolButton>
#include <QTimeLine>
#include <QPixmap>

// Local includes

#include "dnotificationwidget.h"

namespace Digikam
{

class DNotificationWidget::Private : public QObject
{
    Q_OBJECT

public:

    explicit Private(DNotificationWidget* const);
    virtual ~Private();

    void init();
    void createLayout();
    void updateSnapShot();
    void updateLayout();
    int  bestContentHeight() const;

public:

    DNotificationWidget*             q;
    QFrame*                          content;
    QLabel*                          iconLabel;
    QLabel*                          textLabel;
    QToolButton*                     closeButton;
    QTimeLine*                       timeLine;
    QIcon                            icon;

    DNotificationWidget::MessageType messageType;
    bool                             wordWrap;
    QList<QToolButton*>              buttons;
    QPixmap                          contentSnapShot;

private Q_SLOTS:

    void slotTimeLineChanged(qreal);
    void slotTimeLineFinished();
};

} // namespace Digikam

#endif // DNOTIFICATION_WIDGET_P_H
