/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-04-24
 * Description : Qt item view for images
 *
 * Copyright (C) 2009-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2011      by Andi Clemens <andi dot clemens at gmail dot com>
 * Copyright (C) 2013      by Michael G. Hansen <mike at mghansen dot de>
 * Copyright (C) 2014      by Mohamed Anwer <m dot anwer at gmx dot com>
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
    imageFilterModel()->setCategorizationMode((ImageSortSettings::CategorizationMode)settings->getImageSeparationMode());
    imageFilterModel()->setCategorizationSortOrder((ImageSortSettings::SortOrder) settings->getImageSeparationSortOrder());

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

    connect(imageModel()->dragDropHandler(), SIGNAL(assignTags(QList<ImageInfo>,QList<int>)),
            FileActionMngr::instance(), SLOT(assignTags(QList<ImageInfo>,QList<int>)));

    connect(imageModel()->dragDropHandler(), SIGNAL(addToGroup(ImageInfo,QList<ImageInfo>)),
            FileActionMngr::instance(), SLOT(addToGroup(ImageInfo,QList<ImageInfo>)));

    connect(d->utilities, SIGNAL(editorCurrentUrlChanged(QUrl)),
            this, SLOT(setCurrentUrlWhenAvailable(QUrl)));

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

    for (int i = 0 ; i < infos.size() ; i++)
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

    for (int i = 0 ; i < infos.size() ; i++)
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
    emit signalShowContextMenuOnInfo(event, info, QList<QAction*>(), imageFilterModel());
}

void DigikamImageView::showGroupContextMenu(const QModelIndex& index, QContextMenuEvent* event)
{
    Q_UNUSED(index);
    emit signalShowGroupContextMenu(event, selectedImageInfosCurrentFirst(), imageFilterModel());
}

void DigikamImageView::showContextMenu(QContextMenuEvent* event)
{
    emit signalShowContextMenu(event);
}

void DigikamImageView::openFile(const ImageInfo& info)
{
    d->utilities->openInfos(info, allImageInfos(), currentAlbum());
}

void DigikamImageView::deleteSelected(const ImageViewUtilities::DeleteMode deleteMode)
{
    ImageInfoList imageInfoList = selectedImageInfos(true);

    if (d->utilities->deleteImages(imageInfoList, deleteMode))
    {
        awayFromSelection();
    }
}

void DigikamImageView::deleteSelectedDirectly(const ImageViewUtilities::DeleteMode deleteMode)
{
    ImageInfoList imageInfoList = selectedImageInfos(true);

    d->utilities->deleteImagesDirectly(imageInfoList, deleteMode);
    awayFromSelection();
}

void DigikamImageView::assignRating(const QList<QModelIndex>& indexes, int rating)
{
    ImageInfoList infos = imageInfos(indexes, ApplicationSettings::Metadata);
    FileActionMngr::instance()->assignRating(infos, rating);
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

void DigikamImageView::rename()
{
    bool grouping     = needGroupResolving(ApplicationSettings::Rename);
    QList<QUrl>  urls = selectedUrls(grouping);
    NewNamesList newNamesList;

    qCDebug(DIGIKAM_GENERAL_LOG) << "Selected URLs to rename: " << urls;

    QPointer<AdvancedRenameDialog> dlg = new AdvancedRenameDialog(this);
    dlg->slotAddImages(urls);

    if (dlg->exec() == QDialog::Accepted)
    {
        newNamesList = dlg->newNames();

        QUrl nextUrl = nextInOrder(selectedImageInfos(grouping).last(),1).fileUrl();
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
    ImageInfoList infos = imageInfos(indexes, ApplicationSettings::Metadata);
    FileActionMngr::instance()->transform(infos, MetaEngineRotation::Rotate270);
}

void DigikamImageView::slotRotateRight(const QList<QModelIndex>& indexes)
{
    ImageInfoList infos = imageInfos(indexes, ApplicationSettings::Metadata);
    FileActionMngr::instance()->transform(infos, MetaEngineRotation::Rotate90);
}

void DigikamImageView::slotFullscreen(const QList<QModelIndex>& indexes)
{
   QList<ImageInfo> infos = imageInfos(indexes);

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
