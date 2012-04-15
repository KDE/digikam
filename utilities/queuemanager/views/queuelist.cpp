/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-11-21
 * Description : Batch Queue Manager items list.
 *
 * Copyright (C) 2008-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "queuelist.moc"

// Qt includes

#include <QDragEnterEvent>
#include <QFileInfo>
#include <QHeaderView>
#include <QPainter>
#include <QTimer>
#include <QUrl>

// KDE includes

#include <kaction.h>
#include <kactioncollection.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmenu.h>
#include <kurl.h>

// Local includes

#include "albumdb.h"
#include "databasechangesets.h"
#include "databasewatch.h"
#include "ddragobjects.h"
#include "defaultrenameparser.h"
#include "queuemgrwindow.h"
#include "queuetooltip.h"
#include "thumbnailloadthread.h"
#include "thumbnailsize.h"

namespace Digikam
{

class QueueListViewItem::QueueListViewItemPriv
{

public:

    QueueListViewItemPriv() :
        done(false),
        hasThumb(false)
    {
    }

    bool      done;
    bool      hasThumb;

    QString   destFileName;

    QPixmap   preview;

    ImageInfo info;
};

QueueListViewItem::QueueListViewItem(QTreeWidget* view, const ImageInfo& info)
    : QTreeWidgetItem(view), d(new QueueListViewItemPriv)
{
    setThumb(SmallIcon("image-x-generic", KIconLoader::SizeLarge, KIconLoader::DisabledState), false);
    setInfo(info);
}

QueueListViewItem::~QueueListViewItem()
{
    delete d;
}

bool QueueListViewItem::hasValidThumbnail() const
{
    return d->hasThumb;
}

void QueueListViewItem::setInfo(const ImageInfo& info)
{
    d->info = info;
    setText(1, d->info.name());
}

ImageInfo QueueListViewItem::info() const
{
    return d->info;
}

void QueueListViewItem::setPixmap(const QPixmap& pix)
{
    QIcon icon = QIcon(pix);
    //  We make sure the preview icon stays the same regardless of the role
    icon.addPixmap(pix, QIcon::Selected, QIcon::On);
    icon.addPixmap(pix, QIcon::Selected, QIcon::Off);
    icon.addPixmap(pix, QIcon::Active,   QIcon::On);
    icon.addPixmap(pix, QIcon::Active,   QIcon::Off);
    icon.addPixmap(pix, QIcon::Normal,   QIcon::On);
    icon.addPixmap(pix, QIcon::Normal,   QIcon::Off);
    setIcon(0, icon);
}

void QueueListViewItem::setThumb(const QPixmap& pix, bool hasThumb)
{
    QSize iSize = treeWidget()->iconSize();
    QPixmap pixmap(iSize.width() + 2, iSize.height() + 2);
    pixmap.fill(Qt::transparent);
    QPainter p(&pixmap);
    p.drawPixmap((pixmap.width() / 2) - (pix.width() / 2), (pixmap.height() / 2) - (pix.height() / 2), pix);
    d->preview  = pixmap;
    setPixmap(d->preview);
    d->hasThumb = hasThumb;
}

void QueueListViewItem::setProgressIcon(const QPixmap& icon)
{
    QPixmap preview = d->preview;
    QPixmap mask(preview.size());
    mask.fill(QColor(128, 128, 128, 192));
    QPainter p(&preview);
    p.drawPixmap(0, 0, mask);
    p.drawPixmap((preview.width() / 2) - (icon.width() / 2), (preview.height() / 2) - (icon.height() / 2), icon);
    setPixmap(preview);
}

void QueueListViewItem::setCanceled()
{
    setPixmap(d->preview);
    setIcon(1, SmallIcon("dialog-cancel"));
    d->done = false;
}

void QueueListViewItem::setFailed()
{
    setPixmap(d->preview);
    setIcon(1, SmallIcon("dialog-error"));
    d->done = false;
}

void QueueListViewItem::setDone()
{
    setPixmap(d->preview);
    setIcon(1, SmallIcon("dialog-ok"));
    d->done = true;
}

bool QueueListViewItem::isDone()
{
    return d->done;
}

void QueueListViewItem::reset()
{
    setPixmap(d->preview);
    setIcon(1, QIcon());
    d->done = false;
}

void QueueListViewItem::setDestFileName(const QString& str)
{
    d->destFileName = str;
    setText(2, d->destFileName);
}

QString QueueListViewItem::destFileName() const
{
    return d->destFileName;
}

QString QueueListViewItem::destBaseName() const
{
    QFileInfo fi(d->destFileName);
    return fi.completeBaseName();
}

QString QueueListViewItem::destSuffix() const
{
    QFileInfo fi(d->destFileName);
    return fi.suffix();
}

// ---------------------------------------------------------------------------

class QueueListView::QueueListViewPriv
{

public:

