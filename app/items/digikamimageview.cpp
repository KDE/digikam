/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-04-24
 * Description : Qt item view for images
 *
 * Copyright (C) 2009-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2009-2016 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2011      by Andi Clemens <andi dot clemens at gmail dot com>
 * Copyright (C) 2013      by Michael G. Hansen <mike at mghansen dot de>
 * Copyright (C) 2014      by Mohamed Anwer <m dot anwer at gmx dot com>
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

#include "digikamimageview_p.h"
#include "digikamimageview.h"

// Qt includes

#include <QPointer>
#include <QMenu>
#include <QIcon>
#include <QAction>
#include <QUrl>

// Local includes

#include "digikam_debug.h"
#include "albummanager.h"
#include "coredb.h"
#include "advancedrenamedialog.h"
#include "advancedrenameprocessdialog.h"
#include "applicationsettings.h"
#include "assignnameoverlay.h"
#include "contextmenuhelper.h"
#include "coredbaccess.h"
#include "ddragobjects.h"
#include "digikamapp.h"
#include "digikamimagedelegate.h"
#include "digikamimagefacedelegate.h"
#include "dio.h"
#include "groupindicatoroverlay.h"
#include "imagealbumfiltermodel.h"
#include "imagealbummodel.h"
#include "imagedragdrop.h"
#include "imageratingoverlay.h"
#include "imagefsoverlay.h"
#include "imagecoordinatesoverlay.h"
#include "tagslineeditoverlay.h"
#include "imageviewutilities.h"
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

DigikamImageView::DigikamImageView(QWidget* const parent)
    : ImageCategorizedView(parent),
      d(new Private(this))
{
    installDefaultModels();

    d->editPipeline.plugDatabaseEditor();
    d->editPipeline.plugTrainer();
    d->editPipeline.construct();

    connect(&d->editPipeline, SIGNAL(scheduled()),
            this, SLOT(slotInitProgressIndicator()));

    d->normalDelegate = new DigikamImageDelegate(this);
    d->faceDelegate   = new DigikamImageFaceDelegate(this);

    setItemDelegate(d->normalDelegate);
    setSpacing(10);

    ApplicationSettings* const settings = ApplicationSettings::instance();

    imageFilterModel()->setCategorizationMode(ImageSortSettings::CategoryByAlbum);

    imageAlbumModel()->setThumbnailLoadThread(ThumbnailLoadThread::defaultIconViewThread());
    setThumbnailSize((ThumbnailSize::Size)settings->getDefaultIconSize());
    imageAlbumModel()->setPreloadThumbnails(true);

    imageModel()->setDragDropHandler(new ImageDragDropHandler(imageModel()));
    setDragEnabled(true);
    setAcceptDrops(true);
    setDropIndicatorShown(false);

    setToolTipEnabled(settings->showToolTipsIsValid());
    imageFilterModel()->setStringTypeNatural(settings->isStringTypeNatural());
    imageFilterModel()->setSortRole((ImageSortSettings::SortRole)settings->getImageSortOrder());
    imageFilterModel()->setSortOrder((ImageSortSettings::SortOrder)settings->getImageSorting());
    imageFilterModel()->setCategorizationMode((ImageSortSettings::CategorizationMode)settings->getImageGroupMode());
    imageFilterModel()->setCategorizationSortOrder((ImageSortSettings::SortOrder) settings->getImageGroupSortOrder());

    // selection overlay
    addSelectionOverlay(d->normalDelegate);
    addSelectionOverlay(d->faceDelegate);

    // rotation overlays
    d->rotateLeftOverlay  = ImageRotateOverlay::left(this);
    d->rotateRightOverlay = ImageRotateOverlay::right(this);
    d->fullscreenOverlay  = ImageFsOverlay::instance(this);
    d->updateOverlays();

    // rating overlay
    ImageRatingOverlay* const ratingOverlay = new ImageRatingOverlay(this);
    addOverlay(ratingOverlay);

    // face overlays
    // NOTE: order to plug this overlay is important, else rejection cant be suitable (see bug #324759).
    addAssignNameOverlay(d->faceDelegate);
    addRejectionOverlay(d->faceDelegate);

    GroupIndicatorOverlay* const groupOverlay = new GroupIndicatorOverlay(this);
    addOverlay(groupOverlay);

    addOverlay(new ImageCoordinatesOverlay(this));

    connect(ratingOverlay, SIGNAL(ratingEdited(QList<QModelIndex>,int)),
            this, SLOT(assignRating(QList<QModelIndex>,int)));

    connect(groupOverlay, SIGNAL(toggleGroupOpen(QModelIndex)),
            this, SLOT(groupIndicatorClicked(QModelIndex)));

    connect(groupOverlay, SIGNAL(showButtonContextMenu(QModelIndex,QContextMenuEvent*)),
            this, SLOT(showGroupContextMenu(QModelIndex,QContextMenuEvent*)));

    d->utilities = new ImageViewUtilities(this);

    connect(d->utilities, SIGNAL(editorCurrentUrlChanged(QUrl)),
            this, SLOT(setCurrentUrlWhenAvailable(QUrl)));

    connect(imageModel()->dragDropHandler(), SIGNAL(assignTags(QList<ImageInfo>,QList<int>)),
            FileActionMngr::instance(), SLOT(assignTags(QList<ImageInfo>,QList<int>)));

    connect(imageModel()->dragDropHandler(), SIGNAL(addToGroup(ImageInfo,QList<ImageInfo>)),
            FileActionMngr::instance(), SLOT(addToGroup(ImageInfo,QList<ImageInfo>)));

    connect(settings, SIGNAL(setupChanged()),
            this, SLOT(slotSetupChanged()));

    slotSetupChanged();
}

