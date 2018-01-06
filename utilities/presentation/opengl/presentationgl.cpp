/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-01-19
 * Description : a presentation tool.
 *
 * Copyright (C) 2004      by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2006-2009 by Valerio Fuoglio <valerio.fuoglio@gmail.com>
 * Copyright (C) 2009      by Andi Clemens <andi dot clemens at googlemail dot com>
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

#include "presentationgl.h"

// C++ includes

#include <cmath>
#include <cstdlib>

// Qt includes

#include <QApplication>
#include <QCursor>
#include <QDesktopWidget>
#include <QEvent>
#include <QFileInfo>
#include <QFontMetrics>
#include <QImage>
#include <QKeyEvent>
#include <QList>
#include <QMatrix>
#include <QMouseEvent>
#include <QPainter>
#include <QPixmap>
#include <QTimer>
#include <QWheelEvent>
#include <QApplication>

// KDE includes

#include <kconfiggroup.h>
#include <kconfig.h>
#include <klocalizedstring.h>

// Local includes

#include "digikam_config.h"
#include "digikam_debug.h"
#include "presentationcontainer.h"
#include "presentationctrlwidget.h"
#include "presentationloader.h"

#ifdef HAVE_MEDIAPLAYER
#   include "presentationaudiowidget.h"
#endif

namespace Digikam
{

class PresentationGL::Private
{

public:

    Private()
    {
        timer               = 0;
        fileIndex           = 0;
        imageLoader         = 0;
        texture[0]          = 0;
        texture[1]          = 0;
        curr                = 0;
        width               = 0;
        height              = 0;
        xMargin             = 0;
        yMargin             = 0;
        effect              = 0;
        tex1First           = true;
        effectRunning       = false;
        timeout             = 0;
        endOfShow           = false;
        random              = false;
        i                   = 0;
        dir                 = 0;
        slideCtrlWidget     = 0;

#ifdef HAVE_MEDIAPLAYER
        playbackWidget      = 0;
#endif

        mouseMoveTimer      = 0;
        deskX               = 0;
        deskY               = 0;
        deskWidth           = 0;
        deskHeight          = 0;
        sharedData          = 0;
    }

    QMap<QString, EffectMethod>       effects;

    QTimer*                           timer;
    int                               fileIndex;

    PresentationLoader*               imageLoader;
    GLuint                            texture[2];
    bool                              tex1First;
    int                               curr;

    int                               width;
    int                               height;
    int                               xMargin;
    int                               yMargin;


    EffectMethod                      effect;
    bool                              effectRunning;
    int                               timeout;
    bool                              random;
    bool                              endOfShow;

    int                               i;
    int                               dir;
    float                             points[40][40][3];

    PresentationCtrlWidget*           slideCtrlWidget;

#ifdef HAVE_MEDIAPLAYER
    PresentationAudioWidget*          playbackWidget;
#endif

    QTimer*                           mouseMoveTimer;

    int                               deskX;
    int                               deskY;
    int                               deskWidth;
    int                               deskHeight;

    PresentationContainer*            sharedData;
};

PresentationGL::PresentationGL(PresentationContainer* const sharedData)
    : QGLWidget(0, 0, Qt::WindowStaysOnTopHint | Qt::Popup | Qt::X11BypassWindowManagerHint),
      d(new Private)
{
    setAttribute(Qt::WA_DeleteOnClose);
    QRect deskRect = QApplication::desktop()->screenGeometry( QApplication::activeWindow() );
    d->deskX        = deskRect.x();
    d->deskY        = deskRect.y();
    d->deskWidth    = deskRect.width();
    d->deskHeight   = deskRect.height();

    move(d->deskX, d->deskY);
    resize(d->deskWidth, d->deskHeight);

    d->sharedData   = sharedData;

    d->slideCtrlWidget = new PresentationCtrlWidget(this);
    d->slideCtrlWidget->hide();

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

    d->playbackWidget = new PresentationAudioWidget(this, d->sharedData->soundtrackUrls, d->sharedData);
    d->playbackWidget->hide();
    d->playbackWidget->move(d->deskX, d->deskY);

#endif

    int w = d->slideCtrlWidget->width();
    d->slideCtrlWidget->move(d->deskX + d->deskWidth - w - 1, d->deskY);

    // -- Minimal texture size (opengl specs) --------------

    d->width        = 64;
    d->height       = 64;

    // -- Margin -------------------------------------------

    d->xMargin      = int (d->deskWidth / d->width);
    d->yMargin      = int (d->deskWidth / d->height);

    // ------------------------------------------------------------------

    d->fileIndex    = -1; // start with -1
    d->timeout      = d->sharedData->delay;
    d->imageLoader  = new PresentationLoader(d->sharedData, width(), height(), d->fileIndex);

    // --------------------------------------------------

    registerEffects();

    if (d->sharedData->effectNameGL == QString::fromLatin1("Random"))
    {
        d->effect = getRandomEffect();
        d->random = true;
    }
    else
    {
        d->effect = d->effects[d->sharedData->effectNameGL];

        if (!d->effect)
            d->effect = d->effects[QString::fromLatin1("None")];

        d->random = false;
    }

    // --------------------------------------------------

    d->timer = new QTimer(this);

    connect(d->timer, SIGNAL(timeout()),
            this, SLOT(slotTimeOut()));

    d->timer->setSingleShot(true);
    d->timer->start(10);

    // -- hide cursor when not moved --------------------

    d->mouseMoveTimer = new QTimer;

    connect(d->mouseMoveTimer, SIGNAL(timeout()),
            this, SLOT(slotMouseMoveTimeOut()));

    setMouseTracking(true);
    slotMouseMoveTimeOut();
}

PresentationGL::~PresentationGL()
{

    if (d->texture[0])
        glDeleteTextures(1, &d->texture[0]);

    if (d->texture[1])
        glDeleteTextures(1, &d->texture[1]);

    delete d->imageLoader;
    delete d->mouseMoveTimer;
    delete d;
}

void PresentationGL::initializeGL()
{
    // Enable Texture Mapping
    glEnable(GL_TEXTURE_2D);

    // Clear The Background Color
    glClearColor(0.0, 0.0, 0.0, 1.0f);

    // Turn Blending On
    glEnable(GL_BLEND);

    // Blending Function For Translucency Based On Source Alpha Value
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Enable perspective vision
    glClearDepth(1.0f);

    // get the maximum texture value.
    GLint maxTexVal;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTexVal);

