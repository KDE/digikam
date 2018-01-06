/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-11-14
 * Description : a presentation tool.
 *
 * Copyright (C) 2007-2009 by Valerio Fuoglio <valerio dot fuoglio at gmail dot com>
 * Copyright (C) 2012-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * Parts of this code are based on
 * smoothslidesaver by Carsten Weinhold <carsten dot weinhold at gmx dot de>
 * and slideshowgl by Renchi Raju <renchi dot raju at gmail dot com>
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

#include "presentationkb.h"

// C++ includes

#include <cassert>
#include <cmath>

// Qt includes

#include <QList>
#include <QApplication>
#include <QImage>
#include <QPainter>
#include <QFont>
#include <QCursor>
#include <QPixmap>
#include <QMouseEvent>
#include <QDesktopWidget>
#include <QApplication>
#include <QScreen>
#include <QWindow>

// KDE includes

#include <kconfig.h>
#include <kconfiggroup.h>
#include <klocalizedstring.h>

// Local includes

#include "digikam_config.h"
#include "digikam_debug.h"
#include "presentationcontainer.h"
#include "kbimageloader.h"
#include "kbeffect.h"
#include "presentationkb_p.h"

#ifdef HAVE_MEDIAPLAYER
#   include "presentationaudiowidget.h"
#endif

namespace Digikam
{

KBViewTrans::KBViewTrans(bool zoomIn, float relAspect)
{
    m_deltaX     = 0.0;
    m_deltaY     = 0.0;
    m_deltaScale = 0.0;
    m_baseScale  = 0.0;
    m_baseX      = 0.0;
    m_baseY      = 0.0;
    m_xScale     = 0.0;
    m_yScale     = 0.0;
    int i        = 0;

    // randomly select sizes of start and end viewport
    double s[2];

    do
    {
        s[0]  = 0.3 * rnd() + 1.0;
        s[1]  = 0.3 * rnd() + 1.0;
    }
    while ((fabs(s[0] - s[1]) < 0.15) && (++i < 10));

    if ((!zoomIn ||  (s[0] > s[1])) ||
        ( zoomIn || !(s[0] > s[1])))
    {
        double tmp = s[0];
        s[0]       = s[1];
        s[1]       = tmp;
    }

    m_deltaScale = s[1] / s[0] - 1.0;
    m_baseScale  = s[0];

    // additional scale factors to ensure proper m_aspect of the displayed image
    double x[2], y[2], xMargin[2], yMargin[2], bestDist;
    double sx, sy;

    if (relAspect > 1.0)
    {
        sx = 1.0;
        sy = relAspect;
    }
    else
    {
        sx = 1.0 / relAspect;
        sy = 1.0;
    }

    m_xScale   = sx;
    m_yScale   = sy;

    // calculate path
    xMargin[0] = (s[0] * sx - 1.0) / 2.0;
    yMargin[0] = (s[0] * sy - 1.0) / 2.0;
    xMargin[1] = (s[1] * sx - 1.0) / 2.0;
    yMargin[1] = (s[1] * sy - 1.0) / 2.0;

    i        = 0;
    bestDist = 0.0;

    do
    {
        double sign = rndSign();
        x[0]        = xMargin[0] * (0.2 * rnd() + 0.8) *  sign;
        y[0]        = yMargin[0] * (0.2 * rnd() + 0.8) * -sign;
        x[1]        = xMargin[1] * (0.2 * rnd() + 0.8) * -sign;
        y[1]        = yMargin[1] * (0.2 * rnd() + 0.8) *  sign;

        if (fabs(x[1] - x[0]) + fabs(y[1] - y[0]) > bestDist)
        {
            m_baseX  = x[0];
            m_baseY  = y[0];
            m_deltaX = x[1] - x[0];
            m_deltaY = y[1] - y[0];
            bestDist = fabs(m_deltaX) + fabs(m_deltaY);
        }

    }
    while ((bestDist < 0.3) && (++i < 10));
}

KBViewTrans::KBViewTrans()
{
    m_deltaX     = 0.0;
    m_deltaY     = 0.0;
    m_deltaScale = 0.0;
    m_baseScale  = 0.0;
    m_baseX      = 0.0;
    m_baseY      = 0.0;
    m_xScale     = 0.0;
    m_yScale     = 0.0;
}

KBViewTrans::~KBViewTrans()
{
}

float KBViewTrans::transX(float pos) const
{
    return m_baseX + m_deltaX * pos;
}

float KBViewTrans::transY(float pos) const
{
    return m_baseY + m_deltaY * pos;
}

float KBViewTrans::scale (float pos) const
{
    return m_baseScale * (1.0 + m_deltaScale * pos);
}

float KBViewTrans::xScaleCorrect() const
{
    return m_xScale;
}

float KBViewTrans::yScaleCorrect() const
{
    return m_yScale;
}

double KBViewTrans::rnd() const
{
    return (double)qrand() / (double)RAND_MAX;
}

double KBViewTrans::rndSign() const
{
    return (qrand() < RAND_MAX / 2) ? 1.0 : -1.0;
}

// -------------------------------------------------------------------------

KBImage::KBImage(KBViewTrans* const viewTrans, float aspect)
{
    this->m_viewTrans = viewTrans;
    this->m_aspect    = aspect;
    this->m_pos       = 0.0;
    this->m_opacity   = 0.0;
    this->m_paint     = (m_viewTrans) ? true : false;
    this->m_texture   = 0;
}

KBImage::~KBImage()
{
    delete m_viewTrans;

    if (glIsTexture(m_texture))
        glDeleteTextures(1, &m_texture);
}

// -------------------------------------------------------------------------

PresentationKB::PresentationKB(PresentationContainer* const sharedData)
    : QGLWidget(),
      d(new Private)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowFlags(Qt::X11BypassWindowManagerHint | Qt::WindowStaysOnTopHint | Qt::Popup);

