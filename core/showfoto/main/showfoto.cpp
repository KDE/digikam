/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-11-22
 * Description : stand alone digiKam image editor
 *
 * Copyright (C) 2004-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2009-2011 by Andi Clemens <andi dot clemens at gmail dot com>
 * Copyright (C) 2004-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2005-2006 by Tom Albers <tomalbers at kde dot nl>
 * Copyright (C) 2008      by Arnd Baecker <arnd dot baecker at web dot de>
 * Copyright (C) 2013-2015 by Mohamed_Anwer <m_dot_anwer at gmx dot com>
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

#include "showfoto.h"
#include "showfoto_p.h"

namespace ShowFoto
{

ShowFoto::ShowFoto(const QList<QUrl>& urlList)
    : Digikam::EditorWindow(QLatin1String("Showfoto")),
      d(new Private)
{
    setXMLFile(QLatin1String("showfotoui5.rc"));

    m_nonDestructive = false;

    // Show splash-screen at start up.

    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(configGroupName());

    if (group.readEntry(QLatin1String("ShowSplash"), true) && !qApp->isSessionRestored())
    {
        d->splash = new Digikam::DSplashScreen();
        d->splash->show();
    }

    // Setup loading cache and thumbnails interface.

    Digikam::LoadingCacheInterface::initialize();
    Digikam::MetaEngineSettings::instance();

    d->thumbLoadThread = new Digikam::ThumbnailLoadThread();
    d->thumbLoadThread->setThumbnailSize(Digikam::ThumbnailSize::Huge);
    d->thumbLoadThread->setSendSurrogatePixmap(true);

    // Check ICC profiles repository availability

    if (d->splash)
    {
        d->splash->setMessage(i18n("Checking ICC repository..."));
    }

    d->validIccPath = Digikam::SetupICC::iccRepositoryIsValid();

    // Populate Themes

    if (d->splash)
    {
        d->splash->setMessage(i18n("Loading themes..."));
    }

    Digikam::ThemeManager::instance();

    // Load plugins

    if (d->splash)
    {
        d->splash->setMessage(i18n("Load Plugins..."));
    }

    DPluginLoader* const dpl = DPluginLoader::instance();
    dpl->init();

    // -- Build the GUI -----------------------------------

    setupUserArea();
    setupActions();
    setupStatusBar();
    createGUI(xmlFile());
    cleanupActions();

    // Create tool selection view

    setupSelectToolsAction();

    // Create context menu.

    setupContextMenu();

    // Make signals/slots connections

    setupConnections();

    // -- Read settings --------------------------------

    readSettings();
    applySettings();
    setAutoSaveSettings(configGroupName(), true);

    d->rightSideBar->loadState();

    //--------------------------------------------------

    d->thumbBarDock->reInitialize();

    // -- Load current items ---------------------------

    slotDroppedUrls(urlList, false);
}

ShowFoto::~ShowFoto()
{
    delete m_canvas;
    m_canvas = 0;

    Digikam::WSStarter::cleanUp();
    Digikam::ThumbnailLoadThread::cleanUp();
    Digikam::LoadingCacheInterface::cleanUp();
    Digikam::DMediaServerMngr::instance()->saveAtShutdown();

    delete d->model;
    delete d->filterModel;
    delete d->thumbBar;
    delete d->rightSideBar;
    delete d->thumbLoadThread;
    delete d;
}

bool ShowFoto::queryClose()
{
    // wait if a save operation is currently running
    if (!waitForSavingToComplete())
    {
        return false;
    }

    if (!d->thumbBar->currentInfo().isNull() && !promptUserSave(d->thumbBar->currentUrl()))
    {
        return false;
    }

    saveSettings();
    return true;
}

void ShowFoto::show()
{
    // Remove Splashscreen.

    if (d->splash)
    {
        d->splash->finish(this);
        delete d->splash;
        d->splash = 0;
    }

    // Display application window.

    QMainWindow::show();

    // Report errors from ICC repository path.

    KSharedConfig::Ptr config = KSharedConfig::openConfig();

    if (!d->validIccPath)
    {
        QString message = i18n("<p>The ICC profile path seems to be invalid.</p>"
                               "<p>If you want to set it now, select \"Yes\", otherwise "
                               "select \"No\". In this case, \"Color Management\" feature "
                               "will be disabled until you solve this issue</p>");

        if (QMessageBox::warning(this, qApp->applicationName(), message,
                                 QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
        {
            if (!setup(true))
            {
                KConfigGroup group = config->group(QLatin1String("Color Management"));
                group.writeEntry(QLatin1String("EnableCM"), false);
                config->sync();
            }
        }
        else
        {
            KConfigGroup group = config->group(QLatin1String("Color Management"));
            group.writeEntry(QLatin1String("EnableCM"), false);
            config->sync();
        }
    }

    // Start the Media Server if necessary

    Digikam::DMediaServerMngr::instance()->loadAtStartup();
}

void ShowFoto::slotOpenFile()
{
    if (!d->thumbBar->currentInfo().isNull() && !promptUserSave(d->thumbBar->currentUrl()))
    {
        return;
    }

    QList<QUrl> urls = Digikam::ImageDialog::getImageURLs(this, d->lastOpenedDirectory);

    if (urls.count() > 1)
    {
        d->infoList.clear();
        d->model->clearShowfotoItemInfos();

        openUrls(urls);
        emit signalInfoList(d->infoList);

        slotOpenUrl(d->thumbBar->currentInfo());
        toggleNavigation(1);
    }
    else if (urls.count() == 1)
    {
        d->infoList.clear();
        d->model->clearShowfotoItemInfos();

        openFolder(urls.first().adjusted(QUrl::RemoveFilename));
        emit signalInfoList(d->infoList);

        slotOpenUrl(d->thumbBar->findItemByUrl(urls.first()));
        d->thumbBar->setCurrentUrl(urls.first());
    }
}

void ShowFoto::slotOpenFolder()
{
    if (!d->thumbBar->currentInfo().isNull() && !promptUserSave(d->thumbBar->currentUrl()))
    {
        return;
    }

    QUrl url = DFileDialog::getExistingDirectoryUrl(this, i18n("Open Images From Folder"),
                                                    d->lastOpenedDirectory);
    if (!url.isEmpty())
    {
        d->infoList.clear();
        d->model->clearShowfotoItemInfos();

        openFolder(url);
        emit signalInfoList(d->infoList);

        slotOpenUrl(d->thumbBar->currentInfo());
        toggleNavigation(1);
    }
}

void ShowFoto::openUrls(const QList<QUrl> &urls)
{
    if (urls.isEmpty())
    {
        return;
    }

    ShowfotoItemInfo iteminfo;
    DMetadata meta;

    for (QList<QUrl>::const_iterator it = urls.constBegin() ; it != urls.constEnd() ; ++it)
    {
        QFileInfo fi((*it).toLocalFile());
        iteminfo.name      = fi.fileName();
        iteminfo.mime      = fi.suffix();
        iteminfo.size      = fi.size();
        iteminfo.url       = QUrl::fromLocalFile(fi.filePath());
        iteminfo.folder    = fi.path();
        iteminfo.dtime     = fi.created();
        meta.load(fi.filePath());
        iteminfo.ctime     = meta.getItemDateTime();
        iteminfo.width     = meta.getItemDimensions().width();
        iteminfo.height    = meta.getItemDimensions().height();
        iteminfo.photoInfo = meta.getPhotographInformation();

        if (!d->infoList.contains(iteminfo))
            d->infoList << iteminfo;
    }
}

void ShowFoto::openFolder(const QUrl& url)
{
    if (!url.isValid() || !url.isLocalFile())
    {
        return;
    }

    d->lastOpenedDirectory = url;

    // Parse image IO mime types registration to get files filter pattern.

    QString filter;
    QStringList mimeTypes = supportedImageMimeTypes(QIODevice::ReadOnly, filter);

    QString patterns = filter.toLower();
    patterns.append (QLatin1Char(' '));
    patterns.append (filter.toUpper());

    qCDebug(DIGIKAM_SHOWFOTO_LOG) << "patterns=" << patterns;

    // Get all image files from directory.

    QDir dir(url.toLocalFile(), patterns);
    dir.setFilter(QDir::Files);

    if (!dir.exists())
    {
        return;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);

    QFileInfoList fileinfolist = dir.entryInfoList();

    if (fileinfolist.isEmpty())
    {
        QApplication::restoreOverrideCursor();
        return;
    }

    QFileInfoList::const_iterator fi;
    ShowfotoItemInfo iteminfo;
    DMetadata meta;

    // And open all items in image editor.

    for (fi = fileinfolist.constBegin() ; fi != fileinfolist.constEnd() ; ++fi)
    {
        iteminfo.name      = (*fi).fileName();
        iteminfo.mime      = (*fi).suffix();
        iteminfo.size      = (*fi).size();
        iteminfo.folder    = (*fi).path();
        iteminfo.url       = QUrl::fromLocalFile((*fi).filePath());
        iteminfo.dtime     = (*fi).created();
        meta.load((*fi).filePath());
        iteminfo.ctime     = meta.getItemDateTime();
        iteminfo.width     = meta.getItemDimensions().width();
        iteminfo.height    = meta.getItemDimensions().height();
        iteminfo.photoInfo = meta.getPhotographInformation();

        if (!d->infoList.contains(iteminfo))
            d->infoList << iteminfo;
    }

    QApplication::restoreOverrideCursor();
}

void ShowFoto::slotDroppedUrls(const QList<QUrl>& droppedUrls, bool dropped)
{
    if (droppedUrls.isEmpty())
    {
        return;
    }

    QList<QUrl> validUrls;

    foreach (const QUrl& url, droppedUrls)
    {
        if (url.isValid())
        {
            validUrls << url;
        }
    }

    QList<QUrl> imagesUrls;
    QList<QUrl> foldersUrls;

    foreach (const QUrl& url, validUrls)
    {
        if (QMimeDatabase().mimeTypeForUrl(url).name().startsWith(QLatin1String("image"),
                                                                  Qt::CaseInsensitive))
        {
            imagesUrls << url;
        }

        if (QMimeDatabase().mimeTypeForUrl(url).name() == QLatin1String("inode/directory"))
        {
            foldersUrls << url;
        }
    }

    if (!imagesUrls.isEmpty())
    {
        openUrls(imagesUrls);
    }

    if (!foldersUrls.isEmpty())
    {
        foreach (const QUrl& fUrl, foldersUrls)
        {
            openFolder(fUrl);
        }
    }

    if (!d->infoList.isEmpty())
    {
        if (!dropped && foldersUrls.isEmpty() && imagesUrls.count() == 1)
        {
            openFolder(imagesUrls.first().adjusted(QUrl::RemoveFilename));
            d->model->clearShowfotoItemInfos();
            emit signalInfoList(d->infoList);

            slotOpenUrl(d->thumbBar->findItemByUrl(imagesUrls.first()));
            d->thumbBar->setCurrentUrl(imagesUrls.first());
            return;
        }

        d->model->clearShowfotoItemInfos();
        emit signalInfoList(d->infoList);

        slotOpenUrl(d->thumbBar->currentInfo());
    }
    else
    {
        QMessageBox::information(this, qApp->applicationName(),
                                 i18n("There is no dropped item to process."));
        qWarning(DIGIKAM_SHOWFOTO_LOG) << "infolist is empty..";
    }
}

void ShowFoto::slotOpenUrl(const ShowfotoItemInfo& info)
{
    if (d->thumbBar->currentInfo().isNull())
    {
        return;
    }

    QString localFile;

    if (info.url.isLocalFile())
    {
        // file protocol. We do not need the network
        localFile = info.url.toLocalFile();
    }
    else
    {
        QMessageBox::critical(this, i18n("Error Loading File"),
                              i18n("Failed to load file: %1\n"
                                   "Remote file handling is not supported",
                                   info.url.fileName()));
        return;
    }

    d->currentLoadedUrl = info.url;

    m_canvas->load(localFile, m_IOFileSettings);

    //TODO : add preload here like in ImageWindow::slotLoadCurrent() ???
}

void ShowFoto::slotShowfotoItemInfoActivated(const ShowfotoItemInfo& info)
{
    if (!d->thumbBar->currentInfo().isNull() && !promptUserSave(d->currentLoadedUrl))
    {
        d->thumbBar->setCurrentUrl(d->currentLoadedUrl);
        return;
    }

    slotOpenUrl(info);
}

Digikam::ThumbBarDock* ShowFoto::thumbBar() const
{
    return d->thumbBarDock;
}

Digikam::Sidebar* ShowFoto::rightSideBar() const
{
    return (dynamic_cast<Digikam::Sidebar*>(d->rightSideBar));
}

void ShowFoto::slotChanged()
{
    QString mpixels;
    QSize dims(m_canvas->imageWidth(), m_canvas->imageHeight());
    mpixels.setNum(dims.width()*dims.height()/1000000.0, 'f', 2);
    QString str = (!dims.isValid()) ? i18nc("unknown image dimensions", "Unknown")
                                    : i18nc("%1 width, %2 height, %3 mpixels", "%1x%2 (%3Mpx)",
                                            dims.width(),dims.height(),mpixels);
    m_resLabel->setAdjustedText(str);

    if (!d->thumbBar->currentInfo().isNull())
    {
        if (d->thumbBar->currentUrl().isValid())
        {
            QRect sel                = m_canvas->getSelectedArea();
            Digikam::DImg* const img = m_canvas->interface()->getImg();
            d->rightSideBar->itemChanged(d->thumbBar->currentUrl(), sel, img);
        }
    }
}

void ShowFoto::slotUpdateItemInfo()
{
    d->itemsNb = d->thumbBar->showfotoItemInfos().size();
    int index  = 0;
    QString text;

    if (d->itemsNb > 0)
    {
        index = 1;

        for (int i = 0 ; i < d->itemsNb ; ++i)
        {
            QUrl url = d->thumbBar->showfotoItemInfos().at(i).url;

            if (url.matches(d->thumbBar->currentUrl(), QUrl::None))
            {
                break;
            }

            ++index;
        }

        text = i18nc("<Image file name> (<Image number> of <Images in album>)",
                     "%1 (%2 of %3)", d->thumbBar->currentInfo().name,
                     index, d->itemsNb);

        setCaption(QDir::toNativeSeparators(d->thumbBar->currentUrl()
                                            .adjusted(QUrl::RemoveFilename).toLocalFile()));
    }
    else
    {
        text = QLatin1String("");
        setCaption(QLatin1String(""));
    }

    m_nameLabel->setText(text);
    toggleNavigation(index);
}

void ShowFoto::slotFirst()
{
    if (!d->thumbBar->currentInfo().isNull() && !promptUserSave(d->thumbBar->currentUrl()))
    {
        return;
    }

    d->thumbBar->toFirstIndex();
    d->thumbBar->setCurrentInfo(d->thumbBar->showfotoItemInfos().first());
    slotOpenUrl(d->thumbBar->showfotoItemInfos().first());
}

void ShowFoto::slotLast()
{
    if (!d->thumbBar->currentInfo().isNull() && !promptUserSave(d->thumbBar->currentUrl()))
    {
        return;
    }

    d->thumbBar->toLastIndex();
    d->thumbBar->setCurrentInfo(d->thumbBar->showfotoItemInfos().last());
    slotOpenUrl(d->thumbBar->showfotoItemInfos().last());
}

void ShowFoto::slotForward()
{
    if (!d->thumbBar->currentInfo().isNull() && !promptUserSave(d->thumbBar->currentUrl()))
    {
        return;
    }

    bool currentIsNull = d->thumbBar->currentInfo().isNull();

    if (!currentIsNull)
    {
         d->thumbBar->toNextIndex();
         slotOpenUrl(d->thumbBar->currentInfo());
    }
}

void ShowFoto::slotBackward()
{
    if (!d->thumbBar->currentInfo().isNull() && !promptUserSave(d->thumbBar->currentUrl()))
    {
        return;
    }

    bool currentIsNull = d->thumbBar->currentInfo().isNull();

    if (!currentIsNull)
    {
         d->thumbBar->toPreviousIndex();
         slotOpenUrl(d->thumbBar->currentInfo());
    }
}

void ShowFoto::slotPrepareToLoad()
{
    Digikam::EditorWindow::slotPrepareToLoad();

    // Here we enable specific actions on showfoto.
    d->openFilesInFolderAction->setEnabled(true);
    d->fileOpenAction->setEnabled(true);
}

void ShowFoto::slotLoadingStarted(const QString& filename)
{
    Digikam::EditorWindow::slotLoadingStarted(filename);

    // Here we disable specific actions on showfoto.
    d->openFilesInFolderAction->setEnabled(false);
    d->fileOpenAction->setEnabled(false);
}

void ShowFoto::slotLoadingFinished(const QString& filename, bool success)
{
    Digikam::EditorWindow::slotLoadingFinished(filename, success);

    // Here we re-enable specific actions on showfoto.
    d->openFilesInFolderAction->setEnabled(true);
    d->fileOpenAction->setEnabled(true);
}

void ShowFoto::slotSavingStarted(const QString& filename)
{
    Digikam::EditorWindow::slotSavingStarted(filename);

    // Here we disable specific actions on showfoto.
    d->openFilesInFolderAction->setEnabled(false);
    d->fileOpenAction->setEnabled(false);
}

void ShowFoto::moveFile()
{
    /*
     * moveFile() -> moveLocalFile() ->  movingSaveFileFinished()
     *                                     |               |
     *                            finishSaving(true)  save...IsComplete()
     */

    qCDebug(DIGIKAM_SHOWFOTO_LOG) << m_savingContext.destinationURL
                                  << m_savingContext.destinationURL.isLocalFile();

    if (m_savingContext.destinationURL.isLocalFile())
    {
        qCDebug(DIGIKAM_SHOWFOTO_LOG) << "moving a local file";
        EditorWindow::moveFile();
    }
    else
    {
        QMessageBox::critical(this, i18n("Error Saving File"),
                              i18n("Failed to save file: %1",
                              i18n("Remote file handling is not supported")));
    }
}

void ShowFoto::finishSaving(bool success)
{
    Digikam::EditorWindow::finishSaving(success);

    // Here we re-enable specific actions on showfoto.
    d->openFilesInFolderAction->setEnabled(true);
    d->fileOpenAction->setEnabled(true);
}

void ShowFoto::saveIsComplete()
{
    Digikam::LoadingCacheInterface::putImage(m_savingContext.destinationURL.toLocalFile(), m_canvas->currentImage());
    //d->thumbBar->invalidateThumb(d->currentItem);

    // Pop-up a message to bring user when save is done.
    Digikam::DNotificationWrapper(QLatin1String("editorsavefilecompleted"), i18n("Image saved successfully"),
                                  this, windowTitle());

    resetOrigin();
}

void ShowFoto::saveAsIsComplete()
{
    resetOriginSwitchFile();
/*
    Digikam::LoadingCacheInterface::putImage(m_savingContext.destinationURL.toLocalFile(), m_canvas->currentImage());

    // Add the file to the list of thumbbar images if it's not there already
    Digikam::ThumbBarItem* foundItem = d->thumbBar->findItemByUrl(m_savingContext.destinationURL);
    d->thumbBar->invalidateThumb(foundItem);
    qCDebug(DIGIKAM_SHOWFOTO_LOG) << wantedUrls;

    if (!foundItem)
    {
        foundItem = new Digikam::ThumbBarItem(d->thumbBar, m_savingContext.destinationURL);
    }

    // shortcut slotOpenUrl
    d->thumbBar->blockSignals(true);
    d->thumbBar->setSelected(foundItem);
    d->thumbBar->blockSignals(false);
    d->currentItem = foundItem;
    slotUpdateItemInfo();

    // Pop-up a message to bring user when save is done.
    Digikam::DNotificationWrapper("editorsavefilecompleted", i18n("Image saved successfully"),
                                  this, windowTitle());
*/
}

void ShowFoto::saveVersionIsComplete()
{
}

QUrl ShowFoto::saveDestinationUrl()
{
    if (d->thumbBar->currentInfo().isNull())
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "Cannot return the url of the image to save "
                                       << "because no image is selected.";
        return QUrl();
    }

    return d->thumbBar->currentUrl();
}

bool ShowFoto::save()
{
    if (d->thumbBar->currentInfo().isNull())
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "This should not happen";
        return true;
    }

    startingSave(d->currentLoadedUrl);
    return true;
}

bool ShowFoto::saveAs()
{
    if (d->thumbBar->currentInfo().isNull())
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "This should not happen";
        return false;
    }

