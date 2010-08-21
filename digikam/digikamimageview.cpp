/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-04-24
 * Description : Qt item view for images
 *
 * Copyright (C) 2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include "albumsettings.h"
#include "contextmenuhelper.h"
#include "databaseaccess.h"
#include "ddragobjects.h"
#include "digikamapp.h"
#include "digikamimagedelegate.h"
#include "digikamimagefacedelegate.h"
#include "dio.h"
#include "dpopupmenu.h"
#include "imagealbumfiltermodel.h"
#include "imagealbummodel.h"
#include "imagedragdrop.h"
#include "imageratingoverlay.h"
#include "imageviewutilities.h"
#include "imagewindow.h"
#include "metadatamanager.h"
#include "thumbnailloadthread.h"

namespace Digikam
{

DigikamImageView::DigikamImageView(QWidget *parent)
                : ImageCategorizedView(parent), d(new DigikamImageViewPriv(this))
{
    installDefaultModels();

    d->normalDelegate = new DigikamImageDelegate(this);
    d->faceDelegate   = new DigikamImageFaceDelegate(this);
    setItemDelegate(d->normalDelegate);
    setSpacing(10);

    AlbumSettings *settings = AlbumSettings::instance();

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
    addSelectionOverlay();

    // rotation overlays
    d->rotateLeftOverlay = new ImageRotateLeftOverlay(this);
    d->rotateRightOverlay = new ImageRotateRightOverlay(this);
    d->updateOverlays();

    // rating overlay
    ImageRatingOverlay *ratingOverlay = new ImageRatingOverlay(this);
    addOverlay(ratingOverlay);

    connect(ratingOverlay, SIGNAL(ratingEdited(const QModelIndex &, int)),
            this, SLOT(assignRating(const QModelIndex&, int)));

    d->utilities = new ImageViewUtilities(this);

    connect(d->utilities, SIGNAL(editorCurrentUrlChanged(const KUrl&)),
            this, SLOT(setCurrentUrl(const KUrl&)));

    connect(imageModel()->dragDropHandler(), SIGNAL(assignTags(const QList<ImageInfo>&, const QList<int>&)),
            MetadataManager::instance(), SLOT(assignTags(const QList<ImageInfo>&, const QList<int>&)));

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

ImageViewUtilities *DigikamImageView::utilities() const
{
    return d->utilities;
}

void DigikamImageView::setThumbnailSize(const ThumbnailSize& size)
{
    imageThumbnailModel()->setPreloadThumbnailSize(size);
    ImageCategorizedView::setThumbnailSize(size);
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
    if (on)
    {
        setItemDelegate(d->faceDelegate);
    }
    else
    {
        setItemDelegate(d->normalDelegate);
    }
}

void DigikamImageView::activated(const ImageInfo& info)
{
    if (info.isNull())
        return;

    if (AlbumSettings::instance()->getItemLeftClickAction() == AlbumSettings::ShowPreview)
        emit previewRequested(info);
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

void DigikamImageView::showContextMenuOnInfo(QContextMenuEvent* event, const ImageInfo& info)
{
    QList<ImageInfo> selectedInfos = selectedImageInfosCurrentFirst();
    QList<qlonglong> selectedImageIDs;
    foreach (const ImageInfo& info, selectedInfos)
    {
        selectedImageIDs << info.id();
    }

    // Temporary actions --------------------------------------

    QAction  *viewAction = new QAction(SmallIcon("viewimage"), i18nc("View the selected image", "View"),  this);
    viewAction->setEnabled(selectedImageIDs.count() == 1);

    // --------------------------------------------------------

    DPopupMenu popmenu(this);
    ContextMenuHelper cmhelper(&popmenu);

    cmhelper.addAction("move_selection_to_album");
    cmhelper.addAction(viewAction);
    cmhelper.addAction("image_edit");
    cmhelper.addServicesMenu(selectedUrls());
    cmhelper.addGotoMenu(selectedImageIDs);
    cmhelper.addKipiActions(selectedImageIDs);
    cmhelper.addSeparator();
    // --------------------------------------------------------
    cmhelper.addAction("image_find_similar");
    cmhelper.addActionLightTable();
    cmhelper.addQueueManagerMenu();
    cmhelper.addSeparator();
    // --------------------------------------------------------
    cmhelper.addAction("image_rename");
    cmhelper.addAction("cut_album_selection");
    cmhelper.addAction("copy_album_selection");
    cmhelper.addAction("paste_album_selection");
    cmhelper.addActionItemDelete(this, SLOT(deleteSelected()), selectedImageIDs.count());
    cmhelper.addSeparator();
    // --------------------------------------------------------
    cmhelper.addActionThumbnail(selectedImageIDs, currentAlbum());
    // --------------------------------------------------------
    cmhelper.addAssignTagsMenu(selectedImageIDs);
    cmhelper.addRemoveTagsMenu(selectedImageIDs);
    cmhelper.addSeparator();
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

    /**
    * @todo
    */
    if(!data || !KUrl::List::canDecode(data))
        paste->setEnabled(false);

    popmenu.addAction(paste);
    popmenu.exec(event->globalPos());
    delete paste;
}

void DigikamImageView::insertSelectedToLightTable(bool addTo)
{
    // Run Light Table with all selected image files in the current Album.
    // If addTo is false, the light table will be emptied before adding
    // the images.
    ImageInfoList imageInfoList = selectedImageInfos();

    if (!imageInfoList.isEmpty())
    {
        d->utilities->insertToLightTable(imageInfoList, imageInfoList.first(), addTo);
    }
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
        awayFromSelection();
}

void DigikamImageView::deleteSelectedDirectly(bool permanently)
{
    ImageInfoList imageInfoList = selectedImageInfos();
    d->utilities->deleteImagesDirectly(imageInfoList, permanently);
    awayFromSelection();
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

void DigikamImageView::rename()
{
    if (!d->renameThread)
    {
        d->renameThread = new RenameThread(this);

        connect(d->renameThread, SIGNAL(renameFile(const KUrl&, const QString&)),
                d->utilities, SLOT(rename(const KUrl&, const QString&)));

        connect(d->utilities, SIGNAL(imageRenameSucceeded(const KUrl&)),
                d->renameThread, SLOT(slotSuccess(const KUrl&)));

        connect(d->utilities, SIGNAL(imageRenameFailed(const KUrl&)),
                d->renameThread, SLOT(slotFailed(const KUrl&)));

        connect(d->utilities, SIGNAL(renamingAborted()),
                d->renameThread, SLOT(cancel()));
    }

    KUrl::List urls = selectedUrls();

    QPointer<AdvancedRenameDialog> dlg = new AdvancedRenameDialog(this);
    dlg->slotAddImages(urls);

    if (dlg->exec() == KDialog::Accepted)
    {
        NewNamesList newNamesList = dlg->newNames();

/*
 * EXPERMINENTAL DIR CREATION / Turned off for now
 */
//        we need to check if the destination folder(s) exist, if not, we ask the user if the folder should be created
//        DatabaseAccess access;
//
//        foreach (NewNameInfo newNameInfo, newNamesList)
//        {
//            ImageInfo info(newNameInfo.first);
//            KUrl _url = info.databaseUrl();
//
//            Digikam::DatabaseUrl dbUrlSrc(_url);
//            _url.setFileName(newNameInfo.second);
//            Digikam::DatabaseUrl dbUrlDst(_url);
//
//            int dstAlbumID = -1;
//            dstAlbumID     = access.db()->getAlbumForPath(dbUrlDst.albumRootId(), dbUrlDst.album(), false);
//
//            if (dstAlbumID == -1)
//            {
//                // if the new file name contains slashes, split the string and create each album separately
//                QFileInfo tmp(newNameInfo.second);
//                QStringList albums2BeCreated = tmp.path().split(QRegExp("\\/+"), QString::SkipEmptyParts);
//
//                PAlbum *parentPAlbum = AlbumManager::instance()->findPAlbum(info.albumId());
//                foreach (const QString &newAlbum, albums2BeCreated)
//                {
//                    PAlbum *newPAlbum = 0;
//                    QString errorMsg;
//
//                    newPAlbum = AlbumManager::instance()->createPAlbum(parentPAlbum,
//                                                                       newAlbum,
//                                                                       QString(),
//                                                                       QDate::currentDate(),
//                                                                       QString(),
//                                                                       errorMsg
//                    );
//
//                    if (newPAlbum == 0)
//                    {
//                        KMessageBox::error(this, errorMsg, i18n("Error"));
//                        return;
//                    }
//
//                    // all operations went fine, set the new album as parent
//                    parentPAlbum = newPAlbum;
//                }
//            }
//        }

        d->renameThread->addNewNames(newNamesList);
        if (!d->renameThread->isRunning())
        {
            d->renameThread->start();
        }
    }
    delete dlg;
}

void DigikamImageView::slotRotateLeft()
{
    KActionMenu* action = dynamic_cast<KActionMenu*>(ContextMenuHelper::kipiRotateAction());
    if (action)
    {
        QList<QAction*> list = action->menu()->actions();
        foreach(QAction* ac, list)
        {
            if (ac->objectName() == QString("rotate_ccw"))
                ac->trigger();
        }
    }
}

void DigikamImageView::slotRotateRight()
{
    KActionMenu* action = dynamic_cast<KActionMenu*>(ContextMenuHelper::kipiRotateAction());
    if (action)
    {
        QList<QAction*> list = action->menu()->actions();
        foreach(QAction* ac, list)
        {
            if (ac->objectName() == QString("rotate_cw"))
                ac->trigger();
        }
    }
}

} // namespace Digikam
