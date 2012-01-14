/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-04-24
 * Description : Qt item view for images
 *
 * Copyright (C) 2009-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2009-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2011 by Andi Clemens <andi dot clemens at googlemail dot com>
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
#include "digikamimageview.moc"

// Qt includes

#include <QClipboard>
#include <QFileInfo>
#include <QPointer>

// KDE includes

#include <kaction.h>
#include <kactionmenu.h>
#include <kactioncollection.h>
#include <kapplication.h>
#include <kmenu.h>
#include <kmessagebox.h>
#include <kmimetype.h>
#include <krun.h>
#include <kservice.h>
#include <kservicetypetrader.h>
#include <kstandardaction.h>
#include <kurl.h>
#include <kwindowsystem.h>
#include <kdebug.h>

// Local includes

#include "albummanager.h"
#include "albumdb.h"
#include "advancedrenamedialog.h"
#include "advancedrenameprocessdialog.h"
#include "albumsettings.h"
#include "assignnameoverlay.h"
#include "contextmenuhelper.h"
#include "databaseaccess.h"
#include "ddragobjects.h"
#include "digikamapp.h"
#include "digikamimagedelegate.h"
#include "digikamimagefacedelegate.h"
#include "dio.h"
#include "facerejectionoverlay.h"
#include "faceiface.h"
#include "groupindicatoroverlay.h"
#include "imagealbumfiltermodel.h"
#include "imagealbummodel.h"
#include "imagedragdrop.h"
#include "imageratingoverlay.h"
#include "tagslineeditoverlay.h"
#include "imageviewutilities.h"
#include "imagewindow.h"
#include "fileactionmngr.h"
#include "thumbnailloadthread.h"
#include "tagregion.h"
#include "addtagslineedit.h"