DigikamImageView::~DigikamImageView()
{
    delete d;
}

ImageViewUtilities* DigikamImageView::utilities() const
{
    return d->utilities;
}

void DigikamImageView::setThumbnailSize(const ThumbnailSize& size)
{
    imageThumbnailModel()->setPreloadThumbnailSize(size);
    ImageCategorizedView::setThumbnailSize(size);
}

int DigikamImageView::fitToWidthIcons()
{
    return delegate()->calculatethumbSizeToFit(viewport()->size().width());
}

void DigikamImageView::slotSetupChanged()
{
    imageFilterModel()->setStringTypeNatural(ApplicationSettings::instance()->isStringTypeNatural());
    setToolTipEnabled(ApplicationSettings::instance()->showToolTipsIsValid());
    setFont(ApplicationSettings::instance()->getIconViewFont());

    d->updateOverlays();

    ImageCategorizedView::slotSetupChanged();
}

void DigikamImageView::setFaceMode(bool on)
{
    d->faceMode = on;

    if (on)
    {
        // See ImageLister, which creates a search the implements listing tag in the ioslave
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

void DigikamImageView::addRejectionOverlay(ImageDelegate* delegate)
{
    FaceRejectionOverlay* const rejectionOverlay = new FaceRejectionOverlay(this);

    connect(rejectionOverlay, SIGNAL(rejectFaces(QList<QModelIndex>)),
            this, SLOT(removeFaces(QList<QModelIndex>)));

    addOverlay(rejectionOverlay, delegate);
}

/*
void DigikamImageView::addTagEditOverlay(ImageDelegate* delegate)
{
    TagsLineEditOverlay* tagOverlay = new TagsLineEditOverlay(this);

    connect(tagOverlay, SIGNAL(tagEdited(QModelIndex,QString)),
            this, SLOT(assignTag(QModelIndex,QString)));

    addOverlay(tagOverlay, delegate);
}
*/

void DigikamImageView::addAssignNameOverlay(ImageDelegate* delegate)
{
    AssignNameOverlay* const nameOverlay = new AssignNameOverlay(this);
    addOverlay(nameOverlay, delegate);

    connect(nameOverlay, SIGNAL(confirmFaces(QList<QModelIndex>,int)),
            this, SLOT(confirmFaces(QList<QModelIndex>,int)));

    connect(nameOverlay, SIGNAL(removeFaces(QList<QModelIndex>)),
            this, SLOT(removeFaces(QList<QModelIndex>)));
}

void DigikamImageView::confirmFaces(const QList<QModelIndex>& indexes, int tagId)
{
    QList<ImageInfo>    infos;
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
        infos << ImageModel::retrieveImageInfo(index);
        faces << d->faceDelegate->face(index);

        if (needFastRemove)
        {
            sourceIndexes << imageSortFilterModel()->mapToSourceImageModel(index);
        }
    }

    imageAlbumModel()->removeIndexes(sourceIndexes);

    for (int i=0; i<infos.size(); i++)
    {
        d->editPipeline.confirm(infos[i], faces[i], tagId);
    }
}

void DigikamImageView::removeFaces(const QList<QModelIndex>& indexes)
{
    QList<ImageInfo> infos;
    QList<FaceTagsIface> faces;
    QList<QModelIndex> sourceIndexes;

    foreach (const QModelIndex& index, indexes)
    {
        infos << ImageModel::retrieveImageInfo(index);
        faces << d->faceDelegate->face(index);
        sourceIndexes << imageSortFilterModel()->mapToSourceImageModel(index);
    }

    imageAlbumModel()->removeIndexes(sourceIndexes);

    for (int i=0; i<infos.size(); i++)
    {
        d->editPipeline.remove(infos[i], faces[i]);
    }
}

void DigikamImageView::activated(const ImageInfo& info, Qt::KeyboardModifiers modifiers)
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
        d->utilities->openInfosWithDefaultApplication(QList<ImageInfo>() << info);
    }
}

