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

#include <QtGui/QWidget>
#include <QtGui/QColor>
#include <QtGui/QPixmap>
#include <QtGui/QResizeEvent>
#include <QtGui/QMouseEvent>
#include <QtGui/QPaintEvent>
#include <QtCore/QPoint>
#include <QtCore/QEvent>
#include <QtCore/QTimerEvent>

//local includes

#include "clonecontainer.h"
#include "dimg.h"
#include "dcolor.h"
#include "digikam_export.h"

namespace Digikam
{

class ImageIface;

class DIGIKAM_EXPORT ImageCloneWidget : public QWidget
{
    Q_OBJECT

public:

    explicit ImageCloneWidget(QWidget* parent=0, const CloneContainer& settings=CloneContainer());
    ~ImageCloneWidget();

    ImageIface* imageIface() const;

    DImg    getPreview() const;
    //DImg*   getOrigImage();
    DImg*   getMaskImg() const; 
    DImg*   getPreviewMask() const;
    QPoint  getDis() const;
    QPoint  getOriDis();

    void    setContainer(const CloneContainer& settings);
    void    setBackgroundColor(const QColor& bg);
    void    updatePreview();
    void    updateResult();  // show the result image of filter()
    CloneContainer getContainer() const;

Q_SIGNALS:

    void    signalStrokeOver();
    void    signalResized();

public Q_SLOTS:

    void    setPreview();
    void    setPreviewImage(DImg previewImage);

private:

    void    paintEvent(QPaintEvent*);
    void    resizeEvent(QResizeEvent*);
    void    timerEvent(QTimerEvent*);
    void    mousePressEvent(QMouseEvent*);
    void    mouseReleaseEvent(QMouseEvent*);
    void    mouseMoveEvent(QMouseEvent*);

    bool    inimage(DImg img, const int x, const int y);
    bool    inimage(DImg *img, const int x, const int y);
    bool    inBrushpixmap(QImage brushimg, const int x, const int y);

    double  Max(double a, double b);
    double  Min(double a, double b);

    void    TreateAsBordor(DImg image, const int x, const int y);
    void    addToMask(const QPoint& point);
    void    upDis();
    void    updatePixmap();

private:

    class ImageCloneWidgetPriv;
    ImageCloneWidgetPriv* const d;
};

} // namespace Digikam

#endif // IMAGECLONEWIDGET_H
