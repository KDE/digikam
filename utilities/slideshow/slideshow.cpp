/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-04-21
 * Description : slide show tool using preview of pictures.
 *
 * Copyright (C) 2005-2014 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2004      by Enrico Ros <eros dot kde at email dot it>
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

#include "slideshow.moc"

// Qt includes

#include <QColor>
#include <QMenu>
#include <QCursor>
#include <QDesktopWidget>
#include <QFont>
#include <QLayout>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QPixmap>
#include <QTimer>
#include <QWheelEvent>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusReply>

// KDE includes

#include <kaboutdata.h>
#include <kapplication.h>
#include <kdeversion.h>
#include <kdialog.h>
#include <kglobalsettings.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kdebug.h>

// Local includes

#include "config-digikam.h"
#include "dimg.h"
#include "globals.h"
#include "previewloadthread.h"
#include "imagepropertiestab.h"
#include "slidetoolbar.h"
#include "slideosd.h"

namespace Digikam
{

class SlideShow::Private
{
public:

    Private()
        : endOfShow(false),
          pause(false),
          deskX(0),
          deskY(0),
          deskWidth(0),
          deskHeight(0),
          fileIndex(-1),
          screenSaverCookie(-1),
          mouseMoveTimer(0),
          timer(0),
          previewThread(0),
          previewPreloadThread(0),
          toolBar(0),
          osd(0)
    {
    }

    bool                endOfShow;
    bool                pause;

    int                 deskX;
    int                 deskY;
    int                 deskWidth;
    int                 deskHeight;
    int                 fileIndex;
    int                 screenSaverCookie;

    QTimer*             mouseMoveTimer;  // To hide cursor when not moved.
    QTimer*             timer;

    QPixmap             pixmap;

    DImg                preview;

    KUrl                currentImage;

    PreviewLoadThread*  previewThread;
    PreviewLoadThread*  previewPreloadThread;

    SlideToolBar*       toolBar;

    SlideOSD*           osd;

