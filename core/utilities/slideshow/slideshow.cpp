/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-04-21
 * Description : slide show tool using preview of pictures.
 *
 * Copyright (C) 2005-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "slideshow.h"
#include "digikam_config.h"

// Qt includes

#include <QDesktopWidget>
#include <QMimeDatabase>
#include <QApplication>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QKeyEvent>
#include <QCursor>
#include <QTimer>
#include <QColor>
#include <QFont>

#ifdef HAVE_DBUS
#   include <QDBusConnection>
#   include <QDBusMessage>
#   include <QDBusReply>
#endif

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "slidetoolbar.h"
#include "slideimage.h"
#include "slideerror.h"
#include "slideosd.h"
#include "slideend.h"

#ifdef HAVE_MEDIAPLAYER
#   include "slidevideo.h"
#endif //HAVE_MEDIAPLAYER

namespace Digikam
{

class Q_DECL_HIDDEN SlideShow::Private
{

public:

    explicit Private()
        : fileIndex(-1),
          screenSaverCookie(-1),
          mouseMoveTimer(0),
          imageView(0),
#ifdef HAVE_MEDIAPLAYER
          videoView(0),
#endif
          errorView(0),
          endView(0),
          osd(0)
    {
    }

    int               fileIndex;
    int               screenSaverCookie;

    QTimer*           mouseMoveTimer;  // To hide cursor when not moved.

    SlideImage*       imageView;
#ifdef HAVE_MEDIAPLAYER
    SlideVideo*       videoView;
#endif
    SlideError*       errorView;
    SlideEnd*         endView;
    SlideOSD*         osd;

