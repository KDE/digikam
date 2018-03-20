/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-05-21
 * Description : widget to display an imagelist
 *
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2008-2010 by Andi Clemens <andi dot clemens at googlemail dot com>
 * Copyright (C) 2009-2010 by Luka Renko <lure at kubuntu dot org>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "dimageslist.h"

// Qt includes

#include <QDragEnterEvent>
#include <QFileInfo>
#include <QGridLayout>
#include <QGroupBox>
#include <QMimeData>
#include <QHeaderView>
#include <QLabel>
#include <QPainter>
#include <QPushButton>
#include <QUrl>
#include <QTimer>
#include <QFile>
#include <QPointer>
#include <QXmlStreamAttributes>
#include <QStringRef>
#include <QString>
#include <QStandardPaths>
#include <QIcon>
#include <QApplication>
#include <QStyle>

// KDE includes

#include <kconfig.h>
#include <kconfiggroup.h>
#include <klocalizedstring.h>

// Local includes

#include "drawdecoder.h"
#include "imagedialog.h"
#include "digikam_debug.h"
#include "dlayoutbox.h"
#include "dfiledialog.h"
#include "thumbnailloadthread.h"
#include "dworkingpixmap.h"

namespace Digikam
{

const int DEFAULTSIZE = 48;

class DImagesListViewItem::Private
{
public:

    explicit Private()
    {
        rating   = -1;
        view     = 0;
        state    = Waiting;
        hasThumb = false;
    }

    bool              hasThumb;       // True if thumbnails is a real photo thumbs

