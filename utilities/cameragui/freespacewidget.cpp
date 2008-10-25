/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-08-31
 * Description : a widget to display free space for a mount-point.
 *
 * Copyright (C) 2007-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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


#include "freespacewidget.h"
#include "freespacewidget.moc"

// Qt includes.

#include <QPainter>
#include <QPixmap>
#include <QPalette>
#include <QColor>
#include <QTimer>
#include <QFont>
#include <QBoxLayout>
#include <QFontMetrics>

// KDE includes.

#include <kdebug.h>
#include <kurl.h>
#include <klocale.h>
#include <kio/global.h>
#include <kiconloader.h>

// Local includes.

#include "albumsettings.h"

namespace Digikam
{

class FreeSpaceWidgetPriv
{
public:

    FreeSpaceWidgetPriv()
    {
        timer       = 0;
        isValid     = false;
        kBSize      = 0;
        kBUsed      = 0;
        kBAvail     = 0;
        dSizeKb     = 0;
        percentUsed = 0;
        mode        = FreeSpaceWidget::AlbumLibrary;
    }

    bool                            isValid;

    int                             percentUsed;

    unsigned long                   dSizeKb;
    unsigned long                   kBSize;
    unsigned long                   kBUsed;
    unsigned long                   kBAvail;

    QString                         mountPoint;
    QString                         path;

    QTimer                         *timer;

    QPixmap                         pix;
    QPixmap                         iconPix;

    FreeSpaceWidget::FreeSpaceMode  mode;
};

FreeSpaceWidget::FreeSpaceWidget(QWidget* parent, int width)
               : QWidget(parent)
{
    d = new FreeSpaceWidgetPriv;
    setAttribute(Qt::WA_DeleteOnClose);
    setFixedWidth(width);
    setMaximumHeight(fontMetrics().height()+4);
    d->timer = new QTimer(this);

    connect(d->timer, SIGNAL(timeout()),
            this, SLOT(slotTimeout()));
}

FreeSpaceWidget::~FreeSpaceWidget()
{
    d->timer->stop();
    delete d->timer;
    delete d;
}

void FreeSpaceWidget::setMode(FreeSpaceMode mode)
{
    d->mode = mode;
    if (d->mode == FreeSpaceWidget::AlbumLibrary)
    {
        d->iconPix = SmallIcon("folder-image");
    }
    else if (d->mode == FreeSpaceWidget::UMSCamera)
    {
        d->iconPix = SmallIcon("camera");
    }
    else // GPhotoCamera
    {
        d->iconPix = SmallIcon("camera");
    }
    updatePixmap();
}

void FreeSpaceWidget::setPath(const QString& path)
{
    d->path = path;
    refresh();
}

void FreeSpaceWidget::refresh()
{
    d->timer->stop();
    slotTimeout();
    d->timer->start(10000);
}

void FreeSpaceWidget::setInformations(unsigned long kBSize,
                                      unsigned long kBUsed, unsigned long kBAvail,
                                      const QString& mountPoint)
{
    d->mountPoint = mountPoint;
    d->kBSize     = kBSize;
    d->kBUsed     = kBUsed;
    d->kBAvail    = kBAvail;

    if (kBSize <= 0)
    {
        d->percentUsed = 0;
        d->isValid     = false;
    }
    else
    {
        d->percentUsed = 100 - (int)(100.0*kBAvail/kBSize);
        d->isValid     = true;
    }
    updatePixmap();
    repaint();
}

void FreeSpaceWidget::setEstimatedDSizeKb(unsigned long dSize)
{
    d->dSizeKb = dSize;
    updatePixmap();
    repaint();
}

unsigned long FreeSpaceWidget::estimatedDSizeKb()
{
    return d->dSizeKb;
}

bool FreeSpaceWidget::isValid()
{
    return d->isValid;
}

int FreeSpaceWidget::percentUsed()
{
    return d->percentUsed;
}

unsigned long FreeSpaceWidget::kBSize()
{
    return d->kBSize;
}

unsigned long FreeSpaceWidget::kBUsed()
{
    return d->kBUsed;
}

unsigned long FreeSpaceWidget::kBAvail()
{
    return d->kBAvail;
}

QString FreeSpaceWidget::mountPoint()
{
    return d->mountPoint;
}

void FreeSpaceWidget::updatePixmap()
{
    d->pix = QPixmap(size());
    d->pix.fill(palette().background().color());

    QPainter p(&d->pix);
    p.setPen(palette().mid().color());
    p.drawRect(0, 0, d->pix.width()-1, d->pix.height()-1);
    p.drawPixmap(2, d->pix.height()/2-d->iconPix.height()/2,
                 d->iconPix, 0, 0, d->iconPix.width(), d->iconPix.height());

    if (isValid())
    {
        // We will compute the estimated % of space size used to download and process.
        unsigned long eUsedKb = d->dSizeKb + d->kBUsed;
        int peUsed            = (int)(100.0*((double)eUsedKb/(double)d->kBSize));
        int pClamp            = peUsed > 100 ? 100 : peUsed;
        p.setBrush(peUsed > 95 ? Qt::red : Qt::darkGreen);
        p.setPen(Qt::white);
        QRect gRect(d->iconPix.height()+2, 1,
                    (int)(((double)d->pix.width()-3.0-d->iconPix.width()-2.0)*(pClamp/100.0)),
                    d->pix.height()-3);
        p.drawRect(gRect);

        QRect tRect(d->iconPix.height()+2, 1, d->pix.width()-3-d->iconPix.width()-2, d->pix.height()-3);
        QString text        = QString("%1%").arg(peUsed);
        QFontMetrics fontMt = p.fontMetrics();
        QRect fontRect      = fontMt.boundingRect(tRect.x(), tRect.y(),
                                                tRect.width(), tRect.height(), 0, text);
        p.setPen(palette().text().color());
        p.drawText(tRect, Qt::AlignCenter, text);

        QString tipText, value;
        QString header = i18n("Camera Media");
        if (d->mode == FreeSpaceWidget::AlbumLibrary) header = i18n("Album Library");
        QString headBeg("<tr bgcolor=\"#73CAE6\"><td colspan=\"2\">"
                        "<nobr><font size=\"-1\" color=\"black\"><b>");
        QString headEnd("</b></font></nobr></td></tr>");
        QString cellBeg("<tr><td><nobr><font size=-1>");
        QString cellMid("</font></nobr></td><td><nobr><font size=-1>");
        QString cellEnd("</font></nobr></td></tr>");
        tipText  = "<table cellspacing=0 cellpadding=0>";
        tipText += headBeg + header + headEnd;

        if (d->dSizeKb > 0)
        {
            tipText += cellBeg + i18n("Capacity:") + cellMid;
            tipText += KIO::convertSizeFromKiB(d->kBSize) + cellEnd;

            tipText += cellBeg + i18n("Available:") + cellMid;
            tipText += KIO::convertSizeFromKiB(d->kBAvail) + cellEnd;

            tipText += cellBeg + i18n("Require:") + cellMid;
            tipText += KIO::convertSizeFromKiB(d->dSizeKb) + cellEnd;
        }
        else
        {
            tipText += cellBeg + i18n("Capacity:") + cellMid;
            tipText += KIO::convertSizeFromKiB(d->kBSize) + cellEnd;

            tipText += cellBeg + i18n("Available:") + cellMid;
            tipText += KIO::convertSizeFromKiB(d->kBAvail) + cellEnd;
        }

        tipText += "</table>";

        setWhatsThis(tipText);
        setToolTip(tipText);
    }

    p.end();
}

void FreeSpaceWidget::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.drawPixmap(0, 0, d->pix);
    p.end();
}

