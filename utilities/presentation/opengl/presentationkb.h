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

#ifndef PRESENTATION_KB_H
#define PRESENTATION_KB_H

// C++ includes

#ifndef _MSC_VER
#include <cstdlib>
#else
#include <winsock2.h>
#endif

// Qt includes

#include <QList>
#include <QKeyEvent>
#include <QMap>
#include <QMouseEvent>
#include <QPair>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <QGLWidget>

namespace Digikam
{

class PresentationContainer;

class KBViewTrans
{

public:

    KBViewTrans(bool zoomIn, float relAspect);
    KBViewTrans();
    ~KBViewTrans();

    float transX(float pos) const;
    float transY(float pos) const;
    float scale(float pos)  const;
    float xScaleCorrect()   const;
    float yScaleCorrect()   const;

private:

    double rnd() const;
    double rndSign() const;

private:

    // delta and scale values (begin to end) and the needed offsets
    double m_deltaX;
    double m_deltaY;
    double m_deltaScale;
    double m_baseScale;
    double m_baseX;
    double m_baseY;
    float  m_xScale;
    float  m_yScale;
};

// -------------------------------------------------------------------------

class KBImage
{

public:

    explicit KBImage(KBViewTrans* const viewTrans, float aspect = 1.0);
    ~KBImage();

public:

    KBViewTrans* m_viewTrans;
    float        m_aspect;
    float        m_pos;
    float        m_opacity;
    bool         m_paint;
    GLuint       m_texture;
};

// -------------------------------------------------------------------------

class PresentationKB : public QGLWidget
{
    Q_OBJECT

public:

    PresentationKB(PresentationContainer* const sharedData);

    ~PresentationKB();

    static QStringList            effectNames();
    static QMap<QString, QString> effectNamesI18N();

private:

    float    aspect() const;
    bool     setupNewImage(int imageIndex);
    void     startSlideShowOnce();
    void     swapImages();
    void     setNewKBEffect();
    void     endOfShow();

    void     applyTexture(KBImage* const img, const QImage& image);
    void     paintTexture(KBImage* const img);
    unsigned suggestFrameRate(unsigned forceRate);

    void     readSettings();

protected:

    void initializeGL();
    void paintGL();
    void resizeGL(int w, int h);

    void mousePressEvent(QMouseEvent*);
    void mouseMoveEvent(QMouseEvent*);
    void keyPressEvent(QKeyEvent*);

private Q_SLOTS:

    void moveSlot();
    void slotEndOfShow();
    void slotMouseMoveTimeOut();
    void slotClose();

private:

    class Private;
    Private* const d;

    friend class KBEffect;
};

}  // namespace Digikam

#endif // PRESENTATION_KB_H
