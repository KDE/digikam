/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-11-03
 * Description : Calendar month image selection widget.
 *
 * Copyright (C) 2003-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2006      by Tom Albers <tomalbers@kde.nl>
 * Copyright (C) 2007-2008 by Orgad Shaneh <orgads at gmail dot com>
 * Copyright (C) 2012      by Angelo Naselli <anaselli at linux dot it>
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

#include "calmonthwidget.h"

// Qt includes

#include <QDragEnterEvent>
#include <QFileInfo>
#include <QImageReader>
#include <QMatrix>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QPixmap>
#include <QStandardPaths>
#include <QMimeData>
#include <QUrl>

// KDE includes

#include <kcalendarsystem.h>
#include <klocalizedstring.h>

// Local includes

#include "calsettings.h"
#include "imagedialog.h"
#include "digikam_debug.h"

namespace Digikam
{

CalMonthWidget::CalMonthWidget(QWidget* const parent, int month)
    : QPushButton(parent),
      thumbSize(64, 64)
{
    setAcceptDrops(true);
    setFixedSize(QSize(74, 94));
    month_     = month;
    imagePath_ = QUrl();
    setThumb(QPixmap(QIcon::fromTheme(QString::fromLatin1("image-x-generic"))
             .pixmap(32, QIcon::Disabled)));

    thumbLoadThread_ = ThumbnailLoadThread::defaultThread();

    connect(thumbLoadThread_, SIGNAL(signalThumbnailLoaded(LoadingDescription,QPixmap)),
            this, SLOT(slotThumbnail(LoadingDescription,QPixmap)));

    connect(this, SIGNAL(pressed()), 
            this, SLOT(slotMonthSelected()));
}

CalMonthWidget::~CalMonthWidget()
{
}

QUrl CalMonthWidget::imagePath() const
{
    return imagePath_;
}

void CalMonthWidget::paintEvent(QPaintEvent* event)
{
    QRect cr;

    QPushButton::paintEvent(event);
    QPainter painter(this);
    QString name = KLocale::global()->calendar()->monthName(
                   month_, CalSettings::instance()->year(), KCalendarSystem::ShortName);

    cr = contentsRect();
    cr.setBottom(70);
    painter.drawPixmap(cr.width()  / 2 - thumb_.width()  / 2,
                       cr.height() / 2 - thumb_.height() / 2,
                       thumb_);

    cr = contentsRect();
    cr.setTop(70);
    painter.drawText(cr, Qt::AlignHCenter, name);
}

void CalMonthWidget::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasImage())
    {
        event->acceptProposedAction();
    }
}

QPixmap CalMonthWidget::thumb() const
{
    return thumb_;
}

int CalMonthWidget::month()
{
  return month_;
}

void CalMonthWidget::setThumb(const QPixmap& pic)
{
    thumb_ = pic.scaled(thumbSize, Qt::KeepAspectRatio);
    update();
}

void CalMonthWidget::setImage(const QUrl& url)
{
    if (!url.isValid())
    {
        return;
    }

    imagePath_ = url;
    CalSettings::instance()->setImage(month_, imagePath_);

    thumbLoadThread_->find(ThumbnailIdentifier(url.toLocalFile()), thumbSize.width());
}

void CalMonthWidget::dropEvent(QDropEvent* event)
{
    QList<QUrl> srcURLs = event->mimeData()->urls();

    if (srcURLs.isEmpty())
    {
        return;
    }

    QUrl url = srcURLs.first();
    setImage(url);
}

void CalMonthWidget::slotMonthSelected()
{
    emit monthSelected(month_);
}

void CalMonthWidget::slotThumbnail(const LoadingDescription& desc, const QPixmap& pix)
{
    if (QUrl::fromLocalFile(desc.filePath) != imagePath_)
    {
        return;
    }

    setThumb(pix);
}

void CalMonthWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if (!contentsRect().contains(event->pos()))
    {
        return;
    }

    if (event->button() == Qt::LeftButton)
    {
        ImageDialog dlg(this, 
                        QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::PicturesLocation)),
                        true);
        setImage(dlg.url());
    }
    else if (event->button() == Qt::RightButton)
    {
        imagePath_ = QUrl();
        CalSettings::instance()->setImage(month_, imagePath_);
        setThumb(QPixmap(QIcon::fromTheme(QString::fromLatin1("image-x-generic")).pixmap(32, QIcon::Disabled)));
    }
}

}  // Namespace Digikam