    int               rating;         // Image Rating from host.
    QString           comments;       // Image comments from host.
    QStringList       tags;           // List of keywords from host.
    QUrl              url;            // Image url provided by host.
    QPixmap           thumb;          // Image thumbnail.
    DImagesListView*  view;
    State             state;
};

DImagesListViewItem::DImagesListViewItem(DImagesListView* const view, const QUrl& url)
    : QTreeWidgetItem(view),
      d(new Private)
{
    setUrl(url);
    setRating(-1);
    setFlags(Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsSelectable);

    d->view      = view;
    int iconSize = d->view->iconSize().width();
    setThumb(QIcon::fromTheme(QString::fromLatin1("view-preview")).pixmap(iconSize, iconSize, QIcon::Disabled), false);
/*
    qCDebug(DIGIKAM_GENERAL_LOG) << "Creating new ImageListViewItem with url " << d->url
                                 << " for list view " << d->view;
*/
}

DImagesListViewItem::~DImagesListViewItem()
{
    delete d;
}

bool DImagesListViewItem::hasValidThumbnail() const
{
    return d->hasThumb;
}

void DImagesListViewItem::updateInformation()
{
    if (d->view->iface())
    {
        DItemInfo info(d->view->iface()->itemInfo(d->url));

        setComments(info.comment());
        setTags(info.keywords());
        setRating(info.rating());
    }
}

void DImagesListViewItem::setUrl(const QUrl& url)
{
    d->url = url;
    setText(DImagesListView::Filename, d->url.fileName());
}

QUrl DImagesListViewItem::url() const
{
    return d->url;
}

void DImagesListViewItem::setComments(const QString& comments)
{
    d->comments = comments;
}

QString DImagesListViewItem::comments() const
{
    return d->comments;
}

void DImagesListViewItem::setTags(const QStringList& tags)
{
    d->tags = tags;
}

QStringList DImagesListViewItem::tags() const
{
    return d->tags;
}

void DImagesListViewItem::setRating(int rating)
{
    d->rating = rating;
}

int DImagesListViewItem::rating() const
{
    return d->rating;
}

void DImagesListViewItem::setPixmap(const QPixmap& pix)
{
    QIcon icon = QIcon(pix);
    //  We make sure the preview icon stays the same regardless of the role
    icon.addPixmap(pix, QIcon::Selected, QIcon::On);
    icon.addPixmap(pix, QIcon::Selected, QIcon::Off);
    icon.addPixmap(pix, QIcon::Active,   QIcon::On);
    icon.addPixmap(pix, QIcon::Active,   QIcon::Off);
    icon.addPixmap(pix, QIcon::Normal,   QIcon::On);
    icon.addPixmap(pix, QIcon::Normal,   QIcon::Off);
    setIcon(DImagesListView::Thumbnail, icon);
}

void DImagesListViewItem::setThumb(const QPixmap& pix, bool hasThumb)
{
/*
    qCDebug(DIGIKAM_GENERAL_LOG) << "Received new thumbnail for url " << d->url
                             << ". My view is " << d->view;
*/
    if (!d->view)
    {
        qCCritical(DIGIKAM_GENERAL_LOG) << "This item doesn't have a tree view. "
                                    << "This should never happen!";
        return;
    }

    int iconSize = qMax<int>(d->view->iconSize().width(), d->view->iconSize().height());
    QPixmap pixmap(iconSize + 2, iconSize + 2);
    pixmap.fill(Qt::transparent);
    QPainter p(&pixmap);
    p.drawPixmap((pixmap.width() / 2) - (pix.width() / 2), (pixmap.height() / 2) - (pix.height() / 2), pix);
    d->thumb     = pixmap;
    setPixmap(d->thumb);

    d->hasThumb  = hasThumb;
}

void DImagesListViewItem::setProgressAnimation(const QPixmap& pix)
{
    QPixmap overlay = d->thumb;
    QPixmap mask(overlay.size());
    mask.fill(QColor(128, 128, 128, 192));
    QPainter p(&overlay);
    p.drawPixmap(0, 0, mask);
    p.drawPixmap((overlay.width() / 2) - (pix.width() / 2), (overlay.height() / 2) - (pix.height() / 2), pix);
    setPixmap(overlay);
}

void DImagesListViewItem::setProcessedIcon(const QIcon& icon)
{
    setIcon(DImagesListView::Filename, icon);
    // reset thumbnail back to no animation pix
    setPixmap(d->thumb);
}

void DImagesListViewItem::setState(State state)
{
    d->state = state;
}

DImagesListViewItem::State DImagesListViewItem::state() const
{
    return d->state;
}

DImagesListView* DImagesListViewItem::view() const
{
    return d->view;
}

// ---------------------------------------------------------------------------

DImagesListView::DImagesListView(DImagesList* const parent)
    : QTreeWidget(parent)
{
    setup(DEFAULTSIZE);
}

DImagesListView::DImagesListView(int iconSize, DImagesList* const parent)
    : QTreeWidget(parent)
{
    setup(iconSize);
}

DImagesListView::~DImagesListView()
{
}

DInfoInterface* DImagesListView::iface() const
{
    DImagesList* const p = dynamic_cast<DImagesList*>(parent());

    if (p)
    {
        return p->iface();
    }

    return 0;
}

void DImagesListView::setup(int iconSize)
{
    m_iconSize = iconSize;
    setIconSize(QSize(m_iconSize, m_iconSize));
    setAlternatingRowColors(true);
    setSelectionMode(QAbstractItemView::ExtendedSelection);

    enableDragAndDrop(true);

    setSortingEnabled(false);
    setAllColumnsShowFocus(true);
    setRootIsDecorated(false);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    setColumnCount(8);
    setHeaderLabels(QStringList() << i18n("Thumbnail")
                                  << i18n("File Name")
                                  << i18n("User1")
                                  << i18n("User2")
                                  << i18n("User3")
                                  << i18n("User4")
                                  << i18n("User5")
                                  << i18n("User6"));
    hideColumn(User1);
    hideColumn(User2);
    hideColumn(User3);
    hideColumn(User4);
    hideColumn(User5);
    hideColumn(User6);

    header()->setSectionResizeMode(User1, QHeaderView::Interactive);
    header()->setSectionResizeMode(User2, QHeaderView::Interactive);
    header()->setSectionResizeMode(User3, QHeaderView::Interactive);
    header()->setSectionResizeMode(User4, QHeaderView::Interactive);
    header()->setSectionResizeMode(User5, QHeaderView::Interactive);
    header()->setSectionResizeMode(User6, QHeaderView::Stretch);

    connect(this, &DImagesListView::itemClicked,
            this, &DImagesListView::slotItemClicked);
}

void DImagesListView::enableDragAndDrop(const bool enable)
{
    setDragEnabled(enable);
    viewport()->setAcceptDrops(enable);
    setDragDropMode(enable ? QAbstractItemView::InternalMove : QAbstractItemView::NoDragDrop);
    setDragDropOverwriteMode(enable);
    setDropIndicatorShown(enable);
}

void DImagesListView::drawRow(QPainter* p, const QStyleOptionViewItem& opt, const QModelIndex& index) const
{
    DImagesListViewItem* const item = dynamic_cast<DImagesListViewItem*>(itemFromIndex(index));

    if (item && !item->hasValidThumbnail())
    {
        DImagesList* const view = dynamic_cast<DImagesList*>(parent());

        if (view)
        {
            view->updateThumbnail(item->url());
        }
    }

    QTreeWidget::drawRow(p, opt, index);
}

void DImagesListView::slotItemClicked(QTreeWidgetItem* item, int column)
{
    Q_UNUSED(column)

    if (!item)
    {
        return;
    }

    emit signalItemClicked(item);
}

void DImagesListView::setColumnLabel(ColumnType column, const QString& label)
{
    headerItem()->setText(column, label);
}

void DImagesListView::setColumnEnabled(ColumnType column, bool enable)
{
    if (enable)
    {
        showColumn(column);
    }
    else
    {
        hideColumn(column);
    }
}

void DImagesListView::setColumn(ColumnType column, const QString& label, bool enable)
{
    setColumnLabel(column, label);
    setColumnEnabled(column, enable);
}

DImagesListViewItem* DImagesListView::findItem(const QUrl& url)
{
    QTreeWidgetItemIterator it(this);

    while (*it)
    {
        DImagesListViewItem* const lvItem = dynamic_cast<DImagesListViewItem*>(*it);

        if (lvItem && lvItem->url() == url)
        {
            return lvItem;
        }

        ++it;
    }

    return 0;
}

QModelIndex DImagesListView::indexFromItem(DImagesListViewItem* item, int column) const
{
  return QTreeWidget::indexFromItem(item, column);
}

void DImagesListView::contextMenuEvent(QContextMenuEvent* e)
{
    QTreeWidget::contextMenuEvent(e);
    emit signalContextMenuRequested();
}

void DImagesListView::dragEnterEvent(QDragEnterEvent* e)
{
    QTreeWidget::dragEnterEvent(e);

    if (e->mimeData()->hasUrls())
    {
        e->acceptProposedAction();
    }
}

void DImagesListView::dragMoveEvent(QDragMoveEvent* e)
{
    QTreeWidget::dragMoveEvent(e);

    if (e->mimeData()->hasUrls())
    {
        e->acceptProposedAction();
    }
}

void DImagesListView::dropEvent(QDropEvent* e)
{
    QTreeWidget::dropEvent(e);
    QList<QUrl> list = e->mimeData()->urls();
    QList<QUrl> urls;

    foreach(const QUrl& url, list)
    {
        QFileInfo fi(url.toLocalFile());

        if (fi.isFile() && fi.exists())
        {
            urls.append(url);
        }
    }

    if (!urls.isEmpty())
    {
        emit signalAddedDropedItems(urls);
    }
}

// ---------------------------------------------------------------------------

CtrlButton::CtrlButton(const QIcon& icon, QWidget* const parent)
    : QPushButton(parent)
{
    const int btnSize = 32;

    setMinimumSize(btnSize, btnSize);
    setMaximumSize(btnSize, btnSize);
    setIcon(icon);
}

CtrlButton::~CtrlButton()
{
}

// ---------------------------------------------------------------------------

class DImagesList::Private
{
public:

