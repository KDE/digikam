/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-02-12
 * Description : digiKam image editor GUI
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2004-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "imagewindow.h"
#include "imagewindow_p.h"

namespace Digikam
{

ImageWindow* ImageWindow::m_instance = 0;

ImageWindow* ImageWindow::imageWindow()
{
    if (!m_instance)
    {
        new ImageWindow();
    }

    return m_instance;
}

bool ImageWindow::imageWindowCreated()
{
    return m_instance;
}

ImageWindow::ImageWindow()
    : EditorWindow(QLatin1String("Image Editor")),
      d(new Private)
{
    setXMLFile(QLatin1String("imageeditorui5.rc"));

    m_instance = this;
    // We don't want to be deleted on close
    setAttribute(Qt::WA_DeleteOnClose, false);
    setAcceptDrops(true);

    // -- Build the GUI -------------------------------

    setupUserArea();
    setupActions();
    setupStatusBar();
    createGUI(xmlFile());
    cleanupActions();

    showMenuBarAction()->setChecked(!menuBar()->isHidden());  // NOTE: workaround for bug #171080

    // Create tool selection view

    setupSelectToolsAction();

    // Create context menu.

    setupContextMenu();

    // Make signals/slots connections

    setupConnections();

    // -- Read settings --------------------------------

    readSettings();
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(configGroupName());
    applyMainWindowSettings(group);
    d->thumbBarDock->setShouldBeVisible(group.readEntry(d->configShowThumbbarEntry, false));
    setAutoSaveSettings(configGroupName(), true);
    d->viewContainer->setAutoSaveSettings(QLatin1String("ImageViewer Thumbbar"), true);

    //-------------------------------------------------------------

    d->rightSideBar->setConfigGroup(KConfigGroup(&group, QLatin1String("Right Sidebar")));
    d->rightSideBar->loadState();
    d->rightSideBar->populateTags();

    slotSetupChanged();
}

ImageWindow::~ImageWindow()
{
    m_instance = 0;

    delete d->rightSideBar;
    delete d->thumbBar;
    delete d;
}

void ImageWindow::closeEvent(QCloseEvent* e)
{
    if (!queryClose())
    {
        e->ignore();
        return;
    }

    // put right side bar in a defined state
    emit signalNoCurrentItem();

    m_canvas->resetImage();

    // There is one nasty habit with the thumbnail bar if it is floating: it
    // doesn't close when the parent window does, so it needs to be manually
    // closed. If the light table is opened again, its original state needs to
    // be restored.
    // This only needs to be done when closing a visible window and not when
    // destroying a closed window, since the latter case will always report that
    // the thumbnail bar isn't visible.
    if (isVisible())
    {
        thumbBar()->hide();
    }

    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(configGroupName());
    d->rightSideBar->setConfigGroup(KConfigGroup(&group, "Right Sidebar"));
    d->rightSideBar->saveState();
    saveSettings();

    DXmlGuiWindow::closeEvent(e);
}

void ImageWindow::showEvent(QShowEvent*)
{
    // Restore the visibility of the thumbbar and start autosaving again.
    thumbBar()->restoreVisibility();
}

bool ImageWindow::queryClose()
{
    // Note: we re-implement closeEvent above for this window.
    // Additionally, queryClose is called from DigikamApp.

    // wait if a save operation is currently running
    if (!waitForSavingToComplete())
    {
        return false;
    }

    return promptUserSave(d->currentUrl());
}

void ImageWindow::loadItemInfos(const ItemInfoList& imageInfoList, const ItemInfo& imageInfoCurrent,
                                 const QString& caption)
{
    // Very first thing is to check for changes, user may choose to cancel operation
    if (!promptUserSave(d->currentUrl(), AskIfNeeded))
    {
        return;
    }

    d->currentItemInfo = imageInfoCurrent;

    // Note: Addition is asynchronous, indexes not yet available
    // We enable thumbbar as soon as indexes are available
    // If not, we load imageInfoCurrent, then the index 0, then again imageInfoCurrent
    d->thumbBar->setEnabled(false);
    d->imageInfoModel->setItemInfos(imageInfoList);
    d->setThumbBarToCurrent();

    if (!caption.isEmpty())
    {
        setCaption(i18n("Image Editor - %1", caption));
    }
    else
    {
        setCaption(i18n("Image Editor"));
    }

    // it can slightly improve the responsiveness when inserting an event loop run here
    QTimer::singleShot(0, this, SLOT(slotLoadItemInfosStage2()));
}

void ImageWindow::slotLoadItemInfosStage2()
{
    // if window is minimized, show it
    if (isMinimized())
    {
        KWindowSystem::unminimizeWindow(winId());
    }

    slotLoadCurrent();
}

void ImageWindow::slotThumbBarModelReady()
{
    d->thumbBar->setEnabled(true);
}

void ImageWindow::openImage(const ItemInfo& info)
{
    if (d->currentItemInfo == info)
    {
        return;
    }

    d->currentItemInfo = info;
    d->ensureModelContains(d->currentItemInfo);

    slotLoadCurrent();
}

void ImageWindow::slotLoadCurrent()
{
    if (!d->currentIsValid())
    {
        return;
    }

    m_canvas->load(d->currentItemInfo.filePath(), m_IOFileSettings);

    QModelIndex next = d->nextIndex();

    if (next.isValid())
    {
        m_canvas->preload(d->imageInfo(next).filePath());
    }

    d->setThumbBarToCurrent();

    // Do this _after_ the canvas->load(), so that the main view histogram does not load
    // a smaller version if a raw image, and after that the EditorCore loads the full version.
    // So first let EditorCore create its loading task, only then any external objects.
    setViewToURL(d->currentUrl());
}

void ImageWindow::setViewToURL(const QUrl& url)
{
    emit signalURLChanged(url);
}

void ImageWindow::slotThumbBarImageSelected(const ItemInfo& info)
{
    if (d->currentItemInfo == info || !d->thumbBar->isEnabled())
    {
        return;
    }

    if (!promptUserSave(d->currentUrl(), AskIfNeeded, false))
    {
        return;
    }

    d->currentItemInfo = info;
    slotLoadCurrent();
}

void ImageWindow::slotDroppedOnThumbbar(const QList<ItemInfo>& infos)
{
    // Check whether dropped image list is empty

    if (infos.isEmpty())
    {
        return;
    }

    // Create new list and images that are not present currently in the thumbbar

    QList<ItemInfo> toAdd;

    foreach (const ItemInfo& it, infos)
    {
        QModelIndex index(d->imageFilterModel->indexForItemInfo(it));

        if (!index.isValid())
        {
            toAdd.append(it);
        }
    }

    // Loading images if new images are dropped

    if (!toAdd.isEmpty())
    {
        loadItemInfos(ItemInfoList(toAdd), toAdd.first(), QString());
    }
}

void ImageWindow::slotFileOriginChanged(const QString& filePath)
{
    // By redo or undo, we have virtually switched to a new image.
    // So we do _not_ load anything!
    ItemInfo newCurrent = ItemInfo::fromLocalFile(filePath);

    if (newCurrent.isNull() || !d->imageInfoModel->hasImage(newCurrent))
    {
        return;
    }

    d->currentItemInfo = newCurrent;
    d->setThumbBarToCurrent();
    setViewToURL(d->currentUrl());
}

void ImageWindow::loadIndex(const QModelIndex& index)
{
    if (!promptUserSave(d->currentUrl(), AskIfNeeded))
    {
        return;
    }

    if (!index.isValid())
    {
        return;
    }

    d->currentItemInfo = d->imageFilterModel->imageInfo(index);
    slotLoadCurrent();
}

void ImageWindow::slotForward()
{
    loadIndex(d->nextIndex());
}

void ImageWindow::slotBackward()
{
    loadIndex(d->previousIndex());
}

void ImageWindow::slotFirst()
{
    loadIndex(d->firstIndex());
}

void ImageWindow::slotLast()
{
    loadIndex(d->lastIndex());
}

void ImageWindow::slotChanged()
{
    QString mpixels;
    QSize dims(m_canvas->imageWidth(), m_canvas->imageHeight());
    mpixels.setNum(dims.width()*dims.height() / 1000000.0, 'f', 2);
    QString str = (!dims.isValid()) ? i18n("Unknown") : i18n("%1x%2 (%3Mpx)",
                                                             dims.width(), dims.height(), mpixels);

    m_resLabel->setAdjustedText(str);

    if (!d->currentIsValid())
    {
        return;
    }

    DImg* const img           = m_canvas->interface()->getImg();
    DImageHistory history     = m_canvas->interface()->getItemHistory();
    DImageHistory redoHistory = m_canvas->interface()->getImageHistoryOfFullRedo();

    d->rightSideBar->itemChanged(d->currentItemInfo, m_canvas->getSelectedArea(), img, redoHistory);

    // Filters for redo will be turn in grey out
    d->rightSideBar->getFiltersHistoryTab()->setEnabledHistorySteps(history.actionCount());

/*
    if (!d->currentItemInfo.isNull())
    {
    }
    else
    {
        d->rightSideBar->itemChanged(d->currentUrl(), m_canvas->getSelectedArea(), img);
    }
*/
}

void ImageWindow::slotToggleTag(const QUrl& url, int tagID)
{
    toggleTag(ItemInfo::fromUrl(url), tagID);
}

void ImageWindow::toggleTag(int tagID)
{
    toggleTag(d->currentItemInfo, tagID);
}

void ImageWindow::toggleTag(const ItemInfo& info, int tagID)
{
    if (!info.isNull())
    {
        if (info.tagIds().contains(tagID))
        {
            FileActionMngr::instance()->removeTag(info, tagID);
        }
        else
        {
            FileActionMngr::instance()->assignTag(info, tagID);
        }
    }
}

void ImageWindow::slotAssignTag(int tagID)
{
    if (!d->currentItemInfo.isNull())
    {
        FileActionMngr::instance()->assignTag(d->currentItemInfo, tagID);
    }
}

void ImageWindow::slotRemoveTag(int tagID)
{
    if (!d->currentItemInfo.isNull())
    {
        FileActionMngr::instance()->removeTag(d->currentItemInfo, tagID);
    }
}

void ImageWindow::slotAssignPickLabel(int pickId)
{
    assignPickLabel(d->currentItemInfo, pickId);
}

void ImageWindow::slotAssignColorLabel(int colorId)
{
    assignColorLabel(d->currentItemInfo, colorId);
}

void ImageWindow::assignPickLabel(const ItemInfo& info, int pickId)
{
    if (!info.isNull())
    {
        FileActionMngr::instance()->assignPickLabel(info, pickId);
    }
}

void ImageWindow::assignColorLabel(const ItemInfo& info, int colorId)
{
    if (!info.isNull())
    {
        FileActionMngr::instance()->assignColorLabel(info, colorId);
    }
}

void ImageWindow::slotAssignRating(int rating)
{
    assignRating(d->currentItemInfo, rating);
}

void ImageWindow::assignRating(const ItemInfo& info, int rating)
{
    rating = qMin(RatingMax, qMax(RatingMin, rating));

    if (!info.isNull())
    {
        FileActionMngr::instance()->assignRating(info, rating);
    }
}

void ImageWindow::slotRatingChanged(const QUrl& url, int rating)
{
    assignRating(ItemInfo::fromUrl(url), rating);
}

void ImageWindow::slotColorLabelChanged(const QUrl& url, int color)
{
    assignColorLabel(ItemInfo::fromUrl(url), color);
}

void ImageWindow::slotPickLabelChanged(const QUrl& url, int pick)
{
    assignPickLabel(ItemInfo::fromUrl(url), pick);
}

void ImageWindow::slotUpdateItemInfo()
{
    QString text = i18nc("<Image file name> (<Image number> of <Images in album>)",
                         "%1 (%2 of %3)", d->currentItemInfo.name(),
                         d->currentIndex().row() + 1,
                         d->imageFilterModel->rowCount());
    m_nameLabel->setText(text);

    if (!m_actionEnabledState)
    {
        return;
    }

    if (d->imageInfoModel->rowCount() == 1)
    {
        m_backwardAction->setEnabled(false);
        m_forwardAction->setEnabled(false);
        m_firstAction->setEnabled(false);
        m_lastAction->setEnabled(false);
    }
    else
    {
        m_backwardAction->setEnabled(true);
        m_forwardAction->setEnabled(true);
        m_firstAction->setEnabled(true);
        m_lastAction->setEnabled(true);
    }

    if (d->currentIndex() == d->firstIndex())
    {
        m_backwardAction->setEnabled(false);
        m_firstAction->setEnabled(false);
    }

    if (d->currentIndex() == d->lastIndex())
    {
        m_forwardAction->setEnabled(false);
        m_lastAction->setEnabled(false);
    }

/*
    // Disable some menu actions if the current root image URL
    // is not include in the digiKam Albums library database.
    // This is necessary when ImageEditor is opened from cameraclient.

    QUrl u(d->currentUrl().directory());
    PAlbum* palbum = AlbumManager::instance()->findPAlbum(u);

    if (!palbum)
    {
        m_fileDeleteAction->setEnabled(false);
    }
    else
    {
        m_fileDeleteAction->setEnabled(true);
    }
*/
}

void ImageWindow::slotToMainWindow()
{
    close();
}

void ImageWindow::saveIsComplete()
{
    // With save(), we do not reload the image but just continue using the data.
    // This means that a saving operation does not lead to quality loss for
    // subsequent editing operations.

    // put image in cache, the LoadingCacheInterface cares for the details
    LoadingCacheInterface::putImage(m_savingContext.destinationURL.toLocalFile(), m_canvas->currentImage());
    ItemInfo info = ScanController::instance()->scannedInfo(m_savingContext.destinationURL.toLocalFile());

    // Save new face tags to the image
    saveFaceTagsToImage(info);

    // reset the orientation flag in the database
    DMetadata meta(m_canvas->currentImage().getMetadata());
    d->currentItemInfo.setOrientation(meta.getItemOrientation());

    // Pop-up a message to bring user when save is done.
    DNotificationWrapper(QLatin1String("editorsavefilecompleted"), i18n("Image saved successfully"),
                         this, windowTitle());

    resetOrigin();

    QModelIndex next = d->nextIndex();

    if (next.isValid())
    {
        m_canvas->preload(d->imageInfo(next).filePath());
    }

    slotUpdateItemInfo();
    setViewToURL(d->currentItemInfo.fileUrl());
}

void ImageWindow::saveVersionIsComplete()
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "save version done";
    saveAsIsComplete();
}

