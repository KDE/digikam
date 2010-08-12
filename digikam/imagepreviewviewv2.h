/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-21-12
 * Description : a embedded view to show the image preview widget.
 *
 * Copyright (C) 2006-2010 Gilles Caulier  <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009-2010 by Andi Clemens <andi dot clemens at gmx dot net>
 * Copyright (C) 2010 by Aditya Bhatt <adityabhatt1991 at gmail dot com>
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

#ifndef IMAGEPREVIEWVIEWV2_H
#define IMAGEPREVIEWVIEWV2_H

// Local includes

#include "graphicsdimgview.h"
#include "imageinfo.h"
#include "digikam_export.h"

class QPixmap;

namespace Digikam
{

class AlbumWidgetStack;
class ImagePreviewViewV2Priv;
class LoadingDescription;

class ImagePreviewViewV2 : public GraphicsDImgView
{
    Q_OBJECT

public:

    ImagePreviewViewV2(AlbumWidgetStack* parent);
    ~ImagePreviewViewV2();

    void setImageInfo(const ImageInfo& info = ImageInfo(),
                      const ImageInfo& previous = ImageInfo(),
                      const ImageInfo& next = ImageInfo());

    ImageInfo getImageInfo() const;

    void reload();
    void setImagePath(const QString& path=QString());
    void setPreviousNextPaths(const QString& previous, const QString& next);

    void showContextMenu(const ImageInfo& info, QGraphicsSceneContextMenuEvent* event);

    void updateScale();
    void findFaces();
    void trainFaces();
    void suggestFaces();
    void drawFaceItems();
    void clearFaceItems();
    
    bool hasBeenScanned();
    
    void makeFaceItemConnections();
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

private Q_SLOTS:

    void imageLoaded();
    void imageLoadingFailed();

    void slotAssignTag(int tagID);
    void slotRemoveTag(int tagID);
    void slotAssignRating(int rating);

    void slotThemeChanged();
    void slotSetupChanged();

    void slotRotateLeft();
    void slotRotateRight();

    void slotTogglePeople();
    void slotHidePeopleTags();
    void slotShowPeopleTags();
    void slotRefreshPeopleTags();
    void slotAddPersonTag();
    void slotUpdatePersonTagScales();

    void slotForgetFaces();
    
    void slotRemoveFaceTag(const QString&, const QRect&);
    void slotTagPerson(const QString&, const QRect&);
    
private:

    ImagePreviewViewV2Priv* const d;
};

}  // namespace Digikam

#endif /* IMAGEPREVIEWVIEW_H */
