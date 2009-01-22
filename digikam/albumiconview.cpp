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
 * Copyright (C) 2006-2008 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "albumiconview.h"
#include "albumiconview.moc"

// C ANSI includes.

extern "C"
{
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
}

// C++ includes.

#include <cstdio>
#include <cmath>

// Qt includes.

#include <QClipboard>
#include <QCursor>
#include <QDataStream>
#include <QDateTime>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QEvent>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QPainter>
#include <QPixmap>
#include <QPoint>
#include <QPolygon>
#include <QResizeEvent>
#include <QString>
#include <QStringList>
#include <QTimer>

// KDE includes.

#include <kaction.h>
#include <kapplication.h>
#include <kcalendarsystem.h>
#include <kdebug.h>
#include <kdeversion.h>
#include <kglobal.h>
#include <kiconeffect.h>
#include <kiconloader.h>
#include <kinputdialog.h>
#include <kio/jobuidelegate.h>
#include <klocale.h>
#include <kmenu.h>
#include <kmessagebox.h>
#include <kmimetypetrader.h>
#include <kpropsdlg.h>
#include <krun.h>
#include <kservice.h>
#include <kstandardaction.h>
#include <kstandarddirs.h>
#include <ktrader.h>
#include <kurl.h>

// LibKIPI includes.

#include <libkipi/pluginloader.h>
#include <libkipi/plugin.h>

// LibKDcraw includes.

#include <libkdcraw/version.h>
#include <libkdcraw/kdcraw.h>

#if KDCRAW_VERSION < 0x000400
#include <libkdcraw/dcrawbinary.h>
#endif

// Local includes.

#include "album.h"
#include "albumdb.h"
#include "albumicongroupitem.h"
#include "albumiconitem.h"
#include "albumiconviewtooltip.h"
#include "albumlister.h"
#include "albummanager.h"
#include "albumsettings.h"
#include "cameraui.h"
#include "constants.h"
#include "databaseaccess.h"
#include "databasetransaction.h"
#include "ddragobjects.h"
#include "deletedialog.h"
#include "digikamapp.h"
#include "dio.h"
#include "dmetadata.h"
#include "dpopupmenu.h"
#include "imageattributeswatch.h"
#include "imagewindow.h"
#include "lighttablewindow.h"
#include "loadingcacheinterface.h"
#include "metadatahub.h"
#include "ratingpopupmenu.h"
#include "scancontroller.h"
#include "statusprogressbar.h"
#include "tagspopupmenu.h"
#include "themeengine.h"
#include "thumbnailloadthread.h"
#include "thumbnailsize.h"

namespace Digikam
{

class AlbumIconViewPrivate
{
public:

    void init()
    {
        imageLister   = 0;
        currentAlbum  = 0;
        albumSettings = 0;
        toolTip       = 0;

        // Pre-computed star polygon for a 15x15 pixmap.
        starPolygon << QPoint(0,  6);
        starPolygon << QPoint(5,  5);
        starPolygon << QPoint(7,  0);
        starPolygon << QPoint(9,  5);
        starPolygon << QPoint(14, 6);
        starPolygon << QPoint(10, 9);
        starPolygon << QPoint(11, 14);
        starPolygon << QPoint(7,  11);
        starPolygon << QPoint(3,  14);
        starPolygon << QPoint(4,  9);

        starPolygonSize = QSize(15, 15);

        ratingPixmaps   = QVector<QPixmap>(10);
    }

    QString                          albumTitle;
    QString                          albumDate;
    QString                          albumComments;

    QRect                            itemRect;
    QRect                            itemRatingRect;
    QRect                            itemDateRect;
    QRect                            itemModDateRect;
    QRect                            itemPixmapRect;
    QRect                            itemNameRect;
    QRect                            itemCommentsRect;
    QRect                            itemResolutionRect;
    QRect                            itemSizeRect;
    QRect                            itemTagRect;
    QRect                            bannerRect;

    QPixmap                          itemRegPixmap;
    QPixmap                          itemSelPixmap;
    QPixmap                          bannerPixmap;
    QVector<QPixmap>                 ratingPixmaps;

    QFont                            fnReg;
    QFont                            fnCom;
    QFont                            fnXtra;

    QPolygon                         starPolygon;
    QSize                            starPolygonSize;

    QHash<QString, AlbumIconItem*>   itemDict;
    QHash<ImageInfo, AlbumIconItem*> itemInfoMap;

    KUrl                             itemUrlToFind;

    AlbumLister                     *imageLister;
    Album                           *currentAlbum;
    const AlbumSettings             *albumSettings;
    QHash<int, AlbumIconGroupItem*>  albumDict;

    ThumbnailSize                    thumbSize;

