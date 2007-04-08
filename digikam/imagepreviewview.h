/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at gmail dot com>
 * Date   : 2006-21-12
 * Description : a embedded view to show the image preview widget.
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

#include <qstring.h>
#include <qimage.h>

// Local includes

#include "imagepreviewwidget.h"
#include "digikam_export.h"

namespace Digikam
{

class LoadingDescription;
class ImageInfo;
class ImagePreviewViewPriv;

class DIGIKAM_EXPORT ImagePreviewView : public ImagePreviewWidget
{

Q_OBJECT

public:

    ImagePreviewView(QWidget *parent=0);
    ~ImagePreviewView();

    void setImageInfo(ImageInfo* info=0, ImageInfo *previous=0, ImageInfo *next=0);
    ImageInfo* getImageInfo();

    void reload();
    void setImagePath(const QString& path=QString());
    void setPreviousNextPaths(const QString& previous, const QString &next);

signals:

    void signalPreviewStarted();
    void signalNextItem();
    void signalPrevItem();
    void signalDeleteItem();
    void signalEditItem();
    void signalPreviewLoaded();
    void signalBack2Album();
    void signalSlideShow();

private slots:

    void slotGotImagePreview(const LoadingDescription &loadingDescription, const QImage &image);
    void slotNextPreload();
    void slotContextMenu();
    void slotAssignTag(int tagID);
    void slotRemoveTag(int tagID);
    void slotAssignRating(int rating);

private:

    ImagePreviewViewPriv* d;
};

}  // NameSpace Digikam

#endif /* IMAGEPREVIEWVIEW_H */