    return (startingSaveAs(d->currentLoadedUrl));
}

void ShowFoto::slotDeleteCurrentItem()
{
    QUrl urlCurrent(d->thumbBar->currentUrl());

    QString warnMsg(i18n("About to delete file \"%1\"\nAre you sure?",
                         urlCurrent.fileName()));

    if (QMessageBox::warning(this, qApp->applicationName(), warnMsg,
                             QMessageBox::Apply | QMessageBox::Abort)
        !=  QMessageBox::Apply)
    {
        return;
    }
    else
    {
        bool ret = QFile::remove(urlCurrent.toLocalFile());

        if (!ret)
        {
            QMessageBox::critical(this, qApp->applicationName(),
                                  i18n("Cannot delete \"%1\"", urlCurrent.fileName()));
            return;
        }

        // No error, remove item in thumbbar.
        d->model->removeIndex(d->thumbBar->currentIndex());

        // Disable menu actions and SideBar if no current image.

        d->itemsNb = d->thumbBar->showfotoItemInfos().size();

        if (d->itemsNb == 0)
        {
            slotUpdateItemInfo();
            toggleActions(false);
            m_canvas->load(QString(), m_IOFileSettings);
            emit signalNoCurrentItem();
        }
        else
        {
            // If there is an image after the deleted one, make that selected.
            slotOpenUrl(d->thumbBar->currentInfo());
        }
    }
}

