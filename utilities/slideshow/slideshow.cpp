/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date   : 2005-04-21
 * Description : slide show tool using preview of pictures.
 * 
 * Copyright 2005-2007 by Gilles Caulier
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

#include <qtimer.h>
#include <qpixmap.h>
#include <qdesktopwidget.h>
#include <qevent.h>
#include <qcursor.h>
#include <qpainter.h>
#include <qfont.h>

// KDE includes.

#include <klocale.h>
#include <kdeversion.h>
#include <kglobalsettings.h>

// Local includes.

#include "ddebug.h"
#include "toolbar.h"
#include "previewloadthread.h"
#include "slideshow.h"
#include "slideshow.moc"

namespace Digikam
{

class SlideShowPriv
{
public:

    SlideShowPriv()
    {
        previewThread  = 0;
        mouseMoveTimer = 0;
        timer          = 0;
        toolBar        = 0;
        fileIndex      = -1;
        endOfShow      = false;
        exifRotate     = true;
    }

    bool               endOfShow;
    bool               printName;
    bool               loop;
    bool               exifRotate;
    
    int                deskX;
    int                deskY;
    int                deskWidth;
    int                deskHeight;
    int                delay;
    int                fileIndex;

    QTimer            *mouseMoveTimer;  // To hide cursor when not moved.
    QTimer            *timer;

    QPixmap            pixmap;

    QImage             preview;

    KURL               currentImage;

    KURL::List         fileList;

    PreviewLoadThread *previewThread;