void ImageWindow::saveAsIsComplete()
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "Saved" << m_savingContext.srcURL << "to" << m_savingContext.destinationURL;

    // Nothing to be done if operating without database
    if (d->currentItemInfo.isNull())
    {
        return;
    }

    if (CollectionManager::instance()->albumRootPath(m_savingContext.srcURL).isNull() ||
        CollectionManager::instance()->albumRootPath(m_savingContext.destinationURL).isNull())
    {
        // not in-collection operation - nothing to do
        return;
    }

    // copy the metadata of the original file to the new file
    qCDebug(DIGIKAM_GENERAL_LOG) << "was versioned"
             << (m_savingContext.executedOperation == SavingContext::SavingStateVersion)
             << "current" << d->currentItemInfo.id() << d->currentItemInfo.name()
             << "destinations" << m_savingContext.versionFileOperation.allFilePaths();

    ItemInfo sourceInfo = d->currentItemInfo;
    // Set new current index. Employ synchronous scanning for this main file.
    d->currentItemInfo = ScanController::instance()->scannedInfo(m_savingContext.destinationURL.toLocalFile());

    if (m_savingContext.destinationExisted)
    {
        // reset the orientation flag in the database
        DMetadata meta(m_canvas->currentImage().getMetadata());
        d->currentItemInfo.setOrientation(meta.getItemOrientation());
    }

    QStringList derivedFilePaths;

    if (m_savingContext.executedOperation == SavingContext::SavingStateVersion)
    {
        derivedFilePaths = m_savingContext.versionFileOperation.allFilePaths();
    }
    else
    {
        derivedFilePaths << m_savingContext.destinationURL.toLocalFile();
    }

    // Will ensure files are scanned, and then copy attributes in a thread
    FileActionMngr::instance()->copyAttributes(sourceInfo, derivedFilePaths);

    // The model updates asynchronously, so we need to force addition of the main entry
    d->ensureModelContains(d->currentItemInfo);

    // Save new face tags to the image
    saveFaceTagsToImage(d->currentItemInfo);

    // set origin of EditorCore: "As if" the last saved image was loaded directly
    resetOriginSwitchFile();

    // If the DImg is put in the cache under the new name, this means the new file will not be reloaded.
    // This may irritate users who want to check for quality loss in lossy formats.
    // In any case, only do that if the format did not change - too many assumptions otherwise (see bug #138949).
    if (m_savingContext.originalFormat == m_savingContext.format)
    {
        LoadingCacheInterface::putImage(m_savingContext.destinationURL.toLocalFile(), m_canvas->currentImage());
    }

    // all that is done in slotLoadCurrent, except for loading

    d->thumbBar->setCurrentIndex(d->currentIndex());

    QModelIndex next = d->nextIndex();

    if (next.isValid())
    {
        m_canvas->preload(d->imageInfo(next).filePath());
    }

    setViewToURL(d->currentItemInfo.fileUrl());

    slotUpdateItemInfo();

    // Pop-up a message to bring user when save is done.
    DNotificationWrapper(QLatin1String("editorsavefilecompleted"), i18n("Image saved successfully"),
                         this, windowTitle());
}

