/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-08-31
 * Description : a widget to display free space for a mount-point.
 * 
 * Copyright (C) 2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <kurl.h>
#include <ktoolbar.h>
#include <klocale.h>
#include <kdiskfreespace.h>
#include <kmountpoint.h>
#include <kio/global.h>
#include <kiconloader.h>

// Local includes.

#include "ddebug.h"
#include "albumsettings.h"
#include "freespacewidget.h"
#include "freespacewidget.moc"

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
    }

    bool           isValid;
    
    int            percentUsed;

    unsigned long  dSizeKb;
    unsigned long  kBSize;
    unsigned long  kBUsed;
    unsigned long  kBAvail;
    
    QString        mountPoint;

    QTimer        *timer;
    
    QPixmap        pix;
};

FreeSpaceWidget::FreeSpaceWidget(QWidget* parent, int width)
               : QWidget(parent)
{
    d = new FreeSpaceWidgetPriv;
    setAttribute(Qt::WA_DeleteOnClose);
    setFixedWidth(width);
    setMaximumHeight(fontMetrics().height()+4);
    slotTimeout();
    
    d->timer = new QTimer(this);
    
    connect(d->timer, SIGNAL(timeout()),
            this, SLOT(slotTimeout()));

    d->timer->start(10000);
}

FreeSpaceWidget::~FreeSpaceWidget()
{
    d->timer->stop();
    delete d->timer;
    delete d;
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
    QPixmap fimgPix = SmallIcon("folder-image");
    d->pix          = QPixmap(size());
    d->pix.fill(palette().background().color());

    QPainter p(&d->pix);
    p.setPen(palette().mid().color());
    p.drawRect(0, 0, d->pix.width()-1, d->pix.height()-1);
    p.drawPixmap(2, d->pix.height()/2-fimgPix.height()/2, 
                 fimgPix, 0, 0, fimgPix.width(), fimgPix.height());

    if (isValid())
    {
        // We will compute the estimated % of space size used to download and process.
        unsigned long eUsedKb = d->dSizeKb + d->kBUsed;
        int peUsed            = (int)(100.0*((double)eUsedKb/(double)d->kBSize));
        int pClamp            = peUsed > 100 ? 100 : peUsed;
        p.setBrush(peUsed > 95 ? Qt::red : Qt::darkGreen);
        p.setPen(Qt::white);
        QRect gRect(fimgPix.height()+2, 1, 
                    (int)(((double)d->pix.width()-3.0-fimgPix.width()-2.0)*(pClamp/100.0)), 
                    d->pix.height()-3);
        p.drawRect(gRect);

        QRect tRect(fimgPix.height()+2, 1, d->pix.width()-3-fimgPix.width()-2, d->pix.height()-3);
        QString text = QString("%1%").arg(peUsed);
        p.setPen(palette().text().color());
        QFontMetrics fontMt = p.fontMetrics();
        QRect fontRect      = fontMt.boundingRect(tRect.x(), tRect.y(), 
                                                  tRect.width(), tRect.height(), 0, text);
        p.drawText(tRect, Qt::AlignCenter, text);

        QString info = i18n("<p>Capacity: <b>%1</b>"
                            "<p>Available: <b>%2</b>"
                            "<p>Require: <b>%3</b>",
                            KIO::convertSizeFromKiB(d->kBSize),
                            KIO::convertSizeFromKiB(d->kBAvail),
                            KIO::convertSizeFromKiB(d->dSizeKb));
        setWhatsThis(info);
        setToolTip(info);
    }
    
    p.end();
}    

void FreeSpaceWidget::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.drawPixmap(0, 0, d->pix);
    p.end();
}

void FreeSpaceWidget::slotTimeout()
{
    QString path           = AlbumSettings::instance()->getAlbumLibraryPath();
    KMountPoint::List list = KMountPoint::currentMountPoints();
    KMountPoint::Ptr mp    = list.findByPath(path);
    if (mp)
    {
        KDiskFreeSpace *job = new KDiskFreeSpace;
        connect(job, SIGNAL(foundMountPoint(QString, quint64, quint64, quint64)),
                this, SLOT(slotAvailableFreeSpace(QString, quint64, quint64, quint64)));
        job->readDF(mp->mountPoint());
    }
}

void FreeSpaceWidget::slotAvailableFreeSpace(QString mountPoint, quint64 kBSize, 
                                             quint64 kBUsed, quint64 kBAvail)
{
    d->mountPoint  = mountPoint;
    d->kBSize      = kBSize;
    d->kBUsed      = kBUsed;
    d->kBAvail     = kBAvail;
    d->percentUsed = 100 - (int)(100.0*kBAvail/kBSize);
    d->isValid     = true;
    updatePixmap();
    repaint();
}

// -----------------------------------------------------------------------------------

FreeSpaceAction::FreeSpaceAction(QObject* parent)
               : KAction(parent)
{
}

QWidget* FreeSpaceAction::createWidget( QWidget * parent )
{
    QToolBar *bar = qobject_cast<QToolBar*>(parent);
    
    // This action should only be used in a toolbar
    Q_ASSERT(bar != NULL);
    
    QWidget* container  = new QWidget(parent);
    QHBoxLayout* layout = new QHBoxLayout(container);
    m_freeSpaceWidget   = new FreeSpaceWidget(bar, 100);
    
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addStretch();
    layout->addWidget(m_freeSpaceWidget);
    
    return container;
}

FreeSpaceWidget* FreeSpaceAction::freeSpaceWidget() const
{
    return m_freeSpaceWidget;
}

}  // namespace Digikam