void ShowFoto::slotRevert()
{
    if (!promptUserSave(d->thumbBar->currentUrl()))
    {
        return;
    }

    m_canvas->slotRestore();
}

bool ShowFoto::saveNewVersion()
{
    return false;
}

bool ShowFoto::saveCurrentVersion()
{
    return false;
}

bool ShowFoto::saveNewVersionAs()
{
    return false;
}

bool ShowFoto::saveNewVersionInFormat(const QString&)
{
    return false;
}

void ShowFoto::slotSetupMetadataFilters(int tab)
{
    Setup::execMetadataFilters(this, tab+1);
}

void ShowFoto::slotAddedDropedItems(QDropEvent* e)
{
    QList<QUrl> list = e->mimeData()->urls();
    QList<QUrl> urls;

    foreach (const QUrl& url, list)
    {
        QFileInfo fi(url.toLocalFile());

        if (fi.exists())
        {
            urls.append(url);
        }
    }

    e->accept();

    if (!urls.isEmpty())
    {
        slotDroppedUrls(urls, true);
    }
}

void ShowFoto::slotFileWithDefaultApplication()
{
    Digikam::DFileOperations::openFilesWithDefaultApplication(QList<QUrl>() << d->thumbBar->currentUrl());
}

void ShowFoto::slotOpenWith(QAction* action)
{
    openWith(d->thumbBar->currentUrl(), action);
}

DInfoInterface* ShowFoto::infoIface(DPluginAction* const)
{
    DMetaInfoIface* const iface = new DMetaInfoIface(this, d->thumbBar->urls());

    connect(iface, SIGNAL(signalItemChanged(QUrl)),
            this, SLOT(slotChanged()));

    connect(iface, SIGNAL(signalImportedImage(QUrl)),
            this, SLOT(slotImportedImagefromScanner(QUrl)));

    return iface;
}

} // namespace ShowFoto

#include "moc_showfoto.cpp"