void ImageWindow::prepareImageToSave()
{
    if (!d->currentItemInfo.isNull())
    {
        MetadataHub hub;
        hub.load(d->currentItemInfo);

        // Get face tags
        d->newFaceTags.clear();
        QMultiMap<QString, QVariant> faceTags;
        faceTags = hub.loadIntegerFaceTags(d->currentItemInfo);

        if (!faceTags.isEmpty())
        {
            QSize tempS = d->currentItemInfo.dimensions();
            QMap<QString, QVariant>::const_iterator it;

            for (it = faceTags.constBegin() ; it != faceTags.constEnd() ; ++it)
            {
                // Start transform each face rect
                QRect faceRect = it.value().toRect();
                int   tempH    = tempS.height();
                int   tempW    = tempS.width();

                qCDebug(DIGIKAM_GENERAL_LOG) << ">>>>>>>>>face rect before:"
                                             << faceRect.x()     << faceRect.y()
                                             << faceRect.width() << faceRect.height();

                for (int i = 0 ; i < m_transformQue.size() ; i++)
                {
                    EditorWindow::TransformType type = m_transformQue[i];

                    switch (type)
                    {
                        case EditorWindow::TransformType::RotateLeft:
                            faceRect = TagRegion::ajustToRotatedImg(faceRect, QSize(tempW, tempH), 1);
                            std::swap(tempH, tempW);
                            break;
                        case EditorWindow::TransformType::RotateRight:
                            faceRect = TagRegion::ajustToRotatedImg(faceRect, QSize(tempW, tempH), 0);
                            std::swap(tempH, tempW);
                            break;
                        case EditorWindow::TransformType::FlipHorizontal:
                            faceRect = TagRegion::ajustToFlippedImg(faceRect, QSize(tempW, tempH), 0);
                            break;
                        case EditorWindow::TransformType::FlipVertical:
                            faceRect = TagRegion::ajustToFlippedImg(faceRect, QSize(tempW, tempH), 1);
                            break;
                        default:
                            break;
                    }

                    qCDebug(DIGIKAM_GENERAL_LOG) << ">>>>>>>>>face rect transform:"
                                                 << faceRect.x()     << faceRect.y()
                                                 << faceRect.width() << faceRect.height();
                }

                d->newFaceTags.insertMulti(it.key(), QVariant(faceRect));
            }
        }

        // Ensure there is a UUID for the source image in the database,
        // even if not in the source image's metadata
        if (d->currentItemInfo.uuid().isNull())
        {
            QString uuid = m_canvas->interface()->ensureHasCurrentUuid();
            d->currentItemInfo.setUuid(uuid);
        }
        else
        {
            m_canvas->interface()->provideCurrentUuid(d->currentItemInfo.uuid());
        }
    }
}

