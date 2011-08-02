/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-06-20
 * Description : a digiKam image plugin to clone area .
 *
 * Copyright (C) 2011-06-20 by Zhang Jie <zhangjiehangyuan2005 dot at gmail dot com>
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

#ifndef IMAGECLONEWIDGET_H
#define IMAGECLONEWIDGET_H

//QT includes

#include <QMouseEvent>
#include <QPoint>
#include <QWidget>

//local includes

#include "imageguidewidget.h"
#include "clonecontainer.h"
#include "dimg.h"

namespace Digikam
{

class ImageIface;//08/02

class ImageCloneWidget : public QWidget
{
    Q_OBJECT

public:

    explicit ImageCloneWidget(QWidget* parent=0, const CloneContainer& settings=CloneContainer());
    ~ImageCloneWidget();
    void    paintEvent(QPaintEvent*);
    void    mousePressEvent(QMouseEvent*);
    void    mouseReleaseEvent(QMouseEvent*);
    void    mouseMoveEvent(QMouseEvent*);

    void    updatePreview();
 // DImg*   getOrigImage();
    DImg*   getMaskImg();
  //DImg*   getPreview();
    DImg*   getPreviewMask();

    QPoint  getDis();
    QPoint  getOriDis();

    void    setContainer(const CloneContainer& settings);
    void    setBackgroundColor(const QColor&);
Q_SIGNALS:

    void    drawingComplete();
    void    signalResized();

public Q_SLOTS:

    void    setPreview();

private:

    bool    inimage( DImg *img,const int x,const int y);
    bool    inBrushpixmap(QPixmap* brushmap, int x, int y);

    void    TreateAsBordor(DImg *image,const int x,const int y);
    void    addToMask(const QPoint point);
    void    upDis();

    void    paintEvent(QPaintEvent*);
    void    resizeEvent(QResizeEvent*);
    void    timerEvent(QTimerEvent*);
    void    updatePixmap();

private:

    class ImageCloneWidgetPriv;
    ImageCloneWidgetPriv*  const d;
};

} // namespace Digikam

#endif // IMAGECLONEWIDGET_H
