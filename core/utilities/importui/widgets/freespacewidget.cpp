/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-08-31
 * Description : a widget to display free space for a mount-point.
 *
 * Copyright (C) 2007-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// C++ includes

#include <cmath>

// Qt includes

#include <QPainter>
#include <QPixmap>
#include <QPalette>
#include <QColor>
#include <QTimer>
#include <QFont>
#include <QBoxLayout>
#include <QFontMetrics>
#include <QUrl>
#include <QIcon>
#include <QStyle>
#include <QStorageInfo>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "freespacetooltip.h"
#include "applicationsettings.h"
#include "imagepropertiestab.h"

namespace Digikam
{

class MountPointInfo
{
public:

    MountPointInfo()
    {
        isValid = false;
        kBSize  = 0;
        kBUsed  = 0;
        kBAvail = 0;
    }

    bool          isValid;

    unsigned long kBSize;
    unsigned long kBUsed;
    unsigned long kBAvail;

    QString       mountPoint;
};

// ---------------------------------------------------------------------------------

class FreeSpaceWidget::Private
{
public:

    Private() :
        isValid(false),
        percentUsed(-1),
        dSizeKb(0),
        kBSize(0),
        kBUsed(0),
        kBAvail(0),
        timer(0),
        toolTip(0),
        mode(FreeSpaceWidget::AlbumLibrary)
    {
    }

    bool                            isValid;

    int                             percentUsed;

    unsigned long                   dSizeKb;
    unsigned long                   kBSize;
    unsigned long                   kBUsed;
    unsigned long                   kBAvail;

    QStringList                     paths;
    QHash<QString, MountPointInfo>  infos;

    QTimer*                         timer;

    QPixmap                         iconPix;

    FreeSpaceToolTip*               toolTip;

    FreeSpaceWidget::FreeSpaceMode  mode;
};

FreeSpaceWidget::FreeSpaceWidget(QWidget* const parent, int width)
    : QWidget(parent),
      d(new Private)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setFixedWidth(width);
    setMaximumHeight(fontMetrics().height() + 4);
    d->timer   = new QTimer(this);
    d->toolTip = new FreeSpaceToolTip(this);

    connect(d->timer, SIGNAL(timeout()),
            this, SLOT(slotTimeout()));
}

FreeSpaceWidget::~FreeSpaceWidget()
{
    d->timer->stop();
    delete d->timer;
    delete d->toolTip;
    delete d;
}

void FreeSpaceWidget::setMode(FreeSpaceMode mode)
{
    d->mode = mode;

    if (d->mode == FreeSpaceWidget::AlbumLibrary)
    {
        d->iconPix = QIcon::fromTheme(QLatin1String("folder-pictures")).pixmap(style()->pixelMetric(QStyle::PM_SmallIconSize));
    }
    else
    {
        d->iconPix = QIcon::fromTheme(QLatin1String("camera-photo")).pixmap(style()->pixelMetric(QStyle::PM_SmallIconSize));
    }

    update();
}

void FreeSpaceWidget::setPath(const QString& path)
{
    d->paths.clear();
    d->paths << path;
    refresh();
}

void FreeSpaceWidget::setPaths(const QStringList& paths)
{
    d->paths = paths;
    refresh();
}

void FreeSpaceWidget::refresh()
{
    d->timer->stop();
    slotTimeout();
    d->timer->start(10000);
}

void FreeSpaceWidget::addInformation(unsigned long kBSize,
                                     unsigned long kBUsed, unsigned long kBAvail,
                                     const QString& mountPoint)
{
    MountPointInfo info;

    info.mountPoint = mountPoint;
    info.kBSize     = kBSize;
    info.kBUsed     = kBUsed;
    info.kBAvail    = kBAvail;
    info.isValid    = (kBSize > 0);

    d->infos[mountPoint] = info;

    // update cumulative data

    d->kBSize      = 0;
    d->kBUsed      = 0;
    d->kBAvail     = 0;
    d->isValid     = false;
    d->percentUsed = -1;

    foreach(const MountPointInfo& info, d->infos)
    {
        if (info.isValid)
        {
            d->kBSize  += info.kBSize;
            d->kBUsed  += info.kBUsed;
            d->kBAvail += info.kBAvail;
            d->isValid  = true;
        }
    }

    if (kBSize > 0)
        d->percentUsed = lround(100.0 - (100.0 * kBAvail / kBSize));

    updateToolTip();
    update();
}

void FreeSpaceWidget::setEstimatedDSizeKb(unsigned long dSize)
{
    d->dSizeKb = dSize;

    updateToolTip();
    update();
}

unsigned long FreeSpaceWidget::estimatedDSizeKb() const
{
    return d->dSizeKb;
}

bool FreeSpaceWidget::isValid() const
{
    return d->isValid;
}

int FreeSpaceWidget::percentUsed() const
{
    return d->percentUsed;
}

unsigned long FreeSpaceWidget::kBSize() const
{
    return d->kBSize;
}

unsigned long FreeSpaceWidget::kBUsed() const
{
    return d->kBUsed;
}

unsigned long FreeSpaceWidget::kBAvail() const
{
    return d->kBAvail;
}

