/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-21-12
 * Description : a embedded view to show the image preview widget.
 *
 * Copyright (C) 2006-2011 by Gilles Caulier  <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009-2011 by Andi Clemens <andi dot clemens at gmx dot net>
 * Copyright (C) 2010-2011 by Aditya Bhatt <adityabhatt1991 at gmail dot com>
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

// Local includes

#include "graphicsdimgview.h"
#include "imageinfo.h"
#include "digikam_export.h"

class QPixmap;

namespace Digikam
{

class LoadingDescription;

class ImagePreviewView : public GraphicsDImgView
{
    Q_OBJECT

public:

    ImagePreviewView(QWidget* parent);
    ~ImagePreviewView();

    void setImageInfo(const ImageInfo& info     = ImageInfo(),
                      const ImageInfo& previous = ImageInfo(),
                      const ImageInfo& next     = ImageInfo());

    ImageInfo getImageInfo() const;

    void reload();
    void setImagePath(const QString& path=QString());
    void setPreviousNextPaths(const QString& previous, const QString& next);

    void showContextMenu(const ImageInfo& info, QGraphicsSceneContextMenuEvent* event);

Q_SIGNALS:

    void signalNextItem();
    void signalPrevItem();
    void signalDeleteItem();
    void signalEditItem();
    void signalPreviewLoaded(bool success);
    void signalBack2Album();
    void signalSlideShow();
    void signalInsert2LightTable();
    void signalInsert2QueueMgr();
    void signalFindSimilar();
    void signalAddToExistingQueue(int);

    void signalGotoAlbumAndItem(const ImageInfo&);
    void signalGotoDateAndItem(const ImageInfo&);
    void signalGotoTagAndItem(int);
    void signalPopupTagsView();

protected:

    bool acceptsMouseClick(QMouseEvent* e);
    void enterEvent(QEvent* e);
    void leaveEvent(QEvent* e);

private Q_SLOTS:

    void imageLoaded();
    void imageLoadingFailed();

    void slotAssignTag(int tagID);
    void slotRemoveTag(int tagID);
    void slotAssignRating(int rating);
    void slotAssignPickLabel(int pickId);
    void slotAssignColorLabel(int colorId);

    void slotThemeChanged();
    void slotSetupChanged();

    void slotRotateLeft();
    void slotRotateRight();

private:

    class ImagePreviewViewPriv;
    ImagePreviewViewPriv* const d;
};

}  // namespace Digikam

#endif /* IMAGEPREVIEWVIEW_H */
