//////////////////////////////////////////////////////////////////////////////
//
//    ALBUMICONVIEW.H
//
//    Copyright (C) 2002-2004 Renchi Raju <renchi at pooh.tam.uiuc.edu>
//                            Gilles CAULIER <caulier dot gilles at free.fr>
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

namespace Digikam
{
class AlbumInfo;
}

class AlbumIconView : public ThumbView,
                      public Digikam::AlbumItemHandler
{

    Q_OBJECT

public:

    AlbumIconView(QWidget* parent);
    ~AlbumIconView();

    void setAlbum(Digikam::AlbumInfo* album);
    void setThumbnailSize(const ThumbnailSize& thumbSize);
    ThumbnailSize thumbnailSize();

    void applySettings(const AlbumSettings* settings);
    const AlbumSettings* settings();

    void refreshIcon(AlbumIconItem* item);
    void getItemComments(const QString& itemName,
                         QString& comments);
    void albumDescChanged();

    AlbumIconItem* firstSelectedItem();

    QStringList allItems();
    QStringList selectedItems();
    QStringList allItemsPath();
    QStringList selectedItemsPath();

    void refresh();
    void refreshItems(const QStringList& itemList);
    
protected:

    void calcBanner();
    void paintBanner(QPainter *p);
    void updateBanner();

    // DnD
    void startDrag();
    void contentsDragMoveEvent(QDragMoveEvent *e);
    void contentsDropEvent(QDropEvent *e);
    virtual bool eventFilter(QObject *obj, QEvent *ev);

private:

    AlbumIconViewPrivate *d;

    void AlbumIconView::exifRotate(QString filename, QPixmap& pixmap);

private slots:

    void slotImageListerNewItems(const KFileItemList& itemList);
    void slotImageListerDeleteItem(KFileItem* item);
    void slotImageListerClear();
    void slotImageListerCompleted();
    void slotImageListerRefreshItems(const KFileItemList&);

    void slotDoubleClicked(ThumbItem *item);
    void slotRightButtonClicked(ThumbItem *item, const QPoint& pos);
    void slotItemRenamed(ThumbItem *item);

    void slotGotThumbnail(const KURL& url, const QPixmap& pix);
    void slotFailedThumbnail(const KURL& url);
    void slotGotThumbnailKDE(const KFileItem*, const QPixmap&);
    void slotFailedThumbnailKDE(const KFileItem* item);
    void slotSelectionChanged();

    void slot_onDeleteSelectedItemsFinished(KIO::Job* job);

public slots:

    void slot_editImageComments(AlbumIconItem* item);
    void slot_showExifInfo(AlbumIconItem* item);
    void slotRename(AlbumIconItem* item);
    void slot_deleteSelectedItems();
    void slotDisplayItem(AlbumIconItem *item=0);
    void slotProperties(AlbumIconItem* item);

signals:

    void signal_albumCountChanged(const Digikam::AlbumInfo*);
    void signalItemsAdded();

};

#endif // ALBUMICONVIEW_H