    ToolBar           *toolBar;
};  

SlideShow::SlideShow(const KURL::List& fileList, bool exifRotate,
                     int delay, bool printName, bool loop)
         : QWidget(0, 0, WStyle_StaysOnTop | WType_Popup | WX11BypassWM | WDestructiveClose)
{
    d = new SlideShowPriv;

    d->fileList   = fileList;
    d->delay      = QMAX(delay, 300); // at least have 0.3 second delay
    d->loop       = loop;
    d->printName  = printName;
    d->exifRotate = exifRotate;

    // ---------------------------------------------------------------

#if KDE_IS_VERSION(3,2,0)
    QRect deskRect = KGlobalSettings::desktopGeometry(this);
    d->deskX       = deskRect.x();
    d->deskY       = deskRect.y();
    d->deskWidth   = deskRect.width();
    d->deskHeight  = deskRect.height();
#else
    QRect deskRect = QApplication::desktop()->screenGeometry(this);
    d->deskX       = deskRect.x();
    d->deskY       = deskRect.y();
    d->deskWidth   = deskRect.width();
    d->deskHeight  = deskRect.height();
#endif    
    
    move(d->deskX, d->deskY);
    resize(d->deskWidth, d->deskHeight);
    setPaletteBackgroundColor(Qt::black);

    // ---------------------------------------------------------------

    d->toolBar = new ToolBar(this);
    d->toolBar->hide();
    if (!d->loop)
        d->toolBar->setEnabledPrev(false);

    connect(d->toolBar, SIGNAL(signalPause()),
            this, SLOT(slotPause()));

    connect(d->toolBar, SIGNAL(signalPlay()),
            this, SLOT(slotPlay()));

    connect(d->toolBar, SIGNAL(signalNext()),
            this, SLOT(slotNext()));

    connect(d->toolBar, SIGNAL(signalPrev()),
            this, SLOT(slotPrev()));

    connect(d->toolBar, SIGNAL(signalClose()),
            this, SLOT(slotClose()));

    // ---------------------------------------------------------------
    
    d->previewThread  = new PreviewLoadThread();
    d->timer          = new QTimer();
    d->mouseMoveTimer = new QTimer();

    connect(d->previewThread, SIGNAL(signalPreviewLoaded(const LoadingDescription &, const QImage &)),
            this, SLOT(slotGotImagePreview(const LoadingDescription &, const QImage&)));

    connect(d->mouseMoveTimer, SIGNAL(timeout()),
            this, SLOT(slotMouseMoveTimeOut()));

    connect(d->timer, SIGNAL(timeout()), 
            this, SLOT(slotTimeOut()));

    d->timer->start(10, true);
    
    // ---------------------------------------------------------------

    setMouseTracking(true);
    slotMouseMoveTimeOut();
}

SlideShow::~SlideShow()
{
    d->timer->stop();
    d->mouseMoveTimer->stop();

    delete d->timer;
    delete d->mouseMoveTimer;
    delete d->previewThread;
    delete d;
}

void SlideShow::setCurrent(const KURL& url)
{
    int index = d->fileList.findIndex(url);
    if (index != -1)
    {
        d->currentImage = url;
        d->fileIndex    = index-1;
    }        
}

void SlideShow::slotTimeOut()
{
    loadNextImage();
}

void SlideShow::loadNextImage()
{
    d->fileIndex++;
    int num = d->fileList.count();

    if (d->fileIndex >= num)
    {
        if (d->loop)
        {
            d->fileIndex = 0;
        }
    }

    if (!d->loop)
    {
        d->toolBar->setEnabledPrev(d->fileIndex > 0);
        d->toolBar->setEnabledNext(d->fileIndex < num-1);
    }

    d->currentImage = d->fileList[d->fileIndex];
    d->previewThread->load(LoadingDescription(d->currentImage.path(), 1024, d->exifRotate));
}

void SlideShow::loadPrevImage()
{
    d->fileIndex--;
    int num = d->fileList.count();

    if (d->fileIndex < 0)
    {
        if (d->loop)
        {
            d->fileIndex = num-1;
        }
    }

    if (!d->loop)
    {
        d->toolBar->setEnabledPrev(d->fileIndex > 0);
        d->toolBar->setEnabledNext(d->fileIndex < num-1);
    }
    
    d->currentImage = d->fileList[d->fileIndex];
    d->previewThread->load(LoadingDescription(d->currentImage.path(), 1024, d->exifRotate));
}

void SlideShow::slotGotImagePreview(const LoadingDescription&, const QImage& preview)
{
    d->preview = preview;
    d->pixmap  = QPixmap(size());

    updatePixmap();
    update();
    
    if (!d->endOfShow)
        d->timer->start(d->delay, true);
}

void SlideShow::updatePixmap()
{
    d->pixmap.fill(Qt::black);
    QPainter p(&(d->pixmap));

    if (!d->currentImage.path().isEmpty())
    {
        if (!d->preview.isNull())
        {
            // Preview extraction is complete...

            QPixmap pix(d->preview.smoothScale(size(), QImage::ScaleMin));
            p.drawPixmap((width()-pix.width())/2,
                         (height()-pix.height())/2, pix,
                         0, 0, pix.width(), pix.height());

            if (d->printName)
            {
                QString filename = QString("%1 (%2/%3)").arg(d->currentImage.filename())
                                                        .arg(QString::number(d->fileIndex + 1))
                                                        .arg(QString::number(d->fileList.count()));
            
                p.setPen(Qt::black);
                for (int x=9; x<=11; x++)
                    for (int y=21; y>=19; y--)
                        p.drawText(x, height()-y, filename);
            
                p.setPen(Qt::white);
                p.drawText(10, height()-20, filename);
            }   
        }
        else
        {
            // ...or preview is failed.

            p.setPen(Qt::white);
            p.drawText(0, 0, d->pixmap.width(), d->pixmap.height(),
                       Qt::AlignCenter|Qt::WordBreak, 
                       i18n("Cannot display preview for\n\"%1\"")
                       .arg(d->currentImage.fileName()));
        }
    }
    else
    {
        // End of Slide Show.

        QFont fn(font());
        fn.setPointSize(fn.pointSize()+10);
        fn.setBold(true);
    
        p.setFont(fn);
        p.setPen(Qt::white);
        p.drawText(100, 100, i18n("SlideShow Completed."));
        p.drawText(100, 150, i18n("Click To Exit..."));
        p.end();

        d->endOfShow = true;
        d->toolBar->setEnabledPlay(false);
        d->toolBar->setEnabledNext(false);
        d->toolBar->setEnabledPrev(false);
    }

    p.end();
}

void SlideShow::paintEvent(QPaintEvent *)
{
    bitBlt(this, 0, 0, &d->pixmap,
           0, 0, d->pixmap.width(),
           d->pixmap.height(), Qt::CopyROP, true);
}

void SlideShow::keyPressEvent(QKeyEvent *event)
{
    if (!event)
        return;

    d->toolBar->keyPressEvent(event);
}

void SlideShow::mousePressEvent(QMouseEvent *)
{
    if (d->endOfShow)
        close();
}

void SlideShow::mouseMoveEvent(QMouseEvent *e)
{
    setCursor(QCursor(Qt::ArrowCursor));
    d->mouseMoveTimer->start(1000, true);

    if (!d->toolBar->canHide())
        return;
    
    QPoint pos(e->pos());
    
    if ((pos.y() > (d->deskY+20)) &&
        (pos.y() < (d->deskY+d->deskHeight-20-1)))
    {
        if (d->toolBar->isHidden())
            return;
        else
            d->toolBar->hide();
        return;
    }

    int w = d->toolBar->width();
    int h = d->toolBar->height();
    
    if (pos.y() < (d->deskY+20))
    {
        if (pos.x() <= (d->deskX+d->deskWidth/2))
            // position top left
            d->toolBar->move(d->deskX, d->deskY);
        else
            // position top right
            d->toolBar->move(d->deskX+d->deskWidth-w-1, d->deskY);
    }
    else
    {
        if (pos.x() <= (d->deskX+d->deskWidth/2))
            // position bot left
            d->toolBar->move(d->deskX, d->deskY+d->deskHeight-h-1);
        else
            // position bot right
            d->toolBar->move(d->deskX+d->deskWidth-w-1, d->deskY+d->deskHeight-h-1);
    }
    d->toolBar->show();
}

void SlideShow::slotMouseMoveTimeOut()
{
    QPoint pos(QCursor::pos());
    if ((pos.y() < (d->deskY+20)) ||
        (pos.y() > (d->deskY+d->deskHeight-20-1)))
        return;
    
    setCursor(QCursor(Qt::BlankCursor));
}

void SlideShow::slotPause()
{
    d->timer->stop();

    if (d->toolBar->isHidden())
    {
        int w = d->toolBar->width();
        d->toolBar->move(d->deskWidth-w-1,0);
        d->toolBar->show();
    }
}

void SlideShow::slotPlay()
{
    d->toolBar->hide();
    slotTimeOut();
}

void SlideShow::slotPrev()
{
    loadPrevImage();
}

void SlideShow::slotNext()
{
    loadNextImage();
}

void SlideShow::slotClose()
{
    close();    
}

}  // NameSpace Digikam