    enum RemoveItemsType
    {
        ItemsSelected = 0,
        ItemsDone,
        ItemsAll
    };

public:

    QueueListViewPriv()
        : iconSize(64)
    {
        showTips        = false;
        toolTipTimer    = 0;
        toolTip         = 0;
        toolTipItem     = 0;
        thumbLoadThread = ThumbnailLoadThread::defaultThread();
    }

    bool                 showTips;

    const int            iconSize;

    QTimer*              toolTipTimer;

    ThumbnailLoadThread* thumbLoadThread;

    QueueSettings        settings;

    AssignedBatchTools   toolsList;

    QueueToolTip*        toolTip;

    QueueListViewItem*   toolTipItem;
};

QueueListView::QueueListView(QWidget* parent)
    : QTreeWidget(parent), d(new QueueListViewPriv)
{
    setIconSize(QSize(d->iconSize, d->iconSize));
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setWhatsThis(i18n("This is the list of images to batch process."));

    setAcceptDrops(true);
    viewport()->setAcceptDrops(true);
    setDropIndicatorShown(true);
    setDragEnabled(true);
    viewport()->setMouseTracking(true);

    setSortingEnabled(false);
    setAllColumnsShowFocus(true);
    setRootIsDecorated(false);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setColumnCount(3);
    setContextMenuPolicy(Qt::CustomContextMenu);

    QStringList titles;
    titles.append(i18n("Thumbnail"));
    titles.append(i18n("File Name"));
    titles.append(i18n("Target"));
    setHeaderLabels(titles);
    header()->setResizeMode(0, QHeaderView::ResizeToContents);
    header()->setResizeMode(1, QHeaderView::Stretch);
    header()->setResizeMode(2, QHeaderView::Stretch);

    d->toolTip      = new QueueToolTip(this);
    d->toolTipTimer = new QTimer(this);

    // -----------------------------------------------------------

    connect(DatabaseAccess::databaseWatch(), SIGNAL(collectionImageChange(CollectionImageChangeset)),
            this, SLOT(slotCollectionImageChange(CollectionImageChangeset)),
            Qt::QueuedConnection);

    connect(d->thumbLoadThread, SIGNAL(signalThumbnailLoaded(LoadingDescription,QPixmap)),
            this, SLOT(slotThumbnailLoaded(LoadingDescription,QPixmap)));

    connect(this, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(slotContextMenu()));

    connect(d->toolTipTimer, SIGNAL(timeout()),
            this, SLOT(slotToolTip()));
}

QueueListView::~QueueListView()
{
    delete d->toolTip;
    delete d;
}

Qt::DropActions QueueListView::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}

QMimeData* QueueListView::mimeData(const QList<QTreeWidgetItem*> items) const
{
    KUrl::List urls;
    KUrl::List kioURLs;
    QList<int> albumIDs;
    QList<qlonglong> imageIDs;

    foreach(QTreeWidgetItem* itm, items)
    {
        QueueListViewItem* vitem = dynamic_cast<QueueListViewItem*>(itm);

        if (vitem)
        {
            urls.append(vitem->info().fileUrl());
            kioURLs.append(vitem->info().databaseUrl());
            albumIDs.append(vitem->info().albumId());
            imageIDs.append(vitem->info().id());
        }
    }

    DItemDrag* mimeData = new DItemDrag(urls, kioURLs, albumIDs, imageIDs);
    return mimeData;
}