namespace Digikam
{

DigikamImageView::DigikamImageView(QWidget* parent)
    : ImageCategorizedView(parent), d(new DigikamImageViewPriv(this))
{
    installDefaultModels();

    d->editPipeline.plugDatabaseEditor();
    d->editPipeline.plugTrainer();
    d->editPipeline.construct();

    d->normalDelegate = new DigikamImageDelegate(this);
    d->faceDelegate   = new DigikamImageFaceDelegate(this);
    setItemDelegate(d->normalDelegate);
    setSpacing(10);

    AlbumSettings* settings = AlbumSettings::instance();

    imageFilterModel()->setCategorizationMode(ImageSortSettings::CategoryByAlbum);

    imageAlbumModel()->setThumbnailLoadThread(ThumbnailLoadThread::defaultIconViewThread());
    setThumbnailSize((ThumbnailSize::Size)settings->getDefaultIconSize());
    imageAlbumModel()->setPreloadThumbnails(true);

    imageModel()->setDragDropHandler(new ImageDragDropHandler(imageModel()));
    setDragEnabled(true);
    setAcceptDrops(true);
    setDropIndicatorShown(false);

    setToolTipEnabled(settings->showToolTipsIsValid());
    imageFilterModel()->setSortRole((ImageSortSettings::SortRole)settings->getImageSortOrder());
    imageFilterModel()->setSortOrder((ImageSortSettings::SortOrder)settings->getImageSorting());
    imageFilterModel()->setCategorizationMode((ImageSortSettings::CategorizationMode)settings->getImageGroupMode());

    // selection overlay
    addSelectionOverlay(d->normalDelegate);
    addSelectionOverlay(d->faceDelegate);

    // rotation overlays
    d->rotateLeftOverlay = ImageRotateOverlay::left(this);
    d->rotateRightOverlay = ImageRotateOverlay::right(this);
    d->updateOverlays();

    // rating overlay
    ImageRatingOverlay* ratingOverlay = new ImageRatingOverlay(this);
    addOverlay(ratingOverlay);

    // face overlays
    addRejectionOverlay(d->faceDelegate);
    addAssignNameOverlay(d->faceDelegate);

    GroupIndicatorOverlay* groupOverlay = new GroupIndicatorOverlay(this);
    addOverlay(groupOverlay);

    connect(ratingOverlay, SIGNAL(ratingEdited(QList<QModelIndex>,int)),
            this, SLOT(assignRating(QList<QModelIndex>,int)));

    connect(groupOverlay, SIGNAL(toggleGroupOpen(QModelIndex)),
            this, SLOT(groupIndicatorClicked(QModelIndex)));

    connect(groupOverlay, SIGNAL(showButtonContextMenu(QModelIndex,QContextMenuEvent*)),
            this, SLOT(showGroupContextMenu(QModelIndex,QContextMenuEvent*)));

    d->utilities = new ImageViewUtilities(this);

    connect(d->utilities, SIGNAL(editorCurrentUrlChanged(KUrl)),
            this, SLOT(setCurrentUrl(KUrl)));

    connect(imageModel()->dragDropHandler(), SIGNAL(assignTags(QList<ImageInfo>,QList<int>)),
            FileActionMngr::instance(), SLOT(assignTags(QList<ImageInfo>,QList<int>)));

    connect(imageModel()->dragDropHandler(), SIGNAL(addToGroup(ImageInfo,QList<ImageInfo>)),
            FileActionMngr::instance(), SLOT(addToGroup(ImageInfo,QList<ImageInfo>)));

    connect(imageModel()->dragDropHandler(), SIGNAL(dioResult(KJob*)),
            d->utilities, SLOT(slotDIOResult(KJob*)));

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

void DigikamImageView::connectProgressSignals(QObject* progressManager)
{
    connect(&d->editPipeline, SIGNAL(started(QString)),
            progressManager, SLOT(enterProgress(QString)));

    connect(&d->editPipeline, SIGNAL(progressValueChanged(float)),
            progressManager, SLOT(progressValue(float)));

    connect(&d->editPipeline, SIGNAL(finished()),
            progressManager, SLOT(finishProgress()));
}

void DigikamImageView::slotSetupChanged()
{
    setToolTipEnabled(AlbumSettings::instance()->showToolTipsIsValid());
    setFont(AlbumSettings::instance()->getIconViewFont());

    d->updateOverlays();

    ImageCategorizedView::slotSetupChanged();
}

void DigikamImageView::setFaceMode(bool on)
{
    d->faceMode = on;

    if (on)
    {
        // See ImageLister, which creates a search the implements listing tag in the ioslave
        imageAlbumModel()->setSpecialTagListing("faces");
        setItemDelegate(d->faceDelegate);
    }
    else
    {
        imageAlbumModel()->setSpecialTagListing(QString());
        setItemDelegate(d->normalDelegate);
    }
}

void DigikamImageView::addRejectionOverlay(ImageDelegate* delegate)
{
    FaceRejectionOverlay* rejectionOverlay = new FaceRejectionOverlay(this);

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
    AssignNameOverlay* nameOverlay = new AssignNameOverlay(this);
    addOverlay(nameOverlay, delegate);

    connect(nameOverlay, SIGNAL(confirmFaces(QList<QModelIndex>,int)),
            this, SLOT(confirmFaces(QList<QModelIndex>,int)));

    connect(nameOverlay, SIGNAL(removeFaces(QList<QModelIndex>)),
            this, SLOT(removeFaces(QList<QModelIndex>)));
}

void DigikamImageView::confirmFaces(const QList<QModelIndex>& indexes, int tagId)
{
    QList<ImageInfo> infos;
    QList<DatabaseFace> faces;
    QList<QModelIndex> sourceIndexes;

    // fast-remove in the "unknown person" view
    const bool needFastRemove = d->faceMode
                                && imageAlbumModel()->currentAlbum()
                                && tagId != imageAlbumModel()->currentAlbum()->id();

    foreach(const QModelIndex& index, indexes)
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
    QList<DatabaseFace> faces;
    QList<QModelIndex> sourceIndexes;

    foreach(const QModelIndex& index, indexes)
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

void DigikamImageView::activated(const ImageInfo& info)
{
    if (info.isNull())
    {
        return;
    }

    if (AlbumSettings::instance()->getItemLeftClickAction() == AlbumSettings::ShowPreview)
    {
        emit previewRequested(info);
    }
    else
    {
        openInEditor(info);
    }
}

void DigikamImageView::showContextMenuOnInfo(QContextMenuEvent* event, const ImageInfo& info)
{
    QList<ImageInfo> selectedInfos = selectedImageInfosCurrentFirst();
    QList<qlonglong> selectedImageIDs;
    foreach(const ImageInfo& info, selectedInfos)
    {
        selectedImageIDs << info.id();
    }

    // Temporary actions --------------------------------------

    QAction* viewAction = new QAction(SmallIcon("viewimage"), i18nc("View the selected image", "View"), this);
    viewAction->setEnabled(selectedImageIDs.count() == 1);

    // --------------------------------------------------------

    KMenu popmenu(this);
    ContextMenuHelper cmhelper(&popmenu);
    cmhelper.setImageFilterModel(imageFilterModel());

    cmhelper.addAction("full_screen");
    cmhelper.addSeparator();
    // --------------------------------------------------------
    cmhelper.addAction("move_selection_to_album");
    cmhelper.addAction(viewAction);
    cmhelper.addAction("image_edit");
    cmhelper.addServicesMenu(selectedUrls());
    cmhelper.addGotoMenu(selectedImageIDs);
    cmhelper.addRotateMenu(selectedImageIDs);
    cmhelper.addSeparator();
    // --------------------------------------------------------
    cmhelper.addAction("image_find_similar");
    cmhelper.addStandardActionLightTable();
    cmhelper.addQueueManagerMenu();
    cmhelper.addSeparator();
    // --------------------------------------------------------
    cmhelper.addAction("image_rename");
    cmhelper.addAction("cut_album_selection");
    cmhelper.addAction("copy_album_selection");
    cmhelper.addAction("paste_album_selection");
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
    cmhelper.addGroupMenu(selectedImageIDs);

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

    connect(&cmhelper, SIGNAL(signalUngroup()),
            this, SLOT(ungroupSelected()));

    connect(&cmhelper, SIGNAL(signalRemoveFromGroup()),
            this, SLOT(removeSelectedFromGroup()));

    // --------------------------------------------------------

    QAction* choice = cmhelper.exec(event->globalPos());

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
    foreach(const ImageInfo& info, selectedInfos)
    {
        selectedImageIDs << info.id();
    }

    KMenu popmenu(this);
    ContextMenuHelper cmhelper(&popmenu);
    cmhelper.setImageFilterModel(imageFilterModel());

    cmhelper.addGroupActions(selectedImageIDs);

    // special action handling --------------------------------

    connect(&cmhelper, SIGNAL(signalCreateGroup()),
            this, SLOT(createGroupFromSelection()));

    connect(&cmhelper, SIGNAL(signalCreateGroupByTime()),
            this, SLOT(createGroupByTimeFromSelection()));

    connect(&cmhelper, SIGNAL(signalUngroup()),
            this, SLOT(ungroupSelected()));

    connect(&cmhelper, SIGNAL(signalRemoveFromGroup()),
            this, SLOT(removeSelectedFromGroup()));


    cmhelper.exec(event->globalPos());
}

void DigikamImageView::showContextMenu(QContextMenuEvent* event)
{
    Album* album = currentAlbum();

    if (!album ||
        album->isRoot() ||
        (album->type() != Album::PHYSICAL && album->type() != Album::TAG) )
    {
        return;
    }

    KMenu popmenu(this);
    ContextMenuHelper cmhelper(&popmenu);
    cmhelper.setImageFilterModel(imageFilterModel());

    cmhelper.addAction("full_screen");
    cmhelper.addSeparator();
    // --------------------------------------------------------
    cmhelper.addStandardActionPaste(this, SLOT(paste()));
    // --------------------------------------------------------

    cmhelper.exec(event->globalPos());
}

void DigikamImageView::openInEditor(const ImageInfo& info)
{
    d->utilities->openInEditor(info, imageInfos(), currentAlbum());
}

void DigikamImageView::openCurrentInEditor()
{
    ImageInfo info = currentInfo();

    if (!info.isNull())
    {
        d->utilities->openInEditor(info, imageInfos(), currentAlbum());
    }
}

void DigikamImageView::openEditor()
{
    ImageInfoList imageInfoList = imageInfos();
    ImageInfo     singleInfo    = currentInfo();
    if (singleInfo.isNull() && !imageInfoList.isEmpty())
    {
        singleInfo = imageInfoList.first();
    }

    d->utilities->openInEditor(singleInfo, imageInfoList, currentAlbum());
}

void DigikamImageView::setOnLightTable()
{
    d->utilities->insertToLightTableAuto(imageInfos(), selectedImageInfos(), currentInfo());
}

void DigikamImageView::addSelectedToLightTable()
{
    // Run Light Table with all selected image files in the current Album.
    // If addTo is false, the light table will be emptied before adding
    // the images.
    ImageInfoList imageInfoList = selectedImageInfos();

    if (!imageInfoList.isEmpty())
    {
        d->utilities->insertToLightTable(imageInfoList, imageInfoList.first(), true);
    }
}

void DigikamImageView::setSelectedOnLightTable()
{
    // Run Light Table with all selected image files in the current Album.
    // If addTo is false, the light table will be emptied before adding
    // the images.
    ImageInfoList imageInfoList = selectedImageInfos();

    if (!imageInfoList.isEmpty())
    {
        d->utilities->insertToLightTable(imageInfoList, imageInfoList.first(), false);
    }
}

void DigikamImageView::insertToQueue()
{
    ImageInfoList imageInfoList = selectedImageInfos();
    ImageInfo     singleInfo    = currentInfo();
    if (singleInfo.isNull() && !imageInfoList.isEmpty())
    {
        singleInfo = imageInfoList.first();
    }
    if (singleInfo.isNull() && model()->rowCount())
    {
        singleInfo = imageInfos().first();
    }
    d->utilities->insertToQueueManager(imageInfoList, singleInfo, true);
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

void DigikamImageView::deleteSelected(bool permanently)
{
    ImageInfoList imageInfoList = selectedImageInfos();

    if (d->utilities->deleteImages(imageInfoList, permanently))
    {
        awayFromSelection();
    }
}

void DigikamImageView::deleteSelectedDirectly(bool permanently)
{
    ImageInfoList imageInfoList = selectedImageInfos();
    d->utilities->deleteImagesDirectly(imageInfoList, permanently);
    awayFromSelection();
}

void DigikamImageView::toggleTagToSelected(int tagID)
{
    ImageInfoList tagToRemove, tagToAssign;

    foreach(ImageInfo info, selectedImageInfos())
    {
        if (info.tagIds().contains(tagID))
            tagToRemove.append(info);
        else
            tagToAssign.append(info);
    }

    FileActionMngr::instance()->assignTags(tagToAssign, QList<int>() << tagID);
    FileActionMngr::instance()->removeTags(tagToRemove, QList<int>() << tagID);
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
}

void DigikamImageView::createGroupFromSelection()
{
    QList<ImageInfo> selectedInfos = selectedImageInfosCurrentFirst();
    ImageInfo groupLeader = selectedInfos.takeFirst();
    FileActionMngr::instance()->addToGroup(groupLeader, selectedInfos);
}

void DigikamImageView::createGroupByTimeFromSelection()
{
    QList<ImageInfo> selectedInfos = selectedImageInfosCurrentFirst();
    while (selectedInfos.size() > 0) {
        QList<ImageInfo> group;
        ImageInfo groupLeader = selectedInfos.takeFirst();
        QDateTime dateTime = groupLeader.dateTime();
        while (selectedInfos.size()>0 && abs(dateTime.secsTo(selectedInfos.first().dateTime()))<2) {
            group.push_back(selectedInfos.takeFirst());
        }
        FileActionMngr::instance()->addToGroup(groupLeader, group);
    }
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
    KUrl::List urls = selectedUrls();
    NewNamesList newNamesList;

    QPointer<AdvancedRenameDialog> dlg = new AdvancedRenameDialog(this);
    dlg->slotAddImages(urls);

    if (dlg->exec() == KDialog::Accepted)
    {
        newNamesList = dlg->newNames();
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
                                          KExiv2Iface::RotationMatrix::Rotate270);
}

void DigikamImageView::slotRotateRight(const QList<QModelIndex>& indexes)
{
    FileActionMngr::instance()->transform(QList<ImageInfo>() << imageFilterModel()->imageInfos(indexes),
                                          KExiv2Iface::RotationMatrix::Rotate90);
}

} // namespace Digikam