    QRect deskRect = QApplication::desktop()->screenGeometry( QApplication::activeWindow() );
    d->deskX        = deskRect.x();
    d->deskY        = deskRect.y();
    d->deskWidth    = deskRect.width();
    d->deskHeight   = deskRect.height();

    move(d->deskX, d->deskY);
    resize(d->deskWidth, d->deskHeight);

    d->sharedData   = sharedData;

    srand(QTime::currentTime().msec());
    readSettings();

    unsigned frameRate;

    if (d->forceFrameRate == 0)
    {
        int rate = 25;

        QWindow* const handle = windowHandle();

        if (handle)
        {
            QScreen* const screen = handle->screen();

            if (screen)
            {
                rate = (int)screen->refreshRate();
            }
        }

        frameRate = rate * 2;
    }
    else
    {
        frameRate = d->forceFrameRate;
    }

    qCDebug(DIGIKAM_GENERAL_LOG) << "Frame Rate : " << frameRate;

    d->image[0]        = new KBImage(0);
    d->image[1]        = new KBImage(0);
    d->step            = 1.0 / ((float) (d->delay * frameRate));
    d->imageLoadThread = new KBImageLoader(d->sharedData, width(), height());
    d->timer           = new QTimer;

    connect(d->timer, SIGNAL(timeout()),
            this, SLOT(moveSlot()));

    connect(d->imageLoadThread, SIGNAL(signalEndOfShow()),
            this, SLOT(slotEndOfShow()));

    // -- hide cursor when not moved --------------------

    d->mouseMoveTimer = new QTimer;

    connect(d->mouseMoveTimer, SIGNAL(timeout()),
            this, SLOT(slotMouseMoveTimeOut()));

    setMouseTracking(true);

    slotMouseMoveTimeOut();

    // -- playback widget -------------------------------

#ifdef HAVE_MEDIAPLAYER

    d->playbackWidget = new PresentationAudioWidget(this, d->sharedData->soundtrackUrls, d->sharedData);
    d->playbackWidget->hide();
    d->playbackWidget->move(d->deskX, d->deskY);

#endif

    // -- load image and let's start

    d->imageLoadThread->start();
    d->timer->start(1000 / frameRate);
}

PresentationKB::~PresentationKB()
{
    delete d->effect;
    delete d->image[0];
    delete d->image[1];

    d->imageLoadThread->quit();
    bool terminated = d->imageLoadThread->wait(10000);

    if (!terminated)
    {
        d->imageLoadThread->terminate();
        terminated = d->imageLoadThread->wait(3000);
    }

    delete d->imageLoadThread;
    delete d->mouseMoveTimer;
    delete d->timer;
    delete d;
}

float PresentationKB::aspect() const
{
    return (float)width() / (float)height();
}

