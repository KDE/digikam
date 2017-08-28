/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-06-15
 * Description : a brush for use with tool to replace part of the image using another
 *
 * Copyright (C) 2004-2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2017      by Shaza Ismail Kaoud <shaza dot ismail dot k at gmail dot com>
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

#ifndef IMAGEBRUSHGUIDEWIDGET_H
#define IMAGEBRUSHGUIDEWIDGET_H

#include "imageguidewidget.h"

namespace Digikam
{

class ImageBrushGuideWidget : public ImageGuideWidget
{
    Q_OBJECT
public:
    /**
     * Using the parent's constructor
     * Should be changed to get rid of the inheritance
     */
    using ImageGuideWidget::ImageGuideWidget;

    /**
     * @brief setSrcSet fixes/unfixes the source spot
     * @param s
     */
    void setSrcSet(bool s);

    /**
     * @brief isSrcSet
     * @return if the source spot is set
     */
    bool isSrcSet();
Q_SIGNALS:
    /**
     * @brief signalClone emitted when the src is set and the user initiated a brush click
     * and keeps emmitting with motion
     */
    void signalClone(QPoint&,QPoint&);

public Q_SLOTS:
    /**
     * @brief slotSrcSet toggles the fixing of the brush source center
     */
    void slotSrcSet();

private:
    void mouseMoveEvent(QMouseEvent*);
    void mouseReleaseEvent(QMouseEvent*);
    void mousePressEvent(QMouseEvent*);
    bool srcSet=false; //toggle in srcSet slot
    bool released=true;
    QPoint src;
    QPoint dst;
};

}
#endif // IMAGEBRUSHGUIDEWIDGET_H
