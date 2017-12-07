/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-02-16
 * Description : a presentation tool.
 *
 * Copyright (C) 2006-2009 by Valerio Fuoglio <valerio dot fuoglio at gmail dot com>
 * Copyright (C) 2009      by Andi Clemens <andi dot clemens at googlemail dot com>
 * Copyright (C) 2003-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2012-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "presentationwidget.h"

// C++ includes

#include <cassert>
#include <cmath>
#include <cstdlib>
#include <ctime>

// Qt includes

#include <QCursor>
#include <QFont>
#include <QKeyEvent>
#include <QMatrix>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QPolygon>
#include <QTimer>
#include <QWheelEvent>
#include <QApplication>
#include <QDesktopWidget>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_config.h"
#include "presentationcontainer.h"
#include "presentationctrlwidget.h"
#include "presentationloader.h"

#ifdef HAVE_MEDIAPLAYER
#   include "presentationaudiowidget.h"
#   include "slidevideo.h"
#endif

namespace Digikam
{

class PresentationWidget::Private
{

public:

    Private()
    {
        sharedData                   = 0;
        imageLoader                  = 0;

#ifdef HAVE_MEDIAPLAYER
        playbackWidget               = 0;
        videoView                    = 0;
#endif

        timer                        = 0;
        fileIndex                    = 0;
        effect                       = 0;
        effectRunning                = false;
        x                            = 0;
        y                            = 0;
        w                            = 0;
        h                            = 0;
        dx                           = 0;
        dy                           = 0;
        ix                           = 0;
        iy                           = 0;
        i                            = 0;
        j                            = 0;
        subType                      = 0;
        x0                           = 0;
        y0                           = 0;
        x1                           = 0;
        y1                           = 0;
        wait                         = 0;
        fx                           = 0;
        fy                           = 0;
        alpha                        = 0;
        fd                           = 0;
        intArray                     = 0;
        pdone                        = 0;
        pixelMatrix                  = 0;

        slideCtrlWidget              = 0;
        mouseMoveTimer               = 0;
        deskX                        = 0;
        deskY                        = 0;
        deskWidth                    = 0;
        deskHeight                   = 0;
    }

    PresentationContainer*      sharedData;

    // -------------------------

    QMap<QString, EffectMethod> Effects;

    PresentationLoader*         imageLoader;
    QPixmap                     currImage;

#ifdef HAVE_MEDIAPLAYER
    PresentationAudioWidget*    playbackWidget;
    SlideVideo*                 videoView;
#endif

    QTimer*                     timer;
    int                         fileIndex;

    EffectMethod                effect;
    bool                        effectRunning;
    QString                     effectName;

    // values for state of various effects:
    int                         x;
    int                         y;
    int                         w;
    int                         h;
    int                         dx;
    int                         dy;
    int                         ix;
    int                         iy;
    int                         i;
    int                         j;
    int                         subType;
    int                         x0;
    int                         y0;
    int                         x1;
    int                         y1;
    int                         wait;
    double                      fx;
    double                      fy;
    double                      alpha;
    double                      fd;
    int*                        intArray;
    bool                        pdone;
    bool**                      pixelMatrix;

    //static
    QPolygon                    pa;

    PresentationCtrlWidget*     slideCtrlWidget;
    QTimer*                     mouseMoveTimer;