    // allow only maximum texture value of 1024. anything bigger and things slow down
    maxTexVal = qMin(1024, maxTexVal);

    d->width  = QApplication::desktop()->width();
    d->height = QApplication::desktop()->height();

    d->width  = 1 << (int)ceil(log((float)d->width) / log((float)2)) ;
    d->height = 1 << (int)ceil(log((float)d->height) / log((float)2));

    d->width  = qMin( maxTexVal, d->width );
    d->height = qMin( maxTexVal, d->height );
}

void PresentationGL::paintGL()
{
    glDisable(GL_DEPTH_TEST);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    if (d->endOfShow)
        showEndOfShow();
    else
    {
        if (d->effectRunning && d->effect)
            (this->*d->effect)();
        else
            paintTexture();
    }
}

void PresentationGL::resizeGL(int w, int h)
{
    // Reset The Current Viewport And Perspective Transformation
    glViewport(0, 0, (GLint)w, (GLint)h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
}

void PresentationGL::keyPressEvent(QKeyEvent* event)
{
    if (!event)
        return;

    d->slideCtrlWidget->keyPressEvent(event);
#ifdef HAVE_MEDIAPLAYER
    d->playbackWidget->keyPressEvent(event);
#endif
}

void PresentationGL::mousePressEvent(QMouseEvent* e)
{
    if (d->endOfShow)
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

void PresentationGL::mouseMoveEvent(QMouseEvent* e)
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
        if (d->slideCtrlWidget->isHidden()
#ifdef HAVE_MEDIAPLAYER
            || d->playbackWidget->isHidden()
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

    d->slideCtrlWidget->show();
#ifdef HAVE_MEDIAPLAYER
    d->playbackWidget->show();
#endif
}

void PresentationGL::wheelEvent(QWheelEvent* e)
{
    if (!d->sharedData->enableMouseWheel)
        return;

    if (d->endOfShow)
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

void PresentationGL::registerEffects()
{
    d->effects.insert(QString::fromLatin1("None"),    &PresentationGL::effectNone);
    d->effects.insert(QString::fromLatin1("Blend"),   &PresentationGL::effectBlend);
    d->effects.insert(QString::fromLatin1("Fade"),    &PresentationGL::effectFade);
    d->effects.insert(QString::fromLatin1("Rotate"),  &PresentationGL::effectRotate);
    d->effects.insert(QString::fromLatin1("Bend"),    &PresentationGL::effectBend);
    d->effects.insert(QString::fromLatin1("In Out"),  &PresentationGL::effectInOut);
    d->effects.insert(QString::fromLatin1("Slide"),   &PresentationGL::effectSlide);
    d->effects.insert(QString::fromLatin1("Flutter"), &PresentationGL::effectFlutter);
    d->effects.insert(QString::fromLatin1("Cube"),    &PresentationGL::effectCube);
}

QStringList PresentationGL::effectNames()
{
    QStringList effects;

    effects.append(QString::fromLatin1("None"));
    effects.append(QString::fromLatin1("Bend"));
    effects.append(QString::fromLatin1("Blend"));
    effects.append(QString::fromLatin1("Cube"));
    effects.append(QString::fromLatin1("Fade"));
    effects.append(QString::fromLatin1("Flutter"));
    effects.append(QString::fromLatin1("In Out"));
    effects.append(QString::fromLatin1("Rotate"));
    effects.append(QString::fromLatin1("Slide"));
    effects.append(QString::fromLatin1("Random"));

    return effects;
}

QMap<QString, QString> PresentationGL::effectNamesI18N()
{
    QMap<QString, QString> effects;

    effects[QString::fromLatin1("None")]    = i18nc("Filter Effect: No effect",     "None");
    effects[QString::fromLatin1("Bend")]    = i18nc("Filter Effect: Bend",          "Bend");
    effects[QString::fromLatin1("Blend")]   = i18nc("Filter Effect: Blend",         "Blend");
    effects[QString::fromLatin1("Cube")]    = i18nc("Filter Effect: Cube",          "Cube");
    effects[QString::fromLatin1("Fade")]    = i18nc("Filter Effect: Fade",          "Fade");
    effects[QString::fromLatin1("Flutter")] = i18nc("Filter Effect: Flutter",       "Flutter");
    effects[QString::fromLatin1("In Out")]  = i18nc("Filter Effect: In Out",        "In Out");
    effects[QString::fromLatin1("Rotate")]  = i18nc("Filter Effect: Rotate",        "Rotate");
    effects[QString::fromLatin1("Slide")]   = i18nc("Filter Effect: Slide",         "Slide");
    effects[QString::fromLatin1("Random")]  = i18nc("Filter Effect: Random effect", "Random");

    return effects;
}

PresentationGL::EffectMethod PresentationGL::getRandomEffect()
{
    QMap<QString, EffectMethod>  tmpMap(d->effects);

    tmpMap.remove(QString::fromLatin1("None"));
    QStringList t = tmpMap.keys();
    int count     = t.count();
    int i         = (int)((float)(count) * qrand() / (RAND_MAX + 1.0));
    QString key   = t[i];

    return tmpMap[key];
}

void PresentationGL::advanceFrame()
{
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
            d->fileIndex = num - 1;
            d->endOfShow = true;
            d->slideCtrlWidget->setEnabledPlay(false);
            d->slideCtrlWidget->setEnabledNext(false);
            d->slideCtrlWidget->setEnabledPrev(false);
        }
    }

    if (!d->sharedData->loop && !d->endOfShow)
    {
        d->slideCtrlWidget->setEnabledPrev(d->fileIndex > 0);
        d->slideCtrlWidget->setEnabledNext(d->fileIndex < num - 1);
    }

    d->tex1First = !d->tex1First;
    d->curr      = (d->curr == 0) ? 1 : 0;
}

void PresentationGL::previousFrame()
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
            d->fileIndex = 0;
            d->endOfShow = true;
            d->slideCtrlWidget->setEnabledPlay(false);
            d->slideCtrlWidget->setEnabledNext(false);
            d->slideCtrlWidget->setEnabledPrev(false);
        }
    }

    if (!d->sharedData->loop && !d->endOfShow)
    {
        d->slideCtrlWidget->setEnabledPrev(d->fileIndex > 0);
        d->slideCtrlWidget->setEnabledNext(d->fileIndex < num - 1);
    }

    d->tex1First = !d->tex1First;

    d->curr      = (d->curr == 0) ? 1 : 0;
}