#if KDE_IS_VERSION(4,1,68)
void FreeSpaceWidget::slotTimeout()
{
    KDiskFreeSpaceInfo info = KDiskFreeSpaceInfo::freeSpaceInfo(d->path);
    if(info.isValid())
    {
        setInformations(info.size(), info.used(), info.available(), info.mountPoint());
    }
}
#else

void FreeSpaceWidget::slotTimeout()
{
    KMountPoint::List list = KMountPoint::currentMountPoints();
    KMountPoint::Ptr mp    = list.findByPath(d->path);
    if (mp)
    {
        KDiskFreeSpace *job = new KDiskFreeSpace;
        connect(job, SIGNAL(foundMountPoint(QString, quint64, quint64, quint64)),
                this, SLOT(slotAvailableFreeSpace(QString, quint64, quint64, quint64)));
        job->readDF(mp->mountPoint());
    }
}
#endif /* KDE_IS_VERSION(4,1,68) */

void FreeSpaceWidget::slotAvailableFreeSpace(QString mountPoint, quint64 kBSize,
                                             quint64 kBUsed, quint64 kBAvail)
{
#if KDE_IS_VERSION(4,1,68)
    setInformations(kBSize, kBUsed, kBAvail, mountPoint);
#else
    Q_UNUSED(mountPoint);
    Q_UNUSED(kBSize);
    Q_UNUSED(kBUsed);
    Q_UNUSED(kBAvail);
#endif /* KDE_IS_VERSION(4,1,68) */
}

}  // namespace Digikam
