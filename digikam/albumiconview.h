//////////////////////////////////////////////////////////////////////////////
//
//    ALBUMICONVIEW.H
//
//    Copyright (C) 2002-2004 Renchi Raju <renchi at pooh.tam.uiuc.edu>
//                            Gilles Caulier <caulier dot gilles at free.fr>
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef ALBUMICONVIEW_H
#define ALBUMICONVIEW_H

// KDE includes.

#include <qrect.h>
#include <qfont.h>

#include <thumbview.h>
#include <kfileitem.h>
#include <kio/job.h>

// Local includes.

#include "albumitemhandler.h"

class QMouseEvent;
class QResizeEvent;
class QDragMoveEvent;
class QDropEvent;
class QPoint;
class QString;
class QPainter;
class QPixmap;

class AlbumIconItem;
class AlbumSettings;
class AlbumIconViewPrivate;
class ThumbnailSize;
class Album;
class AlbumLister;

class AlbumIconView : public ThumbView,
                      public AlbumItemHandler
{
    Q_OBJECT

public:

    AlbumIconView(QWidget* parent);
    ~AlbumIconView();

    void setAlbum(Album* album);
    void setThumbnailSize(const ThumbnailSize& thumbSize);
    ThumbnailSize thumbnailSize();

    void applySettings(const AlbumSettings* settings);
    const AlbumSettings* settings();

    QString     itemComments(AlbumIconItem *item);
    QStringList itemTagNames(AlbumIconItem* item);

    void    refreshIcon(AlbumIconItem* item);
    void    albumDescChanged();

    AlbumIconItem* firstSelectedItem();
    AlbumLister*   albumLister() const;

    QStringList allItems();
    QStringList selectedItems();
    QStringList allItemsPath();
    QStringList selectedItemsPath();

    void refresh();
    void refreshItems(const QStringList& itemList);

    QRect    itemRect() const;
    QRect    itemDateRect() const;
    QRect    itemPixmapRect() const;
    QRect    itemNameRect() const;
    QRect    itemCommentsRect() const;
    QRect    itemFileCommentsRect() const;
    QRect    itemResolutionRect() const;
    QRect    itemSizeRect() const;
    QRect    itemTagRect() const;

    QPixmap* itemBaseRegPixmap() const;
    QPixmap* itemBaseSelPixmap() const;

    QFont    itemFontReg() const;
    QFont    itemFontCom() const;
    QFont    itemFontXtra() const;
    
protected:

    void calcBanner();
    void paintBanner(QPainter *p);
    void updateBanner();

    // DnD
    void startDrag();
    void contentsDragMoveEvent(QDragMoveEvent *e);
    void contentsDropEvent(QDropEvent *e);
    void focusInEvent(QFocusEvent *e);
    void focusOutEvent(QFocusEvent *e);

private:

    AlbumIconViewPrivate *d;
    
    void           exifRotate(QString filename, QPixmap& pixmap);
    void           updateItemRectsPixmap();
    bool           showMetaInfo();
    AlbumIconItem* findItem(const QString& url) const;

private slots:

    void slotImageListerNewItems(const KFileItemList& itemList);
    void slotImageListerDeleteItem(KFileItem* item);
    void slotImageListerClear();
    void slotImageListerCompleted();
    void slotImageListerRefreshItems(const KFileItemList&);

    void slotDoubleClicked(ThumbItem *item);
    void slotRightButtonClicked(ThumbItem *item, const QPoint& pos);

    void slotGotThumbnail(const KFileItem* item, const QPixmap& pix,
                          const KFileMetaInfo* metaInfo);
    void slotFailedThumbnail(const KFileItem* item);
    void slotGotThumbnailKDE(const KFileItem* item, const QPixmap& pix);
    void slotFailedThumbnailKDE(const KFileItem* item);    
    void slotFinishedThumbnail();
    void slotSelectionChanged();
    void slotSetExifOrientation( const QString filename, int orientation );

    void slotOnDeleteSelectedItemsFinished(KIO::Job* job);
    void slotFilesModified();

    void slotContentsMoving(int x, int y);
    
public slots:

    void slotEditImageComments(AlbumIconItem* item);
    void slotShowExifInfo(AlbumIconItem* item);
    void slotRename(AlbumIconItem* item);
    void slotDeleteSelectedItems();
    void slotDisplayItem(AlbumIconItem *item=0);
    void slotProperties(AlbumIconItem* item);

signals:

    void signalItemsAdded();

};

#endif // ALBUMICONVIEW_H