void PresentationGL::loadImage()
{
    QImage image = d->imageLoader->getCurrent();

    if (!image.isNull())
    {
        int a       = d->tex1First ? 0 : 1;
        GLuint& tex = d->texture[a];

        if (tex)
            glDeleteTextures(1, &tex);

        QImage black(width(), height(), QImage::Format_RGB32);

        black.fill(QColor(0, 0, 0).rgb());

        /*        image = image.smoothScale(width(), height(),
                                          Qt::ScaleMin);*/
        montage(image, black);

        if (!d->sharedData->openGlFullScale)
        {
            black = black.scaled(d->width, d->height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        }

        if (d->sharedData->printFileName)
            printFilename(black);

        if (d->sharedData->printProgress)
            printProgress(black);

        if (d->sharedData->printFileComments)
            printComments(black);

        QImage t = convertToGLFormat(black);

        /* create the texture */
        glGenTextures(1, &tex);

        glBindTexture(GL_TEXTURE_2D, tex);

        /* actually generate the texture */
        glTexImage2D( GL_TEXTURE_2D, 0, 3, t.width(), t.height(), 0,
                      GL_RGBA, GL_UNSIGNED_BYTE, t.bits() );

        /* enable linear filtering  */
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }
}

void PresentationGL::montage(QImage& top, QImage& bot)
{
    int tw = top.width();
    int th = top.height();
    int bw = bot.width();
    int bh = bot.height();

    if (tw > bw || th > bh)
        qFatal("Top Image should be smaller or same size as Bottom Image");

    if (top.depth() != 32) top = top.convertToFormat(QImage::Format_RGB32);

    if (bot.depth() != 32) bot = bot.convertToFormat(QImage::Format_RGB32);

    int sw = bw / 2 - tw / 2; //int ew = bw/2 + tw/2;
    int sh = bh / 2 - th / 2;
    int eh = bh / 2 + th / 2;

    unsigned int* tdata = reinterpret_cast<unsigned int*>(top.scanLine(0));
    unsigned int* bdata = 0;

    for (int y = sh; y < eh; ++y)
    {
        bdata = reinterpret_cast<unsigned int*>(bot.scanLine(y)) + sw;

        for (int x = 0; x < tw; ++x)
        {
            *(bdata++) = *(tdata++);
        }
    }
}

void PresentationGL::printFilename(QImage& layer)
{
    QFileInfo fileinfo(d->sharedData->urlList[d->fileIndex].toLocalFile());
    QString filename = fileinfo.fileName();
    QPixmap pix      = generateOutlinedTextPixmap(filename);

    // --------------------------------------------------------

    QPainter painter;
    painter.begin(&layer);
    painter.drawPixmap(d->xMargin, layer.height() - d->yMargin - pix.height(), pix);
    painter.end();
}

void PresentationGL::printProgress(QImage& layer)
{
    QString progress(QString::number(d->fileIndex + 1) + QLatin1Char('/') + QString::number(d->sharedData->urlList.count()));

    QPixmap pix = generateOutlinedTextPixmap(progress);

    QPainter painter;
    painter.begin(&layer);
    painter.drawPixmap(layer.width() - d->xMargin - pix.width(), d->yMargin, pix);
    painter.end();
}

void PresentationGL::printComments(QImage& layer)
{
    QString comments = d->sharedData->commentsMap.value(d->imageLoader->currPath(), QString());

    int yPos = 5; // Text Y coordinate

    if (d->sharedData->printFileName)
        yPos += 20;

    QStringList commentsByLines;

    uint commentsIndex = 0; // Comments QString index

    while (commentsIndex < (uint) comments.length())
    {
        QString newLine;
        bool breakLine = false; // End Of Line found
        uint currIndex; //  Comments QString current index

        // Check miminal lines dimension

        int commentsLinesLengthLocal = d->sharedData->commentsLinesLength;

        for (currIndex = commentsIndex; currIndex < (uint) comments.length() && !breakLine; ++currIndex)
        {
            if (comments[currIndex] == QLatin1Char('\n') || comments[currIndex].isSpace())
            {
                breakLine = true;
            }
        }

        if (commentsLinesLengthLocal <= (int)((currIndex - commentsIndex)))
            commentsLinesLengthLocal = (currIndex - commentsIndex);

        breakLine = false;

        for ( currIndex = commentsIndex; currIndex <= commentsIndex + commentsLinesLengthLocal &&
                currIndex < (uint) comments.length() && !breakLine; ++currIndex )
        {
            breakLine = (comments[currIndex] == QLatin1Char('\n')) ? true : false;

            if (breakLine)
                newLine.append(QLatin1Char(' '));
            else
                newLine.append( comments[currIndex] );
        }

        commentsIndex = currIndex; // The line is ended

        if ( commentsIndex != (uint) comments.length() )
        {
            while ( !newLine.endsWith(QLatin1Char(' ')) )
            {
                newLine.truncate(newLine.length() - 1);
                commentsIndex--;
            }
        }

        commentsByLines.prepend(newLine.trimmed());
    }

    yPos += int(2.0 * d->sharedData->captionFont->pointSize());

    QFont  font(*d->sharedData->captionFont);
    QColor fgColor(d->sharedData->commentsFontColor);
    QColor bgColor(d->sharedData->commentsBgColor);
    bool   drawTextOutline = d->sharedData->commentsDrawOutline;
    int    opacity = d->sharedData->bgOpacity;

    for ( int lineNumber = 0; lineNumber < (int)commentsByLines.count(); ++lineNumber )
    {
        QPixmap pix = generateCustomOutlinedTextPixmap(commentsByLines[lineNumber],
                                                       font, fgColor, bgColor, opacity, drawTextOutline);

        QPainter painter;
        painter.begin(&layer);

        int xPos = (layer.width() / 2) - (pix.width() / 2);
        painter.drawPixmap(xPos, layer.height() - pix.height() - yPos, pix);

        painter.end();

        yPos += int(pix.height() + d->height / 400);
    }
}

void PresentationGL::showEndOfShow()
{
    QPixmap pix(width(), height());
    pix.fill(Qt::black);

    QFont fn(font());
    fn.setPointSize(fn.pointSize() + 10);
    fn.setBold(true);

    QPainter p(&pix);
    p.setPen(Qt::white);
    p.setFont(fn);
    p.drawText(20, 50, i18n("Slideshow Completed"));
    p.drawText(20, 100, i18n("Click to Exit..."));

//     QPixmap logoPixmap = KPSvgPixmapRenderer(width() / 6, width() / 6).getPixmap();
//     p.drawPixmap(width()-(width()/12)-logoPixmap.width(),
//                  height()-(height()/12)-logoPixmap.height(),
//                  logoPixmap);

    p.end();

    QImage image(pix.toImage());
    QImage t = convertToGLFormat(image);

    GLuint tex;

    /* create the texture */
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    /* actually generate the texture */
    glTexImage2D( GL_TEXTURE_2D, 0, 3, t.width(), t.height(), 0,
                  GL_RGBA, GL_UNSIGNED_BYTE, t.bits() );
    /* enable linear filtering  */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    /* paint the texture */

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glBindTexture(GL_TEXTURE_2D, tex);

    glBegin(GL_QUADS);
    {
        glColor4f(1.0, 1.0, 1.0, 1.0);
        glTexCoord2f(0, 0);
        glVertex3f(-1.0, -1.0, 0);

        glTexCoord2f(1, 0);
        glVertex3f(1.0, -1.0, 0);

        glTexCoord2f(1, 1);
        glVertex3f(1.0, 1.0, 0);

        glTexCoord2f(0, 1);
        glVertex3f(-1.0, 1.0, 0);
    }

    glEnd();
}

void PresentationGL::slotTimeOut()
{
    if (!d->effect)
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "PresentationGL: No transition method";
        d->effect = &PresentationGL::effectNone;
    }

    if (d->effectRunning)
    {
        d->timeout = 10;
    }
    else
    {
        if (d->timeout == 0)
        {
            // effect was running and is complete now
            // run timer while showing current image
            d->timeout = d->sharedData->delay;
            d->i     = 0;
        }
        else
        {

            // timed out after showing current image
            // load next image and start effect
            if (d->random)
                d->effect = getRandomEffect();

            advanceFrame();

            if (d->endOfShow)
            {
                updateGL();
                return;
            }

            loadImage();

            d->timeout       = 10;
            d->effectRunning = true;
            d->i             = 0;

        }
    }

    updateGL();

    if (d->timeout < 0)
        d->timeout = 0;

    d->timer->setSingleShot(true);
    d->timer->start(d->timeout);
}