    explicit Private()
    {
        listView               = 0;
        addButton              = 0;
        removeButton           = 0;
        moveUpButton           = 0;
        moveDownButton         = 0;
        clearButton            = 0;
        loadButton             = 0;
        saveButton             = 0;
        iconSize               = DEFAULTSIZE;
        allowRAW               = true;
        controlButtonsEnabled  = true;
        allowDuplicate         = false;
        progressCount          = 0;
        progressTimer          = 0;
        progressPix            = DWorkingPixmap();
        thumbLoadThread        = ThumbnailLoadThread::defaultThread();
        iface                  = 0;
    }

    bool                       allowRAW;
    bool                       allowDuplicate;
    bool                       controlButtonsEnabled;
    int                        iconSize;

    CtrlButton*                addButton;
    CtrlButton*                removeButton;
    CtrlButton*                moveUpButton;
    CtrlButton*                moveDownButton;
    CtrlButton*                clearButton;
    CtrlButton*                loadButton;
    CtrlButton*                saveButton;

    QList<QUrl>                processItems;
    DWorkingPixmap             progressPix;
    int                        progressCount;
    QTimer*                    progressTimer;

    DImagesListView*           listView;
    ThumbnailLoadThread*       thumbLoadThread;

    DInfoInterface*            iface;
};

DImagesList::DImagesList(QWidget* const parent, int iconSize)
    : QWidget(parent),
      d(new Private)
{
    if (iconSize != -1)  // default = ICONSIZE
    {
        setIconSize(iconSize);
    }

    // --------------------------------------------------------

    d->listView = new DImagesListView(d->iconSize, this);
    d->listView->setSelectionMode(QAbstractItemView::ExtendedSelection);

    // --------------------------------------------------------

    d->addButton      = new CtrlButton(QIcon::fromTheme(QString::fromLatin1("list-add")).pixmap(16, 16),      this);
    d->removeButton   = new CtrlButton(QIcon::fromTheme(QString::fromLatin1("list-remove")).pixmap(16, 16),   this);
    d->moveUpButton   = new CtrlButton(QIcon::fromTheme(QString::fromLatin1("go-up")).pixmap(16, 16),         this);
    d->moveDownButton = new CtrlButton(QIcon::fromTheme(QString::fromLatin1("go-down")).pixmap(16, 16),       this);
    d->clearButton    = new CtrlButton(QIcon::fromTheme(QString::fromLatin1("edit-clear")).pixmap(16, 16),    this);
    d->loadButton     = new CtrlButton(QIcon::fromTheme(QString::fromLatin1("document-open")).pixmap(16, 16), this);
    d->saveButton     = new CtrlButton(QIcon::fromTheme(QString::fromLatin1("document-save")).pixmap(16, 16), this);

    d->addButton->setToolTip(i18n("Add new images to the list"));
    d->removeButton->setToolTip(i18n("Remove selected images from the list"));
    d->moveUpButton->setToolTip(i18n("Move current selected image up in the list"));
    d->moveDownButton->setToolTip(i18n("Move current selected image down in the list"));
    d->clearButton->setToolTip(i18n("Clear the list."));
    d->loadButton->setToolTip(i18n("Load a saved list."));
    d->saveButton->setToolTip(i18n("Save the list."));

    d->progressTimer = new QTimer(this);

    // --------------------------------------------------------

    setControlButtons(Add | Remove | MoveUp | MoveDown | Clear | Save | Load ); // add all buttons      (default)
    setControlButtonsPlacement(ControlButtonsRight);                            // buttons on the right (default)
    enableDragAndDrop(true);                                                    // enable drag and drop (default)

    // --------------------------------------------------------

    connect(d->listView, &DImagesListView::signalAddedDropedItems,
            this, &DImagesList::slotAddImages);

    connect(d->thumbLoadThread, SIGNAL(signalThumbnailLoaded(LoadingDescription,QPixmap)),
            this, SLOT(slotThumbnail(LoadingDescription,QPixmap)));

    connect(d->listView, &DImagesListView::signalItemClicked,
            this, &DImagesList::signalItemClicked);

    connect(d->listView, &DImagesListView::signalContextMenuRequested,
            this, &DImagesList::signalContextMenuRequested);

    // queue this connection because itemSelectionChanged is emitted
    // while items are deleted, and accessing selectedItems at that
    // time causes a crash ...
    connect(d->listView, &DImagesListView::itemSelectionChanged,
            this, &DImagesList::slotImageListChanged, Qt::QueuedConnection);

    connect(this, &DImagesList::signalImageListChanged,
            this, &DImagesList::slotImageListChanged);

    // --------------------------------------------------------

    connect(d->addButton, &CtrlButton::clicked,
            this, &DImagesList::slotAddItems);

    connect(d->removeButton, &CtrlButton::clicked,
            this, &DImagesList::slotRemoveItems);

    connect(d->moveUpButton, &CtrlButton::clicked,
            this, &DImagesList::slotMoveUpItems);

    connect(d->moveDownButton, &CtrlButton::clicked,
            this, &DImagesList::slotMoveDownItems);

    connect(d->clearButton, &CtrlButton::clicked,
            this, &DImagesList::slotClearItems);

    connect(d->loadButton, &CtrlButton::clicked,
            this, &DImagesList::slotLoadItems);

    connect(d->saveButton, &CtrlButton::clicked,
            this, &DImagesList::slotSaveItems);

    connect(d->progressTimer, &QTimer::timeout,
            this, &DImagesList::slotProgressTimerDone);

    // --------------------------------------------------------

    emit signalImageListChanged();
}


DImagesList::~DImagesList()
{
    delete d;
}

void DImagesList::enableControlButtons(bool enable)
{
    d->controlButtonsEnabled = enable;
    slotImageListChanged();
}

void DImagesList::enableDragAndDrop(const bool enable)
{
    d->listView->enableDragAndDrop(enable);
}

void DImagesList::setControlButtonsPlacement(ControlButtonPlacement placement)
{
    delete layout();

    const int spacing = QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);

