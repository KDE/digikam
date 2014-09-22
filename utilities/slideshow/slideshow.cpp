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
#include <QCursor>
#include <QDesktopWidget>
#include <QFont>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QTimer>
#include <QWheelEvent>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusReply>

// KDE includes

#include <kapplication.h>
#include <kdeversion.h>
#include <kdialog.h>
#include <kiconloader.h>
#include <kglobalsettings.h>
#include <klocale.h>
#include <kdebug.h>

// Local includes

#include "config-digikam.h"
#include "globals.h"
#include "imagepropertiestab.h"
#include "slidetoolbar.h"
#include "slideosd.h"
#include "slideview.h"
#include "slideerror.h"
#include "slideend.h"

namespace Digikam
{

class SlideShow::Private
{
public:

    enum SlideShowViewMode
    {
        ErrorView=0,
        ImageView,
        EndView
    };

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

    KUrl                currentImage;

    SlideToolBar*       toolBar;
    SlideView*          imageView;
    SlideError*         errorView;
    SlideEnd*           endView;

    SlideOSD*           osd;

    SlideShowSettings   settings;
};

SlideShow::SlideShow(const SlideShowSettings& settings)
    : QStackedWidget(0),
      d(new Private)
{
    setWindowFlags(Qt::FramelessWindowHint);
    d->settings = settings;

    setAttribute(Qt::WA_DeleteOnClose);
    setWindowState(windowState() | Qt::WindowFullScreen);

    setWindowTitle(KDialog::makeStandardCaption(i18n("Slideshow")));
    setContextMenuPolicy(Qt::PreventContextMenu);
    setMouseTracking(true);

    // ---------------------------------------------------------------

    QRect deskRect = KGlobalSettings::desktopGeometry(kapp->activeWindow());
    d->deskX       = deskRect.x();
    d->deskY       = deskRect.y();
    d->deskWidth   = deskRect.width();
    d->deskHeight  = deskRect.height();

    move(d->deskX, d->deskY);
    resize(d->deskWidth, d->deskHeight);

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

    d->errorView = new SlideError(this);
    d->errorView->installEventFilter(this);

    insertWidget(Private::ErrorView, d->errorView);

    // ---------------------------------------------------------------

    d->imageView = new SlideView(this);
    d->imageView->setLoadFullImageSize(d->settings.useFullSizePreviews);
    d->imageView->installEventFilter(this);

    connect(d->imageView, SIGNAL(signalImageLoaded(bool)),
            this, SLOT(slotImageLoaded(bool)));

    insertWidget(Private::ImageView, d->imageView);

    // ---------------------------------------------------------------

    d->endView = new SlideEnd(this);

    insertWidget(Private::EndView, d->endView);

    // ---------------------------------------------------------------

    d->osd = new SlideOSD(d->settings, this);

    // ---------------------------------------------------------------

    d->timer                = new QTimer(this);
    d->mouseMoveTimer       = new QTimer(this);

    connect(d->mouseMoveTimer, SIGNAL(timeout()),
            this, SLOT(slotMouseMoveTimeOut()));

    connect(d->timer, SIGNAL(timeout()),
            this, SLOT(slotTimeOut()));

    d->timer->setSingleShot(true);
    d->timer->start(10);

    // ---------------------------------------------------------------

    setCurrentIndex(Private::ImageView);
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
        d->imageView->setLoadUrl(d->currentImage.toLocalFile());
    }
    else
    {
        endOfSlide();
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
        d->imageView->setLoadUrl(d->currentImage.toLocalFile());
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
        setCurrentIndex(Private::ImageView);
    }
    else
    {
        d->errorView->setCurrentUrl(d->currentImage);
        setCurrentIndex(Private::ErrorView);
    }

    d->osd->setCurrentInfo(d->settings.pictInfoMap[d->currentImage], d->currentImage);
    d->osd->raise();

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

void SlideShow::endOfSlide()
{
    setCurrentIndex(Private::EndView);
    d->currentImage = KUrl();
    d->endOfShow    = true;
    d->toolBar->setEnabledPlay(false);
    d->toolBar->setEnabledNext(false);
    d->toolBar->setEnabledPrev(false);
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

        d->imageView->setPreloadUrl(d->currentImage.toLocalFile());
    }
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

bool SlideShow::eventFilter(QObject* obj, QEvent* ev)
{
    if (obj == d->imageView ||
        obj == d->errorView)
    {
        if (ev->type() == QEvent::MouseMove)
        {
            onMouseMoveEvent(dynamic_cast<QMouseEvent*>(ev));
            return false;
        }
    }

    // pass the event on to the parent class
    return QWidget::eventFilter(obj, ev);
}

void SlideShow::onMouseMoveEvent(QMouseEvent* const e)
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
        d->toolBar->raise();
    }
    else if (topRightLarger.contains(pos))
    {
        d->toolBar->move(topRight.topLeft());
        d->toolBar->show();
        d->toolBar->raise();
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