void PresentationGL::slotMouseMoveTimeOut()
{
    QPoint pos(QCursor::pos());

    if ((pos.y() < (d->deskY + 20)) ||
            (pos.y() > (d->deskY + d->deskHeight - 20 - 1)))
        return;

    setCursor(QCursor(Qt::BlankCursor));
}

void PresentationGL::paintTexture()
{
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    GLuint& tex = d->texture[d->curr];
    glBindTexture(GL_TEXTURE_2D, tex);

    glBegin(GL_QUADS);
    {
        glColor4f(1.0, 1.0, 1.0, 1.0);
        glTexCoord2f(0, 0);
        glVertex3f(-1.0, -1.0, 0);

        glTexCoord2f(1, 0);
        glVertex3f(1.0, -1.0, 0);

        glTexCoord2f(1, 1);
        glVertex3f(1.0, 1.0, 0);

        glTexCoord2f(0, 1);
        glVertex3f(-1.0, 1.0, 0);
    }

    glEnd();
}

void PresentationGL::effectNone()
{
    paintTexture();
    d->effectRunning = false;
    d->timeout = -1;
    return;
}

void PresentationGL::effectBlend()
{
    if (d->i > 100)
    {
        paintTexture();
        d->effectRunning = false;
        d->timeout = -1;
        return;
    }

    int a = (d->curr == 0) ? 1 : 0;
    int b =  d->curr;

    GLuint& ta = d->texture[a];
    GLuint& tb = d->texture[b];

    glBindTexture(GL_TEXTURE_2D, ta);

    glBegin(GL_QUADS);
    {
        glColor4f(1.0, 1.0, 1.0, 1.0);
        glTexCoord2f(0, 0);
        glVertex3f(-1.0f, -1.0f, 0);

        glTexCoord2f(1, 0);
        glVertex3f(1.0f, -1.0f, 0);

        glTexCoord2f(1, 1);
        glVertex3f(1.0f, 1.0f, 0);

        glTexCoord2f(0, 1);
        glVertex3f(-1.0f, 1.0f, 0);
    }

    glEnd();
    glBindTexture(GL_TEXTURE_2D, tb);

    glBegin(GL_QUADS);
    {
        glColor4f(1.0, 1.0, 1.0, 1.0 / (100.0)*(float)d->i);
        glTexCoord2f(0, 0);
        glVertex3f(-1.0f, -1.0f, 0);

        glTexCoord2f(1, 0);
        glVertex3f(1.0f, -1.0f, 0);

        glTexCoord2f(1, 1);
        glVertex3f(1.0f, 1.0f, 0);

        glTexCoord2f(0, 1);
        glVertex3f(-1.0f, 1.0f, 0);
    }

    glEnd();

    d->i++;
}