    QGridLayout* const mainLayout = new QGridLayout;
    mainLayout->addWidget(d->listView, 1, 1, 1, 1);
    mainLayout->setRowStretch(1, 10);
    mainLayout->setColumnStretch(1, 10);
    mainLayout->setContentsMargins(spacing, spacing, spacing, spacing);
    mainLayout->setSpacing(spacing);

    // --------------------------------------------------------

    QHBoxLayout* const hBtnLayout = new QHBoxLayout;
    hBtnLayout->addStretch(10);
    hBtnLayout->addWidget(d->moveUpButton);
    hBtnLayout->addWidget(d->moveDownButton);
    hBtnLayout->addWidget(d->addButton);
    hBtnLayout->addWidget(d->removeButton);
    hBtnLayout->addWidget(d->loadButton);
    hBtnLayout->addWidget(d->saveButton);
    hBtnLayout->addWidget(d->clearButton);
    hBtnLayout->addStretch(10);

    // --------------------------------------------------------

    QVBoxLayout* const vBtnLayout = new QVBoxLayout;
    vBtnLayout->addStretch(10);
    vBtnLayout->addWidget(d->moveUpButton);
    vBtnLayout->addWidget(d->moveDownButton);
    vBtnLayout->addWidget(d->addButton);
    vBtnLayout->addWidget(d->removeButton);
    vBtnLayout->addWidget(d->loadButton);
    vBtnLayout->addWidget(d->saveButton);
    vBtnLayout->addWidget(d->clearButton);
    vBtnLayout->addStretch(10);

