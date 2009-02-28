/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2002-16-10
 * Description : album icon view
 *
 * Copyright (C) 2002-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2002-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2009 by Marcel Wiesweg <marcel.wiesweg@gmx.de>
 * Copyright (C) 2009 by Andi Clemens <andi dot clemens at gmx dot net>
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

// Qt includes.

#include <QFont>
#include <QList>
#include <QRect>

// KDE includes.

#include <kservice.h>

// Local includes.

#include "albumitemhandler.h"
#include "iconview.h"
#include "imageinfo.h"
#include "imageinfolist.h"
#include "loadingdescription.h"

class QAction;
class QDragMoveEvent;
class QDropEvent;
class QPixmap;
class QPoint;
class QResizeEvent;
class QString;

namespace KIO
{
class Job;
}

class KMenu;
class KJob;

namespace Digikam
{

class Album;
class AlbumIconItem;
class AlbumIconViewPrivate;
class AlbumSettings;
class ThumbnailSize;

class AlbumIconView : public IconView,
                      public AlbumItemHandler
{
    Q_OBJECT

public:

    AlbumIconView(QWidget* parent);
    ~AlbumIconView();

    void setAlbum(Album* album);

    /** set the Url of item to select in Album View when all items will be reloaded
        by setAlbum()*/
    void setAlbumItemToFind(const KUrl& url);

    void setThumbnailSize(const ThumbnailSize& thumbSize);
    ThumbnailSize thumbnailSize() const;

    void applySettings(const AlbumSettings* settings);
    const AlbumSettings* settings() const;

    void refreshIcon(AlbumIconItem* item);

    AlbumIconItem* firstSelectedItem() const;

    KUrl::List allItems();
    KUrl::List selectedItems();

    /** Returns the list of ImageInfos of all items.
        Current selected item ImageInfo will be copied into 'current'.
     */
    ImageInfoList allImageInfos(ImageInfo *current = 0) const;
    ImageInfoList selectedImageInfos() const;

    void refresh();
    void refreshItems(const KUrl::List& itemList);

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

    QPixmap  itemBaseRegPixmap() const;
    QPixmap  itemBaseSelPixmap() const;
    QPixmap  bannerPixmap() const;
    QPixmap  ratingPixmap(int rating, bool selected) const;

    QFont    itemFontReg() const;
    QFont    itemFontCom() const;
    QFont    itemFontXtra() const;

    void     clear(bool update=true);

    AlbumIconItem* findItem(const QPoint& pos);
    AlbumIconItem* findItem(const QString& url) const;
    AlbumIconItem* nextItemToThumbnail() const;

    void insertSelectionToLightTable(bool addTo=false);
    void insertToLightTable(const ImageInfoList& list, const ImageInfo& current, bool addTo=false);

    void insertSelectionToCurrentQueue();
    void insertSelectionToNewQueue();
    void insertToQueueManager(const ImageInfoList& list, const ImageInfo& current, bool newQueue);

Q_SIGNALS:

    void signalPreviewItem(AlbumIconItem*);
    void signalItemsAdded();
    void signalItemDeleted(AlbumIconItem*);
    void signalCleared();
    void signalProgressBarMode(int, const QString&);
    void signalProgressValue(int);
    void signalItemsUpdated(const KUrl::List&);

    void signalGotoAlbumAndItem(ImageInfo&);
    void signalGotoDateAndItem(ImageInfo&);
    void signalGotoTagAndItem(int);
    void signalFindSimilar();

    void changeTagOnImageInfos(const ImageInfoList &list, const QList<int> &tagIDs,
                               bool addOrRemove, bool progress);

public Q_SLOTS:

    void slotSetExifOrientation(int orientation);
    void slotRename(AlbumIconItem* item);
    void slotDeleteSelectedItems(bool deletePermanently=false);
    void slotDeleteSelectedItemsDirectly(bool useTrash);
    void slotDisplayItem(AlbumIconItem *item=0);
    void slotAlbumModified();
    void slotSetAlbumThumbnail(ImageInfo &imageInfo);
    void slotCopy();
    void slotPaste();
    void slotNewAlbumFromSelection();

    void slotAssignRating(int rating);
    void slotAssignRatingNoStar();
    void slotAssignRatingOneStar();
    void slotAssignRatingTwoStar();
    void slotAssignRatingThreeStar();
    void slotAssignRatingFourStar();
    void slotAssignRatingFiveStar();

    void slotHandleGotoRequest();

protected:

    void resizeEvent(QResizeEvent* e);

    // DnD
    void startDrag();
    void contentsDragEnterEvent(QDragEnterEvent *e);
    void contentsDropEvent(QDropEvent *e);

    void prepareRepaint(const QList<IconItem *> &itemsToRepaint);

    bool acceptToolTip(IconItem *item, const QPoint &mousePos);

private Q_SLOTS:

    void slotImageListerNewItems(const ImageInfoList& itemList);
    void slotImageListerDeleteItem(const ImageInfo &item);
    void slotImageListerClear();

    void slotDoubleClicked(IconItem *item);
    void slotRightButtonClicked(const QPoint& pos);
    void slotRightButtonClicked(IconItem *item, const QPoint& pos);

    void slotThumbnailLoaded(const LoadingDescription &loadingDescription, const QPixmap& thumb);
    void slotSelectionChanged();

    void slotFilesModified();
    void slotFilesModified(const KUrl& url);
    void slotFileChanged(const QString &);
    void slotImageWindowURLChanged(const KUrl &url);

    void slotShowToolTip(IconItem* item);

    void slotThemeChanged();

    void slotGotoTag(int tagID);

    void slotEditRatingFromItem(int);

    void slotAssignTag(int tagID);
    void slotRemoveTag(int tagID);

    void slotDIOResult(KJob* job);
    void slotRenamed(KIO::Job*, const KUrl &, const KUrl&);

    void slotImageAttributesChanged(qlonglong imageId);
    void slotAlbumImagesChanged(int albumId);

    void slotChangeTagOnImageInfos(const ImageInfoList &list, const QList<int> &tagIDs,
                                   bool addOrRemove, bool progress);

private:

    void updateRectsAndPixmaps();
    void updateBannerRectPixmap();

private:

    AlbumIconViewPrivate* const d;
};

}  // namespace Digikam

#endif // ALBUMICONVIEW_H