void QueueListView::startDrag(Qt::DropActions /*supportedActions*/)
{
    QList<QTreeWidgetItem*> items = selectedItems();

    if (items.isEmpty())
    {
        return;
    }

    QPixmap icon(DesktopIcon("image-jp2", 48));
    int w = icon.width();
    int h = icon.height();

    QPixmap pix(w + 4, h + 4);
    QString text(QString::number(items.count()));

    QPainter p(&pix);
    p.fillRect(0, 0, pix.width() - 1, pix.height() - 1, QColor(Qt::white));
    p.setPen(QPen(Qt::black, 1));
    p.drawRect(0, 0, pix.width() - 1, pix.height() - 1);
    p.drawPixmap(2, 2, icon);
    QRect r = p.boundingRect(2, 2, w, h, Qt::AlignLeft | Qt::AlignTop, text);
    r.setWidth(qMax(r.width(), r.height()));
    r.setHeight(qMax(r.width(), r.height()));
    p.fillRect(r, QColor(0, 80, 0));
    p.setPen(Qt::white);
    QFont f(font());
    f.setBold(true);
    p.setFont(f);
    p.drawText(r, Qt::AlignCenter, text);
    p.end();

    QDrag* drag = new QDrag(this);
    drag->setMimeData(mimeData(items));
    drag->setPixmap(pix);
    drag->exec();
}

void QueueListView::dragEnterEvent(QDragEnterEvent* e)
{
    QTreeWidget::dragEnterEvent(e);
    e->accept();
}

void QueueListView::dragMoveEvent(QDragMoveEvent* e)
{
    int        albumID;
    QList<int> albumIDs;
    QList<qlonglong> imageIDs;
    KUrl::List urls;
    KUrl::List kioURLs;

    if (DItemDrag::decode(e->mimeData(), urls, kioURLs, albumIDs, imageIDs) ||
        DAlbumDrag::decode(e->mimeData(), urls, albumID) ||
        DTagDrag::canDecode(e->mimeData()))
    {
        if (DItemDrag::decode(e->mimeData(), urls, kioURLs, albumIDs, imageIDs))
        {
            ImageInfoList imageInfoList;

            for (QList<qlonglong>::const_iterator it = imageIDs.constBegin();
                 it != imageIDs.constEnd(); ++it)
            {
                ImageInfo info(*it);

                if (!findItemByInfo(info))
                {
                    imageInfoList.append(info);
                }
            }

            if (!imageInfoList.isEmpty())
            {
                QTreeWidget::dragMoveEvent(e);
                e->accept();
                return;
            }
        }
    }

    e->ignore();
}

void QueueListView::dropEvent(QDropEvent* e)
{
    int        albumID;
    QList<int> albumIDs;
    QList<qlonglong> imageIDs;
    KUrl::List urls;
    KUrl::List kioURLs;

    if (DItemDrag::decode(e->mimeData(), urls, kioURLs, albumIDs, imageIDs))
    {
        ImageInfoList imageInfoList;

        for (QList<qlonglong>::const_iterator it = imageIDs.constBegin();
             it != imageIDs.constEnd(); ++it)
        {
            ImageInfo info(*it);

            if (!findItemByInfo(info))
            {
                imageInfoList.append(info);
            }
        }

        if (!imageInfoList.isEmpty())
        {
            slotAddItems(imageInfoList);
            e->acceptProposedAction();

            QueueListView* vitem = dynamic_cast<QueueListView*>(e->source());

            if (vitem && vitem != this)
            {
                foreach(ImageInfo info, imageInfoList)
                {
                    vitem->removeItemByInfo(info);
                }
            }
        }
    }
    else if (DAlbumDrag::decode(e->mimeData(), urls, albumID))
    {
        QList<qlonglong> itemIDs = DatabaseAccess().db()->getItemIDsInAlbum(albumID);
        ImageInfoList imageInfoList;

        for (QList<qlonglong>::const_iterator it = itemIDs.constBegin();
             it != itemIDs.constEnd(); ++it)
        {
            ImageInfo info(*it);

            if (!findItemByInfo(info))
            {
                imageInfoList.append(info);
            }
        }

        if (!imageInfoList.isEmpty())
        {
            slotAddItems(imageInfoList);
            e->acceptProposedAction();
        }
    }
    else if (DTagDrag::canDecode(e->mimeData()))
    {
        int tagID;

        if (!DTagDrag::decode(e->mimeData(), tagID))
        {
            return;
        }

        QList<qlonglong> itemIDs = DatabaseAccess().db()->getItemIDsInTag(tagID, true);
        ImageInfoList imageInfoList;

        for (QList<qlonglong>::const_iterator it = itemIDs.constBegin();
             it != itemIDs.constEnd(); ++it)
        {
            ImageInfo info(*it);

            if (!findItemByInfo(info))
            {
                imageInfoList.append(info);
            }
        }

        if (!imageInfoList.isEmpty())
        {
            slotAddItems(imageInfoList);
            e->acceptProposedAction();
        }
    }
    else
    {
        e->ignore();
    }

    emit signalQueueContentsChanged();
}