void ImageWindow::saveFaceTagsToImage(const ItemInfo& info)
{
    if (!info.isNull() && !d->newFaceTags.isEmpty())
    {
        // Delete all old faces
        FaceTagsEditor().removeAllFaces(info.id());

        QMap<QString, QVariant>::const_iterator it;

        for (it = d->newFaceTags.constBegin() ; it != d->newFaceTags.constEnd() ; ++it)
        {
            int tagId = FaceTags::getOrCreateTagForPerson(it.key());

            if (tagId)
            {
                TagRegion region(it.value().toRect());
                FaceTagsEditor().add(info.id(), tagId, region, false);
            }
            else
            {
                qCDebug(DIGIKAM_GENERAL_LOG) << "Failed to create a person tag for name" << it.key();
            }
        }

        MetadataHub hub;
        hub.load(info);
        QSize tempS = info.dimensions();
        hub.setFaceTags(d->newFaceTags, tempS);
        hub.write(info.filePath(), MetadataHub::WRITE_ALL);
    }

    m_transformQue.clear();
    d->newFaceTags.clear();
}

VersionManager* ImageWindow::versionManager() const
{
    return &d->versionManager;
}

bool ImageWindow::save()
{
    prepareImageToSave();
    startingSave(d->currentUrl());
    return true;
}

