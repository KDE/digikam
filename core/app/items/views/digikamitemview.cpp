/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-04-24
 * Description : Qt model-view for items
 *
 * Copyright (C) 2009-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2009-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2011      by Andi Clemens <andi dot clemens at gmail dot com>
 * Copyright (C) 2013      by Michael G. Hansen <mike at mghansen dot de>
 * Copyright (C) 2014      by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 * Copyright (C) 2017      by Simon Frei <freisim93 at gmail dot com>
 *
 * This program is free software you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "digikamitemview.h"
#include "digikamitemview_p.h"

// Qt includes

#include <QApplication>
#include <QPointer>
#include <QMenu>
#include <QIcon>
#include <QAction>
#include <QUrl>

// Local includes

#include "digikam_debug.h"
#include "albummanager.h"
#include "coredb.h"
#include "coredboperationgroup.h"
#include "advancedrenamedialog.h"
#include "advancedrenameprocessdialog.h"
#include "applicationsettings.h"
#include "assignnameoverlay.h"
#include "contextmenuhelper.h"
#include "coredbaccess.h"
#include "ddragobjects.h"
#include "digikamapp.h"
#include "digikamitemdelegate.h"
#include "itemfacedelegate.h"
#include "dio.h"
#include "groupindicatoroverlay.h"
#include "itemalbumfiltermodel.h"
#include "itemalbummodel.h"
#include "itemdragdrop.h"
#include "itemratingoverlay.h"
#include "itemfullscreenoverlay.h"
#include "itemcoordinatesoverlay.h"
#include "tagslineeditoverlay.h"
#include "itemviewutilities.h"
#include "imagewindow.h"
#include "fileactionmngr.h"
#include "fileactionprogress.h"
#include "thumbnailloadthread.h"
#include "tagregion.h"
#include "addtagslineedit.h"
#include "facerejectionoverlay.h"
#include "facetagsiface.h"

