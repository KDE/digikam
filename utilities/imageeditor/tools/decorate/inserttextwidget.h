/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-02-14
 * Description : a widget to insert a text over an image.
 *
 * Copyright (C) 2005-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef INSERTTEXTWIDGET_H
#define INSERTTEXTWIDGET_H

// Qt includes

#include <QColor>
#include <QFont>
#include <QImage>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPixmap>
#include <QRect>
#include <QResizeEvent>
#include <QSize>
#include <QString>
#include <QWidget>
#include <QUrl>

// Local includes

#include "dimg.h"

class QPixmap;

namespace Digikam
{

class ImageIface;

class InsertTextWidget : public QWidget
{
    Q_OBJECT

public:

    enum Action
    {
        ALIGN_LEFT = 0,
        ALIGN_RIGHT,
        ALIGN_CENTER,
        ALIGN_BLOCK,
        BORDER_TEXT,
        TRANSPARENT_TEXT
    };

    enum TextRotation
    {
        ROTATION_NONE = 0,
        ROTATION_90,
        ROTATION_180,
        ROTATION_270
    };

    enum BorderMode
    {
        BORDER_NONE = 0,
        BORDER_SUPPORT,
        BORDER_NORMAL
    };

public:

    InsertTextWidget(int w, int h, QWidget* const parent=0);
    ~InsertTextWidget();

    ImageIface* imageIface() const;
    DImg        makeInsertText();

    /** Set background color for pixmap
     */
    void   setBackgroundColor(const QColor& bg);

    void   setText(const QString& text, const QFont& font, const QColor& color, int opacity,
                   int alignMode, bool border, bool transparent, int rotation);

    void   resetEdit();

    void  setPositionHint(const QRect& hint);
    QRect getPositionHint() const;

protected:

    void paintEvent(QPaintEvent*);
    void resizeEvent(QResizeEvent*);
    void mousePressEvent(QMouseEvent*);
    void mouseReleaseEvent(QMouseEvent*);
    void mouseMoveEvent(QMouseEvent*);

    void makePixmap();
    QRect composeImage(DImg* const image, QPainter* const destPainter,
                       int x, int y,
                       QFont font, float pointSize, int textRotation, QColor textColor,
                       int textOpacity, int alignMode, const QString& textString,
                       bool transparentBackground, QColor backgroundColor,
                       BorderMode borderMode, int borderWidth, int spacing, float fontScale=1.0f);

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif /* INSERTTEXTWIDGET_H */