void QueueListView::setEnableToolTips(bool val)
{
    d->showTips = val;

    if (!val)
    {
        hideToolTip();
    }
}

void QueueListView::hideToolTip()
{
    d->toolTipItem = 0;
    d->toolTipTimer->stop();
    slotToolTip();
}

bool QueueListView::acceptToolTip(const QPoint& pos)
{
    if (columnAt(pos.x()) == 0)
    {
        return true;
    }

    return false;
}

void QueueListView::slotToolTip()
{
    d->toolTip->setQueueItem(d->toolTipItem);
}

void QueueListView::mouseMoveEvent(QMouseEvent* e)
{
    if (e->buttons() == Qt::NoButton)
    {
        QueueListViewItem* item = dynamic_cast<QueueListViewItem*>(itemAt(e->pos()));

        if (d->showTips)
        {
            if (!isActiveWindow())
            {
                hideToolTip();
                return;
            }

            if (item != d->toolTipItem)
            {
                hideToolTip();

                if (acceptToolTip(e->pos()))
                {
                    d->toolTipItem = item;
                    d->toolTipTimer->setSingleShot(true);
                    d->toolTipTimer->start(500);
                }
            }

            if (item == d->toolTipItem && !acceptToolTip(e->pos()))
            {
                hideToolTip();
            }
        }

        return;
    }

    hideToolTip();
    QTreeWidget::mouseMoveEvent(e);
}

void QueueListView::wheelEvent(QWheelEvent* e)
{
    hideToolTip();
    QTreeWidget::wheelEvent(e);
}

void QueueListView::keyPressEvent(QKeyEvent* e)
{
    hideToolTip();
    QTreeWidget::keyPressEvent(e);
}

void QueueListView::focusOutEvent(QFocusEvent* e)
{
    hideToolTip();
    QTreeWidget::focusOutEvent(e);
}

void QueueListView::leaveEvent(QEvent* e)
{
    hideToolTip();
    QTreeWidget::leaveEvent(e);
}

void QueueListView::slotAddItems(const ImageInfoList& list)
{
    if (list.count() == 0)
    {
        return;
    }

    for (ImageInfoList::ConstIterator it = list.begin(); it != list.end(); ++it)
    {
        ImageInfo info = *it;

        // Check if the new item already exist in the list.

        bool find               = false;
        QueueListViewItem* item = 0;

        QTreeWidgetItemIterator iter(this);

        while (*iter)
        {
            item = dynamic_cast<QueueListViewItem*>(*iter);

            if (item->info() == info)
            {
                find = true;
            }

            ++iter;
        }

        if (!find)
        {
            item = new QueueListViewItem(this, info);
        }
    }

    updateDestFileNames();
    emit signalQueueContentsChanged();
}

void QueueListView::drawRow(QPainter* p, const QStyleOptionViewItem& opt, const QModelIndex& index) const
{
    QueueListViewItem* item = dynamic_cast<QueueListViewItem*>(itemFromIndex(index));

    if (item && !item->hasValidThumbnail())
    {
        ImageInfo info = item->info();
        d->thumbLoadThread->find(info.fileUrl().toLocalFile());
    }

    QTreeWidget::drawRow(p, opt, index);
}