    int                         deskX;
    int                         deskY;
    int                         deskWidth;
    int                         deskHeight;
};

PresentationWidget::PresentationWidget(PresentationContainer* const sharedData)
    : QWidget(0, Qt::WindowStaysOnTopHint | Qt::Popup | Qt::X11BypassWindowManagerHint),
      d(new Private)
{
    setAttribute(Qt::WA_DeleteOnClose);

    d->sharedData   = sharedData;
    QRect deskRect  = QApplication::desktop()->screenGeometry(QApplication::activeWindow());
    d->deskX        = deskRect.x();
    d->deskY        = deskRect.y();
    d->deskWidth    = deskRect.width();
    d->deskHeight   = deskRect.height();

    move(d->deskX, d->deskY);
    resize(d->deskWidth, d->deskHeight);

    d->slideCtrlWidget = new PresentationCtrlWidget(this);
    d->slideCtrlWidget->hide();
    d->slideCtrlWidget->move(d->deskWidth - d->slideCtrlWidget->width(), d->deskY);

    if (!d->sharedData->loop)
    {
        d->slideCtrlWidget->setEnabledPrev(false);
    }

    connect(d->slideCtrlWidget, SIGNAL(signalPause()),
            this, SLOT(slotPause()));

    connect(d->slideCtrlWidget, SIGNAL(signalPlay()),
            this, SLOT(slotPlay()));

    connect(d->slideCtrlWidget, SIGNAL(signalNext()),
            this, SLOT(slotNext()));

    connect(d->slideCtrlWidget, SIGNAL(signalPrev()),
            this, SLOT(slotPrev()));

    connect(d->slideCtrlWidget, SIGNAL(signalClose()),
            this, SLOT(slotClose()));

#ifdef HAVE_MEDIAPLAYER

    // -- playback widget -------------------------------

    d->playbackWidget = new PresentationAudioWidget(this, d->sharedData->soundtrackUrls, d->sharedData);
    d->playbackWidget->hide();
    d->playbackWidget->move(d->deskX, d->deskY);

    // -- video preview ---------------------------------

    d->videoView = new SlideVideo(this);
    //TODO: pass mouse events from d->videoView to this ?
    //d->videoView->installEventFilter(this);

    connect(d->videoView, SIGNAL(signalVideoLoaded(bool)),
            this, SLOT(slotVideoLoaded(bool)));

    connect(d->videoView, SIGNAL(signalVideoFinished()),
            this, SLOT(slotVideoFinished()));

    d->videoView->hide();
    d->videoView->resize(d->deskWidth, d->deskHeight);

#endif

    // ---------------------------------------------------------------

    d->fileIndex     = -1; // start with -1
    d->effect        = 0;
    d->effectRunning = false;
    d->intArray      = 0;
    m_endOfShow      = false;
    m_simplyShow     = false;
    m_startPainter   = false;
    d->timer         = new QTimer(this);

    connect(d->timer, SIGNAL(timeout()),
            this, SLOT(slotTimeOut()));

    d->pa    = QPolygon(4);
    m_buffer = QPixmap(size());
    m_buffer.fill(Qt::black);

    d->imageLoader = new PresentationLoader(d->sharedData, width(), height(), d->fileIndex);

    // --------------------------------------------------

    registerEffects();

    if (d->sharedData->effectName == QString::fromLatin1("Random"))
    {
        d->effect = getRandomEffect();
    }
    else
    {
        d->effectName = d->sharedData->effectName;
        d->effect     = d->Effects[d->sharedData->effectName];

        if (!d->effect)
        {
            d->effect     = d->Effects[QString::fromLatin1("None")];
            d->effectName = QString::fromLatin1("None");
        }
    }

    d->timer->setSingleShot(true);
    d->timer->start(10);

    // -- hide cursor when not moved --------------------

    d->mouseMoveTimer = new QTimer;

    connect(d->mouseMoveTimer, SIGNAL(timeout()),
            this, SLOT(slotMouseMoveTimeOut()));

    setMouseTracking(true);
    slotMouseMoveTimeOut();
}

PresentationWidget::~PresentationWidget()
{
    d->timer->stop();
    delete d->timer;
    d->mouseMoveTimer->stop();
    delete d->mouseMoveTimer;

    if (d->intArray)
        delete [] d->intArray;

    delete d->imageLoader;
    delete d;
}

void PresentationWidget::readSettings()
{
}

void PresentationWidget::loadNextImage()
{
    if (!d->currImage.isNull())
    {
        m_buffer = d->currImage;
    }
    else
    {
        m_buffer = QPixmap(size());
        m_buffer.fill(Qt::black);
    }

    d->fileIndex++;

    d->imageLoader->next();
    int num = d->sharedData->urlList.count();

    if (d->fileIndex >= num)
    {
        if (d->sharedData->loop)
        {
            d->fileIndex = 0;
        }
        else
        {
            d->currImage = QPixmap(0, 0);
            d->fileIndex = num - 1;
            return;
        }
    }

    if (!d->sharedData->loop)
    {
        d->slideCtrlWidget->setEnabledPrev(d->fileIndex > 0);
        d->slideCtrlWidget->setEnabledNext(d->fileIndex < num - 1);
    }

    QImage img = d->imageLoader->getCurrent();

    QPixmap newPixmap = QPixmap(QPixmap::fromImage(img));
    QPixmap pixmap(width(), height());
    pixmap.fill(Qt::black);
    QPainter p(&pixmap);
    p.drawPixmap((width() - newPixmap.width()) / 2,
                 (height() - newPixmap.height()) / 2, newPixmap,
                 0, 0, newPixmap.width(), newPixmap.height());

    d->currImage = QPixmap(pixmap);

    if (img.isNull())
    {
#ifdef HAVE_MEDIAPLAYER
        d->videoView->setCurrentUrl(d->imageLoader->currPath());
#endif
    }
}

void PresentationWidget::loadPrevImage()
{
    d->fileIndex--;
    d->imageLoader->prev();

    int num = d->sharedData->urlList.count();

    if (d->fileIndex < 0)
    {
        if (d->sharedData->loop)
        {
            d->fileIndex = num - 1;
        }
        else
        {
            d->fileIndex = -1; // set this to -1.
            return;
        }
    }

    if (!d->sharedData->loop)
    {
        d->slideCtrlWidget->setEnabledPrev(d->fileIndex > 0);
        d->slideCtrlWidget->setEnabledNext(d->fileIndex < num - 1);
    }

    QImage img = d->imageLoader->getCurrent();

    QPixmap newPixmap = QPixmap(QPixmap::fromImage(img));
    QPixmap pixmap(width(), height());
    pixmap.fill(Qt::black);
    QPainter p(&pixmap);
    p.drawPixmap((width() - newPixmap.width()) / 2,
                 (height() - newPixmap.height()) / 2, newPixmap,
                 0, 0, newPixmap.width(), newPixmap.height());

    d->currImage = QPixmap(pixmap);

    if (img.isNull())
    {
#ifdef HAVE_MEDIAPLAYER
        d->videoView->setCurrentUrl(d->imageLoader->currPath());
#endif
    }
}

void PresentationWidget::printFilename()
{
    if (d->currImage.isNull())
        return;

    QPainter p;

    p.begin(&d->currImage);
    p.setPen(Qt::black);

    for (int x = 9 ; x <= 11 ; ++x)
    {
        for (int y = 31 ; y >= 29 ; y--)
        {
            p.drawText(x, height() - y, d->imageLoader->currFileName());
        }
    }

    p.setPen(QColor(Qt::white));
    p.drawText(10, height() - 30, d->imageLoader->currFileName());
}

void PresentationWidget::printComments()
{
    if (d->currImage.isNull())
        return;

    QString comments = d->sharedData->commentsMap.value(d->imageLoader->currPath(), QString());

    int yPos = 30; // Text Y coordinate

    if (d->sharedData->printFileName)
        yPos = 50;

    QStringList commentsByLines;

    uint commentsIndex = 0; // Comments QString index

    while (commentsIndex < (uint)comments.length())
    {
        QString newLine;
        bool breakLine = false; // End Of Line found
        uint currIndex; //  Comments QString current index

        // Check minimal lines dimension

        uint commentsLinesLengthLocal = d->sharedData->commentsLinesLength;

        for (currIndex = commentsIndex ; currIndex < (uint)comments.length() && !breakLine ; ++currIndex)
        {
            if (comments[currIndex] == QLatin1Char('\n') || comments[currIndex].isSpace())
            {
                breakLine = true;
            }
        }

        if (commentsLinesLengthLocal <= (currIndex - commentsIndex))
            commentsLinesLengthLocal = (currIndex - commentsIndex);

        breakLine = false;

        for (currIndex = commentsIndex ; currIndex <= commentsIndex + commentsLinesLengthLocal &&
             currIndex < (uint)comments.length() && !breakLine ; ++currIndex)
        {
            breakLine = (comments[currIndex] == QLatin1Char('\n')) ? true : false;

            if (breakLine)
                newLine.append(QLatin1Char(' '));
            else
                newLine.append(comments[currIndex]);
        }

        commentsIndex = currIndex; // The line is ended

        if (commentsIndex != (uint)comments.length())
        {
            while (!newLine.endsWith(QLatin1Char(' ')))
            {
                newLine.truncate(newLine.length() - 1);
                commentsIndex--;
            }
        }

        commentsByLines.prepend(newLine.trimmed());
    }

    QPainter p;

    p.begin(&d->currImage);
    p.setFont(*d->sharedData->captionFont);

    for (int lineNumber = 0 ; lineNumber < (int)commentsByLines.count() ; ++lineNumber)
    {
        p.setPen(QColor(d->sharedData->commentsBgColor));

        // coefficient 1.5 is used to maintain distance between different lines

        for (int x = 9 ; x <= 11 ; ++x)
        {
            for (int y = (int)(yPos + lineNumber * 1.5 * d->sharedData->captionFont->pointSize() + 1) ;
                 y >= (int)(yPos + lineNumber * 1.5 * d->sharedData->captionFont->pointSize() - 1) ; y--)
            {
                p.drawText(x, height() - y, commentsByLines[lineNumber]);
            }
        }

        p.setPen(QColor(d->sharedData->commentsFontColor));
        p.drawText(10, height() - (int)(lineNumber * 1.5 * d->sharedData->captionFont->pointSize() + yPos),
                   commentsByLines[lineNumber]);
    }
}

void PresentationWidget::printProgress()
{
    if (d->currImage.isNull())
        return;

    QPainter p;
    p.begin(&d->currImage);

    QString progress(QString::number(d->fileIndex + 1) + QLatin1Char('/') + QString::number(d->sharedData->urlList.count()));

    int stringLength = p.fontMetrics().width(progress) * progress.length();

    p.setPen(QColor(Qt::black));

    for (int x = 9 ; x <= 11 ; ++x)
    {
        for (int y = 21 ; y >= 19 ; y--)
        {
            p.drawText(width() - stringLength - x, y, progress);
        }
    }

    p.setPen(QColor(Qt::white));
    p.drawText(width() - stringLength - 10, 20, progress);
}

void PresentationWidget::showEndOfShow()
{
    m_endOfShow = true;
    update();

    d->slideCtrlWidget->setEnabledPlay(false);
    d->slideCtrlWidget->setEnabledNext(false);
    d->slideCtrlWidget->setEnabledPrev(false);
}

void PresentationWidget::keyPressEvent(QKeyEvent* event)
{
    if (!event)
        return;

#ifdef HAVE_MEDIAPLAYER
    d->playbackWidget->keyPressEvent(event);
#endif

    d->slideCtrlWidget->keyPressEvent(event);
}

void PresentationWidget::mousePressEvent(QMouseEvent* e)
{
    if (m_endOfShow)
        slotClose();

    if (e->button() == Qt::LeftButton)
    {
        d->timer->stop();
        d->slideCtrlWidget->setPaused(true);
        slotNext();
    }
    else if (e->button() == Qt::RightButton && d->fileIndex - 1 >= 0)
    {
        d->timer->stop();
        d->slideCtrlWidget->setPaused(true);
        slotPrev();
    }
}

void PresentationWidget::mouseMoveEvent(QMouseEvent* e)
{
    setCursor(QCursor(Qt::ArrowCursor));
    d->mouseMoveTimer->setSingleShot(true);
    d->mouseMoveTimer->start(1000);

    if (!d->slideCtrlWidget->canHide()
#ifdef HAVE_MEDIAPLAYER
        || !d->playbackWidget->canHide()
#endif
       )
        return;

    QPoint pos(e->pos());

    if ((pos.y() > (d->deskY + 20)) &&
        (pos.y() < (d->deskY + d->deskHeight - 20 - 1)))
    {
        if (!d->slideCtrlWidget->canHide()
#ifdef HAVE_MEDIAPLAYER
            || !d->playbackWidget->canHide()
#endif
           )
        {
            return;
        }
        else
        {
            d->slideCtrlWidget->hide();
#ifdef HAVE_MEDIAPLAYER
            d->playbackWidget->hide();
#endif
        }

        return;
    }

//    int w = d->slideCtrlWidget->width();
//    int h = d->slideCtrlWidget->height();
//
//    if (pos.y() < (d->deskY + 20))
//    {
//        if (pos.x() <= (d->deskX + d->deskWidth / 2))
//            // position top left
//            d->slideCtrlWidget->move(d->deskX, d->deskY);
//        else
//            // position top right
//            d->slideCtrlWidget->move(d->deskX + d->deskWidth - w - 1, d->deskY);
//    }
//    else
//    {
//        if (pos.x() <= (d->deskX + d->deskWidth / 2))
//            // position bot left
//            d->slideCtrlWidget->move(d->deskX, d->deskY + d->deskHeight - h - 1);
//        else
//            // position bot right
//            d->slideCtrlWidget->move(d->deskX + d->deskWidth - w - 1, d->deskY + d->deskHeight - h - 1);
//    }

    d->slideCtrlWidget->show();
#ifdef HAVE_MEDIAPLAYER
    d->playbackWidget->show();
#endif
}

void PresentationWidget::wheelEvent(QWheelEvent* e)
{
    if (!d->sharedData->enableMouseWheel)
        return;

    if (m_endOfShow)
        slotClose();

    int delta = e->delta();

    if (delta < 0)
    {
        d->timer->stop();
        d->slideCtrlWidget->setPaused(true);
        slotNext();
    }
    else if (delta > 0 && d->fileIndex - 1 >= 0)
    {
        d->timer->stop();
        d->slideCtrlWidget->setPaused(true);
        slotPrev();
    }
}

void PresentationWidget::slotMouseMoveTimeOut()
{
    QPoint pos(QCursor::pos());

    if ((pos.y() < (d->deskY + 20)) ||
        (pos.y() > (d->deskY + d->deskHeight - 20 - 1)))
        return;

    setCursor(QCursor(Qt::BlankCursor));
}

void PresentationWidget::paintEvent(QPaintEvent*)
{
    QPainter p(this);

    if (m_simplyShow)
    {
        if (d->sharedData->printFileName)
            printFilename();

        if (d->sharedData->printProgress)
            printProgress();

        if (d->sharedData->printFileComments)
            printComments();

        p.drawPixmap(0, 0, d->currImage,
                     0, 0, d->currImage.width(), d->currImage.height());

        p.end();

        m_simplyShow = false;

        return;
    }

    if (m_endOfShow)
    {
        p.fillRect(0, 0, width(), height(), Qt::black);

        QFont fn(font());
        fn.setPointSize(fn.pointSize() + 10);
        fn.setBold(true);

        p.setFont(fn);
        p.setPen(Qt::white);
        p.drawText(100, 100, i18n("Slideshow Completed"));
        p.drawText(100, 100 + 10 + fn.pointSize(), i18n("Click to Exit..."));

        p.end();
        return;
    }

    // If execution reach this line, an effect is running
    p.drawPixmap(0, 0, m_buffer);
}

void PresentationWidget::startPainter()
{
    m_startPainter = true;
    repaint();
}

void PresentationWidget::slotPause()
{
    d->timer->stop();

    if (d->slideCtrlWidget->isHidden())
    {
        int w = d->slideCtrlWidget->width();
        d->slideCtrlWidget->move(d->deskWidth - w - 1, 0);
        d->slideCtrlWidget->show();
    }
}

void PresentationWidget::slotPlay()
{
    d->slideCtrlWidget->hide();
    slotTimeOut();
}

void PresentationWidget::slotPrev()
{
    loadPrevImage();

    if (d->currImage.isNull() || d->sharedData->urlList.isEmpty())
    {
        showEndOfShow();
        return;
    }

    d->effectRunning = false;

    showCurrentImage();
}

void PresentationWidget::slotNext()
{
    loadNextImage();

    if (d->currImage.isNull() || d->sharedData->urlList.isEmpty())
    {
        showEndOfShow();
        return;
    }

    d->effectRunning = false;

    showCurrentImage();
}

void PresentationWidget::slotClose()
{
    close();
}

void PresentationWidget::slotVideoLoaded(bool loaded)
{
    if (loaded)
    {
#ifdef HAVE_MEDIAPLAYER
        slotPause();
        d->videoView->show();
#endif
    }
}

void PresentationWidget::slotVideoFinished()
{
#ifdef HAVE_MEDIAPLAYER
    d->videoView->hide();
    slotPlay();
#endif
}

// -- Effects rules --------------------------------------------------------------------------------------------------------

void PresentationWidget::registerEffects()
{
    d->Effects.insert(QString::fromLatin1("None"),             &PresentationWidget::effectNone);
    d->Effects.insert(QString::fromLatin1("Chess Board"),      &PresentationWidget::effectChessboard);
    d->Effects.insert(QString::fromLatin1("Melt Down"),        &PresentationWidget::effectMeltdown);
    d->Effects.insert(QString::fromLatin1("Sweep"),            &PresentationWidget::effectSweep);
    d->Effects.insert(QString::fromLatin1("Mosaic"),           &PresentationWidget::effectMosaic);
    d->Effects.insert(QString::fromLatin1("Cubism"),           &PresentationWidget::effectCubism);
    d->Effects.insert(QString::fromLatin1("Growing"),          &PresentationWidget::effectGrowing);
    d->Effects.insert(QString::fromLatin1("Horizontal Lines"), &PresentationWidget::effectHorizLines);
    d->Effects.insert(QString::fromLatin1("Vertical Lines"),   &PresentationWidget::effectVertLines);
    d->Effects.insert(QString::fromLatin1("Circle Out"),       &PresentationWidget::effectCircleOut);
    d->Effects.insert(QString::fromLatin1("MultiCircle Out"),  &PresentationWidget::effectMultiCircleOut);
    d->Effects.insert(QString::fromLatin1("Spiral In"),        &PresentationWidget::effectSpiralIn);
    d->Effects.insert(QString::fromLatin1("Blobs"),            &PresentationWidget::effectBlobs);
}

QStringList PresentationWidget::effectNames()
{
    QStringList effects;

    effects.append(QString::fromLatin1("None"));
    effects.append(QString::fromLatin1("Chess Board"));
    effects.append(QString::fromLatin1("Melt Down"));
    effects.append(QString::fromLatin1("Sweep"));
    effects.append(QString::fromLatin1("Mosaic"));
    effects.append(QString::fromLatin1("Cubism"));
    effects.append(QString::fromLatin1("Growing"));
    effects.append(QString::fromLatin1("Horizontal Lines"));
    effects.append(QString::fromLatin1("Vertical Lines"));
    effects.append(QString::fromLatin1("Circle Out"));
    effects.append(QString::fromLatin1("MultiCircle Out"));
    effects.append(QString::fromLatin1("Spiral In"));
    effects.append(QString::fromLatin1("Blobs"));
    effects.append(QString::fromLatin1("Random"));

    return effects;
}

QMap<QString, QString> PresentationWidget::effectNamesI18N()
{
    QMap<QString, QString> effects;

    effects[QString::fromLatin1("None")]             = i18nc("Filter Effect: No effect",        "None");
    effects[QString::fromLatin1("Chess Board")]      = i18nc("Filter Effect: Chess Board",      "Chess Board");
    effects[QString::fromLatin1("Melt Down")]        = i18nc("Filter Effect: Melt Down",        "Melt Down");
    effects[QString::fromLatin1("Sweep")]            = i18nc("Filter Effect: Sweep",            "Sweep");
    effects[QString::fromLatin1("Mosaic")]           = i18nc("Filter Effect: Mosaic",           "Mosaic");
    effects[QString::fromLatin1("Cubism")]           = i18nc("Filter Effect: Cubism",           "Cubism");
    effects[QString::fromLatin1("Growing")]          = i18nc("Filter Effect: Growing",          "Growing");
    effects[QString::fromLatin1("Horizontal Lines")] = i18nc("Filter Effect: Horizontal Lines", "Horizontal Lines");
    effects[QString::fromLatin1("Vertical Lines")]   = i18nc("Filter Effect: Vertical Lines",   "Vertical Lines");
    effects[QString::fromLatin1("Circle Out")]       = i18nc("Filter Effect: Circle Out",       "Circle Out");
    effects[QString::fromLatin1("MultiCircle Out")]  = i18nc("Filter Effect: Multi-Circle Out", "Multi-Circle Out");
    effects[QString::fromLatin1("Spiral In")]        = i18nc("Filter Effect: Spiral In",        "Spiral In");
    effects[QString::fromLatin1("Blobs")]            = i18nc("Filter Effect: Blobs",            "Blobs");
    effects[QString::fromLatin1("Random")]           = i18nc("Filter Effect: Random effect",    "Random");

    return effects;
}

void PresentationWidget::slotTimeOut()
{
    if (!d->effect)
        return;                     // No effect -> bye !

    int tmout = -1;

    if (d->effectRunning)           // Effect under progress ?
    {
        tmout = (this->*d->effect)(false);
    }
    else
    {
        loadNextImage();

        if (d->currImage.isNull() || d->sharedData->urlList.isEmpty())   // End of slideshow ?
        {
            showEndOfShow();
            return;
        }

        if (d->sharedData->effectName  == QString::fromLatin1("Random")) // Take a random effect.
        {
            d->effect = getRandomEffect();

            if (!d->effect)
                return;
        }

        d->effectRunning = true;

        tmout = (this->*d->effect)(true);
    }

    if (tmout <= 0)                 // Effect finished -> delay.
    {
        tmout            = d->sharedData->delay;
        d->effectRunning = false;
    }

    d->timer->setSingleShot(true);

    d->timer->start(tmout);
}

void PresentationWidget::showCurrentImage()
{
    if (d->currImage.isNull())
        return;

    m_simplyShow = true;

    repaint();
}

PresentationWidget::EffectMethod PresentationWidget::getRandomEffect()
{
    QStringList effs = d->Effects.keys();
    effs.removeAt(effs.indexOf(QString::fromLatin1("None")));

    int count        = effs.count();
    int i            = qrand() % count;
    QString key      = effs[i];
    d->effectName    = key;

    return d->Effects[key];
}

int PresentationWidget::effectNone(bool /* aInit */)
{
    showCurrentImage();
    return -1;
}

int PresentationWidget::effectChessboard(bool aInit)
{
    if (aInit)
    {
        d->w    = width();
        d->h    = height();
        d->dx   = 8;                             // width of one tile
        d->dy   = 8;                             // height of one tile
        d->j    = (d->w + d->dx - 1) / d->dx;    // number of tiles
        d->x    = d->j * d->dx;                  // shrinking x-offset from screen border
        d->ix   = 0;                             // growing x-offset from screen border
        d->iy   = 0;                             // 0 or d->dy for growing tiling effect
        d->y    = (d->j & 1) ? 0 : d->dy;        // 0 or d->dy for shrinking tiling effect
        d->wait = 800 / d->j;                    // timeout between effects
    }

    if (d->ix >= d->w)
    {
        showCurrentImage();
        return -1;
    }

    d->ix += d->dx;
    d->x  -= d->dx;
    d->iy  = d->iy ? 0 : d->dy;
    d->y   = d->y  ? 0 : d->dy;

    QPainter bufferPainter(&m_buffer);
    QBrush brush = QBrush(d->currImage);

    for (int y = 0 ; y < d->w ; y += (d->dy << 1))
    {
        bufferPainter.fillRect(d->ix, y + d->iy, d->dx, d->dy, brush);
        bufferPainter.fillRect(d->x, y + d->y, d->dx, d->dy, brush);
    }

    repaint();

    return d->wait;
}

int PresentationWidget::effectMeltdown(bool aInit)
{
    int i;

    if (aInit)
    {
        delete [] d->intArray;
        d->w        = width();
        d->h        = height();
        d->dx       = 4;
        d->dy       = 16;
        d->ix       = d->w / d->dx;
        d->intArray = new int[d->ix];

        for (i = d->ix - 1 ; i >= 0 ; --i)
            d->intArray[i] = 0;
    }

    d->pdone = true;

    int y, x;
    QPainter bufferPainter(&m_buffer);

    for (i = 0, x = 0 ; i < d->ix ; ++i, x += d->dx)
    {
        y = d->intArray[i];

        if (y >= d->h)
            continue;

        d->pdone = false;

        if ((qrand() & 15) < 6)
            continue;

        //bufferPainter.drawPixmap(x, y + d->dy, m_buffer, x, y, d->dx, d->h - y - d->dy);
        bufferPainter.drawPixmap(x, y, d->currImage, x, y, d->dx, d->dy);

        d->intArray[i] += d->dy;
    }

    bufferPainter.end();

    repaint();

    if (d->pdone)
    {
        delete [] d->intArray;
        d->intArray = NULL;
        showCurrentImage();
        return -1;
    }

    return 15;
}

int PresentationWidget::effectSweep(bool aInit)
{
    if (aInit)
    {
        // subtype: 0=sweep right to left, 1=sweep left to right
        //          2=sweep bottom to top, 3=sweep top to bottom
        d->subType = qrand() % 4;
        d->w       = width();
        d->h       = height();
        d->dx      = (d->subType == 1 ? 16 : -16);
        d->dy      = (d->subType == 3 ? 16 : -16);
        d->x       = (d->subType == 1 ? 0 : d->w);
        d->y       = (d->subType == 3 ? 0 : d->h);
    }

    if (d->subType == 0 || d->subType == 1)
    {
        // horizontal sweep
        if ((d->subType == 0 && d->x < -64) || (d->subType == 1 && d->x > d->w + 64))
        {
            showCurrentImage();
            return -1;
        }

        int w;
        int x;
        int i;

        for (w = 2, i = 4, x = d->x ; i > 0 ; i--, w <<= 1, x -= d->dx)
        {
            m_px  = x;
            m_py  = 0;
            m_psx = w;
            m_psy = d->h;

            QPainter bufferPainter(&m_buffer);
            bufferPainter.fillRect(m_px, m_py, m_psx, m_psy, QBrush(d->currImage));
            bufferPainter.end();

            repaint();
        }

        d->x += d->dx;
    }
    else
    {
        // vertical sweep
        if ((d->subType == 2 && d->y < -64) || (d->subType == 3 && d->y > d->h + 64))
        {
            showCurrentImage();
            return -1;
        }

        int h;
        int y;
        int i;

        for (h = 2, i = 4, y = d->y ; i > 0 ; i--, h <<= 1, y -= d->dy)
        {
            m_px  = 0;
            m_py  = y;
            m_psx = d->w;
            m_psy = h;

            QPainter bufferPainter(&m_buffer);
            bufferPainter.fillRect(m_px, m_py, m_psx, m_psy, QBrush(d->currImage));
            bufferPainter.end();

            repaint();
        }

        d->y += d->dy;
    }

    return 20;
}

int PresentationWidget::effectMosaic(bool aInit)
{
    int dim    = 10;         // Size of a cell (dim x dim)
    int margin = dim + (int)(dim / 4);

    if (aInit)
    {
        d->i           = 30; // giri totaly
        d->pixelMatrix = new bool*[width()];

        for (int x = 0 ; x < width() ; ++x)
        {
            d->pixelMatrix[x] = new bool[height()];

            for (int y = 0 ; y < height() ; ++y)
            {
                d->pixelMatrix[x][y] = false;
            }
        }
    }

    if (d->i <= 0)
    {
        showCurrentImage();
        return -1;
    }

    int w = width();
    int h = height();

    QPainter bufferPainter(&m_buffer);

    for (int x = 0 ; x < w ; x += (qrand() % margin) + dim)
    {
        for (int y = 0 ; y < h ; y += (qrand() % margin) + dim)
        {
            if (d->pixelMatrix[x][y] == true)
            {
                if (y != 0) y--;

                continue;
            }

            bufferPainter.fillRect(x, y, dim, dim, QBrush(d->currImage));

            for (int i = 0 ; i < dim && (x + i) < w ; ++i)
            {
                for (int j = 0 ; j < dim && (y + j) < h ; ++j)
                {
                    d->pixelMatrix[x+i][y+j] = true;
                }
            }
        }
    }

    bufferPainter.end();
    repaint();
    d->i--;

    return 20;
}

int PresentationWidget::effectCubism(bool aInit)
{
    if (aInit)
    {
        d->alpha = M_PI * 2;
        d->w     = width();
        d->h     = height();
        d->i     = 150;
    }

    if (d->i <= 0)
    {
        showCurrentImage();
        return -1;
    }

    QPainterPath painterPath;
    QPainter bufferPainter(&m_buffer);

    d->x   = qrand() % d->w;
    d->y   = qrand() % d->h;
    int r  = (qrand() % 100) + 100;
    m_px   = d->x - r;
    m_py   = d->y - r;
    m_psx  = r;
    m_psy  = r;

    QMatrix matrix;
    matrix.rotate((qrand() % 20) - 10);
    QRect rect(m_px, m_py, m_psx, m_psy);
    bufferPainter.setMatrix(matrix);
    bufferPainter.fillRect(rect, QBrush(d->currImage));
    bufferPainter.end();
    repaint();

    d->i--;

    return 10;
}

int PresentationWidget::effectRandom(bool /*aInit*/)
{
    d->fileIndex--;

    return -1;
}

int PresentationWidget::effectGrowing(bool aInit)
{
    if (aInit)
    {
        d->w  = width();
        d->h  = height();
        d->x  = d->w >> 1;
        d->y  = d->h >> 1;
        d->i  = 0;
        d->fx = d->x / 100.0;
        d->fy = d->y / 100.0;
    }

    d->x = (d->w >> 1) - (int)(d->i * d->fx);
    d->y = (d->h >> 1) - (int)(d->i * d->fy);
    d->i++;

    if (d->x < 0 || d->y < 0)
    {
        showCurrentImage();
        return -1;
    }

    m_px  = d->x;
    m_py  = d->y;
    m_psx = d->w - (d->x << 1);
    m_psy = d->h - (d->y << 1);

    QPainter bufferPainter(&m_buffer);
    bufferPainter.fillRect(m_px, m_py, m_psx, m_psy, QBrush(d->currImage));
    bufferPainter.end();
    repaint();

    return 20;
}

int PresentationWidget::effectHorizLines(bool aInit)
{
    static int iyPos[] = { 0, 4, 2, 6, 1, 5, 3, 7, -1 };

    if (aInit)
    {
        d->w = width();
        d->h = height();
        d->i = 0;
    }

    if (iyPos[d->i] < 0)
        return -1;

    int iPos;
    int until = d->h;

    QPainter bufferPainter(&m_buffer);
    QBrush brush = QBrush(d->currImage);

    for (iPos = iyPos[d->i] ; iPos < until ; iPos += 8)
        bufferPainter.fillRect(0, iPos, d->w, 1, brush);

    bufferPainter.end();
    repaint();

    d->i++;

    if (iyPos[d->i] >= 0)
        return 160;

    showCurrentImage();

    return -1;
}

int PresentationWidget::effectVertLines(bool aInit)
{
    static int ixPos[] = { 0, 4, 2, 6, 1, 5, 3, 7, -1 };

    if (aInit)
    {
        d->w = width();
        d->h = height();
        d->i = 0;
    }

    if (ixPos[d->i] < 0)
        return -1;

    int iPos;
    int until = d->w;

    QPainter bufferPainter(&m_buffer);
    QBrush brush = QBrush(d->currImage);

    for (iPos = ixPos[d->i] ; iPos < until ; iPos += 8)
        bufferPainter.fillRect(iPos, 0, 1, d->h, brush);

    bufferPainter.end();
    repaint();

    d->i++;

    if (ixPos[d->i] >= 0)
        return 160;

    showCurrentImage();

    return -1;
}

int PresentationWidget::effectMultiCircleOut(bool aInit)
{
    int x, y, i;
    double alpha;

    if (aInit)
    {
        startPainter();
        d->w     = width();
        d->h     = height();
        d->x     = d->w;
        d->y     = d->h >> 1;
        d->pa.setPoint(0, d->w >> 1, d->h >> 1);
        d->pa.setPoint(3, d->w >> 1, d->h >> 1);
        d->fy    = sqrt((double)d->w * d->w + d->h * d->h) / 2;
        d->i     = qrand() % 15 + 2;
        d->fd    = M_PI * 2 / d->i;
        d->alpha = d->fd;
        d->wait  = 10 * d->i;
        d->fx    = M_PI / 32;  // divisor must be powers of 8
    }

    if (d->alpha < 0)
    {
        showCurrentImage();
        return -1;
    }

    for (alpha = d->alpha, i = d->i ; i >= 0 ; i--, alpha += d->fd)
    {
        x    = (d->w >> 1) + (int)(d->fy * cos(-alpha));
        y    = (d->h >> 1) + (int)(d->fy * sin(-alpha));
        d->x = (d->w >> 1) + (int)(d->fy * cos(-alpha + d->fx));
        d->y = (d->h >> 1) + (int)(d->fy * sin(-alpha + d->fx));

        d->pa.setPoint(1, x, y);
        d->pa.setPoint(2, d->x, d->y);

        QPainterPath painterPath;
        painterPath.addPolygon(QPolygon(d->pa));

        QPainter bufferPainter(&m_buffer);
        bufferPainter.fillPath(painterPath, QBrush(d->currImage));
        bufferPainter.end();

        repaint();
    }

    d->alpha -= d->fx;

    return d->wait;
}

int PresentationWidget::effectSpiralIn(bool aInit)
{
    if (aInit)
    {
        update();
        d->w  = width();
        d->h  = height();
        d->ix = d->w / 8;
        d->iy = d->h / 8;
        d->x0 = 0;
        d->x1 = d->w - d->ix;
        d->y0 = d->iy;
        d->y1 = d->h - d->iy;
        d->dx = d->ix;
        d->dy = 0;
        d->i  = 0;
        d->j  = 16 * 16;
        d->x  = 0;
        d->y  = 0;
    }

    if (d->i == 0 && d->x0 >= d->x1)
    {
        showCurrentImage();
        return -1;
    }

    if (d->i == 0 && d->x >= d->x1)      // switch to: down on right side
    {
        d->i   = 1;
        d->dx  = 0;
        d->dy  = d->iy;
        d->x1 -= d->ix;
    }
    else if (d->i == 1 && d->y >= d->y1) // switch to: right to left on bottom side
    {
        d->i   = 2;
        d->dx  = -d->ix;
        d->dy  = 0;
        d->y1 -= d->iy;
    }
    else if (d->i == 2 && d->x <= d->x0) // switch to: up on left side
    {
        d->i   = 3;
        d->dx  = 0;
        d->dy  = -d->iy;
        d->x0 += d->ix;
    }
    else if (d->i == 3 && d->y <= d->y0) // switch to: left to right on top side
    {
        d->i   = 0;
        d->dx  = d->ix;
        d->dy  = 0;
        d->y0 += d->iy;
    }

    m_px  = d->x;
    m_py  = d->y;
    m_psx = d->ix;
    m_psy = d->iy;

    QPainter bufferPainter(&m_buffer);
    bufferPainter.fillRect(m_px, m_py, m_psx, m_psy, QBrush(d->currImage));
    bufferPainter.end();
    repaint();

    d->x += d->dx;
    d->y += d->dy;
    d->j--;

    return 8;
}

int PresentationWidget::effectCircleOut(bool aInit)
{
    int x, y;

    if (aInit)
    {
        startPainter();
        d->w     = width();
        d->h     = height();
        d->x     = d->w;
        d->y     = d->h >> 1;
        d->alpha = 2 * M_PI;
        d->pa.setPoint(0, d->w >> 1, d->h >> 1);
        d->pa.setPoint(3, d->w >> 1, d->h >> 1);
        d->fx    = M_PI / 16;                       // divisor must be powers of 8
        d->fy    = sqrt((double)d->w * d->w + d->h * d->h) / 2;
    }

    if (d->alpha < 0)
    {
        showCurrentImage();
        return -1;
    }

    x         = d->x;
    y         = d->y;
    d->x      = (d->w >> 1) + (int)(d->fy * cos(d->alpha));
    d->y      = (d->h >> 1) + (int)(d->fy * sin(d->alpha));
    d->alpha -= d->fx;

    d->pa.setPoint(1, x, y);
    d->pa.setPoint(2, d->x, d->y);

    QPainterPath painterPath;
    painterPath.addPolygon(QPolygon(d->pa));
    QPainter bufferPainter(&m_buffer);
    bufferPainter.fillPath(painterPath, QBrush(d->currImage));
    bufferPainter.end();
    repaint();

    return 20;
}

int PresentationWidget::effectBlobs(bool aInit)
{
    int r;

    if (aInit)
    {
        d->alpha = M_PI * 2;
        d->w     = width();
        d->h     = height();
        d->i     = 150;
    }

    if (d->i <= 0)
    {
        showCurrentImage();
        return -1;
    }

    d->x   = qrand() % d->w;
    d->y   = qrand() % d->h;
    r      = (qrand() % 200) + 50;
    m_px   = d->x - r;
    m_py   = d->y - r;
    m_psx  = r;
    m_psy  = r;

    QPainterPath painterPath;
    painterPath.addEllipse(m_px, m_py, m_psx, m_psy);
    QPainter bufferPainter(&m_buffer);
    bufferPainter.fillPath(painterPath, QBrush(d->currImage));
    bufferPainter.end();
    repaint();

    d->i--;

    return 10;
}

}  // namespace Digikam