void PresentationGL::effectFade()
{
    if (d->i > 100)
    {
        paintTexture();
        d->effectRunning = false;
        d->timeout = -1;
        return;
    }

    int a;
    float opacity;

    if (d->i <= 50)
    {
        a =  (d->curr == 0) ? 1 : 0;
        opacity = 1.0 - 1.0 / 50.0 * (float)(d->i);
    }
    else
    {
        opacity = 1.0 / 50.0 * (float)(d->i - 50.0);
        a = d->curr;
    }

    GLuint& ta = d->texture[a];
    glBindTexture(GL_TEXTURE_2D, ta);

    glBegin(GL_QUADS);
    {
        glColor4f(1.0, 1.0, 1.0, opacity);
        glTexCoord2f(0, 0);
        glVertex3f(-1.0f, -1.0f, 0);

        glTexCoord2f(1, 0);
        glVertex3f(1.0f, -1.0f, 0);

        glTexCoord2f(1, 1);
        glVertex3f(1.0f, 1.0f, 0);

        glTexCoord2f(0, 1);
        glVertex3f(-1.0f, 1.0f, 0);
    }

    glEnd();


    d->i++;
}

void PresentationGL::effectRotate()
{
    if (d->i > 100)
    {
        paintTexture();
        d->effectRunning = false;
        d->timeout = -1;
        return;
    }

    if (d->i == 0)
        d->dir = (int)((2.0 * qrand() / (RAND_MAX + 1.0)));

    int a = (d->curr == 0) ? 1 : 0;
    int b =  d->curr;

    GLuint& ta = d->texture[a];
    GLuint& tb = d->texture[b];
    glBindTexture(GL_TEXTURE_2D, tb);

    glBegin(GL_QUADS);
    {
        glColor4f(1.0, 1.0, 1.0, 1.0);
        glTexCoord2f(0, 0);
        glVertex3f(-1.0f, -1.0f, 0);

        glTexCoord2f(1, 0);
        glVertex3f(1.0f, -1.0f, 0);

        glTexCoord2f(1, 1);
        glVertex3f(1.0f, 1.0f, 0);

        glTexCoord2f(0, 1);
        glVertex3f(-1.0f, 1.0f, 0);
    }

    glEnd();

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    float rotate = 360.0 / 100.0 * (float)d->i;
    glRotatef( ((d->dir == 0) ? -1 : 1) * rotate, 0.0, 0.0, 1.0);
    float scale  = 1.0 / 100.0 * (100.0 - (float)(d->i));
    glScalef(scale, scale, 1.0);
    glBindTexture(GL_TEXTURE_2D, ta);

    glBegin(GL_QUADS);
    {
        glColor4f(1.0, 1.0, 1.0, 1.0);
        glTexCoord2f(0, 0);
        glVertex3f(-1.0f, -1.0f, 0);

        glTexCoord2f(1, 0);
        glVertex3f(1.0f, -1.0f, 0);

        glTexCoord2f(1, 1);
        glVertex3f(1.0f, 1.0f, 0);

        glTexCoord2f(0, 1);
        glVertex3f(-1.0f, 1.0f, 0);
    }

    glEnd();

    d->i++;
}

void PresentationGL::effectBend()
{
    if (d->i > 100)
    {
        paintTexture();
        d->effectRunning = false;
        d->timeout = -1;
        return;
    }

    if (d->i == 0)
        d->dir = (int)((2.0 * qrand() / (RAND_MAX + 1.0)));

    int a = (d->curr == 0) ? 1 : 0;
    int b =  d->curr;

    GLuint& ta = d->texture[a];
    GLuint& tb = d->texture[b];
    glBindTexture(GL_TEXTURE_2D, tb);

    glBegin(GL_QUADS);

    {
        glColor4f(1.0, 1.0, 1.0, 1.0);
        glTexCoord2f(0, 0);
        glVertex3f(-1.0f, -1.0f, 0);

        glTexCoord2f(1, 0);
        glVertex3f(1.0f, -1.0f, 0);

        glTexCoord2f(1, 1);
        glVertex3f(1.0f, 1.0f, 0);

        glTexCoord2f(0, 1);
        glVertex3f(-1.0f, 1.0f, 0);
    }

    glEnd();

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glRotatef(90.0 / 100.0*(float)d->i,
              (d->dir == 0) ? 1.0 : 0.0,
              (d->dir == 1) ? 1.0 : 0.0,
              0.0);

    glBindTexture(GL_TEXTURE_2D, ta);

    glBegin(GL_QUADS);
    {
        glColor4f(1.0, 1.0, 1.0, 1.0);
        glTexCoord2f(0, 0);
        glVertex3f(-1.0f, -1.0f, 0);

        glTexCoord2f(1, 0);
        glVertex3f(1.0f, -1.0f, 0);

        glTexCoord2f(1, 1);
        glVertex3f(1.0f, 1.0f, 0);

        glTexCoord2f(0, 1);
        glVertex3f(-1.0f, 1.0f, 0);
    }

    glEnd();

    d->i++;
}