unsigned long FreeSpaceWidget::kBAvail(const QString& path) const
{
    int mountPointMatch = 0;
    MountPointInfo selectedInfo;

    foreach(const MountPointInfo& info, d->infos)
    {
        if (info.isValid && !info.mountPoint.isEmpty() && path.startsWith(info.mountPoint))
        {
            int length = info.mountPoint.length();

            if (length > mountPointMatch)
            {
                mountPointMatch = info.mountPoint.length();
                selectedInfo    = info;
            }
        }
    }

    if (!mountPointMatch)
    {
        qCWarning(DIGIKAM_IMPORTUI_LOG) << "Did not identify a valid mount point for" << path;
        return (unsigned long)(-1);
    }

    return selectedInfo.kBAvail;
}

void FreeSpaceWidget::paintEvent(QPaintEvent*)
{
    QPainter p(this);

    p.setPen(palette().mid().color());
    p.drawRect(0, 0, width() - 1, height() - 1);
    p.drawPixmap(2, height() / 2 - d->iconPix.height() / 2,
                 d->iconPix, 0, 0, d->iconPix.width(), d->iconPix.height());

    if (isValid())
    {
        // We will compute the estimated % of space size used to download and process.
        unsigned long eUsedKb = d->dSizeKb + d->kBUsed;
        int peUsed            = (int)(100.0 * ((double)eUsedKb / (double)d->kBSize));
        int pClamp            = (peUsed > 100) ? 100 : peUsed;
        QColor barcol         = QColor(62, 255, 62);          // Smooth Green.

        if (peUsed > 80)
        {
            barcol = QColor(240, 255, 62);   // Smooth Yellow.
        }

        if (peUsed > 95)
        {
            barcol = QColor(255, 62, 62);    // Smooth Red.
        }

        p.setBrush(barcol);
        p.setPen(palette().light().color());
        QRect gRect(d->iconPix.height() + 3, 2,
                    (int)(((double)width() - 3.0 - d->iconPix.width() - 2.0) * (pClamp / 100.0)),
                    height() - 5);
        p.drawRect(gRect);

        QRect tRect(d->iconPix.height() + 3, 2, width() - 3 - d->iconPix.width() - 2, height() - 5);
        QString text        = QString::fromUtf8("%1%").arg(peUsed);
        QFontMetrics fontMt = p.fontMetrics();
        //        QRect fontRect      = fontMt.boundingRect(tRect.x(), tRect.y(),
        //                                                  tRect.width(), tRect.height(), 0, text);
        p.setPen(Qt::black);
        p.drawText(tRect, Qt::AlignCenter, text);
    }
}

void FreeSpaceWidget::updateToolTip()
{
    if (isValid())
    {
        QString value;
        QString header = i18n("Camera Media");

        if (d->mode == FreeSpaceWidget::AlbumLibrary)
        {
            header = i18n("Album Library");
        }

        DToolTipStyleSheet cnt(ApplicationSettings::instance()->getToolTipsFont());
        QString tip = cnt.tipHeader;

        tip += cnt.headBeg + header + cnt.headEnd;

        if (d->dSizeKb > 0)
        {
            tip += cnt.cellBeg + i18nc("@info Storage", "Capacity:") + cnt.cellMid;
            tip += ImagePropertiesTab::humanReadableBytesCount(d->kBSize * 1024) + cnt.cellEnd;

            tip += cnt.cellBeg + i18nc("@info Storage", "Available:") + cnt.cellMid;
            tip += ImagePropertiesTab::humanReadableBytesCount(d->kBAvail * 1024) + cnt.cellEnd;

            tip += cnt.cellBeg + i18nc("@info Storage", "Require:") + cnt.cellMid;
            tip += ImagePropertiesTab::humanReadableBytesCount(d->dSizeKb * 1024) + cnt.cellEnd;
        }
        else
        {
            tip += cnt.cellBeg + i18nc("@info Storage", "Capacity:") + cnt.cellMid;
            tip += ImagePropertiesTab::humanReadableBytesCount(d->kBSize * 1024) + cnt.cellEnd;

            tip += cnt.cellBeg + i18nc("@info Storage", "Available:") + cnt.cellMid;
            tip += ImagePropertiesTab::humanReadableBytesCount(d->kBAvail * 1024) + cnt.cellEnd;
        }

        tip += cnt.tipFooter;

        d->toolTip->setToolTip(tip);
    }
    else
    {
        d->toolTip->setToolTip(QString());
    }
}

void FreeSpaceWidget::enterEvent(QEvent* e)
{
    Q_UNUSED(e)
    d->toolTip->show();
}

void FreeSpaceWidget::leaveEvent(QEvent* e)
{
    Q_UNUSED(e)
    d->toolTip->hide();
}

void FreeSpaceWidget::slotTimeout()
{
    foreach(const QString& path, d->paths)
    {
        QStorageInfo info(path);

        if (info.isValid())
        {
            addInformation((unsigned long)(info.bytesTotal()                         / 1024.0),
                           (unsigned long)((info.bytesTotal()-info.bytesAvailable()) / 1024.0),
                           (unsigned long)(info.bytesAvailable()                     / 1024.0),
                           info.rootPath());
        }
    }
}

}  // namespace Digikam