    // --------------------------------------------------------

    switch (placement)
    {
        case ControlButtonsAbove:
            mainLayout->addLayout(hBtnLayout, 0, 1, 1, 1);
            delete vBtnLayout;
            break;

        case ControlButtonsBelow:
            mainLayout->addLayout(hBtnLayout, 2, 1, 1, 1);
            delete vBtnLayout;
            break;

        case ControlButtonsLeft:
            mainLayout->addLayout(vBtnLayout, 1, 0, 1, 1);
            delete hBtnLayout;
            break;

        case ControlButtonsRight:
            mainLayout->addLayout(vBtnLayout, 1, 2, 1, 1);
            delete hBtnLayout;
            break;

        case NoControlButtons:
        default:
        {
            delete vBtnLayout;
            delete hBtnLayout;
            // set all buttons invisible
            setControlButtons(0x0);
            break;
        }
    }

    setLayout(mainLayout);
}

void DImagesList::setControlButtons(ControlButtons buttonMask)
{
    d->addButton->setVisible(buttonMask & Add);
    d->removeButton->setVisible(buttonMask & Remove);
    d->moveUpButton->setVisible(buttonMask & MoveUp);
    d->moveDownButton->setVisible(buttonMask & MoveDown);
    d->clearButton->setVisible(buttonMask & Clear);
    d->loadButton->setVisible(buttonMask & Load);
    d->saveButton->setVisible(buttonMask & Save);
}

void DImagesList::setIface(DInfoInterface* const iface)
{
    d->iface = iface;
}

DInfoInterface* DImagesList::iface() const
{
    return d->iface;
}

void DImagesList::setAllowDuplicate(bool allow)
{
  d->allowDuplicate = allow;
}

void DImagesList::setAllowRAW(bool allow)
{
    d->allowRAW = allow;
}

void DImagesList::setIconSize(int size)
{
    if (size < 16)
    {
        d->iconSize = 16;
    }
    else if (size > 128)
    {
        d->iconSize = 128;
    }
    else
    {
        d->iconSize = size;
    }
}

int DImagesList::iconSize() const
{
    return d->iconSize;
}

void DImagesList::loadImagesFromCurrentSelection()
{
    bool selection = checkSelection();

    if (selection == true)
    {
        if (!d->iface)
        {
            return;
        }

        QList<QUrl> images = d->iface->currentSelectedItems();

        if (!images.isEmpty())
        {
            slotAddImages(images);
        }
    }
    else
    {
        loadImagesFromCurrentAlbum();
    }
}

void DImagesList::loadImagesFromCurrentAlbum()
{
    if (!d->iface)
    {
        return;
    }

    QList<QUrl> images = d->iface->currentAlbumItems();

    if (!images.isEmpty())
    {
        slotAddImages(images);
    }
}

bool DImagesList::checkSelection()
{
    if (!d->iface)
    {
        return false;
    }

    QList<QUrl> images = d->iface->currentSelectedItems();

    return (!images.isEmpty());
}