void DigikamImageView::showContextMenuOnInfo(QContextMenuEvent* event, const ImageInfo& info)
{
    QList<ImageInfo> selectedInfos = selectedImageInfosCurrentFirst();
    QList<qlonglong> selectedImageIDs;

    foreach (const ImageInfo& info, selectedInfos)
    {
        selectedImageIDs << info.id();
    }

    // Temporary actions --------------------------------------

    QAction* const viewAction = new QAction(QIcon::fromTheme(QLatin1String("view-preview")), i18nc("View the selected image", "Preview"), this);
    viewAction->setEnabled(selectedImageIDs.count() == 1);

    // --------------------------------------------------------

    QMenu popmenu(this);
    ContextMenuHelper cmhelper(&popmenu);
    cmhelper.setImageFilterModel(imageFilterModel());

    cmhelper.addAction(QLatin1String("full_screen"));
    cmhelper.addAction(QLatin1String("options_show_menubar"));
    cmhelper.addSeparator();
    // --------------------------------------------------------
    cmhelper.addAction(QLatin1String("move_selection_to_album"));
    cmhelper.addAction(viewAction);
    cmhelper.addAction(QLatin1String("image_edit"));
    cmhelper.addServicesMenu(selectedUrls());
    cmhelper.addGotoMenu(selectedImageIDs);
    cmhelper.addAction(QLatin1String("image_rotate"));
    cmhelper.addSeparator();
    // --------------------------------------------------------
    cmhelper.addAction(QLatin1String("image_find_similar"));
    cmhelper.addStandardActionLightTable();
    cmhelper.addQueueManagerMenu();
    cmhelper.addSeparator();
    // --------------------------------------------------------
    cmhelper.addAction(QLatin1String("image_rename"));
    cmhelper.addAction(QLatin1String("cut_album_selection"));
    cmhelper.addAction(QLatin1String("copy_album_selection"));
    cmhelper.addAction(QLatin1String("paste_album_selection"));
    cmhelper.addStandardActionItemDelete(this, SLOT(deleteSelected()), selectedImageIDs.count());
    cmhelper.addSeparator();
    // --------------------------------------------------------
    cmhelper.addStandardActionThumbnail(selectedImageIDs, currentAlbum());
    // --------------------------------------------------------
    cmhelper.addAssignTagsMenu(selectedImageIDs);
    cmhelper.addRemoveTagsMenu(selectedImageIDs);
    cmhelper.addSeparator();
    // --------------------------------------------------------
    cmhelper.addLabelsAction();

    if (!d->faceMode)
    {
        cmhelper.addGroupMenu(selectedImageIDs);
    }

    // special action handling --------------------------------

    connect(&cmhelper, SIGNAL(signalAssignTag(int)),
            this, SLOT(assignTagToSelected(int)));

    connect(&cmhelper, SIGNAL(signalPopupTagsView()),
            this, SIGNAL(signalPopupTagsView()));

    connect(&cmhelper, SIGNAL(signalRemoveTag(int)),
            this, SLOT(removeTagFromSelected(int)));

    connect(&cmhelper, SIGNAL(signalGotoTag(int)),
            this, SIGNAL(gotoTagAndImageRequested(int)));

    connect(&cmhelper, SIGNAL(signalGotoAlbum(ImageInfo)),
            this, SIGNAL(gotoAlbumAndImageRequested(ImageInfo)));

    connect(&cmhelper, SIGNAL(signalGotoDate(ImageInfo)),
            this, SIGNAL(gotoDateAndImageRequested(ImageInfo)));

    connect(&cmhelper, SIGNAL(signalAssignPickLabel(int)),
            this, SLOT(assignPickLabelToSelected(int)));

    connect(&cmhelper, SIGNAL(signalAssignColorLabel(int)),
            this, SLOT(assignColorLabelToSelected(int)));

    connect(&cmhelper, SIGNAL(signalAssignRating(int)),
            this, SLOT(assignRatingToSelected(int)));

    connect(&cmhelper, SIGNAL(signalSetThumbnail(ImageInfo)),
            this, SLOT(setAsAlbumThumbnail(ImageInfo)));

    connect(&cmhelper, SIGNAL(signalAddToExistingQueue(int)),
            this, SLOT(insertSelectedToExistingQueue(int)));

    connect(&cmhelper, SIGNAL(signalCreateGroup()),
            this, SLOT(createGroupFromSelection()));

    connect(&cmhelper, SIGNAL(signalCreateGroupByTime()),
            this, SLOT(createGroupByTimeFromSelection()));

    connect(&cmhelper, SIGNAL(signalCreateGroupByType()),
            this, SLOT(createGroupByTypeFromSelection()));

    connect(&cmhelper, SIGNAL(signalUngroup()),
            this, SLOT(ungroupSelected()));

    connect(&cmhelper, SIGNAL(signalRemoveFromGroup()),
            this, SLOT(removeSelectedFromGroup()));

    // --------------------------------------------------------

    QAction* const choice = cmhelper.exec(event->globalPos());

    if (choice && (choice == viewAction))
    {
        emit previewRequested(info);
    }
}

