/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date   : 2006-06-13
 * Description : a widget to display an image preview
 *
 * Copyright 2006-2007 Gilles Caulier
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
#include <klocale.h>

// Local includes.

#include "ddebug.h"
#include "previewloadthread.h"
#include "themeengine.h"
#include "albumsettings.h"
#include "imagepreviewwidget.h"
#include "imagepreviewwidget.moc"

namespace Digikam
{

class ImagePreviewWidgetPriv
{
public:

    ImagePreviewWidgetPriv()
    {
        previewThread        = 0;
        previewPreloadThread = 0;
    }

    QString            path;
    QString            nextPath;
    QString            previousPath;

    QPixmap            pixmap;

    QImage             preview;

    PreviewLoadThread *previewThread;
    PreviewLoadThread *previewPreloadThread;
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
    delete d->previewThread;
    delete d->previewPreloadThread;
    delete d;
}

void ImagePreviewWidget::reload()
{
    // cache is cleaned from AlbumIconView::refreshItems
    setImagePath(d->path);
}

void ImagePreviewWidget::setImagePath(const QString& path)
{
    setCursor( KCursor::waitCursor() );
    d->path = path;

    d->nextPath     = QString();
    d->previousPath = QString();

    if (d->path.isEmpty())
    {
        d->pixmap = QPixmap(contentsRect().size());

        updatePixmap();
        update();
        unsetCursor();
        return;
    }

    if (!d->previewThread)
    {
        d->previewThread = new PreviewLoadThread();
        connect(d->previewThread, SIGNAL(signalPreviewLoaded(const LoadingDescription &, const QImage &)),
                this, SLOT(slotGotImagePreview(const LoadingDescription &, const QImage&)));
    }
    if (!d->previewPreloadThread)
    {
        d->previewPreloadThread = new PreviewLoadThread();
        connect(d->previewPreloadThread, SIGNAL(signalPreviewLoaded(const LoadingDescription &, const QImage &)),
                this, SLOT(slotNextPreload()));
    }

    d->previewThread->load(LoadingDescription(path, 1024, AlbumSettings::instance()->getExifRotate()));

    emit signalPreviewStarted();
}

void ImagePreviewWidget::setPreviousNextPaths(const QString& previous, const QString &next)
{
    d->nextPath     = next;
    d->previousPath = previous;
}

void ImagePreviewWidget::slotGotImagePreview(const LoadingDescription &description, const QImage& preview)
{
    if (description.filePath != d->path)
        return;

    d->preview = preview;
    d->pixmap  = QPixmap(contentsRect().size());

    updatePixmap();
    update();
    unsetCursor();

    if (preview.isNull())
        emit signalPreviewFailed();
    else
        emit signalPreviewComplete();

    slotNextPreload();
}

void ImagePreviewWidget::slotNextPreload()
{
    QString loadPath;
    if (!d->nextPath.isNull())
    {
        loadPath = d->nextPath;
        d->nextPath = QString();
    }
    else if (!d->previousPath.isNull())
    {
        loadPath = d->previousPath;
        d->previousPath = QString();
    }
    else
        return;

    d->previewPreloadThread->load(LoadingDescription(loadPath, 1024, AlbumSettings::instance()->getExifRotate()));
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
        // There is nothing to see: Empty album, or initially waiting to load preview

        /*
        p.setPen(QPen(ThemeEngine::instance()->textRegColor()));
        p.drawText(0, 0, d->pixmap.width(), d->pixmap.height(),
                    Qt::AlignCenter|Qt::WordBreak, 
                    i18n("No item to preview in this album."));
        */
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