void DImagesList::slotAddImages(const QList<QUrl>& list)
{
    if (list.count() == 0)
    {
        return;
    }

    QList<QUrl> urls;
    bool raw = false;

    for (QList<QUrl>::ConstIterator it = list.constBegin(); it != list.constEnd(); ++it)
    {
        QUrl imageUrl = *it;

        // Check if the new item already exist in the list.
        bool found = false;

        QTreeWidgetItemIterator iter(d->listView);

        while (*iter)
        {
            DImagesListViewItem* const item = dynamic_cast<DImagesListViewItem*>(*iter);

            if (item && item->url() == imageUrl)
            {
                found = true;
            }

            ++iter;
        }

        if (d->allowDuplicate || !found)
        {
            // if RAW files are not allowed, skip the image
            if (!d->allowRAW && DRawDecoder::isRawFile(imageUrl))
            {
                raw = true;
                continue;
            }

            new DImagesListViewItem(listView(), imageUrl);
            urls.append(imageUrl);
        }
    }

    emit signalAddItems(urls);
    emit signalImageListChanged();
    emit signalFoundRAWImages(raw);
}

void DImagesList::slotAddItems()
{
    KConfig config;
    KConfigGroup grp = config.group(objectName());
    QUrl lastFileUrl = QUrl::fromLocalFile(grp.readEntry("Last Image Path",
                                           QStandardPaths::writableLocation(QStandardPaths::PicturesLocation)));

    ImageDialog dlg(this, lastFileUrl, false);
    QList<QUrl> urls = dlg.urls();

    if (!urls.isEmpty())
    {
        slotAddImages(urls);
        grp.writeEntry("Last Image Path", urls.first().adjusted(QUrl::RemoveFilename).toLocalFile());
        config.sync();
    }
}

void DImagesList::slotRemoveItems()
{
    QList<QTreeWidgetItem*> selectedItemsList = d->listView->selectedItems();
    QList<int> itemsIndex;

    for (QList<QTreeWidgetItem*>::const_iterator it = selectedItemsList.constBegin();
         it != selectedItemsList.constEnd(); ++it)
    {
        DImagesListViewItem* const item = dynamic_cast<DImagesListViewItem*>(*it);

        if (item)
        {
            itemsIndex.append(d->listView->indexFromItem(item).row());

            if (d->processItems.contains(item->url()))
            {
                d->processItems.removeAll(item->url());
            }

            d->listView->removeItemWidget(*it, 0);
            delete *it;
        }
    }

    emit signalRemovedItems(itemsIndex);
    emit signalImageListChanged();
}

void DImagesList::slotMoveUpItems()
{
    // move above item down, then we don't have to fix the focus
    QModelIndex curIndex = listView()->currentIndex();

    if (!curIndex.isValid())
    {
        return;
    }

    QModelIndex aboveIndex = listView()->indexAbove(curIndex);

    if (!aboveIndex.isValid())
    {
        return;
    }

    QTreeWidgetItem* const temp = listView()->takeTopLevelItem(aboveIndex.row());
    listView()->insertTopLevelItem(curIndex.row(), temp);
    // this is a quick fix. We loose the extra tags in flickr upload, but at list we don't get a crash
    DImagesListViewItem* const uw = dynamic_cast<DImagesListViewItem*>(temp);

    if (uw)
        uw->updateItemWidgets();

    emit signalImageListChanged();
    emit signalMoveUpItem();
}

void DImagesList::slotMoveDownItems()
{
    // move below item up, then we don't have to fix the focus
    QModelIndex curIndex = listView()->currentIndex();

    if (!curIndex.isValid())
    {
        return;
    }

    QModelIndex belowIndex = listView()->indexBelow(curIndex);

    if (!belowIndex.isValid())
    {
        return;
    }

    QTreeWidgetItem* const temp = listView()->takeTopLevelItem(belowIndex.row());
    listView()->insertTopLevelItem(curIndex.row(), temp);

    // This is a quick fix. We can loose extra tags in uploader, but at least we don't get a crash
    DImagesListViewItem* const uw = dynamic_cast<DImagesListViewItem*>(temp);

    if (uw)
        uw->updateItemWidgets();

    emit signalImageListChanged();
    emit signalMoveDownItem();
}

void DImagesList::slotClearItems()
{
    listView()->selectAll();
    slotRemoveItems();
    listView()->clear();
}

