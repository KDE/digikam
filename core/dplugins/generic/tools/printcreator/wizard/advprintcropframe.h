/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2002-30-09
 * Description : a tool to print images
 *
 * Copyright (C) 2002-2003 by Todd Shoemaker <todd at theshoemakers dot net>
 * Copyright (C) 2007-2012 by Angelo Naselli <anaselli at linux dot it>
 * Copyright (C) 2006-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAM_ADV_PRINT_CROP_FRAME_H
#define DIGIKAM_ADV_PRINT_CROP_FRAME_H

// Qt includes

#include <QWidget>

class QResizeEvent;

namespace GenericDigikamPrintCreatorPlugin
{

class AdvPrintPhoto;

class AdvPrintCropFrame : public QWidget
{
    Q_OBJECT

public:

    explicit AdvPrintCropFrame(QWidget* const parent);
    ~AdvPrintCropFrame();

    void   init(AdvPrintPhoto* const photo,
                int  woutlay,
                int  houtlay,
                bool autoRotate,
                bool paint);

    void   setColor(const QColor&);
    QColor color() const;

    void   drawCropRectangle(bool draw = true);

protected:

    virtual void paintEvent(QPaintEvent*);
    virtual void mousePressEvent(QMouseEvent*);
    virtual void mouseReleaseEvent(QMouseEvent*);
    virtual void mouseMoveEvent(QMouseEvent*);
    virtual void keyReleaseEvent(QKeyEvent*);

private:

    QRect screenToPhotoRect(const QRect&) const;
    QRect photoToScreenRect(const QRect&) const;
    void  resizeEvent(QResizeEvent*);
    void  updateImage();

private:

    class Private;
    Private* const d;
};

} // Namespace Digikam

#endif // DIGIKAM_ADV_PRINT_CROP_FRAME_H