    SlideShowSettings   settings;
};

SlideShow::SlideShow(const SlideShowSettings& settings)
    : QWidget(0, Qt::FramelessWindowHint), d(new Private)
{
    d->settings = settings;

    setAttribute(Qt::WA_DeleteOnClose);
    setAttribute(Qt::WA_OpaquePaintEvent);
    setWindowState(windowState() | Qt::WindowFullScreen);

    setWindowTitle(KDialog::makeStandardCaption(i18n("Slideshow")));
    setContextMenuPolicy(Qt::PreventContextMenu);

    // ---------------------------------------------------------------

    QRect deskRect = KGlobalSettings::desktopGeometry(kapp->activeWindow());
    d->deskX       = deskRect.x();
    d->deskY       = deskRect.y();
    d->deskWidth   = deskRect.width();
    d->deskHeight  = deskRect.height();

    move(d->deskX, d->deskY);
    resize(d->deskWidth, d->deskHeight);

    QPalette palette;
    palette.setColor(backgroundRole(), Qt::black);
    setPalette(palette);

    // ---------------------------------------------------------------

    d->toolBar = new SlideToolBar(this);
    d->toolBar->hide();

    if (!d->settings.loop)
    {
        d->toolBar->setEnabledPrev(false);
    }

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

    d->osd = new SlideOSD(d->settings, this);

    // ---------------------------------------------------------------

    d->previewThread        = new PreviewLoadThread();
    d->previewPreloadThread = new PreviewLoadThread();
    d->timer                = new QTimer(this);
    d->mouseMoveTimer       = new QTimer(this);

    d->previewThread->setDisplayingWidget(this);
    d->previewPreloadThread->setDisplayingWidget(this);

    connect(d->previewThread, SIGNAL(signalImageLoaded(LoadingDescription,DImg)),
            this, SLOT(slotGotImagePreview(LoadingDescription,DImg)));

    connect(d->mouseMoveTimer, SIGNAL(timeout()),
            this, SLOT(slotMouseMoveTimeOut()));

    connect(d->timer, SIGNAL(timeout()),
            this, SLOT(slotTimeOut()));

    d->timer->setSingleShot(true);
    d->timer->start(10);

    // ---------------------------------------------------------------

    inhibitScreenSaver();

    setMouseTracking(true);
    slotMouseMoveTimeOut();
}

SlideShow::~SlideShow()
{
    allowScreenSaver();

    d->timer->stop();
    d->mouseMoveTimer->stop();

    delete d->timer;
    delete d->mouseMoveTimer;
    delete d->previewThread;
    delete d->previewPreloadThread;
    delete d;
}

void SlideShow::setCurrentUrl(const KUrl& url)
{
    int index = d->settings.fileList.indexOf(url);

    if (index != -1)
    {
        d->currentImage = url;
        d->fileIndex    = index - 1;
    }
}

KUrl SlideShow::currentUrl() const
{
    return d->currentImage;
}

void SlideShow::slotTimeOut()
{
    loadNextImage();
}

void SlideShow::loadNextImage()
{
    d->fileIndex++;
    int num = d->settings.fileList.count();

    if (d->fileIndex >= num)
    {
        if (d->settings.loop)
        {
            d->fileIndex = 0;
        }
    }

    if (!d->settings.loop)
    {
        d->toolBar->setEnabledPrev(d->fileIndex > 0);
        d->toolBar->setEnabledNext(d->fileIndex < num - 1);
    }

    if (d->fileIndex < num)
    {
        d->currentImage = d->settings.fileList[d->fileIndex];
        if (d->settings.useFullSizePreviews)
        {
            d->previewThread->loadHighQuality(d->currentImage.toLocalFile());
        }
        else
        {
            d->previewThread->load(d->currentImage.toLocalFile(),
                                   qMax(d->deskWidth, d->deskHeight));
        }
   }
    else
    {
        d->currentImage = KUrl();
        d->preview      = DImg();
        updatePixmap();
        update();
    }
}

void SlideShow::loadPrevImage()
{
    d->fileIndex--;
    int num = d->settings.fileList.count();

    if (d->fileIndex < 0)
    {
        if (d->settings.loop)
        {
            d->fileIndex = num - 1;
        }
    }

    if (!d->settings.loop)
    {
        d->toolBar->setEnabledPrev(d->fileIndex > 0);
        d->toolBar->setEnabledNext(d->fileIndex < num - 1);
    }

    if (d->fileIndex >= 0 && d->fileIndex < num)
    {
        d->currentImage = d->settings.fileList[d->fileIndex];
        if (d->settings.useFullSizePreviews)
        {
            d->previewThread->loadHighQuality(d->currentImage.toLocalFile());
        }
        else
        {
            d->previewThread->load(d->currentImage.toLocalFile(),
                                   qMax(d->deskWidth, d->deskHeight));
        }
    }
    else
    {
        d->currentImage = KUrl();
        d->preview      = DImg();
        updatePixmap();
        update();
    }
}

void SlideShow::slotGotImagePreview(const LoadingDescription& desc, const DImg& preview)
{
    if (desc.filePath != d->currentImage.toLocalFile() || desc.isThumbnail())
    {
        return;
    }

    d->preview = preview;

    updatePixmap();
    update();

    if (!d->endOfShow)
    {
        if (!d->pause)
        {
            d->timer->setSingleShot(true);
            d->timer->start(d->settings.delay * 1000);
        }

        preloadNextImage();
    }
}

void SlideShow::preloadNextImage()
{
    int index = d->fileIndex + 1;
    int num   = d->settings.fileList.count();

    if (index >= num)
    {
        if (d->settings.loop)
        {
            index = 0;
        }
    }

    if (index < num)
    {
        if (d->settings.useFullSizePreviews)
        {
            d->previewPreloadThread->loadHighQuality(d->settings.fileList[index].toLocalFile());
        }
        else
        {
            d->previewPreloadThread->load(d->settings.fileList[index].toLocalFile(),
                                          qMax(d->deskWidth, d->deskHeight));
        }
    }
}

void SlideShow::updatePixmap()
{
    if (!d->currentImage.toLocalFile().isEmpty())
    {
        if (!d->preview.isNull())
        {
            // Preview extraction is complete... Draw the image.

            /* For high resolution ("retina") displays, Mac OS X / Qt
               report only half of the physical resolution in terms of
               pixels, i.e. every logical pixels corresponds to 2x2
               physical pixels. However, UI elements and fonts are
               nevertheless rendered at full resolution, and pixmaps
               as well, provided their resolution is high enough (that
               is, higher than the reported, logical resolution).

               To work around this, we render the photos not a logical
               resolution, but with the photo's full resolution, but
               at the screen's aspect ratio. When we later draw this
               high resolution bitmap, it is up to Qt to scale the
               photo to the true physical resolution.  The ratio
               computed below is the ratio between the photo and
               screen resolutions, or equivalently the factor by which
               we need to increase the pixel size of the rendered
               pixmap.
            */
#ifdef USE_QT_SCALING
            double xratio  = double(d->preview.width()) / width();
            double yratio  = double(d->preview.height()) / height();
            double ratio   = qMax(qMin(xratio, yratio), 1.0);
#else
            double ratio   = 1.0;
#endif
            QSize fullSize = QSizeF(ratio*width(), ratio*height()).toSize();
            d->pixmap      = QPixmap(fullSize);
            d->pixmap.fill(Qt::black);
            QPainter p(&(d->pixmap));

            // scale font for the pixmap
            QFont fn(font());
            fn.setPointSize(int(ratio*fn.pointSize()));
            p.setFont(fn);

            QPixmap pix(d->preview.smoothScale(d->pixmap.width(), d->pixmap.height(), Qt::KeepAspectRatio).convertToPixmap());
            p.drawPixmap((d->pixmap.width()  - pix.width())  / 2,
                         (d->pixmap.height() - pix.height()) / 2, pix,
                         0, 0, pix.width(), pix.height());

            d->osd->setCurrentInfo(d->settings.pictInfoMap[d->currentImage], d->currentImage);
        }
        else
        {
            // ...or preview extraction is failed.

            d->pixmap = QPixmap(size());
            d->pixmap.fill(Qt::black);
            QPainter p(&(d->pixmap));

            p.setPen(Qt::white);
            p.drawText(0, 0, d->pixmap.width(), d->pixmap.height(),
                       Qt::AlignCenter | Qt::TextWordWrap,
                       i18n("Cannot display image\n\"%1\"",
                            d->currentImage.fileName()));
        }
    }
    else
    {
        // End of Slide Show.

        d->pixmap = QPixmap(size());
        d->pixmap.fill(Qt::black);
        QPainter p(&(d->pixmap));

        QPixmap logo;

        if (KGlobal::mainComponent().aboutData()->appName() == QString("digikam"))
        {
            logo = QPixmap(KStandardDirs::locate("data", "digikam/data/logo-digikam.png"))
                   .scaled(128, 128, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }
        else
        {
            logo = QPixmap(KStandardDirs::locate("data", "showfoto/data/logo-showfoto.png"))
                   .scaled(128, 128, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }

        QFont fn(font());
        fn.setPointSize(fn.pointSize() + 10);
        fn.setBold(true);

        p.setFont(fn);
        p.setPen(Qt::white);
        p.drawPixmap(50, 100, logo);
        p.drawText(60 + logo.width(), 100 +     logo.height() / 3, i18n("Slideshow Completed."));
        p.drawText(60 + logo.width(), 100 + 2 * logo.height() / 3, i18n("Click To Exit..."));

        d->endOfShow = true;
        d->toolBar->setEnabledPlay(false);
        d->toolBar->setEnabledNext(false);
        d->toolBar->setEnabledPrev(false);
    }
}

void SlideShow::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.drawPixmap(0, 0, width(), height(), d->pixmap,
                 0, 0, d->pixmap.width(), d->pixmap.height());
    p.end();
}

void SlideShow::slotPause()
{
    d->timer->stop();
    d->pause = true;

    if (d->toolBar->isHidden())
    {
        int w = d->toolBar->width();
        d->toolBar->move(d->deskWidth - w - 1, 0);
        d->toolBar->show();
    }
}

void SlideShow::slotPlay()
{
    d->timer->start();
    d->toolBar->hide();
    d->pause = false;
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

void SlideShow::wheelEvent(QWheelEvent* e)
{
    if (e->delta() < 0)
    {
        d->timer->stop();
        d->pause = true;
        d->toolBar->setPaused(true);
        slotNext();
    }

    if (e->delta() > 0 && d->fileIndex - 1 >= 0)
    {
        d->timer->stop();
        d->pause = true;
        d->toolBar->setPaused(true);
        slotPrev();
    }
}

void SlideShow::mousePressEvent(QMouseEvent* e)
{
    if (d->endOfShow)
    {
        close();
    }

    if (e->button() == Qt::LeftButton)
    {
        d->timer->stop();
        d->pause = true;
        d->toolBar->setPaused(true);
        slotNext();
    }
    else if (e->button() == Qt::RightButton && d->fileIndex - 1 >= 0)
    {
        d->timer->stop();
        d->pause = true;
        d->toolBar->setPaused(true);
        slotPrev();
    }
}

void SlideShow::keyPressEvent(QKeyEvent* e)
{
    if (!e)
    {
        return;
    }

    d->toolBar->keyPressEvent(e);
}

void SlideShow::makeCornerRectangles(const QRect& desktopRect, const QSize& size,
                                     QRect* topLeft, QRect* topRight,
                                     QRect* topLeftLarger, QRect* topRightLarger)
{
    QRect sizeRect(QPoint(0, 0), size);
    *topLeft          = sizeRect;
    *topRight         = sizeRect;

    topLeft->moveTo(desktopRect.x(), desktopRect.y());
    topRight->moveTo(desktopRect.x() + desktopRect.width() - sizeRect.width() - 1, topLeft->y());

    const int marginX = 25, marginY = 10;
    *topLeftLarger    = topLeft->adjusted(0, 0, marginX, marginY);
    *topRightLarger   = topRight->adjusted(-marginX, 0, 0, marginY);
}

void SlideShow::mouseMoveEvent(QMouseEvent* e)
{
    setCursor(QCursor(Qt::ArrowCursor));
    d->mouseMoveTimer->setSingleShot(true);
    d->mouseMoveTimer->start(1000);

    if (!d->toolBar->canHide())
    {
        return;
    }

    QPoint pos(e->pos());

    QRect sizeRect(QPoint(0, 0), d->toolBar->size());
    QRect topLeft, topRight;
    QRect topLeftLarger, topRightLarger;
    makeCornerRectangles(QRect(d->deskY, d->deskY, d->deskWidth, d->deskHeight), d->toolBar->size(),
                               &topLeft, &topRight, &topLeftLarger, &topRightLarger);

    if (topLeftLarger.contains(pos))
    {
        d->toolBar->move(topLeft.topLeft());
        d->toolBar->show();
    }
    else if (topRightLarger.contains(pos))
    {
        d->toolBar->move(topRight.topLeft());
        d->toolBar->show();
    }
    else
    {
        if (!d->toolBar->isHidden())
        {
            d->toolBar->hide();
        }
    }
}

void SlideShow::slotMouseMoveTimeOut()
{
    QPoint pos(QCursor::pos());

    QRect sizeRect(QPoint(0, 0), d->toolBar->size());
    QRect topLeft, topRight;
    QRect topLeftLarger, topRightLarger;
    makeCornerRectangles(QRect(d->deskY, d->deskY, d->deskWidth, d->deskHeight), d->toolBar->size(),
                         &topLeft, &topRight, &topLeftLarger, &topRightLarger);

    if (topLeftLarger.contains(pos) || topRightLarger.contains(pos))
    {
        return;
    }

    setCursor(QCursor(Qt::BlankCursor));
}

// from Okular's presentation widget
// TODO: Add OSX and Windows support
void SlideShow::inhibitScreenSaver()
{
    QDBusMessage message = QDBusMessage::createMethodCall("org.freedesktop.ScreenSaver", "/ScreenSaver",
                                                          "org.freedesktop.ScreenSaver", "Inhibit");
    message << QString("digiKam");
    message << i18nc("Reason for inhibiting the screensaver activation, when the presentation mode is active", "Giving a slideshow");

    QDBusReply<uint> reply = QDBusConnection::sessionBus().call(message);

    if (reply.isValid())
    {
        d->screenSaverCookie = reply.value();
    }
}

void SlideShow::allowScreenSaver()
{
    if (d->screenSaverCookie != -1)
    {
        QDBusMessage message = QDBusMessage::createMethodCall("org.freedesktop.ScreenSaver", "/ScreenSaver",
                                                              "org.freedesktop.ScreenSaver", "UnInhibit");
        message << (uint)d->screenSaverCookie;
        QDBusConnection::sessionBus().send(message);
    }
}

void SlideShow::slotAssignRating(int rating)
{
    d->settings.pictInfoMap[d->currentImage].rating = rating;
    dispatchCurrentInfoChange(d->currentImage);
    emit signalRatingChanged(d->currentImage, rating);
}

void SlideShow::slotAssignColorLabel(int color)
{
    d->settings.pictInfoMap[d->currentImage].colorLabel = color;
    dispatchCurrentInfoChange(d->currentImage);
    emit signalColorLabelChanged(d->currentImage, color);
}

void SlideShow::slotAssignPickLabel(int pick)
{
    d->settings.pictInfoMap[d->currentImage].pickLabel = pick;
    dispatchCurrentInfoChange(d->currentImage);
    emit signalPickLabelChanged(d->currentImage, pick);
}

void SlideShow::updateTags(const KUrl& url, const QStringList& tags)
{
    d->settings.pictInfoMap[url].tags = tags;
    dispatchCurrentInfoChange(url);
}

void SlideShow::toggleTag(int tag)
{
    emit signalToggleTag(d->currentImage, tag);
}

void SlideShow::dispatchCurrentInfoChange(const KUrl& url)
{
    if (d->currentImage == url)
        d->osd->setCurrentInfo(d->settings.pictInfoMap[d->currentImage], d->currentImage);
}

void SlideShow::setPaused(bool paused)
{
    d->pause = paused;
    d->toolBar->setPaused(paused);
}

}  // namespace Digikam