bool ImageWindow::saveAs()
{
    prepareImageToSave();
    return startingSaveAs(d->currentUrl());
}

bool ImageWindow::saveNewVersion()
{
    prepareImageToSave();
    return startingSaveNewVersion(d->currentUrl());
}

bool ImageWindow::saveCurrentVersion()
{
    prepareImageToSave();
    return startingSaveCurrentVersion(d->currentUrl());
}

bool ImageWindow::saveNewVersionAs()
{
    prepareImageToSave();
    return startingSaveNewVersionAs(d->currentUrl());
}

bool ImageWindow::saveNewVersionInFormat(const QString& format)
{
    prepareImageToSave();
    return startingSaveNewVersionInFormat(d->currentUrl(), format);
}

QUrl ImageWindow::saveDestinationUrl()
{
    return d->currentUrl();
}

void ImageWindow::slotDeleteCurrentItem()
{
    deleteCurrentItem(true, false);
}

void ImageWindow::slotDeleteCurrentItemPermanently()
{
    deleteCurrentItem(true, true);
}

void ImageWindow::slotDeleteCurrentItemPermanentlyDirectly()
{
    deleteCurrentItem(false, true);
}

void ImageWindow::slotTrashCurrentItemDirectly()
{
    deleteCurrentItem(false, false);
}