    AlbumIconViewToolTip            *toolTip;
};

AlbumIconView::AlbumIconView(QWidget* parent)
             : IconView(parent), d(new AlbumIconViewPrivate)
{
    d->init();
    d->imageLister = AlbumLister::instance();
    d->toolTip     = new AlbumIconViewToolTip(this);

    setAcceptDrops(true);
    viewport()->setAcceptDrops(true);

    // -- ImageLister connections -------------------------------------

    connect(d->imageLister, SIGNAL(signalNewFilteredItems(const ImageInfoList&)),
            this, SLOT(slotImageListerNewItems(const ImageInfoList&)));

    connect(d->imageLister, SIGNAL(signalDeleteFilteredItem(const ImageInfo &)),
            this, SLOT(slotImageListerDeleteItem(const ImageInfo &)) );

    connect(d->imageLister, SIGNAL(signalClear()),
            this, SLOT(slotImageListerClear()));

    // -- Icon connections --------------------------------------------

    connect(this, SIGNAL(signalDoubleClicked(IconItem*)),
            this, SLOT(slotDoubleClicked(IconItem*)));

    connect(this, SIGNAL(signalReturnPressed(IconItem*)),
            this, SLOT(slotDoubleClicked(IconItem*)));

    connect(this, SIGNAL(signalRightButtonClicked(IconItem*, const QPoint &)),
            this, SLOT(slotRightButtonClicked(IconItem*, const QPoint &)));

    connect(this, SIGNAL(signalRightButtonClicked(const QPoint &)),
            this, SLOT(slotRightButtonClicked(const QPoint &)));

    connect(this, SIGNAL(signalSelectionChanged()),
            this, SLOT(slotSelectionChanged()));

    connect(this, SIGNAL(signalShowToolTip(IconItem*)),
            this, SLOT(slotShowToolTip(IconItem*)));

    // -- ThemeEngine connections ---------------------------------------

    connect(ThemeEngine::instance(), SIGNAL(signalThemeChanged()),
            this, SLOT(slotThemeChanged()));

    // -- Pixmap manager connections ------------------------------------

    connect(ThumbnailLoadThread::defaultIconViewThread(), SIGNAL(signalThumbnailLoaded(const LoadingDescription &, const QPixmap&)),
            this, SLOT(slotThumbnailLoaded(const LoadingDescription &, const QPixmap&)));

    // -- ImageAttributesWatch connections ------------------------------

    ImageAttributesWatch *watch = ImageAttributesWatch::instance();

    connect(watch, SIGNAL(signalImageTagsChanged(qlonglong)),
            this, SLOT(slotImageAttributesChanged(qlonglong)));

    connect(watch, SIGNAL(signalImagesChanged(int)),
            this, SLOT(slotAlbumImagesChanged(int)));

    connect(watch, SIGNAL(signalImageRatingChanged(qlonglong)),
            this, SLOT(slotImageAttributesChanged(qlonglong)));

    connect(watch, SIGNAL(signalImageDateChanged(qlonglong)),
            this, SLOT(slotImageAttributesChanged(qlonglong)));

    connect(watch, SIGNAL(signalImageCaptionChanged(qlonglong)),
            this, SLOT(slotImageAttributesChanged(qlonglong)));

    // -- FileWatch connections ------------------------------

    LoadingCacheInterface::connectToSignalFileChanged(this,
            SLOT(slotFileChanged(const QString &)));

    // -- Internal connections ------------------------------

    // defer this action from handling of drag event, see bug #159855
    connect(this, SIGNAL(changeTagOnImageInfos(const ImageInfoList &, const QList<int> &, bool, bool)),
            this, SLOT(slotChangeTagOnImageInfos(const ImageInfoList &, const QList<int> &, bool, bool)),
            Qt::QueuedConnection);
}

AlbumIconView::~AlbumIconView()
{
    delete d->toolTip;
    delete d;
}

void AlbumIconView::applySettings(const AlbumSettings* settings)
{
    if (!settings)
        return;

    d->albumSettings = settings;

    d->thumbSize = (ThumbnailSize::Size)d->albumSettings->getDefaultIconSize();

    setEnableToolTips(d->albumSettings->getShowToolTips());

    updateRectsAndPixmaps();

    d->imageLister->stop();
    clear();

    ThumbnailLoadThread::defaultIconViewThread()->setThumbnailSize(d->thumbSize.size());

    if (d->currentAlbum)
    {
        d->imageLister->openAlbum(d->currentAlbum);
    }
}

void AlbumIconView::setThumbnailSize(const ThumbnailSize& thumbSize)
{
    if ( d->thumbSize != thumbSize)
    {
        d->thumbSize = thumbSize;
        ThumbnailLoadThread::defaultIconViewThread()->setThumbnailSize(d->thumbSize.size());

        updateRectsAndPixmaps();
        IconItem *currentIconItem = currentItem();
        triggerRearrangement();
        setStoredVisibleItem(currentIconItem);
    }
}

void AlbumIconView::setAlbum(Album* album)
{
    if (!album)
    {
        d->currentAlbum = 0;
        d->imageLister->stop();
        clear();

        return;
    }

    if (d->currentAlbum == album)
        return;

    d->imageLister->stop();
    clear();

    d->currentAlbum = album;
    d->imageLister->openAlbum(d->currentAlbum);

    updateRectsAndPixmaps();
}

void AlbumIconView::setAlbumItemToFind(const KUrl& url)
{
    d->itemUrlToFind = url;
}

void AlbumIconView::refreshIcon(AlbumIconItem* item)
{
    if (!item)
        return;

    emit signalSelectionChanged();
}

void AlbumIconView::clear(bool update)
{
    emit signalCleared();

    d->itemDict.clear();
    d->albumDict.clear();
    d->itemInfoMap.clear();

    IconView::clear(update);

    emit signalSelectionChanged();
}

void AlbumIconView::slotImageListerNewItems(const ImageInfoList& itemList)
{
    if (!d->currentAlbum || d->currentAlbum->isRoot())
        return;

    for (ImageInfoList::const_iterator it = itemList.constBegin(); it != itemList.constEnd(); ++it)
    {
        KUrl url( it->fileUrl() );
        url.cleanPath();

        if (d->itemInfoMap.contains(*it))
        {
            // TODO: Make sure replacing slotImageListerDeleteItem with continue here is not wrong
            //slotImageListerDeleteItem((*itMap)->imageInfo());
            continue;
        }

        AlbumIconGroupItem* group = d->albumDict.value(it->albumId());
        if (!group)
        {
            group = new AlbumIconGroupItem(this, it->albumId());
            d->albumDict.insert(it->albumId(), group);
        }

        PAlbum *album = AlbumManager::instance()->findPAlbum(it->albumId());
        if (!album)
        {
            kWarning(50003) << "No album for item: " << it->name()
                            << ", albumID: " << it->albumId() << endl;
            continue;
        }

        AlbumIconItem* iconItem = new AlbumIconItem(group, (*it));

        d->itemDict.insert(url.url(), iconItem);
        d->itemInfoMap.insert((*it), iconItem);
    }

    // Make the icon, specified by d->itemUrlToFind, the current one
    // in the album icon view and make it visible.
    // This is for example used after a "Go To",
    // e.g. from tags (or date) view to folder view.
    // Note that AlbumIconView::slotImageListerNewItems may
    // be called several times after another, because images get
    // listed in packages of 200.
    // Therefore the item might not always be available in the very
    // first call when there are sufficiently many images.
    // Also, because of this, we cannot reset the item which is to be found,
    // i.e. something like d->itemUrlToFind = 0, after the item was found,
    // as then the visibility of this item is lost in a subsequent call.
    if (!d->itemUrlToFind.isEmpty())
    {
        AlbumIconItem* icon = findItem(d->itemUrlToFind.url());
        if (icon)
        {
            clearSelection();
            updateContents();
            setCurrentItem(icon);
            ensureItemVisible(icon);

            // make the item really visible
            // (the previous ensureItemVisible does not work)
            setStoredVisibleItem(icon);
            triggerRearrangement();
        }
    }

    emit signalItemsAdded();
}

void AlbumIconView::slotImageListerDeleteItem(const ImageInfo &item)
{
    QHash<ImageInfo, AlbumIconItem*>::iterator itMap = d->itemInfoMap.find(item);
    if (itMap == d->itemInfoMap.constEnd())
        return;

    AlbumIconItem* iconItem = (*itMap);

    /*
    // ?? Necessary? For what situation?
    KUrl url(item->kurl());
    url.cleanPath();

    AlbumIconItem *oldItem = d->itemDict[url.url()];

    if( oldItem &&
       (oldItem->imageInfo()->id() != iconItem->imageInfo()->id()))
    {
        return;
    }
    */

    //d->pixMan->deleteThumbnail(item->kurl());

    emit signalItemDeleted(iconItem);

    delete iconItem;

    d->itemInfoMap.remove(item);
    d->itemDict.remove(item.fileUrl().url());

    IconGroupItem* group = firstGroup();
    IconGroupItem* tmp;

    while (group)
    {
        tmp = group->nextGroup();

        if (group->count() == 0)
        {
            d->albumDict.remove(((AlbumIconGroupItem*)group)->albumID());
            delete group;
        }

        group = tmp;
    }
}

void AlbumIconView::slotImageListerClear()
{
    clear();
}

void AlbumIconView::slotDoubleClicked(IconItem *item)
{
    if (!item) return;

    if (d->albumSettings->getItemRightClickAction() == AlbumSettings::ShowPreview)
    {
        // icon effect takes too much time
        //KIconEffect::visualActivate(viewport(), contentsRectToViewport(item->rect()));
        signalPreviewItem(static_cast<AlbumIconItem *>(item));
    }
    else
    {
        // FIXME: this method has diseapear from kdelibs4
        //KIconEffect::visualActivate(viewport(), contentsRectToViewport(item->rect()));
        slotDisplayItem(static_cast<AlbumIconItem *>(item));
    }
}

void AlbumIconView::slotRightButtonClicked(const QPoint& pos)
{
    if (!d->currentAlbum)
        return;

    if (d->currentAlbum->isRoot() ||
         (   d->currentAlbum->type() != Album::PHYSICAL
          && d->currentAlbum->type() != Album::TAG))
    {
        return;
    }

    KMenu popmenu(this);
    KAction *paste        = KStandardAction::paste(this, SLOT(slotPaste()), 0);
    const QMimeData *data = kapp->clipboard()->mimeData(QClipboard::Clipboard);

    if(!data || !KUrl::List::canDecode(data))
        paste->setEnabled(false);

    popmenu.addAction(paste);
    popmenu.exec(pos);
    delete paste;
}

void AlbumIconView::slotRightButtonClicked(IconItem *item, const QPoint& pos)
{
    if (!item)
        return;

    AlbumIconItem* iconItem = static_cast<AlbumIconItem *>(item);

    //-- Open With Actions ------------------------------------

    KMimeType::Ptr mimePtr = KMimeType::findByUrl(iconItem->imageInfo().fileUrl(), 0, true, true);

    QMap<QAction*, KService::Ptr> serviceMap;

    const KService::List offers = KMimeTypeTrader::self()->query(mimePtr->name());
    KService::List::ConstIterator iter;
    KService::Ptr ptr;

    KMenu openWithMenu;

    for( iter = offers.constBegin(); iter != offers.constEnd(); ++iter )
    {
        ptr = *iter;
        QAction *serviceAction = openWithMenu.addAction(SmallIcon(ptr->icon()), ptr->name());
        serviceMap[serviceAction] = ptr;
    }

    if (openWithMenu.isEmpty())
        openWithMenu.menuAction()->setEnabled(false);

    // --------------------------------------------------------

    // Obtain a list of all selected images.
    // This is needed both for the goto tags submenu here and also
    // for the "move to trash" and further actions below.
    QList<qlonglong> selectedImageIDs;

    for (IconItem *it = firstItem(); it; it=it->nextItem())
    {
        if (it->isSelected())
        {
            AlbumIconItem *selItem = static_cast<AlbumIconItem *>(it);
            selectedImageIDs.append(selItem->imageInfo().id());
        }
    }

    // --------------------------------------------------------
    // Provide Goto folder and/or date pop-up menu
    KMenu gotoMenu;

    QAction *gotoAlbum = gotoMenu.addAction(SmallIcon("folder-image"),        i18n("Album"));
    QAction *gotoDate  = gotoMenu.addAction(SmallIcon("view-calendar-month"), i18n("Date"));

    TagsPopupMenu gotoTagsPopup(selectedImageIDs, TagsPopupMenu::DISPLAY);
    QAction *gotoTag = gotoMenu.addMenu(&gotoTagsPopup);
    gotoTag->setIcon(SmallIcon("tag"));
    gotoTag->setText(i18n("Tag"));

    // Disable the goto Tag popup menu, if there are no tags at all.
    if (!DatabaseAccess().db()->hasTags(selectedImageIDs))
        gotoTag->setEnabled(false);

    connect(&gotoTagsPopup, SIGNAL(signalTagActivated(int)),
            this, SLOT(slotGotoTag(int)));

    if (d->currentAlbum->type() == Album::PHYSICAL )
    {
        // If the currently selected album is the same as album to
        // which the image belongs, then disable the "Go To" Album.
        // (Note that in recursive album view these can be different).
        if (iconItem->imageInfo().albumId() == d->currentAlbum->id())
            gotoAlbum->setEnabled(false);
    }
    else if (d->currentAlbum->type() == Album::DATE )
    {
        gotoDate->setEnabled(false);
    }

    // --------------------------------------------------------

    DPopupMenu popmenu(this);
    QAction *viewAction         = popmenu.addAction(SmallIcon("viewimage"),     i18n("View..."));
    QAction *editAction         = popmenu.addAction(SmallIcon("editimage"),     i18n("Edit..."));
    QAction *lighttableAction   = popmenu.addAction(SmallIcon("lighttableadd"), i18n("Add to Light Table"));
    QAction *findSimilarAction  = popmenu.addAction(SmallIcon("tools-wizard"),  i18n("Find Similar"));
    QAction *gotoAction         = popmenu.addMenu(&gotoMenu);
    gotoAction->setIcon(SmallIcon("go-jump"));
    gotoAction->setText(i18n("Go To"));

    // If there is more than one image selected, disable the goto menu entry.
    if (selectedImageIDs.count() > 1)
    {
        gotoAction->setEnabled(false);
    }

    popmenu.addMenu(&openWithMenu);
    openWithMenu.menuAction()->setText(i18n("Open With"));

    // Merge in the KIPI plugins actions ----------------------------

    KIPI::PluginLoader* kipiPluginLoader      = KIPI::PluginLoader::instance();
    KIPI::PluginLoader::PluginList pluginList = kipiPluginLoader->pluginList();

    for (KIPI::PluginLoader::PluginList::const_iterator it = pluginList.constBegin();
         it != pluginList.constEnd(); ++it)
    {
        KIPI::Plugin* plugin = (*it)->plugin();

        if (plugin && (*it)->name() == "JPEGLossless")
        {
            kDebug(50003) << "Found JPEGLossless plugin" << endl;

            QList<KAction*> actionList = plugin->actions();

            for (QList<KAction*>::const_iterator iter = actionList.constBegin();
                iter != actionList.constEnd(); ++iter)
            {
                KAction* action = *iter;

                if (action->objectName().toLatin1() == QString::fromLatin1("jpeglossless_rotate"))
                {
                    popmenu.addAction(action);
                }
            }
        }
    }

    // --------------------------------------------------------

    QAction *renameAction = popmenu.addAction(SmallIcon("edit-rename"), i18n("Rename..."));
    popmenu.addSeparator();

    // --------------------------------------------------------

    QAction *thumbnailAction = 0;
    if (d->currentAlbum)
    {
        if (d->currentAlbum->type() == Album::PHYSICAL )
        {
            thumbnailAction = popmenu.addAction(i18n("Set as Album Thumbnail"));
            popmenu.addSeparator();
        }
        else if (d->currentAlbum->type() == Album::TAG )
        {
            thumbnailAction = popmenu.addAction(i18n("Set as Tag Thumbnail"));
            popmenu.addSeparator();
        }
    }

    // --------------------------------------------------------

    KAction *copy         = KStandardAction::copy(this, SLOT(slotCopy()), 0);
    KAction *paste        = KStandardAction::paste(this, SLOT(slotPaste()), 0);
    const QMimeData *data = kapp->clipboard()->mimeData(QClipboard::Clipboard);

    if(!data || !KUrl::List::canDecode(data))
        paste->setEnabled(false);

    popmenu.addAction(copy);
    popmenu.addAction(paste);
    popmenu.addSeparator();

    // --------------------------------------------------------

    QAction *trashAction = popmenu.addAction(SmallIcon("user-trash"),
                           i18np("Move to Trash", "Move %1 Files to Trash", selectedImageIDs.count()));

    popmenu.addSeparator();

    // Bulk assignment/removal of tags --------------------------

    TagsPopupMenu assignTagsPopup(selectedImageIDs, TagsPopupMenu::ASSIGN);
    TagsPopupMenu removeTagsPopup(selectedImageIDs, TagsPopupMenu::REMOVE);

    connect(&assignTagsPopup, SIGNAL(signalTagActivated(int)),
            this, SLOT(slotAssignTag(int)));

    connect(&removeTagsPopup, SIGNAL(signalTagActivated(int)),
            this, SLOT(slotRemoveTag(int)));

    popmenu.addMenu(&assignTagsPopup);
    assignTagsPopup.menuAction()->setText(i18n("Assign Tag"));

    popmenu.addMenu(&removeTagsPopup);
    removeTagsPopup.menuAction()->setText(i18n("Remove Tag"));

    // Performance: Only check for tags if there are <250 images selected
    // Also disable the remove Tag popup menu, if there are no tags at all.
    if (selectedImageIDs.count() > 250 ||
        !DatabaseAccess().db()->hasTags(selectedImageIDs))
        removeTagsPopup.menuAction()->setEnabled(false);

    popmenu.addSeparator();

    // Assign Star Rating -------------------------------------------

    RatingPopupMenu ratingMenu;

    connect(&ratingMenu, SIGNAL(signalRatingChanged(int)),
            this, SLOT(slotAssignRating(int)));

    popmenu.addMenu(&ratingMenu);
    ratingMenu.menuAction()->setText(i18n("Assign Rating"));

    // --------------------------------------------------------

    QAction *choice = popmenu.exec(pos);

    if (choice)
    {
        if (choice == editAction)
        {
            slotDisplayItem(iconItem);
        }
        else if (choice == renameAction)
        {
            slotRename(iconItem);
        }
        else if (choice == trashAction)
        {
            slotDeleteSelectedItems();
        }
        else if (choice == thumbnailAction)
        {
            slotSetAlbumThumbnail(iconItem);
        }
        else if (choice == viewAction)
        {
            signalPreviewItem(iconItem);
        }
        else if (choice == lighttableAction)
        {
            //  add images to existing images in the light table
            insertSelectionToLightTable(true);
        }
        else if (choice == findSimilarAction)
        {
            // send a signal to the parent widget (digikamview.cpp)
            emit signalFindSimilar();
        }
        else if (choice == gotoAlbum)
        {
            // send a signal to the parent widget (digikamview.cpp)
            emit signalGotoAlbumAndItem(iconItem);
        }
        else if (choice == gotoDate)
        {
            // send a signal to the parent widget (digikamview.cpp)
            emit signalGotoDateAndItem(iconItem);
        }
        else
        {
            if (serviceMap.contains(choice))
            {
                KService::Ptr imageServicePtr = serviceMap[choice];
                KUrl::List urlList;
                for (IconItem *it = firstItem(); it; it=it->nextItem())
                {
                    if (it->isSelected())
                    {
                        AlbumIconItem *selItem = static_cast<AlbumIconItem *>(it);
                        urlList.append(selItem->imageInfo().fileUrl());
                    }
                }
                if (urlList.count())
                    KRun::run(*imageServicePtr, urlList, this);
            }
        }
    }

    //---------------------------------------------------------------

    popmenu.deleteLater();
    delete copy;
    delete paste;
}

void AlbumIconView::slotCopy()
{
    if (!d->currentAlbum)
        return;

    KUrl::List urls;
    KUrl::List kioURLs;
    QList<int> albumIDs;
    QList<int> imageIDs;

    for (IconItem *it = firstItem(); it; it=it->nextItem())
    {
        if (it->isSelected())
        {
            AlbumIconItem *albumItem = static_cast<AlbumIconItem *>(it);
            ImageInfo info = albumItem->imageInfo();
            urls.append(info.fileUrl());
            kioURLs.append(info.databaseUrl());
            imageIDs.append(info.id());
        }
    }
    albumIDs.append(d->currentAlbum->id());

    if (urls.isEmpty())
        return;

    kapp->clipboard()->setMimeData(new DItemDrag(urls, kioURLs, albumIDs, imageIDs));
}

void AlbumIconView::slotPaste()
{
    const QMimeData *data = kapp->clipboard()->mimeData(QClipboard::Clipboard);
    if(!data)
        return;

    Album *album = 0;

    // Check if we working on grouped items view.
    if (groupCount() > 1)
    {
        AlbumIconGroupItem *grp = dynamic_cast<AlbumIconGroupItem*>(findGroup(QCursor::pos()));
        if (grp)
        {
            if(d->currentAlbum->type() == Album::PHYSICAL)
                album = dynamic_cast<Album*>(AlbumManager::instance()->findPAlbum(grp->albumID()));
            else if(d->currentAlbum->type() == Album::TAG)
                album = dynamic_cast<Album*>(AlbumManager::instance()->findTAlbum(grp->albumID()));
        }
    }
    if (!album)
        album = d->currentAlbum;

    if (d->currentAlbum->type() == Album::PHYSICAL)
    {
        if (DItemDrag::canDecode(data))
        {
            // Drag & drop inside of digiKam

            PAlbum* palbum = (PAlbum*)album;

            // B.K.O #119205: do not handle root album.
            if (palbum->isRoot())
                return;

            KUrl::List urls;
            KUrl::List kioURLs;
            QList<int> albumIDs;
            QList<int> imageIDs;

            if (!DItemDrag::decode(data, urls, kioURLs, albumIDs, imageIDs))
                return;

            if (urls.isEmpty() || kioURLs.isEmpty() || albumIDs.isEmpty() || imageIDs.isEmpty())
                return;

            // Check if items dropped come from outside current album.
            KUrl::List extUrls;
            QList<qlonglong> extImageIDs;
            for (QList<int>::ConstIterator it = imageIDs.constBegin(); it != imageIDs.constEnd(); ++it)
            {
                ImageInfo info(*it);
                if (info.albumId() != album->id())
                {
                    extUrls << info.databaseUrl();
                    extImageIDs << *it;
                }
            }

            if(extUrls.isEmpty())
                return;

            KIO::Job* job = DIO::copy(kioURLs, extImageIDs, palbum);
            connect(job, SIGNAL(result(KJob*)),
                    this, SLOT(slotDIOResult(KJob*)));
        }
        else if (KUrl::List::canDecode(data))
        {
            PAlbum* palbum = (PAlbum*)album;

            // B.K.O #119205: do not handle root album.
            if (palbum->isRoot())
                return;

            KUrl::List srcURLs = KUrl::List::fromMimeData(data);

            KIO::Job* job = DIO::copy(srcURLs, palbum);
            connect(job, SIGNAL(result(KJob*)),
                    this, SLOT(slotDIOResult(KJob*)));
        }
    }
    else if(d->currentAlbum->type() == Album::TAG && DItemDrag::canDecode(data))
    {
        TAlbum* talbum = (TAlbum*)album;

        // B.K.O #119205: do not handle root album.
        if (talbum->isRoot())
            return;

        KUrl::List urls;
        KUrl::List kioURLs;
        QList<int> albumIDs;
        QList<int> imageIDs;

        if (!DItemDrag::decode(data, urls, kioURLs, albumIDs, imageIDs))
            return;

        if (urls.isEmpty() || kioURLs.isEmpty() || albumIDs.isEmpty() || imageIDs.isEmpty())
            return;

        ImageInfoList list;
        for (QList<int>::const_iterator it = imageIDs.constBegin();
             it != imageIDs.constEnd(); ++it)
        {
            ImageInfo info(*it);
            list.append(info);
        }

        emit changeTagOnImageInfos(list, QList<int>() << talbum->id(), true, true);
    }
}

void AlbumIconView::slotSetAlbumThumbnail(AlbumIconItem *iconItem)
{
    if(!d->currentAlbum)
        return;

    if(d->currentAlbum->type() == Album::PHYSICAL)
    {
        PAlbum *album = static_cast<PAlbum*>(d->currentAlbum);

        QString err;
        AlbumManager::instance()->updatePAlbumIcon( album,
                                                    iconItem->imageInfo().id(),
                                                    err );
    }
    else if (d->currentAlbum->type() == Album::TAG)
    {
        TAlbum *album = static_cast<TAlbum*>(d->currentAlbum);

        QString err;
        AlbumManager::instance()->updateTAlbumIcon( album,
                                                    QString(),
                                                    iconItem->imageInfo().id(),
                                                    err );
    }
}

void AlbumIconView::slotRename(AlbumIconItem* item)
{
    if (!item)
        return;

    // Create a copy of the item. After entering the event loop
    // in the dialog, we cannot be sure about the item's status.
    ImageInfo renameInfo = item->imageInfo();

    QFileInfo fi(renameInfo.name());
    QString ext  = QString(".") + fi.suffix();
    QString name = fi.fileName();
    name.truncate(fi.fileName().length() - ext.length());

    bool ok;

    QString newName = KInputDialog::getText(i18n("Rename Item (%1)",fi.fileName()),
                                            i18n("Enter new name (without extension):"),
                                            name, &ok, this);
    if (!ok)
        return;

    KIO::CopyJob* job = DIO::rename(renameInfo, newName + ext);

    connect(job, SIGNAL(result(KJob*)),
            this, SLOT(slotDIOResult(KJob*)));

    connect(job, SIGNAL(copyingDone(KIO::Job *, const KUrl &, const KUrl &, bool, bool)),
            this, SLOT(slotRenamed(KIO::Job*, const KUrl &, const KUrl&)));

    //TODO: The explanation is outdated. Check if we can safely remove.
    // The AlbumManager KDirWatch will trigger a DIO::scan.
    // When this is completed, DIO will call AlbumLister::instance()->refresh().
    // Usually the AlbumLister will ignore changes to already listed items.
    // So the renamed item need explicitly be invalidated.
    d->imageLister->invalidateItem(renameInfo);
}

void AlbumIconView::slotRenamed(KIO::Job*, const KUrl &, const KUrl&newURL)
{
    // reconstruct file path from digikamalbums:// URL
    KUrl fileURL;
    fileURL.setPath(newURL.user());
    fileURL.addPath(newURL.path());

    // refresh thumbnail
    ThumbnailLoadThread::deleteThumbnail(fileURL.path());
    // clean LoadingCache as well - be pragmatic, do it here.
    LoadingCacheInterface::fileChanged(fileURL.path());
}

void AlbumIconView::slotDeleteSelectedItems(bool deletePermanently)
{
    KUrl::List  urlList;
    KUrl::List  kioUrlList;

    for (IconItem *it = firstItem(); it; it=it->nextItem())
    {
        if (it->isSelected())
        {
            AlbumIconItem *iconItem = static_cast<AlbumIconItem *>(it);
            ImageInfo info = iconItem->imageInfo();
            urlList.append(info.fileUrl());
            kioUrlList.append(info.databaseUrl());
        }
    }

    if (urlList.count() <= 0)
        return;

    DeleteDialog dialog(this);

    if (!dialog.confirmDeleteList(urlList,
                                  DeleteDialogMode::Files,
                                  deletePermanently ?
                                  DeleteDialogMode::NoChoiceDeletePermanently :
                                  DeleteDialogMode::NoChoiceTrash))
        return;

    bool useTrash = !dialog.shouldDelete();

    // trash does not like non-local URLs, put is not implemented
    KIO::Job* job = DIO::del(useTrash ? urlList : kioUrlList, useTrash);

    connect(job, SIGNAL(result(KJob*)),
            this, SLOT(slotDIOResult(KJob*)));
}

void AlbumIconView::slotDeleteSelectedItemsDirectly(bool useTrash)
{
    // This method deletes the selected items directly, without confirmation.
    // It is not used in the default setup.

    KUrl::List kioUrlList;
    KUrl::List urlList;

    for (IconItem *it = firstItem(); it; it=it->nextItem())
    {
        if (it->isSelected())
        {
            AlbumIconItem *iconItem = static_cast<AlbumIconItem *>(it);
            ImageInfo info = iconItem->imageInfo();
            kioUrlList.append(info.databaseUrl());
            urlList.append(info.fileUrl());
        }
    }

    if (kioUrlList.count() <= 0)
        return;

    // trash does not like non-local URLs, put is not implemented
    KIO::Job* job = DIO::del(useTrash ? urlList : kioUrlList , useTrash);

    connect(job, SIGNAL(result(KJob*)),
            this, SLOT(slotDIOResult(KJob*)));
}

void AlbumIconView::slotFilesModified()
{
    d->imageLister->refresh();
}

void AlbumIconView::slotFilesModified(const KUrl& url)
{
    refreshItems(url);
}

void AlbumIconView::slotImageWindowURLChanged(const KUrl &url)
{
    IconItem* item = findItem(url.url());
    if (item)
        setCurrentItem(item);
}

void AlbumIconView::slotDisplayItem(AlbumIconItem *item)
{
    if (!item) return;

    AlbumSettings *settings = AlbumSettings::instance();

    if (!settings) return;

    QString currentFileExtension = item->imageInfo().name().section( '.', -1 );
    QString imagefilter = settings->getImageFileFilter().toLower() +
                          settings->getImageFileFilter().toUpper();

#if KDCRAW_VERSION < 0x000400
    if (KDcrawIface::DcrawBinary::instance()->versionIsRight())
    {
        // add raw files only if dcraw is available
        imagefilter += settings->getRawFileFilter().toLower() +
                       settings->getRawFileFilter().toUpper();
    }
#else
    // add raw files only if dcraw is available
    imagefilter += settings->getRawFileFilter().toLower() +
                   settings->getRawFileFilter().toUpper();
#endif

    // If the current item is not an image file.
    if ( !imagefilter.contains(currentFileExtension) )
    {
        KMimeType::Ptr mimePtr = KMimeType::findByUrl(item->imageInfo().fileUrl(), 0, true, true);
        const KService::List offers = KServiceTypeTrader::self()->query(mimePtr->name(), "Type == 'Application'");

        if (offers.isEmpty())
            return;

        KService::Ptr ptr = offers.first();
        // Run the dedicated app to show the item.
        KRun::run(*ptr, item->imageInfo().fileUrl(), this);
        return;
    }

    // Run digiKam ImageEditor with all image from current Album.

    ImageInfoList list;
    ImageInfo     current;

    for (IconItem *it = firstItem() ; it ; it = it->nextItem())
    {
        AlbumIconItem *iconItem = static_cast<AlbumIconItem*>(it);
        ImageInfo info          = iconItem->imageInfo();
        QString fileExtension   = info.fileUrl().fileName().section( '.', -1 );

        if ( imagefilter.indexOf(fileExtension) != -1 )
        {
            list << info;
            if (iconItem == item)
                current = info;
        }
    }

    ImageWindow *imview = ImageWindow::imagewindow();

    imview->disconnect(this);

    //connect(imview, SIGNAL(signalFileAdded(const KUrl&)),
      //      this, SLOT(slotFilesModified()));

    //connect(imview, SIGNAL(signalFileModified(const KUrl&)),
      //      this, SLOT(slotFilesModified(const KUrl&)));

    //connect(imview, SIGNAL(signalFileDeleted(const KUrl&)),
      //      this, SLOT(slotFilesModified()));

    connect(imview, SIGNAL(signalURLChanged(const KUrl&)),
            this, SLOT(slotImageWindowURLChanged(const KUrl &)));

    imview->loadImageInfos(list, current,
                           d->currentAlbum ? i18n("Album \"%1\"",d->currentAlbum->title()) : QString(),
                           true);

    if (imview->isHidden())
        imview->show();

    imview->raise();
    imview->setFocus();
}

void AlbumIconView::insertSelectionToLightTable(bool addTo)
{
    // Run Light Table with all selected image files in the current Album.
    // If addTo is false, the light table will be emptied before adding
    // the images.
    ImageInfoList imageInfoList;

    for (IconItem *it = firstItem() ; it ; it = it->nextItem())
    {
        if ((*it).isSelected())
        {
            AlbumIconItem *iconItem = static_cast<AlbumIconItem *>(it);
            imageInfoList << iconItem->imageInfo();
        }
    }

    insertToLightTable(imageInfoList, imageInfoList.first(), addTo);
}

void AlbumIconView::insertToLightTable(const ImageInfoList& list, const ImageInfo &current, bool addTo)
{
    LightTableWindow *ltview = LightTableWindow::lightTableWindow();

    ltview->disconnect(this);

    connect(ltview, SIGNAL(signalFileDeleted(const KUrl&)),
           this, SLOT(slotFilesModified()));

    connect(this, SIGNAL(signalItemsUpdated(const KUrl::List&)),
           ltview, SLOT(slotItemsUpdated(const KUrl::List&)));

    if (ltview->isHidden())
        ltview->show();

    ltview->raise();
    ltview->setFocus();
    // If addTo is false, the light table will be emptied before adding
    // the images.
    ltview->loadImageInfos(list, current, addTo);
    ltview->setLeftRightItems(list, addTo);
}

// ------------------------------------------------------------------------------

AlbumIconItem* AlbumIconView::firstSelectedItem() const
{
    AlbumIconItem *iconItem = 0;
    for (IconItem *it = firstItem(); it; it = it->nextItem())
    {
        if (it->isSelected())
        {
            iconItem = static_cast<AlbumIconItem *>(it);
            break;
        }
    }

    return iconItem;
}

const AlbumSettings* AlbumIconView::settings() const
{
    return d->albumSettings;
}

ThumbnailSize AlbumIconView::thumbnailSize() const
{
    return d->thumbSize;
}

void AlbumIconView::resizeEvent(QResizeEvent *e)
{
    IconView::resizeEvent(e);

    if (d->bannerRect.width() != frameRect().width())
        updateRectsAndPixmaps();
}

// -- DnD ---------------------------------------------------

void AlbumIconView::startDrag()
{
    if (!d->currentAlbum)
        return;

    KUrl::List urls;
    KUrl::List kioURLs;
    QList<int> albumIDs;
    QList<int> imageIDs;

    for (IconItem *it = firstItem(); it; it=it->nextItem())
    {
        if (it->isSelected())
        {
            AlbumIconItem *albumItem = static_cast<AlbumIconItem *>(it);
            ImageInfo info = albumItem->imageInfo();
            urls.append(info.fileUrl());
            kioURLs.append(info.databaseUrl());
            imageIDs.append(info.id());
        }
    }
    albumIDs.append(d->currentAlbum->id());

    if (urls.isEmpty())
        return;

    QPixmap icon(DesktopIcon("image-jp2", 48));
    int w = icon.width();
    int h = icon.height();

    QPixmap pix(w+4, h+4);
    QString text(QString::number(urls.count()));

    QPainter p(&pix);
    p.fillRect(0, 0, pix.width()-1, pix.height()-1, QColor(Qt::white));
    p.setPen(QPen(Qt::black, 1));
    p.drawRect(0, 0, pix.width()-1, pix.height()-1);
    p.drawPixmap(2, 2, icon);
    QRect r = p.boundingRect(2, 2, w, h, Qt::AlignLeft|Qt::AlignTop, text);
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
    drag->setMimeData(new DItemDrag(urls, kioURLs, albumIDs, imageIDs));
    drag->setPixmap(pix);
    drag->exec();
}

void AlbumIconView::contentsDragEnterEvent(QDragEnterEvent *e)
{
    if (!d->currentAlbum || (DAlbumDrag::canDecode(e->mimeData()) ||
                            (!KUrl::List::canDecode(e->mimeData())          &&
                             !DCameraDragObject::canDecode(e->mimeData())   &&
                             !DTagListDrag::canDecode(e->mimeData())        &&
                             !DTagDrag::canDecode(e->mimeData())            &&
                             !DCameraItemListDrag::canDecode(e->mimeData()) &&
                             !DItemDrag::canDecode(e->mimeData()))))
    {
        return;
    }
    e->acceptProposedAction();
}

void AlbumIconView::contentsDropEvent(QDropEvent *e)
{
    if (!d->currentAlbum || (DAlbumDrag::canDecode(e->mimeData()) ||
                            (!KUrl::List::canDecode(e->mimeData())          &&
                             !DCameraDragObject::canDecode(e->mimeData())   &&
                             !DTagListDrag::canDecode(e->mimeData())        &&
                             !DTagDrag::canDecode(e->mimeData())            &&
                             !DCameraItemListDrag::canDecode(e->mimeData()) &&
                             !DItemDrag::canDecode(e->mimeData()))))
    {
        e->ignore();
        return;
    }

    Album *album = 0;

    // Check if we working on grouped items view.
    if (groupCount() > 1)
    {
        AlbumIconGroupItem *grp = dynamic_cast<AlbumIconGroupItem*>(findGroup(QCursor::pos()));
        if (grp)
        {
            if(d->currentAlbum->type() == Album::PHYSICAL)
                album = dynamic_cast<Album*>(AlbumManager::instance()->findPAlbum(grp->albumID()));
            else if(d->currentAlbum->type() == Album::TAG)
                album = dynamic_cast<Album*>(AlbumManager::instance()->findTAlbum(grp->albumID()));
        }
    }
    if (!album)
        album = d->currentAlbum;

    KUrl::List urls;
    KUrl::List kioURLs;
    QList<int> albumIDs;
    QList<int> imageIDs;

    if (DItemDrag::decode(e->mimeData(), urls, kioURLs, albumIDs, imageIDs))
    {
        // Drag & drop inside of digiKam

        // Check if items dropped come from outside current album.
        KUrl::List extUrls;
        QList<qlonglong> extImageIDs;
        for (QList<int>::iterator it = imageIDs.begin(); it != imageIDs.end(); ++it)
        {
            ImageInfo info(*it);
            if (info.albumId() != album->id())
            {
                extUrls << info.databaseUrl();
                extImageIDs << *it;
            }
        }

        if(extUrls.isEmpty())
        {
            e->ignore();
            return;
        }
        else if (album->type() == Album::PHYSICAL)
        {
            PAlbum* palbum = (PAlbum*)album;

            KMenu popMenu(this);
            QAction *moveAction = popMenu.addAction( SmallIcon("go-jump"), i18n("&Move Here"));
            QAction *copyAction = popMenu.addAction( SmallIcon("edit-copy"), i18n("&Copy Here"));
            popMenu.addSeparator();
            popMenu.addAction( SmallIcon("dialog-cancel"), i18n("C&ancel") );

            popMenu.setMouseTracking(true);
            QAction *choice = popMenu.exec(QCursor::pos());
            if (choice == moveAction)
            {
                KIO::Job* job = DIO::move(extUrls, extImageIDs, palbum);
                connect(job, SIGNAL(result(KJob*)),
                        this, SLOT(slotDIOResult(KJob*)));
            }
            else if (choice == copyAction)
            {
                KIO::Job* job = DIO::copy(extUrls, extImageIDs, palbum);
                connect(job, SIGNAL(result(KJob*)),
                        this, SLOT(slotDIOResult(KJob*)));
            }
        }
    }
    else if (KUrl::List::canDecode(e->mimeData()) && d->currentAlbum->type() == Album::PHYSICAL)
    {
        // Drag & drop outside of digiKam
        PAlbum* palbum = (PAlbum*)album;

        KUrl::List srcURLs = KUrl::List::fromMimeData(e->mimeData());

        KMenu popMenu(this);
        QAction *moveAction = popMenu.addAction( SmallIcon("go-jump"), i18n("&Move Here"));
        QAction *copyAction = popMenu.addAction( SmallIcon("edit-copy"), i18n("&Copy Here"));
        popMenu.addSeparator();
        popMenu.addAction( SmallIcon("dialog-cancel"), i18n("C&ancel") );

        popMenu.setMouseTracking(true);
        QAction *choice = popMenu.exec(QCursor::pos());
        if (choice == moveAction)
        {
            KIO::Job* job = DIO::move(srcURLs, palbum);
            connect(job, SIGNAL(result(KJob*)),
                    this, SLOT(slotDIOResult(KJob*)));
        }
        else if (choice == copyAction)
        {
            KIO::Job* job = DIO::copy(srcURLs, palbum);
            connect(job, SIGNAL(result(KJob*)),
                    this, SLOT(slotDIOResult(KJob*)));
        }
    }
    else if(DTagDrag::canDecode(e->mimeData()))
    {
        int tagID;
        if (!DTagDrag::decode(e->mimeData(), tagID))
            return;

        AlbumManager* man = AlbumManager::instance();
        TAlbum* talbum    = man->findTAlbum(tagID);

        if (talbum)
        {
            KMenu popMenu(this);

            bool moreItemsSelected = false;
            bool itemDropped = false;

            AlbumIconItem *albumItem = findItem(e->pos());
            if (albumItem)
                itemDropped = true;

            for (IconItem *it = firstItem(); it; it = it->nextItem())
            {
                if (it->isSelected() && it != albumItem)
                {
                    moreItemsSelected = true;
                    break;
                }
            }

            QAction *assignToSelectedAction = 0;
            if (moreItemsSelected)
                assignToSelectedAction =
                        popMenu.addAction(SmallIcon("tag"), i18n("Assign '%1' to &Selected Items",talbum->tagPath().mid(1)));

            QAction *assignToThisAction = 0;
            if (itemDropped)
                assignToThisAction =
                        popMenu.addAction(SmallIcon("tag"), i18n("Assign '%1' to &This Item",talbum->tagPath().mid(1)));

            QAction *assignToAllAction =
                popMenu.addAction(SmallIcon("tag"), i18n("Assign '%1' to &All Items",talbum->tagPath().mid(1)));

            popMenu.addSeparator();
            popMenu.addAction(SmallIcon("dialog-cancel"), i18n("&Cancel"));

            popMenu.setMouseTracking(true);
            QAction *choice = popMenu.exec(QCursor::pos());
            if (choice)
            {
                if (choice == assignToSelectedAction)    // Selected Items
                {
                    emit changeTagOnImageInfos(selectedImageInfos(), QList<int>() << tagID, true, true);
                }
                else if (choice == assignToAllAction)    // All Items
                {
                    emit changeTagOnImageInfos(allImageInfos(), QList<int>() << tagID, true, true);
                }
                else if (choice == assignToThisAction)  // Dropped Item only.
                {
                    AlbumIconItem *albumItem = findItem(e->pos());
                    if (albumItem)
                    {
                        ImageInfoList infos;
                        infos << albumItem->imageInfo();
                        emit changeTagOnImageInfos(infos, QList<int>() << tagID, true, false);
                    }
                }
            }
        }
    }
    else if(DTagListDrag::canDecode(e->mimeData()))
    {
        QList<int> tagIDs;
        DTagListDrag::decode(e->mimeData(), tagIDs);

        KMenu popMenu(this);

        bool moreItemsSelected = false;
        bool itemDropped = false;

        AlbumIconItem *albumItem = findItem(e->pos());
        if (albumItem)
            itemDropped = true;

        for (IconItem *it = firstItem(); it; it = it->nextItem())
        {
            if (it->isSelected() && it != albumItem)
            {
                moreItemsSelected = true;
                break;
            }
        }

        QAction *assignToSelectedAction = 0;
        if (moreItemsSelected)
            assignToSelectedAction = popMenu.addAction(SmallIcon("tag"), i18n("Assign Tags to &Selected Items"));

        QAction *assignToThisAction = 0;
        if (itemDropped)
            assignToThisAction = popMenu.addAction(SmallIcon("tag"), i18n("Assign Tags to &This Item"));

        QAction *assignToAllAction =
            popMenu.addAction(SmallIcon("tag"), i18n("Assign Tags to &All Items"));

        popMenu.addSeparator();
        popMenu.addAction(SmallIcon("dialog-cancel"), i18n("&Cancel"));

        popMenu.setMouseTracking(true);
        QAction *choice = popMenu.exec(QCursor::pos());
        if (choice)
        {
            if (choice == assignToSelectedAction)    // Selected Items
            {
                slotChangeTagOnImageInfos(selectedImageInfos(), tagIDs, true, true);
            }
            else if (choice == assignToAllAction)    // All Items
            {
                slotChangeTagOnImageInfos(allImageInfos(), tagIDs, true, true);
            }
            else if (choice == assignToThisAction)    // Dropped item only.
            {
                AlbumIconItem *albumItem = findItem(e->pos());
                if (albumItem)
                {
                    ImageInfoList infos;
                    infos << albumItem->imageInfo();
                    slotChangeTagOnImageInfos(infos, tagIDs, true, false);
                }
            }
        }
    }
    else if(DCameraItemListDrag::canDecode(e->mimeData()))
    {
        CameraUI *ui = dynamic_cast<CameraUI*>(e->source());
        if (ui)
        {
            KMenu popMenu(this);
            popMenu.addTitle(SmallIcon("digikam"), i18n("My Albums"));
            QAction *downAction    = popMenu.addAction(SmallIcon("file-export"),
                                                       i18n("Download from camera"));
            QAction *downDelAction = popMenu.addAction(SmallIcon("file-export"),
                                                       i18n("Download && Delete from camera"));
            popMenu.addSeparator();
            popMenu.addAction(SmallIcon("dialog-cancel"), i18n("C&ancel"));
            popMenu.setMouseTracking(true);
            QAction *choice = popMenu.exec(QCursor::pos());
            if (choice)
            {
                if (choice == downAction)
                    ui->slotDownload(true, false, album);
                else if (choice == downDelAction)
                    ui->slotDownload(true, true, album);
            }
        }
    }
    else
    {
        e->ignore();
    }
}

void AlbumIconView::slotChangeTagOnImageInfos(const ImageInfoList &list, const QList<int> &tagIDs, bool addOrRemove, bool progress)
{
    float cnt = list.count();
    int i     = 0;

    if (progress)
    {
        if (addOrRemove)
            emit signalProgressBarMode(StatusProgressBar::ProgressBarMode,
                                       i18n("Assigning image tags. Please wait..."));
        else
            emit signalProgressBarMode(StatusProgressBar::ProgressBarMode,
                                       i18n("Removing image tags. Please wait..."));
    }

    d->imageLister->blockSignals(true);
    ScanController::instance()->suspendCollectionScan();
    DatabaseTransaction transaction;
    foreach(const ImageInfo &info, list)
    {
        MetadataHub hub;

        hub.load(info);

        for (QList<int>::const_iterator tagIt = tagIDs.constBegin(); tagIt != tagIDs.constEnd(); ++tagIt)
        {
            hub.setTag(*tagIt, addOrRemove);
        }

        QString filePath = info.filePath();
        hub.write(info, MetadataHub::PartialWrite);
        bool fileChanged = hub.write(filePath, MetadataHub::FullWriteIfChanged);
        if (fileChanged)
            ScanController::instance()->scanFileDirectly(filePath);

        if (progress)
        {
            emit signalProgressValue((int)((i++/cnt)*100.0));
            kapp->processEvents();
        }
    }
    ScanController::instance()->resumeCollectionScan();
    d->imageLister->blockSignals(false);

    if (progress)
        emit signalProgressBarMode(StatusProgressBar::TextMode, QString());

    if (d->currentAlbum && d->currentAlbum->type() == Album::TAG)
    {
        d->imageLister->refresh();
    }
    updateContents();
}

bool AlbumIconView::acceptToolTip(IconItem *item, const QPoint &mousePos)
{
    AlbumIconItem *iconItem = dynamic_cast<AlbumIconItem*>(item);

    if (iconItem && iconItem->clickToOpenRect().contains(mousePos))
    {
        return true;
    }
    else
    {
        return false;
    }
}

void AlbumIconView::slotShowToolTip(IconItem* item)
{
    d->toolTip->setIconItem(dynamic_cast<AlbumIconItem*>(item));
}

KUrl::List AlbumIconView::allItems()
{
    KUrl::List itemList;

     for (IconItem *it = firstItem(); it; it = it->nextItem())
     {
         AlbumIconItem *item = (AlbumIconItem*) it;
         itemList.append(item->imageInfo().fileUrl());
     }

    return itemList;
}

KUrl::List AlbumIconView::selectedItems()
{
    KUrl::List itemList;

     for (IconItem *it = firstItem(); it; it = it->nextItem())
     {
         if (it->isSelected())
         {
             AlbumIconItem *item = (AlbumIconItem*) it;
             itemList.append(item->imageInfo().fileUrl());
         }
     }

    return itemList;
}

ImageInfoList AlbumIconView::allImageInfos(ImageInfo* current) const
{
    if (current)
    {
        // As default copy the first item info as current;
        // will be changed later when a current item is found
        if (firstItem())
            *current = static_cast<AlbumIconItem*>(firstItem())->imageInfo();
        else
            *current = ImageInfo();
    }

    IconItem *currentIconItem = currentItem();
    ImageInfoList list;
    for (IconItem *it = firstItem() ; it ; it = it->nextItem())
    {
        AlbumIconItem *iconItem = static_cast<AlbumIconItem*>(it);
        ImageInfo info          = iconItem->imageInfo();

        list << info;

        // If we found the current item, set current to its ImageInfo
        if (current && iconItem == currentIconItem)
            *current = info;
    }

    return list;
}

ImageInfoList AlbumIconView::selectedImageInfos() const
{
    // Returns the list of ImageInfos of currently selected items,
    // with the extra feature that the currentItem is the first in the list.
    ImageInfoList list;
    for (IconItem *it = firstItem(); it; it = it->nextItem())
    {
        AlbumIconItem *iconItem = static_cast<AlbumIconItem *>(it);
        if (it->isSelected())
        {
            ImageInfo info = iconItem->imageInfo();

            if (iconItem == currentItem())
                list.prepend(info);
            else
                list.append(info);
        }
    }
    return list;
}

void AlbumIconView::refresh()
{
    d->imageLister->stop();
    clear();

    d->imageLister->openAlbum(d->currentAlbum);
}

void AlbumIconView::refreshItems(const KUrl::List& urlList)
{
    if (!d->currentAlbum || urlList.empty())
        return;

    // we do two things here:
    // 1. refresh the imageinfo for the file
    // 2. refresh the thumbnails

    for (KUrl::List::const_iterator it = urlList.constBegin();
         it != urlList.constEnd(); ++it)
    {
        AlbumIconItem* iconItem = findItem((*it).url());
        if (!iconItem)
            continue;

        ThumbnailLoadThread::deleteThumbnail((*it).path());
        // clean LoadingCache as well - be pragmatic, do it here.
        LoadingCacheInterface::fileChanged((*it).path());
    }

    emit signalItemsUpdated(urlList);

    // trigger a delayed rearrangement, in case we need to resort items
    triggerRearrangement();
}

void AlbumIconView::slotFileChanged(const QString &filePath)
{
    if (!d->currentAlbum || filePath.isEmpty())
        return;

    KUrl url = KUrl::fromPath(filePath);

    AlbumIconItem* iconItem = findItem(url.url());
    if (!iconItem)
        return;
    iconItem->update();

    emit signalItemsUpdated(KUrl::List() << url);
}

void AlbumIconView::slotThumbnailLoaded(const LoadingDescription &loadingDescription, const QPixmap&)
{
    AlbumIconItem* iconItem = findItem(KUrl::fromPath(loadingDescription.filePath).url());
    if (!iconItem)
        return;

    iconItem->update();
}

void AlbumIconView::prepareRepaint(const QList<IconItem *> &itemsToRepaint)
{
    QStringList filePaths;
    foreach(IconItem *iconItem, itemsToRepaint)
    {
        AlbumIconItem *item = static_cast<AlbumIconItem *>(iconItem);
        filePaths << item->filePath();
    }
    ThumbnailLoadThread::defaultIconViewThread()->findGroup(filePaths);
}

void AlbumIconView::slotSelectionChanged()
{
    if (firstSelectedItem())
        emitItemsSelected(true);
    else
        emitItemsSelected(false);
}

void AlbumIconView::slotSetExifOrientation( int orientation )
{
    KUrl::List urlList;
    int i = 0;

    for (IconItem *it = firstItem(); it; it=it->nextItem())
    {
        if (it->isSelected())
        {
            AlbumIconItem *iconItem = static_cast<AlbumIconItem *>(it);
            urlList.append(iconItem->imageInfo().fileUrl());
        }
    }

    if (urlList.count() <= 0) return;

    QStringList failedItems;
    KUrl::List::Iterator it;
    float cnt = (float)urlList.count();
    emit signalProgressBarMode(StatusProgressBar::ProgressBarMode,
                                i18n("Revising Exif Orientation tags. Please wait..."));

    for( it = urlList.begin(); it != urlList.end(); ++it )
    {
        kDebug(50003) << "Setting Exif Orientation tag to " << orientation << endl;

        DMetadata metadata((*it).path());
        DMetadata::ImageOrientation o = (DMetadata::ImageOrientation)orientation;
        metadata.setImageOrientation(o);

        if (!metadata.applyChanges())
        {
            failedItems.append((*it).fileName());
        }
        else
        {
            ImageAttributesWatch::instance()->fileMetadataChanged((*it));
        }

        emit signalProgressValue((int)((i++/cnt)*100.0));
        kapp->processEvents();
    }

    emit signalProgressBarMode(StatusProgressBar::TextMode, QString());

    if (!failedItems.isEmpty())
    {
        if (failedItems.count() == 1)
        {
            KMessageBox::error(0, i18n("Failed to revise Exif orientation for file %1.",
                                       failedItems[0]));
        }
        else
        {
            KMessageBox::errorList(0, i18n("Failed to revise Exif orientation these files:"),
                                   failedItems);
        }
    }

    refreshItems(urlList);
}

QRect AlbumIconView::itemRect() const
{
    return d->itemRect;
}

QRect AlbumIconView::itemRatingRect() const
{
    return d->itemRatingRect;
}

QRect AlbumIconView::itemDateRect() const
{
    return d->itemDateRect;
}

QRect AlbumIconView::itemModDateRect() const
{
    return d->itemModDateRect;
}

QRect AlbumIconView::itemPixmapRect() const
{
    return d->itemPixmapRect;
}

QRect AlbumIconView::itemNameRect() const
{
    return d->itemNameRect;
}

QRect AlbumIconView::itemCommentsRect() const
{
    return d->itemCommentsRect;
}

QRect AlbumIconView::itemResolutionRect() const
{
    return d->itemResolutionRect;
}

QRect AlbumIconView::itemTagRect() const
{
    return d->itemTagRect;
}

QRect AlbumIconView::itemSizeRect() const
{
    return d->itemSizeRect;
}

QRect AlbumIconView::bannerRect() const
{
    return d->bannerRect;
}

QPixmap AlbumIconView::itemBaseRegPixmap() const
{
    return d->itemRegPixmap;
}

QPixmap AlbumIconView::itemBaseSelPixmap() const
{
    return d->itemSelPixmap;
}

QPixmap AlbumIconView::bannerPixmap() const
{
    return d->bannerPixmap;
}

QPixmap AlbumIconView::ratingPixmap(int rating, bool selected) const
{
    if (rating < 1 || rating > 5)
    {
        QPixmap pix;
        if (selected)
            pix = d->itemSelPixmap.copy(d->itemRatingRect);
        else
            pix = d->itemRegPixmap.copy(d->itemRatingRect);

        return pix;
    }

    rating--;
    if (selected)
        return d->ratingPixmaps[5 + rating];
    else
        return d->ratingPixmaps[rating];
}

QFont AlbumIconView::itemFontReg() const
{
    return d->fnReg;
}

QFont AlbumIconView::itemFontCom() const
{
    return d->fnCom;
}

QFont AlbumIconView::itemFontXtra() const
{
    return d->fnXtra;
}

void AlbumIconView::updateBannerRectPixmap()
{
    d->bannerRect = QRect(0, 0, 0, 0);

    // Title --------------------------------------------------------

    QFont fn(font());
    int fnSize = fn.pointSize();
    bool usePointSize;
    if (fnSize > 0)
    {
        fn.setPointSize(fnSize+2);
        usePointSize = true;
    }
    else
    {
        fnSize = fn.pixelSize();
        fn.setPixelSize(fnSize+2);
        usePointSize = false;
    }

    fn.setBold(true);
    QFontMetrics fm(fn);
    QRect tr = fm.boundingRect(0, 0, frameRect().width(),
                               0xFFFFFFFF, Qt::AlignLeft | Qt::AlignVCenter,
                               "XXX");
    d->bannerRect.setHeight(tr.height());

    if (usePointSize)
        fn.setPointSize(font().pointSize());
    else
        fn.setPixelSize(font().pixelSize());

    fn.setBold(false);
    fm = QFontMetrics(fn);

    tr = fm.boundingRect(0, 0, frameRect().width(),
                         0xFFFFFFFF, Qt::AlignLeft | Qt::AlignVCenter,
                         "XXX");

    d->bannerRect.setHeight(d->bannerRect.height() + tr.height() + 10);
    d->bannerRect.setWidth(frameRect().width());

    d->bannerPixmap = ThemeEngine::instance()->bannerPixmap(d->bannerRect.width(),
                                                            d->bannerRect.height());
}

void AlbumIconView::updateRectsAndPixmaps()
{
    updateBannerRectPixmap();

    d->itemRect           = QRect(0, 0, 0, 0);
    d->itemRatingRect     = QRect(0, 0, 0, 0);
    d->itemDateRect       = QRect(0, 0, 0, 0);
    d->itemModDateRect    = QRect(0, 0, 0, 0);
    d->itemPixmapRect     = QRect(0, 0, 0, 0);
    d->itemNameRect       = QRect(0, 0, 0, 0);
    d->itemCommentsRect   = QRect(0, 0, 0, 0);
    d->itemResolutionRect = QRect(0, 0, 0, 0);
    d->itemSizeRect       = QRect(0, 0, 0, 0);
    d->itemTagRect        = QRect(0, 0, 0, 0);

    d->fnReg  = font();
    d->fnCom  = font();
    d->fnXtra = font();
    d->fnCom.setItalic(true);

    int fnSz = d->fnReg.pointSize();
    if (fnSz > 0)
    {
        d->fnCom.setPointSize(fnSz-1);
        d->fnXtra.setPointSize(fnSz-2);
    }
    else
    {
        fnSz = d->fnReg.pixelSize();
        d->fnCom.setPixelSize(fnSz-1);
        d->fnXtra.setPixelSize(fnSz-2);
    }

    const int radius = 3;
    const int margin = 5;
    int w            = d->thumbSize.size() + 2*radius;

    QFontMetrics fm(d->fnReg);
    QRect oneRowRegRect = fm.boundingRect(0, 0, w, 0xFFFFFFFF,
                                          Qt::AlignTop | Qt::AlignHCenter,
                                          "XXXXXXXXX");
    fm = QFontMetrics(d->fnCom);
    QRect oneRowComRect = fm.boundingRect(0, 0, w, 0xFFFFFFFF,
                                          Qt::AlignTop | Qt::AlignHCenter,
                                          "XXXXXXXXX");
    fm = QFontMetrics(d->fnXtra);
    QRect oneRowXtraRect = fm.boundingRect(0, 0, w, 0xFFFFFFFF,
                                           Qt::AlignTop | Qt::AlignHCenter,
                                           "XXXXXXXXX");

    QSize starPolygonSize(15, 15);

    int y = margin;

    d->itemPixmapRect = QRect(margin, y, w, d->thumbSize.size() + 2*radius);
    y = d->itemPixmapRect.bottom();

    if (d->albumSettings->getIconShowRating())
    {
        d->itemRatingRect = QRect(margin, y, w, starPolygonSize.height());
        y = d->itemRatingRect.bottom();
    }

    if (d->albumSettings->getIconShowName())
    {
        d->itemNameRect = QRect(margin, y, w-margin, oneRowRegRect.height());
        y = d->itemNameRect.bottom();
    }

    if (d->albumSettings->getIconShowComments())
    {
        d->itemCommentsRect = QRect(margin, y, w, oneRowComRect.height());
        y = d->itemCommentsRect.bottom();
    }

    if (d->albumSettings->getIconShowDate())
    {
        d->itemDateRect = QRect(margin, y, w, oneRowXtraRect.height());
        y = d->itemDateRect.bottom();
    }

    if (d->albumSettings->getIconShowModDate())
    {
        d->itemModDateRect = QRect(margin, y, w, oneRowXtraRect.height());
        y = d->itemModDateRect.bottom();
    }

    if (d->albumSettings->getIconShowResolution())
    {
        d->itemResolutionRect = QRect(margin, y, w, oneRowXtraRect.height());
        y = d->itemResolutionRect.bottom() ;
    }

    if (d->albumSettings->getIconShowSize())
    {
        d->itemSizeRect = QRect(margin, y, w, oneRowXtraRect.height());
        y = d->itemSizeRect.bottom();
    }

    if (d->albumSettings->getIconShowTags())
    {
        d->itemTagRect = QRect(margin, y, w, oneRowComRect.height());
        y = d->itemTagRect.bottom();
    }

    d->itemRect      = QRect(0, 0, w + 2*margin, y+margin+radius);

    d->itemRegPixmap = ThemeEngine::instance()->thumbRegPixmap(d->itemRect.width(),
                                                               d->itemRect.height());

    d->itemSelPixmap = ThemeEngine::instance()->thumbSelPixmap(d->itemRect.width(),
                                                               d->itemRect.height());

    // -- Generate rating pixmaps ------------------------------------------

    // We use antialiasing and want to pre-render the pixmaps.
    // So we need the background at the time of painting,
    // and the background may be a gradient, and will be different for selected items.
    // This makes 5*2 (small) pixmaps.
    if (d->albumSettings->getIconShowRating())
    {
        for (int sel=0; sel<2; sel++)
        {
            QPixmap basePix;

            // do this once for regular, once for selected backgrounds
            if (sel)
                basePix = d->itemSelPixmap.copy(d->itemRatingRect);
            else
                basePix = d->itemRegPixmap.copy(d->itemRatingRect);

            for (int rating=1; rating<=5; rating++)
            {
                // we store first the 5 regular, then the 5 selected pixmaps, for simplicity
                int index = (sel * 5 + rating) - 1;

                // copy background
                d->ratingPixmaps[index] = basePix;
                // open a painter
                QPainter painter(&d->ratingPixmaps[index]);

                // use antialiasing
                painter.setRenderHint(QPainter::Antialiasing, true);
                painter.setBrush(ThemeEngine::instance()->textSpecialRegColor());
                QPen pen(ThemeEngine::instance()->textRegColor());
                // set a pen which joins the lines at a filled angle
                pen.setJoinStyle(Qt::MiterJoin);
                painter.setPen(pen);

                // move painter while drawing polygons
                painter.translate( lround((d->itemRatingRect.width() - margin - rating*(starPolygonSize.width()+1))/2.0) + 2, 1 );
                for (int s=0; s<rating; s++)
                {
                    painter.drawPolygon(d->starPolygon, Qt::WindingFill);
                    painter.translate(starPolygonSize.width() + 1, 0);
                }
            }
        }
    }

    clearThumbnailBorderCache();
}

void AlbumIconView::slotThemeChanged()
{
    updateRectsAndPixmaps();
    viewport()->update();
}

AlbumIconItem* AlbumIconView::findItem(const QPoint& pos)
{
    return dynamic_cast<AlbumIconItem*>(IconView::findItem(pos));
}

AlbumIconItem* AlbumIconView::findItem(const QString& url) const
{
    return d->itemDict.value(url);
}

AlbumIconItem* AlbumIconView::nextItemToThumbnail() const
{
    QRect r(contentsX(), contentsY(), visibleWidth(), visibleHeight());
    IconItem *fItem = findFirstVisibleItem(r);
    IconItem *lItem = findLastVisibleItem(r);
    if (!fItem || !lItem)
        return 0;

    AlbumIconItem* firstItem = static_cast<AlbumIconItem*>(fItem);
    AlbumIconItem* lastItem  = static_cast<AlbumIconItem*>(lItem);
    AlbumIconItem* item      = firstItem;
    while (item)
    {
        if (item->isDirty())
            return item;
        if (item == lastItem)
            break;
        item = (AlbumIconItem*)item->nextItem();
    }

    return 0;
}

void AlbumIconView::slotAlbumModified()
{
    d->imageLister->stop();
    clear();

    d->imageLister->openAlbum(d->currentAlbum);

    updateRectsAndPixmaps();
}

void AlbumIconView::slotGotoTag(int tagID)
{
    // send a signal to the parent widget (digikamview.cpp) to change
    // to Tag view and the corresponding item

    emit signalGotoTagAndItem(tagID);
}

void AlbumIconView::slotAssignTag(int tagID)
{
    slotChangeTagOnImageInfos(selectedImageInfos(), QList<int>() << tagID, true, true);
}

void AlbumIconView::slotRemoveTag(int tagID)
{
    slotChangeTagOnImageInfos(selectedImageInfos(), QList<int>() << tagID, false, true);
}

void AlbumIconView::slotAssignRating(int rating)
{
    emit signalProgressBarMode(StatusProgressBar::ProgressBarMode,
                                i18n("Assigning image ratings. Please wait..."));

    int   i   = 0;
    float cnt = (float)countSelected();
    rating    = qMin(RatingMax, qMax(RatingMin, rating));
    MetadataHub hub;

    QList<ImageInfo> infos;
    for (IconItem *it = firstItem() ; it ; it = it->nextItem())
    {
        if (it->isSelected())
        {
            AlbumIconItem *albumItem = dynamic_cast<AlbumIconItem *>(it);
            if (albumItem)
                infos << albumItem->imageInfo();
        }
    }

    d->imageLister->blockSignals(true);
    ScanController::instance()->suspendCollectionScan();
    DatabaseTransaction transaction;
    foreach (const ImageInfo &info, infos)
    {
        hub.load(info);

        hub.setRating(rating);

        QString filePath = info.filePath();
        hub.write(info, MetadataHub::PartialWrite);
        hub.write(info.filePath(), MetadataHub::FullWriteIfChanged);
        bool fileChanged = hub.write(filePath, MetadataHub::FullWriteIfChanged);

        if (fileChanged)
            ScanController::instance()->scanFileDirectly(filePath);

        emit signalProgressValue((int)((i++/cnt)*100.0));
        kapp->processEvents();
    }
    ScanController::instance()->resumeCollectionScan();
    d->imageLister->blockSignals(false);

    emit signalProgressBarMode(StatusProgressBar::TextMode, QString());
    updateContents();
}

void AlbumIconView::slotAssignRatingNoStar()
{
    slotAssignRating(0);
}

void AlbumIconView::slotAssignRatingOneStar()
{
    slotAssignRating(1);
}

void AlbumIconView::slotAssignRatingTwoStar()
{
    slotAssignRating(2);
}

void AlbumIconView::slotAssignRatingThreeStar()
{
    slotAssignRating(3);
}

void AlbumIconView::slotAssignRatingFourStar()
{
    slotAssignRating(4);
}

void AlbumIconView::slotAssignRatingFiveStar()
{
    slotAssignRating(5);
}

void AlbumIconView::slotDIOResult(KJob* kjob)
{
    KIO::Job *job = static_cast<KIO::Job*>(kjob);
    if (job->error())
    {
        job->ui()->setWindow(this);
        job->ui()->showErrorMessage();
    }
}

void AlbumIconView::slotImageAttributesChanged(qlonglong imageId)
{
    AlbumIconItem *firstItem = static_cast<AlbumIconItem *>(findFirstVisibleItem());
    AlbumIconItem *lastItem  = static_cast<AlbumIconItem *>(findLastVisibleItem());
    for (AlbumIconItem *item = firstItem; item;
         item = static_cast<AlbumIconItem *>(item->nextItem()))
    {
        if (item->imageInfo().id() == imageId)
        {
            updateContents();
            return;
        }
        if (item == lastItem)
            break;
    }
}

void AlbumIconView::slotAlbumImagesChanged(int /*albumId*/)
{
    updateContents();
}

void AlbumIconView::slotEditRatingFromItem(int rating)
{
    rating = qMin(RatingMax, qMax(RatingMin, rating));

    d->imageLister->blockSignals(true);
    ScanController::instance()->suspendCollectionScan();
    DatabaseTransaction transaction;
    AlbumIconItem *albumItem = dynamic_cast<AlbumIconItem*>(ratingItem());
    if (albumItem)
    {
        ImageInfo info = albumItem->imageInfo();

        MetadataHub hub;
        hub.load(info);
        hub.setRating(rating);

        QString filePath = info.filePath();
        hub.write(info, MetadataHub::PartialWrite);
        hub.write(info.filePath(), MetadataHub::FullWriteIfChanged);
        bool fileChanged = hub.write(filePath, MetadataHub::FullWriteIfChanged);

        if (fileChanged)
            ScanController::instance()->scanFileDirectly(filePath);
    }

    ScanController::instance()->resumeCollectionScan();
    d->imageLister->blockSignals(false);
    updateContents();
}

}  // namespace Digikam
