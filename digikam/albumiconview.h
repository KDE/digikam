#ifndef ALBUMICONVIEW_H
#define ALBUMICONVIEW_H

#include <thumbview.h>
#include <kfileitem.h>
#include <kio/job.h>

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

#endif