namespace Digikam
{

DigikamItemView::DigikamItemView(QWidget* const parent)
    : ItemCategorizedView(parent),
      d(new Private(this))
{
    installDefaultModels();

    d->editPipeline.plugDatabaseEditor();
    d->editPipeline.plugTrainer();
    d->editPipeline.construct();

    connect(&d->editPipeline, SIGNAL(scheduled()),
            this, SLOT(slotInitProgressIndicator()));

    d->normalDelegate = new DigikamItemDelegate(this);
    d->faceDelegate   = new ItemFaceDelegate(this);

    setItemDelegate(d->normalDelegate);
    setSpacing(10);

    ApplicationSettings* const settings = ApplicationSettings::instance();

    imageFilterModel()->setCategorizationMode(ItemSortSettings::CategoryByAlbum);

    imageAlbumModel()->setThumbnailLoadThread(ThumbnailLoadThread::defaultIconViewThread());
    setThumbnailSize(ThumbnailSize(settings->getDefaultIconSize()));
    imageAlbumModel()->setPreloadThumbnails(true);

    imageModel()->setDragDropHandler(new ItemDragDropHandler(imageModel()));
    setDragEnabled(true);
    setAcceptDrops(true);
    setDropIndicatorShown(false);

    setToolTipEnabled(settings->showToolTipsIsValid());
    imageFilterModel()->setStringTypeNatural(settings->isStringTypeNatural());
    imageFilterModel()->setSortRole((ItemSortSettings::SortRole)settings->getImageSortOrder());
    imageFilterModel()->setSortOrder((ItemSortSettings::SortOrder)settings->getImageSorting());
    imageFilterModel()->setCategorizationMode((ItemSortSettings::CategorizationMode)settings->getImageSeparationMode());
    imageFilterModel()->setCategorizationSortOrder((ItemSortSettings::SortOrder) settings->getImageSeparationSortOrder());

    // selection overlay
    addSelectionOverlay(d->normalDelegate);
    addSelectionOverlay(d->faceDelegate);

    // rotation overlays
    d->rotateLeftOverlay  = ItemRotateOverlay::left(this);
    d->rotateRightOverlay = ItemRotateOverlay::right(this);
    d->fullscreenOverlay  = ItemFullScreenOverlay::instance(this);
    d->updateOverlays();

    // rating overlay
    ItemRatingOverlay* const ratingOverlay = new ItemRatingOverlay(this);
    addOverlay(ratingOverlay);

    // face overlays
    // NOTE: order to plug this overlay is important, else rejection cant be suitable (see bug #324759).
    addAssignNameOverlay(d->faceDelegate);
    addRejectionOverlay(d->faceDelegate);

    GroupIndicatorOverlay* const groupOverlay = new GroupIndicatorOverlay(this);
    addOverlay(groupOverlay);

    addOverlay(new ItemCoordinatesOverlay(this));

    connect(ratingOverlay, SIGNAL(ratingEdited(QList<QModelIndex>,int)),
            this, SLOT(assignRating(QList<QModelIndex>,int)));

    connect(groupOverlay, SIGNAL(toggleGroupOpen(QModelIndex)),
            this, SLOT(groupIndicatorClicked(QModelIndex)));

    connect(groupOverlay, SIGNAL(showButtonContextMenu(QModelIndex,QContextMenuEvent*)),
            this, SLOT(showGroupContextMenu(QModelIndex,QContextMenuEvent*)));

    d->utilities = new ItemViewUtilities(this);

    connect(imageModel()->dragDropHandler(), SIGNAL(assignTags(QList<ItemInfo>,QList<int>)),
            FileActionMngr::instance(), SLOT(assignTags(QList<ItemInfo>,QList<int>)));

    connect(imageModel()->dragDropHandler(), SIGNAL(addToGroup(ItemInfo,QList<ItemInfo>)),
            FileActionMngr::instance(), SLOT(addToGroup(ItemInfo,QList<ItemInfo>)));

    connect(imageModel()->dragDropHandler(), SIGNAL(dragDropSort(ItemInfo,QList<ItemInfo>)),
            this, SLOT(dragDropSort(ItemInfo,QList<ItemInfo>)));

    connect(d->utilities, SIGNAL(editorCurrentUrlChanged(QUrl)),
            this, SLOT(setCurrentUrlWhenAvailable(QUrl)));

    connect(settings, SIGNAL(setupChanged()),
            this, SLOT(slotSetupChanged()));

    slotSetupChanged();
}

DigikamItemView::~DigikamItemView()
{
    delete d;
}

ItemViewUtilities* DigikamItemView::utilities() const
{
    return d->utilities;
}

void DigikamItemView::setThumbnailSize(const ThumbnailSize& size)
{
    imageThumbnailModel()->setPreloadThumbnailSize(size);
    ItemCategorizedView::setThumbnailSize(size);
}

ItemInfoList DigikamItemView::allItemInfos(bool grouping) const
{
    if (grouping)
    {
        return resolveGrouping(ItemCategorizedView::allItemInfos());
    }

    return ItemCategorizedView::allItemInfos();
}

ItemInfoList DigikamItemView::selectedItemInfos(bool grouping) const
{
    if (grouping)
    {
        return resolveGrouping(ItemCategorizedView::selectedItemInfos());
    }

    return ItemCategorizedView::selectedItemInfos();
}

ItemInfoList DigikamItemView::selectedItemInfosCurrentFirst(bool grouping) const
{
    if (grouping)
    {
        return resolveGrouping(ItemCategorizedView::selectedItemInfosCurrentFirst());
    }

    return ItemCategorizedView::selectedItemInfosCurrentFirst();
}

void DigikamItemView::dragDropSort(const ItemInfo& pick, const QList<ItemInfo>& infos)
{
    if (pick.isNull() || infos.isEmpty())
    {
        return;
    }

    ItemInfoList infoList = allItemInfos(false);
    qlonglong counter      = pick.manualOrder();
    bool order             = (ApplicationSettings::instance()->
                                getImageSorting() == Qt::AscendingOrder);
    bool found             = false;

    QApplication::setOverrideCursor(Qt::WaitCursor);

    CoreDbOperationGroup group;
    group.setMaximumTime(200);

    foreach (ItemInfo info, infoList)
    {
        if (!found && info.id() == pick.id())
        {
            foreach (ItemInfo dropInfo, infos)
            {
                dropInfo.setManualOrder(counter);
                counter += (order ? 1 : -1);
            }

            info.setManualOrder(counter);
            found = true;
        }
        else if (found && !infos.contains(info))
        {
            if (( order && info.manualOrder() > counter) ||
                (!order && info.manualOrder() < counter))
            {
                break;
            }

            counter += (order ? 100 : -100);
            info.setManualOrder(counter);
        }

        group.allowLift();
    }

    QApplication::restoreOverrideCursor();

    imageFilterModel()->invalidate();
 }

bool DigikamItemView::allNeedGroupResolving(const ApplicationSettings::OperationType type) const
{
    return needGroupResolving(type, allItemInfos());
}

bool DigikamItemView::selectedNeedGroupResolving(const ApplicationSettings::OperationType type) const
{
    return needGroupResolving(type, selectedItemInfos());
}

int DigikamItemView::fitToWidthIcons()
{
    return delegate()->calculatethumbSizeToFit(viewport()->size().width());
}

void DigikamItemView::slotSetupChanged()
{
    imageFilterModel()->setStringTypeNatural(ApplicationSettings::instance()->isStringTypeNatural());
    setToolTipEnabled(ApplicationSettings::instance()->showToolTipsIsValid());
    setFont(ApplicationSettings::instance()->getIconViewFont());

    d->updateOverlays();

    ItemCategorizedView::slotSetupChanged();
}

bool DigikamItemView::hasHiddenGroupedImages(const ItemInfo& info) const
{
    return info.hasGroupedImages() && !imageFilterModel()->isGroupOpen(info.id());
}

ItemInfoList DigikamItemView::imageInfos(const QList<QModelIndex>& indexes,
                                           ApplicationSettings::OperationType type) const
{
    ItemInfoList infos = ItemCategorizedView::imageInfos(indexes);

    if (needGroupResolving(type, infos))
    {
        return resolveGrouping(infos);
    }

    return infos;
}

void DigikamItemView::setFaceMode(bool on)
{
    d->faceMode = on;

    if (on)
    {
        // See ItemLister, which creates a search the implements listing tag in the ioslave
        imageAlbumModel()->setSpecialTagListing(QLatin1String("faces"));
        setItemDelegate(d->faceDelegate);
        // grouping is not very much compatible with faces
        imageFilterModel()->setAllGroupsOpen(true);
    }
    else
    {
        imageAlbumModel()->setSpecialTagListing(QString());
        setItemDelegate(d->normalDelegate);
        imageFilterModel()->setAllGroupsOpen(false);
    }
}

void DigikamItemView::addRejectionOverlay(ItemDelegate* delegate)
{
    FaceRejectionOverlay* const rejectionOverlay = new FaceRejectionOverlay(this);

    connect(rejectionOverlay, SIGNAL(rejectFaces(QList<QModelIndex>)),
            this, SLOT(removeFaces(QList<QModelIndex>)));

    addOverlay(rejectionOverlay, delegate);
}

/*
void DigikamItemView::addTagEditOverlay(ItemDelegate* delegate)
{
    TagsLineEditOverlay* tagOverlay = new TagsLineEditOverlay(this);

    connect(tagOverlay, SIGNAL(tagEdited(QModelIndex,QString)),
            this, SLOT(assignTag(QModelIndex,QString)));

    addOverlay(tagOverlay, delegate);
}
*/

void DigikamItemView::addAssignNameOverlay(ItemDelegate* delegate)
{
    AssignNameOverlay* const nameOverlay = new AssignNameOverlay(this);
    addOverlay(nameOverlay, delegate);

    connect(nameOverlay, SIGNAL(confirmFaces(QList<QModelIndex>,int)),
            this, SLOT(confirmFaces(QList<QModelIndex>,int)));

    connect(nameOverlay, SIGNAL(removeFaces(QList<QModelIndex>)),
            this, SLOT(removeFaces(QList<QModelIndex>)));
}

void DigikamItemView::confirmFaces(const QList<QModelIndex>& indexes, int tagId)
{
    QList<ItemInfo>    infos;
    QList<FaceTagsIface> faces;
    QList<QModelIndex>  sourceIndexes;

    // fast-remove in the "unknown person" view

    bool needFastRemove = false;

    if (imageAlbumModel()->currentAlbums().size() == 1)
    {
        needFastRemove = d->faceMode && (tagId != imageAlbumModel()->currentAlbums().first()->id());
    }

    foreach (const QModelIndex& index, indexes)
    {
        infos << ItemModel::retrieveItemInfo(index);
        faces << d->faceDelegate->face(index);

        if (needFastRemove)
        {
            sourceIndexes << imageSortFilterModel()->mapToSourceItemModel(index);
        }
    }

    imageAlbumModel()->removeIndexes(sourceIndexes);

    for (int i = 0 ; i < infos.size() ; ++i)
    {
        d->editPipeline.confirm(infos[i], faces[i], tagId);
    }
}

void DigikamItemView::removeFaces(const QList<QModelIndex>& indexes)
{
    QList<ItemInfo> infos;
    QList<FaceTagsIface> faces;
    QList<QModelIndex> sourceIndexes;

    foreach (const QModelIndex& index, indexes)
    {
        infos << ItemModel::retrieveItemInfo(index);
        faces << d->faceDelegate->face(index);
        sourceIndexes << imageSortFilterModel()->mapToSourceItemModel(index);
    }

    imageAlbumModel()->removeIndexes(sourceIndexes);

    for (int i = 0 ; i < infos.size() ; ++i)
    {
        d->editPipeline.remove(infos[i], faces[i]);
    }
}

void DigikamItemView::activated(const ItemInfo& info, Qt::KeyboardModifiers modifiers)
{
    if (info.isNull())
    {
        return;
    }

    if (modifiers != Qt::MetaModifier)
    {
        if (ApplicationSettings::instance()->getItemLeftClickAction() == ApplicationSettings::ShowPreview)
        {
            emit previewRequested(info);
        }
        else
        {
            openFile(info);
        }
    }
    else
    {
        d->utilities->openInfosWithDefaultApplication(QList<ItemInfo>() << info);
    }
}

void DigikamItemView::showContextMenuOnInfo(QContextMenuEvent* event, const ItemInfo& info)
{
    emit signalShowContextMenuOnInfo(event, info, QList<QAction*>(), imageFilterModel());
}

void DigikamItemView::showGroupContextMenu(const QModelIndex& index, QContextMenuEvent* event)
{
    Q_UNUSED(index);
    emit signalShowGroupContextMenu(event, selectedItemInfosCurrentFirst(), imageFilterModel());
}

void DigikamItemView::showContextMenu(QContextMenuEvent* event)
{
    emit signalShowContextMenu(event);
}

void DigikamItemView::openFile(const ItemInfo& info)
{
    d->utilities->openInfos(info, allItemInfos(), currentAlbum());
}

void DigikamItemView::deleteSelected(const ItemViewUtilities::DeleteMode deleteMode)
{
    ItemInfoList imageInfoList = selectedItemInfos(true);

    if (d->utilities->deleteImages(imageInfoList, deleteMode))
    {
        awayFromSelection();
    }
}

void DigikamItemView::deleteSelectedDirectly(const ItemViewUtilities::DeleteMode deleteMode)
{
    ItemInfoList imageInfoList = selectedItemInfos(true);

    d->utilities->deleteImagesDirectly(imageInfoList, deleteMode);
    awayFromSelection();
}

void DigikamItemView::assignRating(const QList<QModelIndex>& indexes, int rating)
{
    ItemInfoList infos = imageInfos(indexes, ApplicationSettings::Metadata);
    FileActionMngr::instance()->assignRating(infos, rating);
}

void DigikamItemView::groupIndicatorClicked(const QModelIndex& index)
{
    ItemInfo info = imageFilterModel()->imageInfo(index);

    if (info.isNull())
    {
        return;
    }

    setCurrentIndex(index);
    imageFilterModel()->toggleGroupOpen(info.id());
    imageAlbumModel()->ensureHasGroupedImages(info);
}

void DigikamItemView::rename()
{
    ItemInfoList infos = selectedItemInfos();

    if (needGroupResolving(ApplicationSettings::Rename, infos))
    {
        infos = resolveGrouping(infos);
    }

    QList<QUrl> urls = infos.toImageUrlList();
    bool loop        = false;
    NewNamesList newNamesList;

    do
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Selected URLs to rename: " << urls;

        QPointer<AdvancedRenameDialog> dlg = new AdvancedRenameDialog(this);
        dlg->slotAddImages(urls);

        if (dlg->exec() != QDialog::Accepted)
        {
            delete dlg;
            break;
        }

        if (!loop)
        {
            QUrl nextUrl = nextInOrder(infos.last(), 1).fileUrl();
            setCurrentUrl(nextUrl);
            loop = true;
        }

        newNamesList = dlg->newNames();
        delete dlg;

        setFocus();
        qApp->processEvents();

        if (!newNamesList.isEmpty())
        {
            QPointer<AdvancedRenameProcessDialog> dlg = new AdvancedRenameProcessDialog(newNamesList, this);
            dlg->exec();

            imageFilterModel()->invalidate();
            urls = dlg->failedUrls();
            delete dlg;
        }
    }
    while (!urls.isEmpty() && !newNamesList.isEmpty());
}

