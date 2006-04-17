/* ============================================================
 * Authors: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *          Caulier Gilles <caulier dot gilles at kdemail dot net>
 * Date  : 2002-16-10
 * Description : 
 * 
 * Copyright 2002-2005 by Renchi Raju and Gilles Caulier
 * Copyright      2006 by Gilles Caulier
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

#ifndef ALBUMICONVIEW_H
#define ALBUMICONVIEW_H

// KDE includes.

#include <qrect.h>
#include <qfont.h>

// Local includes.

#include "iconview.h"
#include "imageinfo.h"
#include "albumitemhandler.h"

class QMouseEvent;
class QResizeEvent;
class QDragMoveEvent;
class QDropEvent;
class QPoint;
class QString;
class QPainter;
class QPixmap;

namespace KIO
{
class Job;
}

namespace Digikam
{

class AlbumIconItem;
class AlbumSettings;
class AlbumIconViewPrivate;
class ThumbnailSize;
class Album;
class PixmapManager;

class AlbumIconView : public IconView,
                      public AlbumItemHandler
{
    Q_OBJECT

public:

    AlbumIconView(QWidget* parent);
    ~AlbumIconView();

    void setAlbum(Album* album);
    void setThumbnailSize(const ThumbnailSize& thumbSize);
    ThumbnailSize thumbnailSize() const;

    void applySettings(const AlbumSettings* settings);
    const AlbumSettings* settings() const;

    void refreshIcon(AlbumIconItem* item);

    AlbumIconItem* firstSelectedItem() const;

    KURL::List allItems();
    KURL::List selectedItems();

    void refresh();
    void refreshItems(const KURL::List& itemList);

    QRect    itemRect() const;
    QRect    itemRatingRect() const;
    QRect    itemDateRect() const;
    QRect    itemModDateRect() const;
    QRect    itemPixmapRect() const;
    QRect    itemNameRect() const;
    QRect    itemCommentsRect() const;
    QRect    itemResolutionRect() const;
    QRect    itemSizeRect() const;
    QRect    itemTagRect() const;
    QRect    bannerRect() const;

    QPixmap* itemBaseRegPixmap() const;
    QPixmap* itemBaseSelPixmap() const;
    QPixmap  bannerPixmap() const;
    QPixmap  ratingPixmap() const;

    QFont    itemFontReg() const;
    QFont    itemFontCom() const;
    QFont    itemFontXtra() const;

    void     clear(bool update=true);

    AlbumIconItem* findItem(const QPoint& pos);
    AlbumIconItem* findItem(const QString& url) const;
    AlbumIconItem* nextItemToThumbnail() const;
    PixmapManager* pixmapManager() const;
    
protected:

    void resizeEvent(QResizeEvent* e);

    // DnD
    void startDrag();
    void contentsDragMoveEvent(QDragMoveEvent *e);
    void contentsDropEvent(QDropEvent *e);

    bool acceptToolTip(IconItem *item, const QPoint &mousePos);
    
private slots:

    void slotImageListerNewItems(const ImageInfoList& itemList);
    void slotImageListerDeleteItem(ImageInfo* item);
    void slotImageListerClear();

    void slotDoubleClicked(IconItem *item);
    void slotRightButtonClicked(const QPoint& pos);
    void slotRightButtonClicked(IconItem *item, const QPoint& pos);

    void slotGotThumbnail(const KURL& url);
    void slotSelectionChanged();

    void slotFilesModified();
    void slotFilesModified(const KURL& url);

    void slotShowToolTip(IconItem* item);

    void slotThemeChanged();

    void slotAssignTag(int tagID);
    void slotRemoveTag(int tagID);

    void slotDIOResult(KIO::Job* job);

    void slotImageAttributesChanged(Q_LLONG imageId);
    void slotAlbumImagesChanged(int albumId);

public slots:

    void slotSetExifOrientation(int orientation);
    void slotRename(AlbumIconItem* item);
    void slotDeleteSelectedItems();
    void slotDisplayItem(AlbumIconItem *item=0);
    void slotAlbumModified();
    void slotSetAlbumThumbnail(AlbumIconItem *iconItem);
    void slotCopy();
    void slotPaste();
    
    void slotAssignRating(int rating);
    void slotAssignRatingNoStar();
    void slotAssignRatingOneStar();
    void slotAssignRatingTwoStar();
    void slotAssignRatingThreeStar();
    void slotAssignRatingFourStar();
    void slotAssignRatingFiveStar();
    
signals:

    void signalItemsAdded();
    void signalItemDeleted(AlbumIconItem* iconItem);
    void signalCleared();

private:
    
    void updateBannerRectPixmap();
    void updateItemRectsPixmap();
        
private:

    AlbumIconViewPrivate *d;
    
};

}  // namespace Digikam

#endif // ALBUMICONVIEW_H