void PresentationGL::effectInOut()
{
    if (d->i > 100)
    {
        paintTexture();
        d->effectRunning = false;
        d->timeout       = -1;
        return;
    }

    if (d->i == 0)
    {
        d->dir = 1 + (int) ((4.0 * qrand() / (RAND_MAX + 1.0)));
    }

    int a;
    bool out;

    if (d->i <= 50)
    {
        a = (d->curr == 0) ? 1 : 0;
        out = 1;
    }
    else
    {
        a = d->curr;
        out = 0;
    }

    GLuint& ta = d->texture[a];
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    float t    = out ? 1.0 / 50.0 * (50.0 - d->i) : 1.0 / 50.0 * (d->i - 50.0);
    glScalef(t, t, 1.0);
    t          = 1.0 - t;
    glTranslatef((d->dir % 2 == 0) ? ((d->dir == 2) ? 1 : -1) * t : 0.0,
                 (d->dir % 2 == 1) ? ((d->dir == 1) ? 1 : -1) * t : 0.0,
                 0.0);

    glBindTexture(GL_TEXTURE_2D, ta);

    glBegin(GL_QUADS);
    {
        glColor4f(1.0, 1.0, 1.0, 1.0);

        glColor4f(1.0, 1.0, 1.0, 1.0);
        glTexCoord2f(0, 0);
        glVertex3f(-1.0f, -1.0f, 0);

        glTexCoord2f(1, 0);
        glVertex3f(1.0f, -1.0f, 0);

        glTexCoord2f(1, 1);
        glVertex3f(1.0f, 1.0f, 0);

        glTexCoord2f(0, 1);
        glVertex3f(-1.0f, 1.0f, 0);
    }

    glEnd();

    d->i++;
}

void PresentationGL::effectSlide()
{
    if (d->i > 100)
    {
        paintTexture();
        d->effectRunning = false;
        d->timeout = -1;
        return;
    }

    if (d->i == 0)
        d->dir = 1 + (int)((4.0 * qrand() / (RAND_MAX + 1.0)));

    int a      = (d->curr == 0) ? 1 : 0;
    int b      =  d->curr;
    GLuint& ta = d->texture[a];
    GLuint& tb = d->texture[b];
    glBindTexture(GL_TEXTURE_2D, tb);

    glBegin(GL_QUADS);

    {
        glColor4f(1.0, 1.0, 1.0, 1.0);
        glTexCoord2f(0, 0);
        glVertex3f(-1.0f, -1.0f, 0);

        glTexCoord2f(1, 0);
        glVertex3f(1.0f, -1.0f, 0);

        glTexCoord2f(1, 1);
        glVertex3f(1.0f, 1.0f, 0);

        glTexCoord2f(0, 1);
        glVertex3f(-1.0f, 1.0f, 0);
    }

    glEnd();

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    float trans = 2.0 / 100.0 * (float)d->i;
    glTranslatef((d->dir % 2 == 0) ? ((d->dir == 2) ? 1 : -1) * trans : 0.0,
                 (d->dir % 2 == 1) ? ((d->dir == 1) ? 1 : -1) * trans : 0.0,
                 0.0);

    glBindTexture(GL_TEXTURE_2D, ta);

    glBegin(GL_QUADS);
    {
        glColor4f(1.0, 1.0, 1.0, 1.0);

        glColor4f(1.0, 1.0, 1.0, 1.0);
        glTexCoord2f(0, 0);
        glVertex3f(-1.0f, -1.0f, 0);

        glTexCoord2f(1, 0);
        glVertex3f(1.0f, -1.0f, 0);

        glTexCoord2f(1, 1);
        glVertex3f(1.0f, 1.0f, 0);

        glTexCoord2f(0, 1);
        glVertex3f(-1.0f, 1.0f, 0);
    }

    glEnd();

    d->i++;
}

void PresentationGL::effectFlutter()
{
    if (d->i > 100)
    {
        paintTexture();
        d->effectRunning = false;
        d->timeout = -1;
        return;
    }

    int a      = (d->curr == 0) ? 1 : 0;
    int b      =  d->curr;
    GLuint& ta = d->texture[a];
    GLuint& tb = d->texture[b];

    if (d->i == 0)
    {
        for (int x = 0; x < 40; ++x)
        {
            for (int y = 0; y < 40; ++y)
            {
                d->points[x][y][0] = (float) (x / 20.0f - 1.0f);
                d->points[x][y][1] = (float) (y / 20.0f - 1.0f);
                d->points[x][y][2] = (float) sin((x / 20.0f - 1.0f) * 3.141592654 * 2.0f) / 5.0;
            }
        }
    }

    glBindTexture(GL_TEXTURE_2D, tb);

    glBegin(GL_QUADS);
    {
        glColor4f(1.0, 1.0, 1.0, 1.0);
        glTexCoord2f(0, 0);
        glVertex3f(-1.0f, -1.0f, 0);

        glTexCoord2f(1, 0);
        glVertex3f(1.0f, -1.0f, 0);

        glTexCoord2f(1, 1);
        glVertex3f(1.0f, 1.0f, 0);

        glTexCoord2f(0, 1);
        glVertex3f(-1.0f, 1.0f, 0);
    }

    glEnd();

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    float rotate = 60.0 / 100.0 * (float)d->i;
    glRotatef(rotate, 1.0f, 0.0f, 0.0f);
    float scale  = 1.0 / 100.0 * (100.0 - (float)d->i);
    glScalef(scale, scale, scale);
    glTranslatef(1.0 / 100.0*(float)d->i, 1.0 / 100.0*(float)d->i, 0.0);
    glBindTexture(GL_TEXTURE_2D, ta);

    glBegin(GL_QUADS);
    {
        glColor4f(1.0, 1.0, 1.0, 1.0);

        float float_x, float_y, float_xb, float_yb;
        int x, y;

        for (x = 0; x < 39; ++x)
        {
            for (y = 0; y < 39; ++y)
            {
                float_x  = (float) x / 40.0f;
                float_y  = (float) y / 40.0f;
                float_xb = (float) (x + 1) / 40.0f;
                float_yb = (float) (y + 1) / 40.0f;
                glTexCoord2f(float_x, float_y);
                glVertex3f(d->points[x][y][0], d->points[x][y][1], d->points[x][y][2]);
                glTexCoord2f(float_x, float_yb);
                glVertex3f(d->points[x][y + 1][0], d->points[x][y + 1][1], d->points[x][y + 1][2]);
                glTexCoord2f(float_xb, float_yb);
                glVertex3f(d->points[x + 1][y + 1][0], d->points[x + 1][y + 1][1], d->points[x + 1][y + 1][2]);
                glTexCoord2f(float_xb, float_y);
                glVertex3f(d->points[x + 1][y][0], d->points[x + 1][y][1], d->points[x + 1][y][2]);
            }
        }
    }

    glEnd();

    // wave every two iterations

    if (d->i % 2 == 0)
    {

        float hold;
        int x, y;

        for (y = 0; y < 40; ++y)
        {
            hold = d->points[0][y][2];

            for (x = 0; x < 39; ++x)
            {
                d->points[x][y][2] = d->points[x + 1][y][2];
            }

            d->points[39][y][2] = hold;
        }
    }

    d->i++;
}

