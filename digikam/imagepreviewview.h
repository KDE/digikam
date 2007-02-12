/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date   : 2006-21-12
 * Description : a embeded view to show the image preview widget.
 * 
 * Copyright 2006-2007 Gilles Caulier
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

#ifndef IMAGEPREVIEWVIEW_H
#define IMAGEPREVIEWVIEW_H

// Qt includes.

#include <qvbox.h>

// Local includes

#include "imagepreviewwidget.h"
#include "digikam_export.h"

namespace Digikam
{

class ImagePreviewViewPriv;

class DIGIKAM_EXPORT ImagePreviewView : public QVBox
{
Q_OBJECT

public:

    ImagePreviewView(QWidget *parent=0);
    ~ImagePreviewView();

    void setImageInfo(ImageInfo* info=0, ImageInfo *previous=0, ImageInfo *next=0);
    ImageInfo* getImageInfo();

    void reload();

signals:

    void signalNextItem();
    void signalPrevItem();
    void signalDeleteItem();
    void signalEditItem();
    void signalPreviewLoaded();
    void signalBack2Album();

private slots:

    void slotThemeChanged();
    void slotAssignTag(int tagID);
    void slotRemoveTag(int tagID);
    void slotAssignRating(int rating);

private:

    void mousePressEvent(QMouseEvent* e);

private:

    ImagePreviewViewPriv* d;
};

}  // NameSpace Digikam

#endif /* IMAGEPREVIEWVIEW_H */
