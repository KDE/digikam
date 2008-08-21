/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-21-12
 * Description : a embedded view to show the image preview widget.
 * 
 * Copyright (C) 2006-2008 Gilles Caulier  <caulier dot gilles at gmail dot com>
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

// Local includes.

#include "dimg.h"
#include "previewwidget.h"
#include "digikam_export.h"

class QPixmap;

namespace Digikam
{

class AlbumWidgetStack;
class LoadingDescription;
class ImageInfo;
class ImagePreviewViewPriv;

class DIGIKAM_EXPORT ImagePreviewView : public PreviewWidget
{

Q_OBJECT

public:

    ImagePreviewView(AlbumWidgetStack *parent=0);
    ~ImagePreviewView();

    void setLoadFullImageSize(bool b);

    void setImage(const DImg& image);
    DImg& getImage() const;

    void setImageInfo(ImageInfo* info=0, ImageInfo *previous=0, ImageInfo *next=0);
    ImageInfo* getImageInfo() const;

    void reload();
    void setImagePath(const QString& path=QString());
    void setPreviousNextPaths(const QString& previous, const QString &next);

signals:

    void signalNextItem();
    void signalPrevItem();
    void signalDeleteItem();
    void signalEditItem();
    void signalPreviewLoaded(bool success);
    void signalBack2Album();
    void signalSlideShow();
    void signalInsert2LightTable();

protected:

    void resizeEvent(QResizeEvent* e);

private slots:

    void slotGotImagePreview(const LoadingDescription &loadingDescription, const DImg &image);
    void slotNextPreload();
    void slotContextMenu();
    void slotAssignTag(int tagID);
    void slotRemoveTag(int tagID);
    void slotAssignRating(int rating);
    void slotThemeChanged();
    void slotCornerButtonPressed();
    void slotPanIconSelectionMoved(const QRect&, bool);
    void slotPanIconHiden();

private:

    int  previewWidth();
    int  previewHeight();
    bool previewIsNull();
    void resetPreview();
    void zoomFactorChanged(double zoom);
    void updateZoomAndSize(bool alwaysFitToWindow);
    inline void paintPreview(QPixmap *pix, int sx, int sy, int sw, int sh);

private:

    ImagePreviewViewPriv* d;
};

}  // NameSpace Digikam

#endif /* IMAGEPREVIEWVIEW_H */