void ImageWindow::deleteCurrentItem(bool ask, bool permanently)
{
    // This function implements all four of the above slots.
    // The meaning of permanently differs depending on the value of ask

    if (d->currentItemInfo.isNull())
    {
        return;
    }

    if (!promptUserDelete(d->currentUrl()))
    {
        return;
    }

    bool useTrash;

    if (ask)
    {
        bool preselectDeletePermanently = permanently;

        DeleteDialog dialog(this);

        QList<QUrl> urlList;
        urlList << d->currentUrl();

        if (!dialog.confirmDeleteList(urlList,
                                      DeleteDialogMode::Files,
                                      preselectDeletePermanently ?
                                      DeleteDialogMode::NoChoiceDeletePermanently : DeleteDialogMode::NoChoiceTrash))
        {
            return;
        }

        useTrash = !dialog.shouldDelete();
    }
    else
    {
        useTrash = !permanently;
    }

    DIO::del(d->currentItemInfo, useTrash);

    // bring all (sidebar) to a defined state without letting them sit on the deleted file
    emit signalNoCurrentItem();

    // We have database information, which means information will get through
    // everywhere. Just do it asynchronously.

    removeCurrent();
}

void ImageWindow::removeCurrent()
{
    if (!d->currentIsValid())
    {
        return;
    }

    if (m_canvas->interface()->undoState().hasChanges)
    {
        m_canvas->slotRestore();
    }

    d->imageInfoModel->removeItemInfo(d->currentItemInfo);

    if (d->imageInfoModel->isEmpty())
    {
        // No image in the current Album -> Quit ImageEditor...

        QMessageBox::information(this, i18n("No Image in Current Album"),
                                 i18n("There is no image to show in the current album.\n"
                                      "The image editor will be closed."));

        close();
    }
}

void ImageWindow::slotFileMetadataChanged(const QUrl& url)
{
    if (url == d->currentUrl())
    {
        m_canvas->interface()->readMetadataFromFile(url.toLocalFile());
    }
}

/*
 * Should all be done by ItemViewCategorized
 *
void ImageWindow::slotRowsAboutToBeRemoved(const QModelIndex& parent, int start, int end)
{

// ignore when closed
if (!isVisible() || !d->currentIsValid())
{
    return;
}

QModelIndex currentIndex = d->currentIndex();
if (currentIndex.row() >= start && currentIndex.row() <= end)
{
    promptUserSave(d->currentUrl(), AlwaysNewVersion, false);

    // ensure selection
    int totalToRemove = end - start + 1;
    if (d->imageFilterModel->rowCount(parent) > totalToRemove)
    {
        if (end == d->imageFilterModel->rowCount(parent) - 1)
        {
            loadIndex(d->imageFilterModel->index(start - 1, 0));    // last remaining, no next one left
        }
        else
        {
            loadIndex(d->imageFilterModel->index(end + 1, 0));    // next remaining
        }
    }
}
}*/