void QueueListView::slotThumbnailLoaded(const LoadingDescription& desc, const QPixmap& pix)
{
    QTreeWidgetItemIterator it(this);

    while (*it)
    {
        QueueListViewItem* item = dynamic_cast<QueueListViewItem*>(*it);

        if (item->info().fileUrl() == KUrl(desc.filePath))
        {
            if (pix.isNull())
            {
                item->setThumb(SmallIcon("image-x-generic", d->iconSize, KIconLoader::DisabledState));
            }
            else
            {
                item->setThumb(pix.scaled(d->iconSize, d->iconSize, Qt::KeepAspectRatio));
            }

            return;
        }

        ++it;
    }
}

void QueueListView::slotClearList()
{
    removeItems(QueueListViewPriv::ItemsAll);
    emit signalQueueContentsChanged();
}

void QueueListView::slotRemoveSelectedItems()
{
    removeItems(QueueListViewPriv::ItemsSelected);
    emit signalQueueContentsChanged();
}

void QueueListView::slotRemoveItemsDone()
{
    removeItems(QueueListViewPriv::ItemsDone);
    emit signalQueueContentsChanged();
}

void QueueListView::removeItems(int removeType)
{
    hideToolTip();

    bool find;

    do
    {
        find = false;
        QTreeWidgetItemIterator it(this);

        while (*it)
        {
            QueueListViewItem* item = dynamic_cast<QueueListViewItem*>(*it);

            switch (removeType)
            {
                case QueueListViewPriv::ItemsSelected:
                {
                    if (item->isSelected())
                    {
                        delete item;
                        find = true;
                    }

                    break;
                }

                case QueueListViewPriv::ItemsDone:
                {
                    if (item->isDone())
                    {
                        delete item;
                        find = true;
                    }

                    break;
                }

                default:  // ItemsAll
                {
                    delete item;
                    find = true;
                    break;
                }
            }

            ++it;
        }
    }
    while (find);

    emit signalQueueContentsChanged();
}

void QueueListView::removeItemByInfo(const ImageInfo& info)
{
    removeItemById(info.id());
}

void QueueListView::removeItemById(qlonglong id)
{
    hideToolTip();

    bool find;

    do
    {
        find = false;
        QTreeWidgetItemIterator it(this);

        while (*it)
        {
            QueueListViewItem* item = dynamic_cast<QueueListViewItem*>(*it);

            if (item->info().id() == id)
            {
                delete item;
                find = true;
                break;
            }

            ++it;
        }
    }
    while (find);

    emit signalQueueContentsChanged();
}

bool QueueListView::findItemByInfo(const ImageInfo& info)
{
    return (findItemById(info.id()) ? true : false);
}

QueueListViewItem* QueueListView::findItemById(qlonglong id)
{
    QTreeWidgetItemIterator it(this);

    while (*it)
    {
        QueueListViewItem* item = dynamic_cast<QueueListViewItem*>(*it);

        if (item->info().id() == id)
        {
            return item;
        }

        ++it;
    }

    return 0;
}

QueueListViewItem* QueueListView::findItemByUrl(const KUrl& url)
{
    QTreeWidgetItemIterator it(this);

    while (*it)
    {
        QueueListViewItem* item = dynamic_cast<QueueListViewItem*>(*it);

        if (item->info().fileUrl() == url)
        {
            return item;
        }

        ++it;
    }

    return 0;
}

int QueueListView::itemsCount()
{
    int count = 0;
    QTreeWidgetItemIterator it(this);

    while (*it)
    {
        QueueListViewItem* item = dynamic_cast<QueueListViewItem*>(*it);

        if (item)
        {
            ++count;
        }

        ++it;
    }

    return count;
}

ImageInfoList QueueListView::pendingItemsList()
{
    ImageInfoList list;
    QTreeWidgetItemIterator it(this);

    while (*it)
    {
        QueueListViewItem* item = dynamic_cast<QueueListViewItem*>(*it);

        if (item && !item->isDone())
        {
            list.append(item->info());
        }

        ++it;
    }

    return list;
}

int QueueListView::pendingItemsCount()
{
    return pendingItemsList().count();
}

int QueueListView::pendingTasksCount()
{
    int count = 0;
    QTreeWidgetItemIterator it(this);

    while (*it)
    {
        QueueListViewItem* item = dynamic_cast<QueueListViewItem*>(*it);

        if (item && !item->isDone())
        {
            count += assignedTools().m_toolsMap.count();
        }

        ++it;
    }

    return count;
}