void DImagesList::slotLoadItems()
{
    KConfig config;
    KConfigGroup grp = config.group(objectName());
    QUrl lastFileUrl = QUrl::fromLocalFile(grp.readEntry("Last Images List Path",
                                           QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)));
    QUrl loadLevelsFile;
    loadLevelsFile = DFileDialog::getOpenFileUrl(this, i18n("Select the image file list to load"), lastFileUrl,
                                                 i18n("All Files (*)"));

    if (loadLevelsFile.isEmpty())
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "empty url";
        return;
    }

    QFile file(loadLevelsFile.toLocalFile());

    qCDebug(DIGIKAM_GENERAL_LOG) << "file path " << loadLevelsFile.toLocalFile();

    if (!file.open(QIODevice::ReadOnly))
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Cannot open file";
        return;
    }

    QXmlStreamReader xmlReader;
    xmlReader.setDevice(&file);

    while (!xmlReader.atEnd())
    {
        if (xmlReader.isStartElement() && xmlReader.name() == QString::fromLatin1("Image"))
        {
            // get all attributes and its value of a tag in attrs variable.
            QXmlStreamAttributes attrs = xmlReader.attributes();
            // get value of each attribute from QXmlStreamAttributes
            QStringRef url = attrs.value(QString::fromLatin1("url"));

            if (url.isEmpty())
            {
                xmlReader.readNext();
                continue;
            }

            QList<QUrl> urls;
            urls.append(QUrl(url.toString()));

            if (!urls.isEmpty())
            {
                //allow tools to append a new file
                slotAddImages(urls);
                // read tool Image custom attributes and children element
                emit signalXMLLoadImageElement(xmlReader);
            }
        }
        else if (xmlReader.isStartElement() && xmlReader.name() != QString::fromLatin1("Images"))
        {
            // unmanaged start element (it should be tools one)
            emit signalXMLCustomElements(xmlReader);
        }
        else if (xmlReader.isEndElement() && xmlReader.name() == QString::fromLatin1("Images"))
        {
            // if EndElement is Images return
            grp.writeEntry("Last Images List Path", loadLevelsFile.adjusted(QUrl::RemoveFilename).toLocalFile());
            config.sync();
            return;
        }

        xmlReader.readNext();
    }
}

void DImagesList::slotSaveItems()
{
    KConfig config;
    KConfigGroup grp = config.group(objectName());
    QUrl lastFileUrl = QUrl::fromLocalFile(grp.readEntry("Last Images List Path",
                                           QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)));
    QUrl saveLevelsFile;
    saveLevelsFile = DFileDialog::getSaveFileUrl(this, i18n("Select the image file list to save"), lastFileUrl,
                                                 i18n("All Files (*)"));

    qCDebug(DIGIKAM_GENERAL_LOG) << "file url " << saveLevelsFile.toDisplayString();

    if (saveLevelsFile.isEmpty())
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "empty url";
        return;
    }

    QFile file(saveLevelsFile.toLocalFile());

    if (!file.open(QIODevice::WriteOnly))
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Cannot open target file";
        return;
    }

    QXmlStreamWriter xmlWriter;
    xmlWriter.setDevice(&file);

    xmlWriter.setAutoFormatting(true);
    xmlWriter.writeStartDocument();

    xmlWriter.writeStartElement(QString::fromLatin1("Images"));

    QTreeWidgetItemIterator it(listView());

    while (*it)
    {
        DImagesListViewItem* const lvItem = dynamic_cast<DImagesListViewItem*>(*it);

        if (lvItem)
        {
            xmlWriter.writeStartElement(QString::fromLatin1("Image"));

            xmlWriter.writeAttribute(QString::fromLatin1("url"), lvItem->url().toDisplayString());

            emit signalXMLSaveItem(xmlWriter, listView()->indexFromItem(lvItem).row());

            xmlWriter.writeEndElement(); // Image
        }

        ++it;
    }

    emit signalXMLCustomElements(xmlWriter);

    xmlWriter.writeEndElement();  // Images

    xmlWriter.writeEndDocument(); // end document

    grp.writeEntry("Last Images List Path", saveLevelsFile.adjusted(QUrl::RemoveFilename).toLocalFile());
    config.sync();
}

void DImagesList::removeItemByUrl(const QUrl& url)
{
    bool found;
    QList<int> itemsIndex;

    do
    {
        found = false;
        QTreeWidgetItemIterator it(d->listView);

        while (*it)
        {
            DImagesListViewItem* const item = dynamic_cast<DImagesListViewItem*>(*it);

            if (item && item->url() == url)
            {
                itemsIndex.append(d->listView->indexFromItem(item).row());

                if (d->processItems.contains(item->url()))
                {
                    d->processItems.removeAll(item->url());
                }

                delete item;
                found = true;
                break;
            }

            ++it;
        }
    }
    while (found);

    emit signalRemovedItems(itemsIndex);
    emit signalImageListChanged();
}

QList<QUrl> DImagesList::imageUrls(bool onlyUnprocessed) const
{
    QList<QUrl> list;
    QTreeWidgetItemIterator it(d->listView);

    while (*it)
    {
        DImagesListViewItem* const item = dynamic_cast<DImagesListViewItem*>(*it);

        if (item)
        {
            if ((onlyUnprocessed == false) || (item->state() != DImagesListViewItem::Success))
            {
                list.append(item->url());
            }
        }

        ++it;
    }

    return list;
}