void PresentationGL::effectCube()
{
    int tot      = 200;
    int rotStart = 50;

    if (d->i > tot)
    {
        paintTexture();
        d->effectRunning = false;
        d->timeout       = -1;
        return;
    }

    // Enable perspective vision
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    int a      = (d->curr == 0) ? 1 : 0;
    int b      =  d->curr;
    GLuint& ta = d->texture[a];
    GLuint& tb = d->texture[b];
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

//    float PI    = 4.0 * atan(1.0);
    float znear = 3.0;
//    float theta = 2.0 * atan2((float)2.0 / (float)2.0, (float)znear);
//    theta       = theta * 180.0 / PI;

    glFrustum(-1.0, 1.0, -1.0, 1.0, znear - 0.01, 10.0);

    static float xrot;
    static float yrot;
//    static float zrot;

    if (d->i == 0)
    {
        xrot = 0.0;
        yrot = 0.0;
//        zrot = 0.0;
    }

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();

    float trans = 5.0 * (float)((d->i <= tot / 2) ? d->i : tot - d->i) / (float)tot;
    glTranslatef(0.0, 0.0, -znear - 1.0 - trans);

    glRotatef(xrot, 1.0f, 0.0f, 0.0f);
    glRotatef(yrot, 0.0f, 1.0f, 0.0f);
    glBindTexture(GL_TEXTURE_2D, 0);

    glBegin(GL_QUADS);
    {
        glColor4f(0.0f, 0.0f, 0.0f, 1.0f);

        /* Front Face */
        glVertex3f( -1.00f, -1.00f,  0.99f );
        glVertex3f(  1.00f, -1.00f,  0.99f );
        glVertex3f(  1.00f,  1.00f,  0.99f );
        glVertex3f( -1.00f,  1.00f,  0.99f );

        /* Back Face */
        glVertex3f( -1.00f, -1.00f, -0.99f );
        glVertex3f( -1.00f,  1.00f, -0.99f );
        glVertex3f(  1.00f,  1.00f, -0.99f );
        glVertex3f(  1.00f, -1.00f, -0.99f );

        /* Top Face */
        glVertex3f( -1.00f,  0.99f, -1.00f );
        glVertex3f( -1.00f,  0.99f,  1.00f );
        glVertex3f(  1.00f,  0.99f,  1.00f );
        glVertex3f(  1.00f,  0.99f, -1.00f );

        /* Bottom Face */
        glVertex3f( -1.00f, -0.99f, -1.00f );
        glVertex3f(  1.00f, -0.99f, -1.00f );
        glVertex3f(  1.00f, -0.99f,  1.00f );
        glVertex3f( -1.00f, -0.99f,  1.00f );

        /* Right face */
        glVertex3f( 0.99f, -1.00f, -1.00f );
        glVertex3f( 0.99f,  1.00f, -1.00f );
        glVertex3f( 0.99f,  1.00f,  1.00f );
        glVertex3f( 0.99f, -1.00f,  1.00f );

        /* Left Face */
        glVertex3f( -0.99f, -1.00f, -1.00f );
        glVertex3f( -0.99f, -1.00f,  1.00f );
        glVertex3f( -0.99f,  1.00f,  1.00f );
        glVertex3f( -0.99f,  1.00f, -1.00f );

    }

    glEnd();
    glBindTexture(GL_TEXTURE_2D, ta);

    glBegin(GL_QUADS);
    {
        glColor4d(1.0, 1.0, 1.0, 1.0);

        // Front Face
        glTexCoord2f( 0.0f, 0.0f );
        glVertex3f( -1.0f, -1.0f,  1.00f );
        glTexCoord2f( 1.0f, 0.0f );
        glVertex3f(  1.0f, -1.0f,  1.00f );
        glTexCoord2f( 1.0f, 1.0f );
        glVertex3f(  1.0f,  1.0f,  1.00f );
        glTexCoord2f( 0.0f, 1.0f );
        glVertex3f( -1.0f,  1.0f,  1.00f );


        // Top Face
        glTexCoord2f( 1.0f, 1.0f );
        glVertex3f( -1.0f,  1.00f, -1.0f );
        glTexCoord2f( 1.0f, 0.0f );
        glVertex3f( -1.0f,  1.00f,  1.0f );
        glTexCoord2f( 0.0f, 0.0f );
        glVertex3f(  1.0f,  1.00f,  1.0f );
        glTexCoord2f( 0.0f, 1.0f );
        glVertex3f(  1.0f,  1.00f, -1.0f );

        // Bottom Face
        glTexCoord2f( 0.0f, 1.0f );
        glVertex3f( -1.0f, -1.00f, -1.0f );
        glTexCoord2f( 1.0f, 1.0f );
        glVertex3f(  1.0f, -1.00f, -1.0f );
        glTexCoord2f( 1.0f, 0.0f );
        glVertex3f(  1.0f, -1.00f,  1.0f );
        glTexCoord2f( 0.0f, 0.0f );
        glVertex3f( -1.0f, -1.00f,  1.0f );

        // Right face
        glTexCoord2f( 0.0f, 0.0f );
        glVertex3f( 1.00f, -1.0f, -1.0f );
        glTexCoord2f( 0.0f, 1.0f );
        glVertex3f( 1.00f, -1.0f,  1.0f );
        glTexCoord2f( 1.0f, 1.0f );
        glVertex3f( 1.00f,  1.0f,  1.0f );
        glTexCoord2f( 1.0f, 0.0f );
        glVertex3f( 1.00f,  1.0f, -1.0f );

        // Left Face
        glTexCoord2f( 1.0f, 0.0f );
        glVertex3f( -1.00f, -1.0f, -1.0f );
        glTexCoord2f( 0.0f, 0.0f );
        glVertex3f( -1.00f,  1.0f, -1.0f );
        glTexCoord2f( 0.0f, 1.0f );
        glVertex3f( -1.00f,  1.0f,  1.0f );
        glTexCoord2f( 1.0f, 1.0f );
        glVertex3f( -1.00f, -1.0f,  1.0f );

    }

    glEnd();
    glBindTexture(GL_TEXTURE_2D, tb);

    glBegin(GL_QUADS);
    {
        glColor4d(1.0, 1.0, 1.0, 1.0);

        // Back Face
        glTexCoord2f( 1.0f, 0.0f );
        glVertex3f( -1.0f, -1.0f, -1.00f );
        glTexCoord2f( 1.0f, 1.0f );
        glVertex3f( -1.0f,  1.0f, -1.00f );
        glTexCoord2f( 0.0f, 1.0f );
        glVertex3f(  1.0f,  1.0f, -1.00f );
        glTexCoord2f( 0.0f, 0.0f );
        glVertex3f(  1.0f, -1.0f, -1.00f );
    }

    glEnd();

    if ((d->i >= rotStart) && (d->i < (tot - rotStart)))
    {
        xrot += 360.0f / (float)(tot - 2 * rotStart);
        yrot += 180.0f / (float)(tot - 2 * rotStart);
    }

    d->i++;
}