void DigikamImageView::showGroupContextMenu(const QModelIndex& index, QContextMenuEvent* event)
{
    Q_UNUSED(index);
    QList<ImageInfo> selectedInfos = selectedImageInfosCurrentFirst();
    QList<qlonglong> selectedImageIDs;

    foreach (const ImageInfo& info, selectedInfos)
    {
        selectedImageIDs << info.id();
    }

    QMenu popmenu(this);
    ContextMenuHelper cmhelper(&popmenu);
    cmhelper.setImageFilterModel(imageFilterModel());
    cmhelper.addGroupActions(selectedImageIDs);

    // special action handling --------------------------------

    connect(&cmhelper, SIGNAL(signalCreateGroup()),
            this, SLOT(createGroupFromSelection()));

    connect(&cmhelper, SIGNAL(signalCreateGroupByTime()),
            this, SLOT(createGroupByTimeFromSelection()));

    connect(&cmhelper, SIGNAL(signalCreateGroupByType()),
            this, SLOT(createGroupByTypeFromSelection()));

    connect(&cmhelper, SIGNAL(signalUngroup()),
            this, SLOT(ungroupSelected()));

    connect(&cmhelper, SIGNAL(signalRemoveFromGroup()),
            this, SLOT(removeSelectedFromGroup()));


    cmhelper.exec(event->globalPos());
}