/*
void ImageWindow::slotCollectionImageChange(const CollectionImageChangeset& changeset)
{

    bool needLoadCurrent = false;

    switch (changeset.operation())
    {
        case CollectionImageChangeset::Removed:

            for (int i=0; i<d->imageInfoList.size(); ++i)
            {
                if (changeset.containsImage(d->imageInfoList[i].id()))
                {
                    if (d->currentItemInfo == d->imageInfoList[i])
                    {
                        promptUserSave(d->currentUrl(), AlwaysNewVersion, false);

                        if (removeItem(i))
                        {
                            needLoadCurrent = true;
                        }
                    }
                    else
                    {
                        removeItem(i);
                    }

                    --i;
                }
            }

            break;
        case CollectionImageChangeset::RemovedAll:

            for (int i=0; i<d->imageInfoList.size(); ++i)
            {
                if (changeset.containsAlbum(d->imageInfoList[i].albumId()))
                {
                    if (d->currentItemInfo == d->imageInfoList[i])
                    {
                        promptUserSave(d->currentUrl(), AlwaysNewVersion, false);

                        if (removeItem(i))
                        {
                            needLoadCurrent = true;
                        }
                    }
                    else
                    {
                        removeItem(i);
                    }

                    --i;
                }
            }

            break;
        default:
            break;
    }

    if (needLoadCurrent)
    {
        QTimer::singleShot(0, this, SLOT(slotLoadCurrent()));
    }
}
*/

void ImageWindow::dragMoveEvent(QDragMoveEvent* e)
{
    int              albumID;
    QList<int>       albumIDs;
    QList<qlonglong> imageIDs;
    QList<QUrl>      urls;

    if (DItemDrag::decode(e->mimeData(), urls, albumIDs, imageIDs) ||
        DAlbumDrag::decode(e->mimeData(), urls, albumID)                    ||
        DTagListDrag::canDecode(e->mimeData()))
    {
        e->accept();
        return;
    }

    e->ignore();
}

void ImageWindow::dropEvent(QDropEvent* e)
{
    int              albumID;
    QList<int>       albumIDs;
    QList<qlonglong> imageIDs;
    QList<QUrl>      urls;

    if (DItemDrag::decode(e->mimeData(), urls, albumIDs, imageIDs))
    {
        ItemInfoList imageInfoList(imageIDs);

        if (imageInfoList.isEmpty())
        {
            e->ignore();
            return;
        }

        QString ATitle;
        AlbumManager* const man = AlbumManager::instance();
        PAlbum* const palbum    = man->findPAlbum(albumIDs.first());

        if (palbum)
        {
            ATitle = palbum->title();
        }

        loadItemInfos(imageInfoList, imageInfoList.first(),
                       i18n("Album \"%1\"", ATitle));
        e->accept();
    }
    else if (DAlbumDrag::decode(e->mimeData(), urls, albumID))
    {
        AlbumManager* const man  = AlbumManager::instance();
        QList<qlonglong> itemIDs = CoreDbAccess().db()->getItemIDsInAlbum(albumID);
        ItemInfoList imageInfoList(itemIDs);

        if (imageInfoList.isEmpty())
        {
            e->ignore();
            return;
        }

        QString ATitle;
        PAlbum* const palbum = man->findPAlbum(albumIDs.first());

        if (palbum)
        {
            ATitle = palbum->title();
        }

        loadItemInfos(imageInfoList, imageInfoList.first(),
                       i18n("Album \"%1\"", ATitle));
        e->accept();
    }
    else if (DTagListDrag::canDecode(e->mimeData()))
    {
        QList<int> tagIDs;

        if (!DTagListDrag::decode(e->mimeData(), tagIDs))
        {
            return;
        }

        AlbumManager* const man  = AlbumManager::instance();
        QList<qlonglong> itemIDs = CoreDbAccess().db()->getItemIDsInTag(tagIDs.first(), true);
        ItemInfoList imageInfoList(itemIDs);

        if (imageInfoList.isEmpty())
        {
            e->ignore();
            return;
        }

        QString ATitle;
        TAlbum* const talbum = man->findTAlbum(tagIDs.first());

        if (talbum)
        {
            ATitle = talbum->title();
        }

        loadItemInfos(imageInfoList, imageInfoList.first(),
                       i18n("Album \"%1\"", ATitle));
        e->accept();
    }
    else
    {
        e->ignore();
    }
}

void ImageWindow::slotRevert()
{
    if (!promptUserSave(d->currentUrl(), AskIfNeeded))
    {
        return;
    }

    if (m_canvas->interface()->undoState().hasChanges)
    {
        m_canvas->slotRestore();
    }
}

