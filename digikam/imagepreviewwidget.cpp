/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2006-06-13
 * Description : a widget to display an image preview
 *
 * Copyright 2006 Gilles Caulier
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

#include <qpainter.h>
#include <qimage.h>
#include <qfileinfo.h>
#include <qpixmap.h>
#include <qrect.h>
#include <qtimer.h>
#include <qguardedptr.h>

// KDE include.

#include <kcursor.h>
#include <kprocess.h>
#include <kdebug.h>
#include <klocale.h>

// Local includes.

#include "themeengine.h"
#include "albumsettings.h"
#include "imagepreviewjob.h"
#include "imagepreviewwidget.h"
#include "imagepreviewwidget.moc"

namespace Digikam
{

class ImagePreviewWidgetPriv
{
public:

    ImagePreviewWidgetPriv()
    {
        previewJob        = 0;
    }

    QString                       path;

    QPixmap                       pixmap;

    QImage                        preview;
    
    QGuardedPtr<ImagePreviewJob>  previewJob;
};

ImagePreviewWidget::ImagePreviewWidget(QWidget *parent)
                  : QFrame(parent, 0, Qt::WDestructiveClose)
{
    d = new ImagePreviewWidgetPriv;
    setBackgroundMode(Qt::NoBackground);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setFocusPolicy(QWidget::StrongFocus);
    setFrameStyle(QFrame::NoFrame);
    setMargin(0);
    setLineWidth(0);
    
    // ---------------------------------------------------------------
            
    connect(ThemeEngine::instance(), SIGNAL(signalThemeChanged()),
            this, SLOT(slotThemeChanged()));
}

ImagePreviewWidget::~ImagePreviewWidget()
{
    if (!d->previewJob.isNull())
    {
        d->previewJob->kill();
        d->previewJob = 0;
    }

    delete d;
}

void ImagePreviewWidget::setImagePath( const QString& path )
{
    if (path == d->path) return;

    setCursor( KCursor::waitCursor() );
    d->path = path;

    if (d->path.isEmpty())
	slotFailedImagePreview(KURL());

    if (!d->previewJob.isNull())
    {
        d->previewJob->kill();
        d->previewJob = 0;
    }

    d->previewJob = new ImagePreviewJob(KURL(path), 1024, AlbumSettings::instance()->getExifRotate());

    connect(d->previewJob, SIGNAL(signalImagePreview(const KURL&, const QImage&)),
            this, SLOT(slotGotImagePreview(const KURL&, const QImage&)));

    connect(d->previewJob, SIGNAL(signalFailed(const KURL&)),
            this, SLOT(slotFailedImagePreview(const KURL&)));   

    emit previewStarted();
}

void ImagePreviewWidget::slotGotImagePreview(const KURL&, const QImage& preview)
{
    d->preview = preview;
    d->pixmap  = QPixmap(contentsRect().size());

    // It is very important to kill the thumbnail job properly
    // so that is frees its shared memory. Otherwise the memory
    // will _never_ be freed, see b.k.o. #131277
    if (!d->previewJob.isNull())
    {
        d->previewJob->kill();
        d->previewJob = 0;
    }

    updatePixmap();
    repaint(false);
    unsetCursor();
    emit previewComplete();
}

void ImagePreviewWidget::slotFailedImagePreview(const KURL&)
{
    if (!d->previewJob.isNull())
    {
        d->previewJob->kill();
        d->previewJob = 0;
    }

    d->preview = QImage();
    d->pixmap  = QPixmap(contentsRect().size());
    updatePixmap();
    repaint(false);
    unsetCursor();
    emit previewFailed();
}

void ImagePreviewWidget::updatePixmap( void )
{
    d->pixmap.fill(ThemeEngine::instance()->baseColor());
    QPainter p(&(d->pixmap));

    if (!d->path.isEmpty())
    {
        // Preview extraction is complete...
        
        if (!d->preview.isNull())
        {
            QPixmap pix(d->preview.smoothScale(contentsRect().size(), QImage::ScaleMin));
            p.drawPixmap((contentsRect().width()-pix.width())/2,
                            (contentsRect().height()-pix.height())/2, pix,
                            0, 0, pix.width(), pix.height());
        }
        else
        {
            // ...or failed...
            QFileInfo info(d->path);
            p.setPen(QPen(ThemeEngine::instance()->textRegColor()));
            p.drawText(0, 0, d->pixmap.width(), d->pixmap.height(),
                        Qt::AlignCenter|Qt::WordBreak, 
                        i18n("Cannot display preview for\n\"%1\"")
                        .arg(info.fileName()));
        }
    }
    else
    {
        // There is nothing to see.
        
        p.setPen(QPen(ThemeEngine::instance()->textRegColor()));
        p.drawText(0, 0, d->pixmap.width(), d->pixmap.height(),
                    Qt::AlignCenter|Qt::WordBreak, 
                    i18n("No item to preview in this album."));
    }
    
    p.end();
}

void ImagePreviewWidget::drawContents(QPainter *)
{
    bitBlt(this, contentsRect().topLeft(), &(d->pixmap), contentsRect(), Qt::CopyROP);
}

void ImagePreviewWidget::slotThemeChanged()
{
    updatePixmap();
    repaint(false);
}

void ImagePreviewWidget::resizeEvent(QResizeEvent *)
{
    blockSignals(true);
    d->pixmap = QPixmap(contentsRect().size());
    updatePixmap();
    repaint(false);
    blockSignals(false);
}

void ImagePreviewWidget::wheelEvent( QWheelEvent * e )
{
    if (e->delta() > 0)
        emit signalPrevItem();

    if (e->delta() < 0)
        emit signalNextItem();
}

}  // NameSpace Digikam