void DigikamImageView::showContextMenu(QContextMenuEvent* event)
{
    Album* const album = currentAlbum();

    if (!album          ||
        album->isRoot() ||
        (album->type() != Album::PHYSICAL && album->type() != Album::TAG) )
    {
        return;
    }

    QMenu popmenu(this);
    ContextMenuHelper cmhelper(&popmenu);
    cmhelper.setImageFilterModel(imageFilterModel());

    cmhelper.addAction(QLatin1String("full_screen"));
    cmhelper.addAction(QLatin1String("options_show_menubar"));
    cmhelper.addSeparator();
    // --------------------------------------------------------
    cmhelper.addStandardActionPaste(this, SLOT(paste()));
    // --------------------------------------------------------

    cmhelper.exec(event->globalPos());
}

void DigikamImageView::openFile(const ImageInfo& info)
{
    d->utilities->openInfos(info, imageInfos(), currentAlbum());
}

void DigikamImageView::insertSelectedToCurrentQueue()
{
    ImageInfoList imageInfoList = selectedImageInfos();

    if (!imageInfoList.isEmpty())
    {
        d->utilities->insertToQueueManager(imageInfoList, imageInfoList.first(), false);
    }
}

void DigikamImageView::insertSelectedToNewQueue()
{
    ImageInfoList imageInfoList = selectedImageInfos();

    if (!imageInfoList.isEmpty())
    {
        d->utilities->insertToQueueManager(imageInfoList, imageInfoList.first(), true);
    }
}

void DigikamImageView::insertSelectedToExistingQueue(int queueid)
{
    ImageInfoList imageInfoList = selectedImageInfos();

    if (!imageInfoList.isEmpty())
    {
        d->utilities->insertSilentToQueueManager(imageInfoList, imageInfoList.first(), queueid);
    }
}

void DigikamImageView::deleteSelected(const ImageViewUtilities::DeleteMode deleteMode)
{
    ImageInfoList imageInfoList = selectedImageInfos();

    if (d->utilities->deleteImages(imageInfoList, deleteMode))
    {
        awayFromSelection();
    }
}

void DigikamImageView::deleteSelectedDirectly(const ImageViewUtilities::DeleteMode deleteMode)
{
    ImageInfoList imageInfoList = selectedImageInfos();

    d->utilities->deleteImagesDirectly(imageInfoList, deleteMode);
    awayFromSelection();
}

void DigikamImageView::assignTagToSelected(int tagID)
{
    FileActionMngr::instance()->assignTags(selectedImageInfos(), QList<int>() << tagID);
}

void DigikamImageView::removeTagFromSelected(int tagID)
{
    FileActionMngr::instance()->removeTags(selectedImageInfos(), QList<int>() << tagID);
}

void DigikamImageView::assignPickLabelToSelected(int pickId)
{
    FileActionMngr::instance()->assignPickLabel(selectedImageInfos(), pickId);
}

void DigikamImageView::assignPickLabel(const QModelIndex& index, int pickId)
{
    FileActionMngr::instance()->assignPickLabel(QList<ImageInfo>() << imageFilterModel()->imageInfo(index), pickId);
}

void DigikamImageView::assignColorLabelToSelected(int colorId)
{
    FileActionMngr::instance()->assignColorLabel(selectedImageInfos(), colorId);
}

void DigikamImageView::assignColorLabel(const QModelIndex& index, int colorId)
{
    FileActionMngr::instance()->assignColorLabel(QList<ImageInfo>() << imageFilterModel()->imageInfo(index), colorId);
}