void QueueListView::setSettings(const QueueSettings& settings)
{
    d->settings = settings;
    resetQueue();
    updateDestFileNames();
}

QueueSettings QueueListView::settings()
{
    return d->settings;
}

void QueueListView::setAssignedTools(const AssignedBatchTools& tools)
{
    d->toolsList = tools;
    updateDestFileNames();
}

AssignedBatchTools QueueListView::assignedTools()
{
    return d->toolsList;
}

void QueueListView::slotAssignedToolsChanged(const AssignedBatchTools& tools)
{
    setAssignedTools(tools);
    resetQueue();
}

void QueueListView::resetQueue()
{
    //reset all items
    QTreeWidgetItemIterator it(this);

    while (*it)
    {
        QueueListViewItem* item = dynamic_cast<QueueListViewItem*>(*it);

        if (item)
        {
            item->reset();
        }

        it++;
    }
}

void QueueListView::updateDestFileNames()
{
    QMap<QString, QString> renamingResults;

    if (settings().renamingRule == QueueSettings::CUSTOMIZE)
    {
        AdvancedRenameManager manager;

        ParseSettings psettings;
        psettings.parseString = settings().renamingParser;

        QList<ParseSettings> files;
        QTreeWidgetItemIterator it(this);

        while (*it)
        {
            QueueListViewItem* item = dynamic_cast<QueueListViewItem*>(*it);

            if (item)
            {
                // Update base name using queue renaming rules.
                ImageInfo info = item->info();
                QFileInfo fi(info.filePath());

                ParseSettings ps;
                ps.fileUrl = KUrl(fi.absoluteFilePath());
                files << ps;
            }

            ++it;
        }

        manager.addFiles(files);
        manager.parseFiles(psettings);
        renamingResults = manager.newFileList();
    }

    AssignedBatchTools tools = assignedTools();

    QTreeWidgetItemIterator it(this);

    while (*it)
    {
        QueueListViewItem* item = dynamic_cast<QueueListViewItem*>(*it);

        if (item)
        {
            // Update base name using queue renaming rules.
            ImageInfo info = item->info();
            QFileInfo fi(info.filePath());

            // Update suffix using assigned batch tool rules.
            bool extensionSet = false;
            tools.m_itemUrl   = item->info().fileUrl();
            QString newSuffix = tools.targetSuffix(&extensionSet);
            QString newName   = QString("%1.%2").arg(fi.completeBaseName()).arg(newSuffix);

            if (settings().renamingRule == QueueSettings::CUSTOMIZE && !renamingResults.isEmpty())
            {
                QFileInfo fi2(renamingResults[fi.absoluteFilePath()]);

                if (extensionSet)
                {
                    newName = QString("%1.%2").arg(fi2.completeBaseName())
                              .arg(newSuffix);
                }
                else
                {
                    newName = fi2.fileName();
                }
            }

            item->setDestFileName(newName);
        }

        ++it;
    }
}

void QueueListView::slotContextMenu()
{
    if (!viewport()->isEnabled())
    {
        return;
    }

    KActionCollection* acol = QueueMgrWindow::queueManagerWindow()->actionCollection();
    KMenu popmenu(this);
    popmenu.addAction(acol->action("queuemgr_removeitemssel"));
    popmenu.addSeparator();
    popmenu.addAction(acol->action("queuemgr_clearlist"));
    popmenu.exec(QCursor::pos());
}

void QueueListView::slotCollectionImageChange(const CollectionImageChangeset& changeset)
{
    if (QueueMgrWindow::queueManagerWindow()->isBusy())
    {
        return;
    }

    switch (changeset.operation())
    {
        case CollectionImageChangeset::Removed:
        case CollectionImageChangeset::RemovedAll:
        {
            foreach(const qlonglong& id, changeset.ids())
            {
                removeItemById(id);
            }
            break;
        }
        default:
        {
            break;
        }
    }
}

void QueueListView::reloadThumbs(const KUrl& url)
{
    d->thumbLoadThread->find(url.toLocalFile());
}

}  // namespace Digikam