void ImageWindow::slotOpenOriginal()
{
    if (!hasOriginalToRestore())
    {
        return;
    }

    if (!promptUserSave(d->currentUrl(), AskIfNeeded))
    {
        return;
    }

    // this time, with mustBeAvailable = true
    DImageHistory availableResolved = ItemScanner::resolvedImageHistory(m_canvas->interface()->getItemHistory(), true);

    QList<HistoryImageId> originals = availableResolved.referredImagesOfType(HistoryImageId::Original);
    HistoryImageId originalId       = m_canvas->interface()->getItemHistory().originalReferredImage();

    if (originals.isEmpty())
    {
        //TODO: point to remote collection
        QMessageBox::warning(this, i18nc("@title", "File Not Available"),
                             i18nc("@info", "<qt>The original file (<b>%1</b>) is currently not available</qt>",
                                   originalId.m_fileName));
        return;
    }

    QList<ItemInfo> imageInfos;

    foreach(const HistoryImageId& id, originals)
    {
        QUrl url = QUrl::fromLocalFile(id.m_filePath);
        url      = url.adjusted(QUrl::StripTrailingSlash);
        url.setPath(url.path() + QLatin1Char('/') + (id.m_fileName));
        imageInfos << ItemInfo::fromUrl(url);
    }

    ItemScanner::sortByProximity(imageInfos, d->currentItemInfo);

    if (!imageInfos.isEmpty() && !imageInfos.first().isNull())
    {
        openImage(imageInfos.first());
    }
}

bool ImageWindow::hasOriginalToRestore()
{
    // not implemented for db-less situation, so check for ItemInfo
    return !d->currentItemInfo.isNull() && EditorWindow::hasOriginalToRestore();
}

DImageHistory ImageWindow::resolvedImageHistory(const DImageHistory& history)
{
    return ItemScanner::resolvedImageHistory(history);
}

ThumbBarDock* ImageWindow::thumbBar() const
{
    return d->thumbBarDock;
}

Sidebar* ImageWindow::rightSideBar() const
{
    return (dynamic_cast<Sidebar*>(d->rightSideBar));
}

void ImageWindow::slotComponentsInfo()
{
    showDigikamComponentsInfo();
}

void ImageWindow::slotDBStat()
{
    showDigikamDatabaseStat();
}

void ImageWindow::slotAddedDropedItems(QDropEvent* e)
{
    int              albumID;
    QList<int>       albumIDs;
    QList<qlonglong> imageIDs;
    QList<QUrl>      urls;
    ItemInfoList    imgList;

    if (DItemDrag::decode(e->mimeData(), urls, albumIDs, imageIDs))
    {
        imgList = ItemInfoList(imageIDs);
    }
    else if (DAlbumDrag::decode(e->mimeData(), urls, albumID))
    {
        QList<qlonglong> itemIDs = CoreDbAccess().db()->getItemIDsInAlbum(albumID);

        imgList = ItemInfoList(itemIDs);
    }
    else if (DTagListDrag::canDecode(e->mimeData()))
    {
        QList<int> tagIDs;

        if (!DTagListDrag::decode(e->mimeData(), tagIDs))
        {
            return;
        }

        QList<qlonglong> itemIDs = CoreDbAccess().db()->getItemIDsInTag(tagIDs.first(), true);
        imgList = ItemInfoList(itemIDs);
    }

    e->accept();

    if (!imgList.isEmpty())
    {
        loadItemInfos(imgList, imgList.first(), QString());
    }
}

void ImageWindow::slotFileWithDefaultApplication()
{
    DFileOperations::openFilesWithDefaultApplication(QList<QUrl>() << d->currentUrl());
}

void ImageWindow::slotOpenWith(QAction* action)
{
    openWith(d->currentUrl(), action);
}

void ImageWindow::slotRightSideBarActivateTitles()
{
    d->rightSideBar->setActiveTab(d->rightSideBar->imageDescEditTab());
    d->rightSideBar->imageDescEditTab()->setFocusToTitlesEdit();
}

void ImageWindow::slotRightSideBarActivateComments()
{
    d->rightSideBar->setActiveTab(d->rightSideBar->imageDescEditTab());
    d->rightSideBar->imageDescEditTab()->setFocusToCommentsEdit();
}

void ImageWindow::slotRightSideBarActivateAssignedTags()
{
    d->rightSideBar->setActiveTab(d->rightSideBar->imageDescEditTab());
    d->rightSideBar->imageDescEditTab()->activateAssignedTagsButton();
}

} // namespace Digikam