void PresentationKB::setNewKBEffect()
{
    KBEffect::Type type;
    bool needFadeIn = ((d->effect == 0) || (d->effect->type() == KBEffect::Fade));

    // we currently only have two effects

    if (d->disableFadeInOut)
        type = KBEffect::Blend;
    else if (d->disableCrossFade)
        type = KBEffect::Fade;
    else
        type = KBEffect::chooseKBEffect((d->effect) ? d->effect->type() : KBEffect::Fade);

    delete d->effect;

    switch (type)
    {

        case KBEffect::Fade:
            d->effect = new FadeKBEffect(this, needFadeIn);
            break;

        case KBEffect::Blend:
            d->effect = new BlendKBEffect(this, needFadeIn);
            break;

        default:
            qCDebug(DIGIKAM_GENERAL_LOG) << "Unknown transition effect, falling back to crossfade";
            d->effect = new BlendKBEffect(this, needFadeIn);
            break;
    }
}

void PresentationKB::moveSlot()
{
    if (d->initialized)
    {
        if (d->effect->done())
        {
            setNewKBEffect();
            d->imageLoadThread->requestNewImage();
        }

        d->effect->advanceTime(d->step);
    }

    updateGL();
}

bool PresentationKB::setupNewImage(int idx)
{
    assert(idx >= 0 && idx < 2);

    if ( !d->haveImages)
        return false;

    bool ok  = false;
    d->zoomIn = !d->zoomIn;

    if (d->imageLoadThread->grabImage())
    {
        delete d->image[idx];

        // we have the image lock and there is an image
        float imageAspect            = d->imageLoadThread->imageAspect();
        KBViewTrans* const viewTrans = new KBViewTrans(d->zoomIn, aspect() / imageAspect);
        d->image[idx]                = new KBImage(viewTrans, imageAspect);

        applyTexture(d->image[idx], d->imageLoadThread->image());
        ok = true;

    }
    else
    {
        d->haveImages = false;
    }

    // don't forget to release the lock on the copy of the image
    // owned by the image loader thread
    d->imageLoadThread->ungrabImage();

    return ok;
}

void PresentationKB::startSlideShowOnce()
{
    // when the image loader thread is ready, it will already have loaded
    // the first image
    if (d->initialized == false && d->imageLoadThread->ready())
    {
        setupNewImage(0);                      // setup the first image and
        d->imageLoadThread->requestNewImage(); // load the next one in background
        setNewKBEffect();                      // set the initial effect

        d->initialized = true;
    }
}

void PresentationKB::swapImages()
{
    KBImage* const tmp = d->image[0];
    d->image[0]         = d->image[1];
    d->image[1]         = tmp;
}

void PresentationKB::initializeGL()
{
    // Enable Texture Mapping
    glEnable(GL_TEXTURE_2D);

    // Clear The Background Color
    glClearColor(0.0, 0.0, 0.0, 1.0f);

    glEnable (GL_TEXTURE_2D);
    glShadeModel (GL_SMOOTH);

    // Turn Blending On
    glEnable(GL_BLEND);
    // Blending Function For Translucency Based On Source Alpha Value
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Enable perspective vision
    glClearDepth(1.0f);
}

void PresentationKB::paintGL()
{
    startSlideShowOnce();

    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    // only clear the color buffer, if none of the active images is fully opaque

    if (!((d->image[0]->m_paint && d->image[0]->m_opacity == 1.0) ||
        (d->image[1]->m_paint && d->image[1]->m_opacity == 1.0)))
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    if (d->endOfShow && d->image[0]->m_paint && d->image[1]->m_paint)
    {
        endOfShow();
        d->timer->stop();
    }
    else
    {
        if (d->image[1]->m_paint)
            paintTexture(d->image[1]);

        if (d->image[0]->m_paint)
            paintTexture(d->image[0]);
    }

    glFlush();
}

void PresentationKB::resizeGL(int w, int h)
{
    glViewport(0, 0, (GLint) w, (GLint) h);
}

void PresentationKB::applyTexture(KBImage* const img, const QImage &texture)
{
    /* create the texture */
    glGenTextures(1, &img->m_texture);
    glBindTexture(GL_TEXTURE_2D, img->m_texture);

    /* actually generate the texture */
    glTexImage2D(GL_TEXTURE_2D, 0, 3, texture.width(), texture.height(), 0,GL_RGBA, GL_UNSIGNED_BYTE, texture.bits());

    /* enable linear filtering  */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}