void PresentationGL::slotPause()
{
    d->timer->stop();

    if (d->slideCtrlWidget->isHidden())
    {
        int w = d->slideCtrlWidget->width();
        d->slideCtrlWidget->move(d->deskWidth - w - 1, 0);
        d->slideCtrlWidget->show();
    }
}

void PresentationGL::slotPlay()
{
    d->slideCtrlWidget->hide();
    slotTimeOut();
}

void PresentationGL::slotPrev()
{
    previousFrame();

    if (d->endOfShow)
    {
        updateGL();
        return;
    }

    d->effectRunning = false;

    loadImage();
    updateGL();
}

void PresentationGL::slotNext()
{
    advanceFrame();

    if (d->endOfShow)
    {
        updateGL();
        return;
    }

    d->effectRunning = false;

    loadImage();
    updateGL();
}

void PresentationGL::slotClose()
{
    close();
}

QPixmap PresentationGL::generateOutlinedTextPixmap(const QString& text)
{
    QFont fn(font());
    fn.setPointSize(fn.pointSize());
    fn.setBold(true);

    return generateOutlinedTextPixmap(text, fn);
}

QPixmap PresentationGL::generateOutlinedTextPixmap(const QString& text, QFont& fn)
{
    QColor fgColor(Qt::white);
    QColor bgColor(Qt::black);
    return generateCustomOutlinedTextPixmap(text, fn, fgColor, bgColor, 0, true);
}

QPixmap PresentationGL::generateCustomOutlinedTextPixmap(const QString& text, QFont& fn,
                                                      QColor& fgColor, QColor& bgColor,
                                                      int opacity, bool drawTextOutline)
{
    QFontMetrics fm(fn);
    QRect rect = fm.boundingRect(text);
    rect.adjust( -fm.maxWidth(), -fm.height(), fm.maxWidth(), fm.height() / 2 );

    QPixmap pix(rect.width(), rect.height());
    pix.fill(Qt::transparent);

    if(opacity > 0)
    {
        QPainter pbg(&pix);
        pbg.setBrush(bgColor);
        pbg.setPen(bgColor);
        pbg.setOpacity(opacity / 10.0);
        pbg.drawRoundedRect(0, 0, (int)pix.width(), (int)pix.height(), (int)pix.height()/3, (int)pix.height()/3);
    }

    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setBrush(QBrush());
    p.setPen(QPen());

    // draw outline
    QPainterPath path;
    path.addText(fm.maxWidth(), fm.height() * 1.5, fn, text);

    QPainterPathStroker stroker;
    stroker.setWidth(2);
    stroker.setCapStyle(Qt::RoundCap);
    stroker.setJoinStyle(Qt::RoundJoin);
    QPainterPath outline = stroker.createStroke(path);

    if (drawTextOutline)
        p.fillPath(outline, Qt::black);

    p.fillPath(path,    QBrush(fgColor));

    p.setRenderHint(QPainter::Antialiasing, false);
    p.end();

    return pix;
}

}  // namespace Digikam