void DImagesList::slotProgressTimerDone()
{
    if (!d->processItems.isEmpty())
    {
        foreach(const QUrl& url, d->processItems)
        {
            DImagesListViewItem* const item = listView()->findItem(url);

            if (item)
                item->setProgressAnimation(d->progressPix.frameAt(d->progressCount));
        }

        d->progressCount++;

        if (d->progressCount == 8)
        {
            d->progressCount = 0;
        }

        d->progressTimer->start(300);
    }
}

void DImagesList::processing(const QUrl& url)
{
    DImagesListViewItem* const item = listView()->findItem(url);

    if (item)
    {
        d->processItems.append(url);
        d->listView->setCurrentItem(item, true);
        d->listView->scrollToItem(item);
        d->progressTimer->start(300);
    }
}

void DImagesList::processed(const QUrl& url, bool success)
{
    DImagesListViewItem* const item = listView()->findItem(url);

    if (item)
    {
        d->processItems.removeAll(url);
        item->setProcessedIcon(QIcon::fromTheme(success ? QString::fromLatin1("dialog-ok-apply")
                                                        : QString::fromLatin1("dialog-cancel")).pixmap(16, 16));
        item->setState(success ? DImagesListViewItem::Success
                               : DImagesListViewItem::Failed);

        if (d->processItems.isEmpty())
            d->progressTimer->stop();
    }
}

void DImagesList::cancelProcess()
{
    foreach(const QUrl& url, d->processItems)
    {
        processed(url, false);
    }
}

void DImagesList::clearProcessedStatus()
{
    QTreeWidgetItemIterator it(d->listView);

    while (*it)
    {
        DImagesListViewItem* const lvItem = dynamic_cast<DImagesListViewItem*>(*it);

        if (lvItem)
        {
            lvItem->setProcessedIcon(QIcon());
        }

        ++it;
    }
}

DImagesListView* DImagesList::listView() const
{
    return d->listView;
}

void DImagesList::slotImageListChanged()
{
    const QList<QTreeWidgetItem*> selectedItemsList = d->listView->selectedItems();
    const bool haveImages                           = !(imageUrls().isEmpty())         && d->controlButtonsEnabled;
    const bool haveSelectedImages                   = !(selectedItemsList.isEmpty())   && d->controlButtonsEnabled;
    const bool haveOnlyOneSelectedImage             = (selectedItemsList.count() == 1) && d->controlButtonsEnabled;

    d->removeButton->setEnabled(haveSelectedImages);
    d->moveUpButton->setEnabled(haveOnlyOneSelectedImage);
    d->moveDownButton->setEnabled(haveOnlyOneSelectedImage);
    d->clearButton->setEnabled(haveImages);

    // All buttons are enabled / disabled now, but the "Add" button should always be
    // enabled, if the buttons are not explicitly disabled with enableControlButtons()
    d->addButton->setEnabled(d->controlButtonsEnabled);

    // TODO: should they be enabled by default now?
    d->loadButton->setEnabled(d->controlButtonsEnabled);
    d->saveButton->setEnabled(d->controlButtonsEnabled);
}

void DImagesList::updateThumbnail(const QUrl& url)
{
    d->thumbLoadThread->find(ThumbnailIdentifier(url.toLocalFile()));
}

void DImagesList::slotThumbnail(const LoadingDescription& desc, const QPixmap& pix)
{
    QTreeWidgetItemIterator it(d->listView);

    while (*it)
    {
        DImagesListViewItem* const item = dynamic_cast<DImagesListViewItem*>(*it);

        if (item && item->url() == QUrl::fromLocalFile(desc.filePath))
        {
            if (!pix.isNull())
            {
                item->setThumb(pix.scaled(d->iconSize, d->iconSize, Qt::KeepAspectRatio));
            }

            if (!d->allowDuplicate)
            {
                return;
            }
        }

        ++it;
    }
}

DImagesListViewItem* DImagesListView::getCurrentItem() const
{
    QTreeWidgetItem* const currentTreeItem = currentItem();

    if (!currentTreeItem)
    {
        return 0;
    }

    return dynamic_cast<DImagesListViewItem*>(currentTreeItem);
}

QUrl DImagesList::getCurrentUrl() const
{
    DImagesListViewItem* const currentItem = d->listView->getCurrentItem();

    if (!currentItem)
    {
        return QUrl();
    }

    return currentItem->url();
}

} // namespace Digikam
