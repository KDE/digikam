/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2006-06-13
 * Description : a widget to display an image preview
 *
 * Copyright 2006 Gilles Caulier
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

#ifndef IMAGEPREVIEWWIDGET_H
#define IMAGEPREVIEWWIDGET_H

// Qt includes.

#include <qframe.h>
#include <qstring.h>

// Local includes

#include "digikam_export.h"

class QPixmap;

namespace Digikam
{

class ImagePreviewWidgetPriv;

class DIGIKAM_EXPORT ImagePreviewWidget : public QFrame
{
Q_OBJECT

public:

    ImagePreviewWidget(QWidget *parent=0);
    ~ImagePreviewWidget();

    void setImagePath(const QString& path);

signals:

    void nextItem();
    void prevItem();
    void lastItem();
    void firstItem();
    void escapeSignal();

public slots:

    void slotThemeChanged();

protected:

    void drawContents(QPainter *);
    void resizeEvent(QResizeEvent *);
    void wheelEvent(QWheelEvent * e);
    void keyPressEvent(QKeyEvent * e);

private:

    bool loadDCRawPreview(QImage& image, const QString& path);
    void updatePixmap(void);
    void exifRotate(const QString& filePath, QImage& thumb);

private:

    ImagePreviewWidgetPriv* d;

};

}  // NameSpace Digikam

#endif /* IMAGEPREVIEWWIDGET_H */