void DigikamImageView::assignRatingToSelected(int rating)
{
    FileActionMngr::instance()->assignRating(selectedImageInfos(), rating);
}

void DigikamImageView::assignRating(const QList<QModelIndex>& indexes, int rating)
{
    FileActionMngr::instance()->assignRating(imageFilterModel()->imageInfos(indexes), rating);
}

void DigikamImageView::setAsAlbumThumbnail(const ImageInfo& setAsThumbnail)
{
    d->utilities->setAsAlbumThumbnail(currentAlbum(), setAsThumbnail);
}

void DigikamImageView::createNewAlbumForSelected()
{
    d->utilities->createNewAlbumForInfos(selectedImageInfos(), currentAlbum());
}

void DigikamImageView::groupIndicatorClicked(const QModelIndex& index)
{
    ImageInfo info = imageFilterModel()->imageInfo(index);

    if (info.isNull())
    {
        return;
    }

    setCurrentIndex(index);
    imageFilterModel()->toggleGroupOpen(info.id());
    imageAlbumModel()->ensureHasGroupedImages(info);
}

void DigikamImageView::createGroupFromSelection()
{
    QList<ImageInfo> selectedInfos = selectedImageInfosCurrentFirst();
    ImageInfo groupLeader          = selectedInfos.takeFirst();
    FileActionMngr::instance()->addToGroup(groupLeader, selectedInfos);
}

void DigikamImageView::createGroupByTimeFromSelection()
{
    const QList<ImageInfo> selectedInfos = selectedImageInfos();
    d->utilities->createGroupByTimeFromInfoList(selectedInfos);
}

void DigikamImageView::createGroupByTypeFromSelection()
{
    const QList<ImageInfo> selectedInfos = selectedImageInfos();
    d->utilities->createGroupByTypeFromInfoList(selectedInfos);
}

void DigikamImageView::ungroupSelected()
{
    FileActionMngr::instance()->ungroup(selectedImageInfos());
}

void DigikamImageView::removeSelectedFromGroup()
{
    FileActionMngr::instance()->removeFromGroup(selectedImageInfos());
}

void DigikamImageView::rename()
{
    QList<QUrl>   urls = selectedUrls();
    NewNamesList newNamesList;

    qCDebug(DIGIKAM_GENERAL_LOG) << "Selected URLs to rename: " << urls;

    QPointer<AdvancedRenameDialog> dlg = new AdvancedRenameDialog(this);
    dlg->slotAddImages(urls);

    if (dlg->exec() == QDialog::Accepted)
    {
        newNamesList = dlg->newNames();

        QUrl nextUrl = nextInOrder(selectedImageInfos().last(),1).fileUrl();
        setCurrentUrl(nextUrl);
    }

    delete dlg;

    if (!newNamesList.isEmpty())
    {
        QPointer<AdvancedRenameProcessDialog> dlg = new AdvancedRenameProcessDialog(newNamesList);
        dlg->exec();
        delete dlg;
    }
}

void DigikamImageView::slotRotateLeft(const QList<QModelIndex>& indexes)
{
    FileActionMngr::instance()->transform(QList<ImageInfo>() << imageFilterModel()->imageInfos(indexes),
                                          MetaEngineRotation::Rotate270);
}

void DigikamImageView::slotRotateRight(const QList<QModelIndex>& indexes)
{
    FileActionMngr::instance()->transform(QList<ImageInfo>() << imageFilterModel()->imageInfos(indexes),
                                          MetaEngineRotation::Rotate90);
}

void DigikamImageView::slotFullscreen(const QList<QModelIndex>& indexes)
{
   QList<ImageInfo> infos = imageFilterModel()->imageInfos(indexes);

   if (infos.isEmpty())
   {
        return;
   }

   // Just fullscreen the first.
   const ImageInfo& info = infos.at(0);

   emit fullscreenRequested(info);
}

void DigikamImageView::slotInitProgressIndicator()
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
