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

#ifndef PRESENTATION_GL_H
#define PRESENTATION_GL_H

#ifdef _MSC_VER
#   include <winsock2.h>
#endif

// Qt includes

#include <QGLWidget>
#include <QKeyEvent>
#include <QList>
#include <QMap>
#include <QMouseEvent>
#include <QPair>
#include <QString>
#include <QStringList>
#include <QWheelEvent>

namespace Digikam
{

class PresentationContainer;

class PresentationGL : public QGLWidget
{
    Q_OBJECT

public:

    PresentationGL(PresentationContainer* const sharedData);
    ~PresentationGL();

    void registerEffects();

    static QStringList effectNames();
    static QMap<QString, QString> effectNamesI18N();

protected:

    void initializeGL();
    void paintGL();
    void resizeGL(int w, int h);

    void mousePressEvent(QMouseEvent*);
    void mouseMoveEvent(QMouseEvent*);
    void wheelEvent(QWheelEvent*);
    void keyPressEvent(QKeyEvent*);

private:

    typedef void (PresentationGL::*EffectMethod)();

    QPixmap       generateOutlinedTextPixmap(const QString& text);
    QPixmap       generateOutlinedTextPixmap(const QString& text, QFont& fn);
    QPixmap       generateCustomOutlinedTextPixmap(const QString& text,
                                                   QFont& fn, QColor& fgColor, QColor& bgColor,
                                                   int opacity, bool transBg = true);

    void          paintTexture();
    void          advanceFrame();
    void          previousFrame();
    void          loadImage();
    void          montage(QImage& top, QImage& bot);
    EffectMethod  getRandomEffect();
    void          showEndOfShow();
    void          printFilename(QImage& layer);
    void          printProgress(QImage& layer);
    void          printComments(QImage& layer);

    void          effectNone();
    void          effectBlend();
    void          effectFade();
    void          effectRotate();
    void          effectBend();
    void          effectInOut();
    void          effectSlide();
    void          effectFlutter();
    void          effectCube();

private Q_SLOTS:

    void slotTimeOut();
    void slotMouseMoveTimeOut();

    void slotPause();
    void slotPlay();
    void slotPrev();
    void slotNext();
    void slotClose();

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif /* PRESENTATION_GL_H */