void DigikamItemView::slotRotateLeft(const QList<QModelIndex>& indexes)
{
    ItemInfoList infos = imageInfos(indexes, ApplicationSettings::Metadata);
    FileActionMngr::instance()->transform(infos, MetaEngineRotation::Rotate270);
}

void DigikamItemView::slotRotateRight(const QList<QModelIndex>& indexes)
{
    ItemInfoList infos = imageInfos(indexes, ApplicationSettings::Metadata);
    FileActionMngr::instance()->transform(infos, MetaEngineRotation::Rotate90);
}

void DigikamItemView::slotFullscreen(const QList<QModelIndex>& indexes)
{
   QList<ItemInfo> infos = imageInfos(indexes, ApplicationSettings::Slideshow);

   if (infos.isEmpty())
   {
        return;
   }

   // Just fullscreen the first.
   const ItemInfo& info = infos.at(0);

   emit fullscreenRequested(info);
}

void DigikamItemView::slotInitProgressIndicator()
{
    if (!ProgressManager::instance()->findItembyId(QLatin1String("FaceActionProgress")))
    {
        FileActionProgress* const item = new FileActionProgress(QLatin1String("FaceActionProgress"));

        connect(&d->editPipeline, SIGNAL(started(QString)),
                item, SLOT(slotProgressStatus(QString)));

        connect(&d->editPipeline, SIGNAL(progressValueChanged(float)),
                item, SLOT(slotProgressValue(float)));

        connect(&d->editPipeline, SIGNAL(finished()),
                item, SLOT(slotCompleted()));
    }
}

} // namespace Digikam