    SlideShowSettings settings;
};

SlideShow::SlideShow(const SlideShowSettings& settings)
    : QStackedWidget(0),
      d(new Private)
{
    d->settings = settings;

    setAttribute(Qt::WA_DeleteOnClose);
    setWindowFlags(Qt::FramelessWindowHint);
    setContextMenuPolicy(Qt::PreventContextMenu);
    setWindowState(windowState() | Qt::WindowFullScreen);

    setWindowTitle(i18n("Slideshow"));
    setMouseTracking(true);

    // ---------------------------------------------------------------

    d->errorView = new SlideError(this);
    d->errorView->installEventFilter(this);

    insertWidget(ErrorView, d->errorView);

    // ---------------------------------------------------------------

    d->imageView = new SlideImage(this);
    d->imageView->setPreviewSettings(d->settings.previewSettings);
    d->imageView->installEventFilter(this);

    connect(d->imageView, SIGNAL(signalImageLoaded(bool)),
            this, SLOT(slotImageLoaded(bool)));

    insertWidget(ImageView, d->imageView);

    // ---------------------------------------------------------------

#ifdef HAVE_MEDIAPLAYER
    d->videoView = new SlideVideo(this);
    d->videoView->installEventFilter(this);

    connect(d->videoView, SIGNAL(signalVideoLoaded(bool)),
            this, SLOT(slotVideoLoaded(bool)));

    connect(d->videoView, SIGNAL(signalVideoFinished()),
            this, SLOT(slotVideoFinished()));

    insertWidget(VideoView, d->videoView);
#endif

    // ---------------------------------------------------------------

    d->endView = new SlideEnd(this);
    d->endView->installEventFilter(this);

    insertWidget(EndView, d->endView);

    // ---------------------------------------------------------------

    d->osd = new SlideOSD(d->settings, this);
    d->osd->installEventFilter(this);

    // ---------------------------------------------------------------

    d->mouseMoveTimer = new QTimer(this);
    d->mouseMoveTimer->setSingleShot(true);
    d->mouseMoveTimer->setInterval(1000);

    connect(d->mouseMoveTimer, SIGNAL(timeout()),
            this, SLOT(slotMouseMoveTimeOut()));

    // ---------------------------------------------------------------

    QDesktopWidget const* desktop = qApp->desktop();
    const int preferenceScreen    = d->settings.slideScreen;
    int screen                    = 0;

    if (preferenceScreen == -2)
    {
        screen = desktop->screenNumber(qApp->activeWindow());
    }
    else if (preferenceScreen == -1)
    {
        screen = desktop->primaryScreen();
    }
    else if ((preferenceScreen >= 0) && (preferenceScreen < desktop->numScreens()))
    {
        screen = preferenceScreen;
    }
    else
    {
        screen                  = desktop->screenNumber(qApp->activeWindow());
        d->settings.slideScreen = -2;
        d->settings.writeToConfig();
    }

    slotScreenSelected(screen);

    // ---------------------------------------------------------------

    setCurrentView(ImageView);
    inhibitScreenSaver();
    slotMouseMoveTimeOut();
}

SlideShow::~SlideShow()
{
    emit signalLastItemUrl(currentItem());

    d->mouseMoveTimer->stop();

    allowScreenSaver();

    delete d;
}

void SlideShow::setCurrentView(SlideShowViewMode view)
{
    switch(view)
    {
        case ErrorView:
            d->osd->video(false);
            d->errorView->setCurrentUrl(currentItem());

            setCurrentIndex(view);
            d->osd->setCurrentInfo(d->settings.pictInfoMap[currentItem()], currentItem());
            break;

        case ImageView:
#ifdef HAVE_MEDIAPLAYER
            d->videoView->stop();
            d->osd->video(false);
#endif
            setCurrentIndex(view);
            d->osd->setCurrentInfo(d->settings.pictInfoMap[currentItem()], currentItem());
            break;

        case VideoView:
#ifdef HAVE_MEDIAPLAYER
            d->osd->video(true);
            d->osd->pause(false);
            setCurrentIndex(view);
            d->osd->setCurrentInfo(d->settings.pictInfoMap[currentItem()], currentItem());
#endif
            break;

        default : // EndView
#ifdef HAVE_MEDIAPLAYER
            d->videoView->stop();
            d->osd->video(false);
#endif
            d->osd->pause(true);
            setCurrentIndex(view);
            break;
    }
}

void SlideShow::setCurrentItem(const QUrl& url)
{
    int index = d->settings.indexOf(url);

    if (index != -1)
    {
        d->fileIndex = index - 1;
    }
}

QUrl SlideShow::currentItem() const
{
    return d->settings.fileList.value(d->fileIndex);
}

void SlideShow::slotLoadNextItem()
{
    int num = d->settings.count();

    if (d->fileIndex == (num - 1))
    {
        if (d->settings.loop)
        {
            d->fileIndex = -1;
        }
    }

    d->fileIndex++;

    qCDebug(DIGIKAM_GENERAL_LOG) << "fileIndex: " << d->fileIndex;

    if (!d->settings.loop)
    {
        d->osd->toolBar()->setEnabledPrev(d->fileIndex > 0);
        d->osd->toolBar()->setEnabledNext(d->fileIndex < (num - 1));
    }

    if (d->fileIndex >= 0 && d->fileIndex < num)
    {

#ifdef HAVE_MEDIAPLAYER
        QMimeDatabase mimeDB;

        if (mimeDB.mimeTypeForFile(currentItem().toLocalFile())
                                   .name().startsWith(QLatin1String("video/")))
        {
            d->videoView->setCurrentUrl(currentItem());
            return;
        }
#endif

        d->imageView->setLoadUrl(currentItem());
    }
    else
    {
        endOfSlide();
    }
}

void SlideShow::slotLoadPrevItem()
{
    int num = d->settings.count();

    if (d->fileIndex == 0)
    {
        if (d->settings.loop)
        {
            d->fileIndex = num;
        }
    }

    d->fileIndex--;

    qCDebug(DIGIKAM_GENERAL_LOG) << "fileIndex: " << d->fileIndex;

    if (!d->settings.loop)
    {
        d->osd->toolBar()->setEnabledPrev(d->fileIndex > 0);
        d->osd->toolBar()->setEnabledNext(d->fileIndex < (num - 1));
    }

    if (d->fileIndex >= 0 && d->fileIndex < num)
    {

#ifdef HAVE_MEDIAPLAYER
        QMimeDatabase mimeDB;

        if (mimeDB.mimeTypeForFile(currentItem().toLocalFile())
                                   .name().startsWith(QLatin1String("video/")))
        {
            d->videoView->setCurrentUrl(currentItem());
            return;
        }
#endif

        d->imageView->setLoadUrl(currentItem());
    }
    else
    {
        endOfSlide();
    }
}

void SlideShow::slotImageLoaded(bool loaded)
{
    if (loaded)
    {
        setCurrentView(ImageView);

        if (d->fileIndex != -1)
        {
            if (!d->osd->isPaused())
            {
                d->osd->pause(false);
            }

            preloadNextItem();
        }
    }
    else
    {
#ifdef HAVE_MEDIAPLAYER
        // Try to load item as video
        d->videoView->setCurrentUrl(currentItem());
#else
        slotVideoLoaded(false);
#endif
    }
}

void SlideShow::slotVideoLoaded(bool loaded)
{
    if (loaded)
    {
        setCurrentView(VideoView);
    }
    else
    {
        // Failed to load item
        setCurrentView(ErrorView);

        if (d->fileIndex != -1)
        {
            if (!d->osd->isPaused())
            {
                d->osd->pause(false);
            }
        }
    }

    preloadNextItem();
}

void SlideShow::slotVideoFinished()
{
    if (d->fileIndex != -1)
    {
        d->osd->video(false);
        slotLoadNextItem();
    }
}

void SlideShow::endOfSlide()
{
    setCurrentView(EndView);
    d->fileIndex = -1;
    d->osd->toolBar()->setEnabledPlay(false);
    d->osd->toolBar()->setEnabledNext(false);
    d->osd->toolBar()->setEnabledPrev(false);
}

void SlideShow::preloadNextItem()
{
    int index = d->fileIndex + 1;
    int num   = d->settings.count();

    if (index >= num)
    {
        if (d->settings.loop)
        {
            index = 0;
        }
    }

    if (index < num)
    {
        QUrl nextItem = d->settings.fileList.value(index);

#ifdef HAVE_MEDIAPLAYER
        QMimeDatabase mimeDB;

        if (mimeDB.mimeTypeForFile(nextItem.toLocalFile())
                                   .name().startsWith(QLatin1String("video/")))
        {
            return;
        }
#endif

        d->imageView->setPreloadUrl(nextItem);
    }
}

void SlideShow::wheelEvent(QWheelEvent* e)
{
    if (e->delta() < 0)
    {
        d->osd->pause(true);
        slotLoadNextItem();
    }

    if (e->delta() > 0)
    {
        if (d->fileIndex == -1)
        {
            // EndView => backward.
            d->fileIndex = d->settings.count();
        }

        d->osd->pause(true);
        slotLoadPrevItem();
    }
}

void SlideShow::mousePressEvent(QMouseEvent* e)
{
    if (d->fileIndex == -1)
    {
        // EndView => close Slideshow view.
        close();
    }

    if (e->button() == Qt::LeftButton)
    {
        d->osd->pause(true);
        slotLoadNextItem();
    }
    else if (e->button() == Qt::RightButton)
    {
        if (d->fileIndex == -1)
        {
            // EndView => backward.
            d->fileIndex = d->settings.count() - 1;
        }

        d->osd->pause(true);
        slotLoadPrevItem();
    }
}

void SlideShow::keyPressEvent(QKeyEvent* e)
{
    if (!e)
    {
        return;
    }

    d->osd->toolBar()->keyPressEvent(e);
}

bool SlideShow::eventFilter(QObject* obj, QEvent* ev)
{
    if (ev->type() == QEvent::MouseMove)
    {
        setCursor(QCursor(Qt::ArrowCursor));

#ifdef HAVE_MEDIAPLAYER
        d->videoView->showIndicator(true);
#endif

        d->mouseMoveTimer->start();
        return false;
    }

    // pass the event on to the parent class
    return QWidget::eventFilter(obj, ev);
}

void SlideShow::slotMouseMoveTimeOut()
{
    if (!d->osd->toolBar()->underMouse())
    {
        setCursor(QCursor(Qt::BlankCursor));
    }

#ifdef HAVE_MEDIAPLAYER
    d->videoView->showIndicator(false);
#endif
}

// From Okular's presentation widget
// TODO: Add OSX and Windows support
void SlideShow::inhibitScreenSaver()
{
#ifdef HAVE_DBUS
    QDBusMessage message = QDBusMessage::createMethodCall(QLatin1String("org.freedesktop.ScreenSaver"),
                                                          QLatin1String("/ScreenSaver"),
                                                          QLatin1String("org.freedesktop.ScreenSaver"),
                                                          QLatin1String("Inhibit"));
    message << QLatin1String("digiKam");
    message << i18nc("Reason for inhibiting the screensaver activation, when the presentation mode is active", "Giving a slideshow");

    QDBusReply<uint> reply = QDBusConnection::sessionBus().call(message);

    if (reply.isValid())
    {
        d->screenSaverCookie = reply.value();
    }
#endif
}

void SlideShow::allowScreenSaver()
{
#ifdef HAVE_DBUS
    if (d->screenSaverCookie != -1)
    {
        QDBusMessage message = QDBusMessage::createMethodCall(QLatin1String("org.freedesktop.ScreenSaver"),
                                                              QLatin1String("/ScreenSaver"),
                                                              QLatin1String("org.freedesktop.ScreenSaver"),
                                                              QLatin1String("UnInhibit"));
        message << (uint)d->screenSaverCookie;
        QDBusConnection::sessionBus().send(message);
    }
#endif
}

void SlideShow::slotAssignRating(int rating)
{
    d->settings.pictInfoMap[currentItem()].rating = rating;
    dispatchCurrentInfoChange(currentItem());
    emit signalRatingChanged(currentItem(), rating);
}

void SlideShow::slotAssignColorLabel(int color)
{
    d->settings.pictInfoMap[currentItem()].colorLabel = color;
    dispatchCurrentInfoChange(currentItem());
    emit signalColorLabelChanged(currentItem(), color);
}

void SlideShow::slotAssignPickLabel(int pick)
{
    d->settings.pictInfoMap[currentItem()].pickLabel = pick;
    dispatchCurrentInfoChange(currentItem());
    emit signalPickLabelChanged(currentItem(), pick);
}

void SlideShow::updateTags(const QUrl& url, const QStringList& tags)
{
    d->settings.pictInfoMap[url].tags = tags;
    dispatchCurrentInfoChange(url);
}

void SlideShow::toggleTag(int tag)
{
    emit signalToggleTag(currentItem(), tag);
}

void SlideShow::dispatchCurrentInfoChange(const QUrl& url)
{
    if (currentItem() == url)
    {
        d->osd->setCurrentInfo(d->settings.pictInfoMap[currentItem()], currentItem());
    }
}

void SlideShow::slotPause()
{
#ifdef HAVE_MEDIAPLAYER
    if (currentIndex() == VideoView)
    {
        d->videoView->pause(true);
    }
    else
#endif
    {
        d->osd->pause(true);
    }
}

void SlideShow::slotPlay()
{
#ifdef HAVE_MEDIAPLAYER
    if (currentIndex() == VideoView)
    {
        d->videoView->pause(false);
    }
    else
#endif
    {
        d->osd->pause(false);
    }
}

void SlideShow::slotScreenSelected(int screen)
{
    QRect deskRect = qApp->desktop()->screenGeometry(screen);

    move(deskRect.x(), deskRect.y());
    resize(deskRect.width(), deskRect.height());

    qCDebug(DIGIKAM_GENERAL_LOG) << "Slideshow: move to screen: " << screen
                                 << " :: " << deskRect;
}

} // namespace Digikam
