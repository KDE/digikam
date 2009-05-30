/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-04-24
 * Description : Qt item view for images
 *
 * Copyright (C) 2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "digikamimageview.h"
#include "digikamimageview.moc"

// Qt includes

#include <QClipboard>
#include <QFileInfo>

// KDE includes

#include <kaction.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kmenu.h>
#include <kmimetype.h>
#include <krun.h>
#include <kservice.h>
#include <kservicetypetrader.h>
#include <kstandardaction.h>
#include <kurl.h>
#include <kwindowsystem.h>

// Local includes

#include "albumsettings.h"
#include "contextmenuhelper.h"
#include "ddragobjects.h"
#include "dio.h"
#include "dpopupmenu.h"
#include "imagealbummodel.h"
#include "imagealbumfiltermodel.h"
#include "imagedragdrop.h"
#include "imageratingoverlay.h"
#include "imageviewutilities.h"
#include "imagewindow.h"
#include "metadatamanager.h"
#include "thumbnailloadthread.h"

namespace Digikam
{

class DigikamImageViewPriv
{
public:
    DigikamImageViewPriv()
    {
        utilities = 0;
    }

    ImageViewUtilities *utilities;
};

DigikamImageView::DigikamImageView(QWidget *parent)
                : ImageCategorizedView(parent), d(new DigikamImageViewPriv)
{
    imageFilterModel()->setCategorizationMode(ImageFilterModel::CategoryByAlbum);

    imageModel()->setThumbnailLoadThread(ThumbnailLoadThread::defaultIconViewThread());
    setThumbnailSize((ThumbnailSize::Size)AlbumSettings::instance()->getDefaultIconSize());

    imageModel()->setDragDropHandler(new ImageDragDropHandler(imageModel()));
    setDragEnabled(true);
    setAcceptDrops(true);
    setDropIndicatorShown(false);

    setToolTipEnabled(AlbumSettings::instance()->getShowToolTips());

    ImageRatingOverlay *ratingOverlay = new ImageRatingOverlay(this);
    addOverlay(ratingOverlay);

    d->utilities = new ImageViewUtilities(this);

    connect(d->utilities, SIGNAL(editorCurrentUrlChanged(const KUrl &)),
            this, SLOT(setCurrentUrl(const KUrl &)));

    connect(imageModel()->dragDropHandler(), SIGNAL(dioResult(KJob *)),
            d->utilities, SLOT(slotDIOResult(KJob*)));

    connect(ratingOverlay, SIGNAL(ratingEdited(const QModelIndex &, int)),
            this, SLOT(assignRating(const QModelIndex &,int)));
}

DigikamImageView::~DigikamImageView()
{
    delete d;
}

ImageViewUtilities *DigikamImageView::utilities() const
{
    return d->utilities;
}

void DigikamImageView::slotSetupChanged()
{
    setToolTipEnabled(AlbumSettings::instance()->getShowToolTips());
    ImageCategorizedView::slotSetupChanged();
}

void DigikamImageView::activated(const ImageInfo& info)
{
    if (info.isNull())
        return;

    if (AlbumSettings::instance()->getItemRightClickAction() == AlbumSettings::ShowPreview)
        previewRequested(info);
    else
        openInEditor(info);
}

void DigikamImageView::openInEditor(const ImageInfo& info)
{
    d->utilities->openInEditor(info, imageInfos(), currentAlbum());
}

void DigikamImageView::openCurrentInEditor()
{
    ImageInfo info = currentInfo();
    if (!info.isNull())
        d->utilities->openInEditor(info, imageInfos(), currentAlbum());
}

void DigikamImageView::showContextMenu(QContextMenuEvent* event, const ImageInfo& info)
{
    QList<ImageInfo> selectedInfos = selectedImageInfosCurrentFirst();
    QList<qlonglong> selectedImageIDs;
    foreach (ImageInfo info, selectedInfos)
    {
        selectedImageIDs << info.id();
    }

    // Temporary actions --------------------------------------

    QAction  *viewAction = new QAction(SmallIcon("viewimage"), i18nc("View the selected image", "View"),  this);
    viewAction->setEnabled(selectedImageIDs.count() == 1);

    // --------------------------------------------------------

    DPopupMenu popmenu(this);
    ContextMenuHelper cmhelper(&popmenu);

    cmhelper.addAction("album_new_from_selection");
    cmhelper.addAction(viewAction);
    cmhelper.addAction("image_edit");
    cmhelper.addServicesMenu(selectedUrls());
    cmhelper.addKipiActions();
    popmenu.addSeparator();
    // --------------------------------------------------------
    cmhelper.addAction("image_find_similar");
    cmhelper.addActionLightTable();
    cmhelper.addQueueManagerMenu();
    cmhelper.addGotoMenu(selectedImageIDs);
    cmhelper.addAction("image_rename");
    popmenu.addSeparator();
    // --------------------------------------------------------
    cmhelper.addActionCopy(this, SLOT(copy()));
    cmhelper.addActionPaste(this, SLOT(paste()));
    cmhelper.addActionItemDelete(this, SLOT(deleteSelected()), selectedImageIDs.count());
    popmenu.addSeparator();
    // --------------------------------------------------------
    cmhelper.addActionThumbnail(selectedImageIDs, currentAlbum());
    // --------------------------------------------------------
    cmhelper.addAssignTagsMenu(selectedImageIDs);
    cmhelper.addRemoveTagsMenu(selectedImageIDs);
    popmenu.addSeparator();
    // --------------------------------------------------------
    cmhelper.addRatingMenu();

    // special action handling --------------------------------

    connect(&cmhelper, SIGNAL(signalAssignTag(int)),
            this, SLOT(assignTagToSelected(int)));

    connect(&cmhelper, SIGNAL(signalRemoveTag(int)),
            this, SLOT(removeTagFromSelected(int)));

    connect(&cmhelper, SIGNAL(signalGotoTag(int)),
            this, SIGNAL(gotoTagAndImageRequested(int)));

    connect(&cmhelper, SIGNAL(signalGotoAlbum(const ImageInfo&)),
            this, SIGNAL(gotoAlbumAndImageRequested(const ImageInfo&)));

    connect(&cmhelper, SIGNAL(signalGotoDate(const ImageInfo&)),
            this, SIGNAL(gotoDateAndImageRequested(const ImageInfo&)));

    connect(&cmhelper, SIGNAL(signalAssignRating(int)),
            this, SLOT(assignRatingToSelected(int)));

    connect(&cmhelper, SIGNAL(signalSetThumbnail(const ImageInfo&)),
            this, SLOT(setAsAlbumThumbnail(const ImageInfo&)));

    connect(&cmhelper, SIGNAL(signalAddToExistingQueue(int)),
            this, SLOT(insertSelectedToExistingQueue(int)));

    // --------------------------------------------------------

    QAction *choice = cmhelper.exec(event->globalPos());
    if (choice)
    {
        if (choice == viewAction)
            emit previewRequested(info);
    }
}

void DigikamImageView::showContextMenu(QContextMenuEvent* event)
{
    Album *album = currentAlbum();

    if (!album ||
         album->isRoot() ||
        (album->type() != Album::PHYSICAL && album->type() != Album::TAG) )
    {
        return;
    }

    KMenu popmenu(this);
    KAction *paste        = KStandardAction::paste(this, SLOT(paste()), 0);
    const QMimeData *data = kapp->clipboard()->mimeData(QClipboard::Clipboard);

    //TODO
    if(!data || !KUrl::List::canDecode(data))
        paste->setEnabled(false);

    popmenu.addAction(paste);
    popmenu.exec(event->globalPos());
    delete paste;
}

void DigikamImageView::copy()
{
    QMimeData *data = imageModel()->dragDropHandler()->createMimeData(selectedImageInfos());
    if (data)
        kapp->clipboard()->setMimeData(data);
}

void DigikamImageView::paste()
{
    const QMimeData *data = kapp->clipboard()->mimeData(QClipboard::Clipboard);
    if (!data)
        return;
    QDropEvent event(mapFromGlobal(QCursor::pos()), Qt::CopyAction, data, Qt::NoButton, Qt::ControlModifier);
    QModelIndex index = indexAt(event.pos());
    if (!imageModel()->dragDropHandler()->accepts(&event, index))
        return;
    imageModel()->dragDropHandler()->dropEvent(this, &event, index);
}

void DigikamImageView::insertSelectedToLightTable(bool addTo)
{
    // Run Light Table with all selected image files in the current Album.
    // If addTo is false, the light table will be emptied before adding
    // the images.
    ImageInfoList imageInfoList = selectedImageInfos();
    d->utilities->insertToLightTable(imageInfoList, imageInfoList.first(), addTo);
}

void DigikamImageView::insertSelectedToCurrentQueue()
{
    ImageInfoList imageInfoList = selectedImageInfos();
    d->utilities->insertToQueueManager(imageInfoList, imageInfoList.first(), false);
}

void DigikamImageView::insertSelectedToNewQueue()
{
    ImageInfoList imageInfoList = selectedImageInfos();
    d->utilities->insertToQueueManager(imageInfoList, imageInfoList.first(), true);
}

void DigikamImageView::insertSelectedToExistingQueue(int queueid)
{
    ImageInfoList imageInfoList = selectedImageInfos();
    d->utilities->insertSilentToQueueManager(imageInfoList, imageInfoList.first(), queueid);
}

void DigikamImageView::deleteSelected(bool permanently)
{
    ImageInfoList imageInfoList = selectedImageInfos();
    d->utilities->deleteImages(imageInfoList, permanently);
}

void DigikamImageView::deleteSelectedDirectly(bool permanently)
{
    ImageInfoList imageInfoList = selectedImageInfos();
    d->utilities->deleteImagesDirectly(imageInfoList, permanently);
}

void DigikamImageView::assignTagToSelected(int tagID)
{
    MetadataManager::instance()->assignTags(selectedImageInfos(), QList<int>() << tagID);
}

void DigikamImageView::removeTagFromSelected(int tagID)
{
    MetadataManager::instance()->removeTags(selectedImageInfos(), QList<int>() << tagID);
}

void DigikamImageView::assignRatingToSelected(int rating)
{
    MetadataManager::instance()->assignRating(selectedImageInfos(), rating);
}

void DigikamImageView::assignRating(const QModelIndex &index, int rating)
{
    MetadataManager::instance()->assignRating(QList<ImageInfo>() << imageFilterModel()->imageInfo(index), rating);
}

void DigikamImageView::setAsAlbumThumbnail(const ImageInfo& setAsThumbnail)
{
    d->utilities->setAsAlbumThumbnail(currentAlbum(), setAsThumbnail);
}

void DigikamImageView::createNewAlbumForSelected()
{
    d->utilities->createNewAlbumForInfos(selectedImageInfos(), currentAlbum());
}

void DigikamImageView::setExifOrientationOfSelected(int orientation)
{
    MetadataManager::instance()->setExifOrientation(selectedImageInfos(), orientation);
}

void DigikamImageView::renameCurrent()
{
    d->utilities->rename(currentInfo());
}

} // namespace Digikam
