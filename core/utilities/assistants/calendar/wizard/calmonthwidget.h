/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-11-03
 * Description : Calendar month image selection widget.
 *
 * Copyright (C) 2003-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2006      by Tom Albers <tomalbers at kde dot nl>
 * Copyright (C) 2007-2008 by Orgad Shaneh <orgads at gmail dot com>
 * Copyright (C) 2012      by Angelo Naselli <anaselli at linux dot it>
 * Copyright (C) 2012-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef CAL_MONTH_WIDGET_H
#define CAL_MONTH_WIDGET_H

// Qt includes

#include <QPixmap>
#include <QPushButton>
#include <QUrl>

// Local includes

#include "loadingdescription.h"
#include "thumbnailloadthread.h"

class QDragEnterEvent;
class QDropEvent;
class QMouseEvent;
class QPaintEvent;

namespace Digikam
{

class CalMonthWidget : public QPushButton
{
    Q_OBJECT

public:

    explicit CalMonthWidget(QWidget* const parent, int month);
    ~CalMonthWidget();

    QUrl imagePath() const;
    void setImage(const QUrl& url);
    int  month();

Q_SIGNALS:

    void monthSelected(int);

protected:

    void dragEnterEvent(QDragEnterEvent* e);
    void dropEvent(QDropEvent* e);
    void paintEvent(QPaintEvent* e);
    void mouseReleaseEvent(QMouseEvent* e);

private Q_SLOTS:

    void slotThumbnail(const LoadingDescription&, const QPixmap&);
    void slotMonthSelected();

private:

    QPixmap thumb() const;
    void    setThumb(const QPixmap& pic);

    Q_PROPERTY(QPixmap thumb READ thumb WRITE setThumb)

private:

    class Private;
    Private* const d;
};

} // Namespace Digikam

#endif // CAL_MONTH_WIDGET_H