void PresentationKB::paintTexture(KBImage* const img)
{
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    float sx = img->m_viewTrans->xScaleCorrect();
    float sy = img->m_viewTrans->yScaleCorrect();

    glTranslatef(img->m_viewTrans->transX(img->m_pos) * 2.0, img->m_viewTrans->transY(img->m_pos) * 2.0, 0.0);
    glScalef(img->m_viewTrans->scale(img->m_pos), img->m_viewTrans->scale(img->m_pos), 0.0);

    GLuint& tex = img->m_texture;

    glBindTexture(GL_TEXTURE_2D, tex);

    glBegin(GL_QUADS);
    {
        glColor4f(1.0, 1.0, 1.0, img->m_opacity);
        glTexCoord2f(0, 0);
        glVertex3f(-sx, -sy, 0);

        glTexCoord2f(1, 0);
        glVertex3f(sx, -sy, 0);

        glTexCoord2f(1, 1);
        glVertex3f(sx, sy, 0);

        glTexCoord2f(0, 1);
        glVertex3f(-sx, sy, 0);
    }

    glEnd();
}

void PresentationKB::readSettings()
{
    KConfig config;
    KConfigGroup group = config.group("Presentation Settings");

    d->delay            = group.readEntry("Delay", 8000) / 1000;
    d->disableFadeInOut = group.readEntry("KB Disable FadeInOut", false);
    d->disableCrossFade = group.readEntry("KB Disable Crossfade", false);
    d->forceFrameRate   = group.readEntry("KB Force Framerate", 0);

    if (d->delay < 5)
        d->delay = 5;

    if (d->forceFrameRate > 120)
        d->forceFrameRate = 120;
}

void PresentationKB::endOfShow()
{
    QPixmap pix(512, 512);
    pix.fill(Qt::black);

    QFont fn(font());
    fn.setPointSize(fn.pointSize() + 10);
    fn.setBold(true);

    QPainter p(&pix);
    p.setPen(Qt::white);
    p.setFont(fn);
    p.drawText(20, 50, i18n("SlideShow Completed"));
    p.drawText(20, 100, i18n("Click to Exit..."));
    p.end();

    QImage image = pix.toImage();
    QImage t     = convertToGLFormat(image);

    GLuint tex;

    /* create the texture */
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    /* actually generate the texture */
    glTexImage2D(GL_TEXTURE_2D, 0, 3, t.width(), t.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, t.bits());

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

    d->showingEnd = true;
}

QStringList PresentationKB::effectNames()
{
    QStringList effects;

    effects.append(QString::fromLatin1("Ken Burns"));
    return effects;
}

QMap<QString, QString> PresentationKB::effectNamesI18N()
{
    QMap<QString, QString> effects;

    effects[QString::fromLatin1("Ken Burns")] = i18n("Ken Burns");

    return effects;
}

void PresentationKB::keyPressEvent(QKeyEvent* event)
{
    if (!event)
        return;

#ifdef HAVE_MEDIAPLAYER
    d->playbackWidget->keyPressEvent(event);
#endif

    if (event->key() == Qt::Key_Escape)
        close();
}

void PresentationKB::mousePressEvent(QMouseEvent* e)
{
    if ( !e )
        return;

    if (d->endOfShow && d->showingEnd)
        slotClose();
}

void PresentationKB::mouseMoveEvent(QMouseEvent* e)
{
    setCursor(QCursor(Qt::ArrowCursor));
    d->mouseMoveTimer->start(1000);
    d->mouseMoveTimer->setSingleShot(true);

#ifdef HAVE_MEDIAPLAYER
    if (!d->playbackWidget->canHide())
        return;

    QPoint pos(e->pos());

    if ((pos.y() > (d->deskY + 20)) && (pos.y() < (d->deskY + d->deskHeight - 20 - 1)))
    {
        if (d->playbackWidget->isHidden())
            return;
        else
            d->playbackWidget->hide();

        return;
    }

    d->playbackWidget->show();
#else
    Q_UNUSED(e);
#endif
}

void PresentationKB::slotEndOfShow()
{
    d->endOfShow = true;
}

void PresentationKB::slotMouseMoveTimeOut()
{
    QPoint pos(QCursor::pos());

    if ((pos.y() < (d->deskY + 20)) || (pos.y() > (d->deskY + d->deskHeight - 20 - 1)))
        return;

    setCursor(QCursor(Qt::BlankCursor));
}

void PresentationKB::slotClose()
{
    close();
}

}  // namespace Digikam
