/* ============================================================
 * File  : camerauiview.cpp
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2003-01-21
 * Description :
 *
 * Copyright 2003 by Renchi Raju

 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published bythe Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include <klocale.h>
#include <kcombobox.h>
#include <kurl.h>
#include <kfiledialog.h>
#include <kstandarddirs.h>
#include <kfileitem.h>
#include <krun.h>
#include <kservice.h>
#include <kfiledialog.h>
#include <kinputdialog.h>
#include <kmessagebox.h>
#include <kdirlister.h>
#include <kio/job.h>
#include <libkexif/kexif.h>

#include <qpushbutton.h>
#include <qsplitter.h>
#include <qlayout.h>
#include <qvaluelist.h>
#include <qhgroupbox.h>
#include <qimage.h>
#include <qlabel.h>
#include <qdir.h>
#include <qstringlist.h>
#include <qfileinfo.h>

#include "gpcontroller.h"
#include "gpeventfilter.h"
#include "gpfileiteminfo.h"
#include "gpfileitemcontainer.h"
#include "camerafolderview.h"
#include "camerafolderitem.h"
#include "cameraiconitem.h"
#include "cameraiconview.h"
#include "savefiledialog.h"
#include "camerainfodialog.h"
#include "cameraui.h"
#include "camerasettings.h"
#include "camerauiview.h"
#include "dmessagebox.h"

#include "imageview.h"

CameraUIView::CameraUIView(QWidget* parent,
                           const CameraType& ctype)
    : QWidget(parent)
{

    // Setup the view ---------------------------------------
    
    QBoxLayout *l = new QVBoxLayout(this);

    QSplitter *splitter = new QSplitter(this);
    l->addWidget(splitter);
    splitter->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,
                                        QSizePolicy::Expanding));

    folderView_ = new CameraFolderView(splitter);
    iconView_   = new CameraIconView(splitter);

    splitter->setOpaqueResize(true);
    splitter->setResizeMode(folderView_,  QSplitter::Stretch);
    splitter->setResizeMode(iconView_,  QSplitter::Stretch);
    QValueList<int> sizeList;
    sizeList.push_back(2);
    sizeList.push_back(5);
    splitter->setSizes (sizeList);

    QHGroupBox *bottomBox = new QHGroupBox(this);
    bottomBox->setFrameShape(QFrame::NoFrame);
    bottomBox->setInsideMargin(2);
    bottomBox->setInsideSpacing(2);
    l->addWidget(bottomBox);
    bottomBox->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,
                                         QSizePolicy::Minimum));

    (void) new QLabel(i18n("Download To Album : "), bottomBox);

    downloadAlbumBox_ = new KComboBox(true, bottomBox);
    downloadAlbumBox_->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,
                                                 QSizePolicy::Minimum));
    downloadAlbumBox_->setEditable(false);
    downloadAlbumBox_->setDuplicatesEnabled( false );

    newAlbumButton_ = new QPushButton(i18n("New Album"), bottomBox);
    newAlbumButton_->setSizePolicy(QSizePolicy(QSizePolicy::Minimum,
                                               QSizePolicy::Minimum));

    // Now, setup the components ----------------------------------

    parent_ = (CameraUI*) parent;
    cameraType_ = ctype;

    container_  = new GPFileItemContainer(this, folderView_, iconView_);
    efilter_    = new GPEventFilter(this);
    controller_ = new GPController(this, cameraType_);
    controller_->start();
    cameraConnected_ = false;
    creatingNewAlbum = false;
    
    dirLister_ = new KDirLister;
    dirLister_->setDirOnlyMode(true);
    
    setupConnections();

}

CameraUIView::~CameraUIView()
{
    delete dirLister_;
    delete controller_;
    delete container_;

    folderView_->clear();
    iconView_->clear();
}

void CameraUIView::applySettings(CameraSettings *cameraSettings)
{
    thumbSize_ = (ThumbnailSize::Size) cameraSettings->getDefaultIconSize();
    iconView_->setThumbnailSize(thumbSize_);
}

void CameraUIView::setLibraryPath(const QString& libraryPath)
{
    libraryPath_ = libraryPath;
    dirLister_->stop();
    dirLister_->openURL(KURL(libraryPath_));
}

void CameraUIView::setCurrentAlbum(const QString& album)
{
    currentAlbum_ = album;

    for (int i = 0; i < downloadAlbumBox_->count(); i++) {
        KURL url1(libraryPath_ + QString("/") +
                 downloadAlbumBox_->text(i));
        KURL url2(libraryPath_ + QString("/") +
                  currentAlbum_);
        if (url1.equals(url2, true)) {
            downloadAlbumBox_->setCurrentItem(i);
            return;
        }
    }
}

void CameraUIView::setupConnections()
{
    // Event Filter connections

    connect(efilter_, SIGNAL(signalStatusMsg(const QString&)),
            this, SIGNAL(signalStatusMsg(const QString&)));
    connect(efilter_, SIGNAL(signalProgressVal(int)),
            this, SIGNAL(signalProgressVal(int)));
    connect(efilter_, SIGNAL(signalBusy(bool)),
            this, SIGNAL(signalBusy(bool))); 

    // FolderView connections

    connect(folderView_, SIGNAL(signalFolderChanged(CameraFolderItem*)),
            this, SLOT(slotFolderSelected(CameraFolderItem*)));

    // IconView Connections

    connect(iconView_, SIGNAL(signalOpenItem(const QString&,
                                             const QString&)),
            this, SLOT(slotCameraOpenItem(const QString&,
                                          const QString&)));

    connect(iconView_, SIGNAL(signalOpenItem(const QString&,
                                             const QString&,
                                             const QString&)),
            this, SLOT(slotCameraOpenItem(const QString&,
                                          const QString&,
                                          const QString&)));

    connect(iconView_, SIGNAL(signalDownloadSelectedItems()),
            this, SLOT(slotCameraDownloadSelected()));

    connect(iconView_, SIGNAL(signalDeleteSelectedItems()),
            this, SLOT(slotCameraDeleteSelected()));

    connect(iconView_, SIGNAL(signalExifInformation(const QString&,
                                                    const QString&)),
            this, SLOT(slotCameraExifInformation(const QString&,
                                                 const QString&)));


    // DirLister connections

    connect(dirLister_, SIGNAL(newItems(const KFileItemList&)),
            this, SLOT(slotNewAlbums(const KFileItemList&)));
    connect(dirLister_, SIGNAL(deleteItem(KFileItem*)),
            this, SLOT(slotDeleteAlbum(KFileItem*)));
    connect(dirLister_, SIGNAL(clear()),
            this, SLOT(slotClearAlbums()));
    
    // New Album Button connection

     connect(newAlbumButton_, SIGNAL(clicked()),
             this, SLOT(slotCreateNewAlbum()));
}


void CameraUIView::cameraInitialized(bool val)
{
    if (val) {

        cameraConnected_ = true;
        parent_->setCameraConnected(true);

        container_->addVirtualFolder(cameraType_.title());
        container_->addRootFolder(cameraType_.path());

        controller_->requestGetSubFolders(cameraType_.path());
        controller_->requestGetAllItemsInfo(cameraType_.path());
        folderView_->virtualFolder()->setSelected(true);
    }
}

void CameraUIView::cameraSubFolder(const QString& folder,
                                   const QString& subFolder)
{
    container_->addFolder(folder, subFolder);
}

void CameraUIView::cameraNewItems(const QString& folder,
                                  const GPFileItemInfoList& infoList)
{
    QListViewItem *item = folderView_->currentItem();
    if (!item) return;

    CameraFolderItem *folderItem = static_cast<CameraFolderItem *>(item);
    if (folderItem->folderPath() != folder && !folderItem->isVirtualFolder())
        return;

    container_->addFiles(folder, infoList);
    
    GPFileItemInfoList::const_iterator it;
    for (it = infoList.begin(); it != infoList.end(); it++) {
        if ((*it).mime.contains("image"))
            controller_->requestGetThumbnail(folder, (*it).name,
                                             thumbSize_);
    }
}

void CameraUIView::cameraNewItems(const GPFileItemInfoList& infoList)
{
    QListViewItem *item = folderView_->currentItem();
    if (!item) return;

    CameraFolderItem *folderItem = static_cast<CameraFolderItem *>(item);
    if (!folderItem->isVirtualFolder()){
        return;
    }

    container_->addFiles(infoList);
    
    GPFileItemInfoList::const_iterator it;
    for (it = infoList.begin(); it != infoList.end(); it++) {
        if ((*it).mime.contains("image"))
            controller_->requestGetThumbnail((*it).folder, (*it).name,
                                             thumbSize_);
    }
}

void CameraUIView::cameraNewThumbnail(const QString& folder,
                                      const QString& itemName,
                                      const QImage& thumbnail)
{
    CameraIconItem *iconItem = container_->findItem(folder,
                                                    itemName);
    if (!iconItem) return;
    
    iconView_->setThumbnail(iconItem, thumbnail);    
}

void CameraUIView::cameraDownloadedItem(const QString& folder,
                                        const QString& itemName)
{
    CameraIconItem *iconItem = container_->findItem(folder,
                                                    itemName);
    if (!iconItem) return;

    iconView_->markDownloaded(iconItem);
}

void CameraUIView::cameraDeletedItem(const QString& folder,
                                     const QString& itemName)
{
    container_->delFile(folder, itemName);
}

void CameraUIView::cameraDeletedAllItems()
{
    container_->delAllFiles();
}

void CameraUIView::cameraOpenedItem(const QString& fileName)
{
    ImageView *imgView = new ImageView(0, KURL(fileName), true);
    imgView->show();
}

void CameraUIView::cameraOpenedItem(const QString& fileName,
                                        const QString& serviceName)
{
    KService::Ptr ptr =  KService::serviceByDesktopName(serviceName);
    if (ptr)
        KRun::run(*ptr, KURL(fileName));
}

void CameraUIView::cameraExifInfo(const QString&,
                                  const QString& itemName,
                                  char* data, int size)
{
    KExif *exifViewer = new KExif;
    int result = exifViewer->loadData(itemName, data, size);
    if (result != 0) 
        delete exifViewer;
    else
    exifViewer->show();
    if (data)
        delete [] data;
}

void CameraUIView::cameraErrorMsg(const QString& msg)
{
    DMessageBox::showMsg(msg);
}

// -- public slots ----------------------------------------------------

void CameraUIView::slotCameraConnectToggle()
{
    parent_->setCameraConnected(false);
    
    if (!cameraConnected_) {
        controller_->requestInitialize();
    }
    else {
        delete controller_;
        controller_ = new GPController(this, cameraType_);
        controller_->start();
        cameraConnected_ = false;

        iconView_->clear();
        folderView_->clear();
    }
}

void CameraUIView::slotCameraDownloadSelected()
{
    if (!cameraConnected_) return;

    QString dir = libraryPath_ + QString("/") +
                  downloadAlbumBox_->currentText();

    QDir qdir(dir);
    if (!qdir.exists()) {
        KMessageBox::error(0, i18n("'%1' Directory does not exist").arg(dir));
        return;
    }

    int count = 0;
    for (ThumbItem *i = iconView_->firstItem(); i;
         i=i->nextItem() ) {
        if (i->isSelected()) ++count;
    }
    if (count == 0) return;

    bool proceed = true;
    bool overwriteAll = false;

    for (ThumbItem *i = iconView_->firstItem(); i;
         i=i->nextItem()) {

        if (i->isSelected()) {
            CameraIconItem *item =
                static_cast<CameraIconItem*>(i);
            if (!item) continue;
            downloadOneItem(item->fileInfo()->name,
                            item->fileInfo()->folder,
                            dir,
                            proceed, overwriteAll);
            if (!proceed) return;
        }
    }

}

void CameraUIView::slotCameraDownloadAll()
{
    if (!cameraConnected_) return;

    QString dir = libraryPath_ + QString("/") +
                  downloadAlbumBox_->currentText();

    QDir qdir(dir);
    if (!qdir.exists()) {
        KMessageBox::error(0, i18n("'%1' Directory does not exist").arg(dir));
        return;
    }

    bool proceed = true;
    bool overwriteAll = false;

    QPtrList<GPFileItemInfo> itemList = container_->allFiles();
    QPtrListIterator<GPFileItemInfo> it( itemList );
    for (; it.current(); ++it) {

        downloadOneItem(it.current()->name,
                        it.current()->folder,
                        dir,
                        proceed, overwriteAll);
        if (!proceed) return;
    }

}


void CameraUIView::slotCameraDeleteSelected()
{
   if (!cameraConnected_) return;

   QStringList deleteList;

   for (ThumbItem *i = iconView_->firstItem(); i;
         i=i->nextItem()) {
        if (i->isSelected()) {
            CameraIconItem *item =
                static_cast<CameraIconItem*>(i);
            deleteList.append(item->fileInfo()->name);
        }
    }

   if (deleteList.isEmpty()) return;

   QString warnMsg(i18n("About to delete these Image(s)\n"
                        "Are you sure?"));
   if (KMessageBox::warningContinueCancelList(this, warnMsg,
                                               deleteList,
                                               i18n("Warning"),
                                               i18n("Delete"))
       ==  KMessageBox::Continue) {

       CameraIconItem *item =
           static_cast<CameraIconItem*>(iconView_->firstItem());

        while(item) {
            CameraIconItem *nextItem =
                static_cast<CameraIconItem *>(item->nextItem());

            if (item->isSelected())
                controller_->requestDeleteItem(item->fileInfo()->folder,
                                               item->fileInfo()->name);
            item = nextItem;
        }
   }
}

void CameraUIView::slotCameraDeleteAll()
{
     if (!cameraConnected_) return;

     QString warnMsg(i18n("About to delete all Images in Camera\n"
                          "Are you sure?"));
     if (KMessageBox::warningContinueCancel(this, warnMsg,
                                            i18n("Warning"),
                                            i18n("Delete"))
         ==  KMessageBox::Continue) {

         controller_->requestDeleteAllItems(cameraType_.path());
   }
}

void CameraUIView::slotCameraUpload()
{
    QString reason;
    if (! cameraReadyForUpload(reason) ) {
        KMessageBox::error(0, reason);
        return;
    }

    CameraFolderItem *folderItem =
        static_cast<CameraFolderItem *>(folderView_->selectedItem());

    QStringList list =
        KFileDialog::getOpenFileNames(QString::null);

    bool ok;

    for (QStringList::Iterator it = list.begin();
         it != list.end(); ++it ) {

        QFileInfo info(*it);
        if (!info.exists()) continue;
        if (info.isDir()) continue;

        QString uploadName = info.fileName();

        while (container_->findItem(folderItem->folderPath(),
                                    uploadName)) {
            QString msg(i18n("Camera Folder '%1' contains item '%2'\n Please, enter New Name")
                        .arg(folderItem->folderName()).arg(uploadName));
            uploadName =
                KInputDialog::getText("",msg,uploadName,&ok,this);
            if (!ok) return;
        }

        controller_->requestUploadItem(folderItem->folderPath(),
                                       info.absFilePath(),
                                       uploadName);
    }    
    
}

void CameraUIView::slotCameraOpenItem(const QString& folder,
                                      const QString& itemName)
{
    if (!cameraConnected_) return;

    QString saveFile = locateLocal("tmp", itemName);
    controller_->requestOpenItem(folder, itemName, saveFile);
}

void CameraUIView::slotCameraCancel()
{
    controller_->cancel();    
}

void CameraUIView::slotThumbSizePlus()
{
    switch(thumbSize_.size()) {

    case(ThumbnailSize::Small): {
        thumbSize_ = ThumbnailSize(ThumbnailSize::Medium);
        break;
    }
    case (ThumbnailSize::Medium): {
        thumbSize_ = ThumbnailSize(ThumbnailSize::Large);
        break;
    }
    case (ThumbnailSize::Large): {
        thumbSize_ = ThumbnailSize(ThumbnailSize::Huge);
        break;
    }
    case (ThumbnailSize::Huge): {
        thumbSize_ = ThumbnailSize(ThumbnailSize::Huge);
        break;
    }
    default:
        return;
    }

    if (thumbSize_.size() == ThumbnailSize::Huge) {
        parent_->enableThumbSizePlusAction(false);
    }
    parent_->enableThumbSizeMinusAction(true);

    iconView_->clear();
    iconView_->setThumbnailSize(thumbSize_);
    if (cameraConnected_) {

        QListViewItem *item = folderView_->currentItem();
        if (!item) return;

        CameraFolderItem *folderItem =
            static_cast<CameraFolderItem *>(item);
        if (folderItem->isVirtualFolder())
            controller_->requestGetAllItemsInfo(cameraType_.path());
        else
            controller_->requestGetItemsInfo(folderItem->folderPath());
    }

}

void CameraUIView::slotThumbSizeMinus()
{
    switch(thumbSize_.size()) {

    case (ThumbnailSize::Small): {
        thumbSize_ = ThumbnailSize(ThumbnailSize::Small);
        break;
    }
    case (ThumbnailSize::Medium): {
        thumbSize_ = ThumbnailSize(ThumbnailSize::Small);
        break;
    }
    case (ThumbnailSize::Large): {
        thumbSize_ = ThumbnailSize(ThumbnailSize::Medium);
        break;
    }
    case (ThumbnailSize::Huge): {
        thumbSize_ = ThumbnailSize(ThumbnailSize::Large);
        break;
    }
    default:
        return;
    }

    if (thumbSize_.size() == ThumbnailSize::Small) {
        parent_->enableThumbSizeMinusAction(false);
    }
    parent_->enableThumbSizePlusAction(true);

    iconView_->clear();
    iconView_->setThumbnailSize(thumbSize_);
    if (cameraConnected_) {

        QListViewItem *item = folderView_->currentItem();
        if (!item) return;

        CameraFolderItem *folderItem =
            static_cast<CameraFolderItem *>(item);
        if (folderItem->isVirtualFolder())
            controller_->requestGetAllItemsInfo(cameraType_.path());
        else
            controller_->requestGetItemsInfo(folderItem->folderPath());
    }

}

void CameraUIView::slotShowFoldersToggle()
{
    if (folderView_->isHidden())
        folderView_->show();
    else
        folderView_->hide();
}

void CameraUIView::slotSelectAll()
{
    iconView_->selectAll();    
}

void CameraUIView::slotSelectNone()
{
    iconView_->clearSelection();
}

void CameraUIView::slotSelectInvert()
{
    iconView_->invertSelection();
}

void CameraUIView::slotSelectNew()
{
    iconView_->clearSelection();

    for (ThumbItem *it = iconView_->firstItem(); it;
         it = it->nextItem()) {
        CameraIconItem *item = static_cast<CameraIconItem *>(it);
        if (item->fileInfo()->downloaded == 0)
            item->setSelected(true, false);
    }
}

// -- private slots ----------------------------------------------------


// folderview slots -------------------------------------------------

void CameraUIView::slotFolderSelected(CameraFolderItem *folderItem)
{
    if (!folderItem) return;

    controller_->cancel();
    iconView_->clear();

    if (folderItem->isVirtualFolder())
        controller_->requestGetAllItemsInfo(cameraType_.path());
    else
        controller_->requestGetItemsInfo(folderItem->folderPath());
}

// new album button slot --------------------------------------------

void CameraUIView::slotCreateNewAlbum()
{
    QDir libraryDir(libraryPath_);
    if (!libraryDir.exists()) {
        KMessageBox::error(0, i18n("Album Library has not been set correctly\n"
                                   "Please run Setup"));
        return;
    }
    
    bool ok;
    QString newDir =
        KInputDialog::getText(i18n("New Album Name"),
                              i18n("Enter New Album Name: "),
                              "", &ok, this);
    if (!ok) return;

    KURL newAlbumURL(libraryPath_);
    newAlbumURL.addPath(newDir);

    creatingNewAlbum = true;
    
    KIO::SimpleJob* job = KIO::mkdir(newAlbumURL);
    connect(job, SIGNAL(result(KIO::Job*)),
            this, SLOT(slotOnAlbumCreate(KIO::Job*)));
}

void CameraUIView::slotOnAlbumCreate(KIO::Job* job)
{
    if (job->error()) {
        job->showErrorDialog(this);
    }
}

// dir lister slots --------------------------------------------------

void CameraUIView::slotNewAlbums(const KFileItemList& fileItemList)
{
    KFileItem* item = 0;

    for (KFileItemListIterator it(fileItemList);
         (item = it.current()); ++it) {
        downloadAlbumBox_->insertItem(item->url().filename(), 0);
        downloadAlbumBox_->setCurrentItem(0);
    }

    if (creatingNewAlbum) {
        creatingNewAlbum = false;
        return;
    }
    
    for (int i = 0; i < downloadAlbumBox_->count(); i++) {
        KURL url1(libraryPath_ + QString("/") +
                 downloadAlbumBox_->text(i));
        KURL url2(libraryPath_ + QString("/") +
                  currentAlbum_);
        if (url1.equals(url2, true)) {
            downloadAlbumBox_->setCurrentItem(i);
            return;
        }
    }
}

void CameraUIView::slotDeleteAlbum(KFileItem* fileItem)
{
    if (!fileItem) return;

    for (int i = 0; i < downloadAlbumBox_->count(); i++) {
        KURL url(libraryPath_ + QString("/") +
                 downloadAlbumBox_->text(i));
        if (fileItem->url().equals(url, true)) {
            downloadAlbumBox_->removeItem(i);
            return;
        }
    }
}

void CameraUIView::slotClearAlbums()
{
    downloadAlbumBox_->clear();    
}


// iconview slots ----------------------------------------------------


void CameraUIView::slotCameraOpenItem(const QString& folder,
                                      const QString& itemName,
                                      const QString& serviceName)
{
    if (!cameraConnected_) return;

    QString saveFile = locateLocal("tmp", itemName);
    controller_->requestOpenItemWithService(folder, itemName,
                                            saveFile, serviceName);
}

void CameraUIView::slotCameraExifInformation(const QString& folder,
                                             const QString& itemName)
{
    if (!cameraConnected_) return;

    controller_->requestExifInfo(folder, itemName);
}

void CameraUIView::slotCameraInformation()
{
    if (!cameraConnected_) return;

    QString summary, manual, about;
    controller_->getInformation(summary, manual, about);

    CameraInfoDialog *infoDlg =
        new CameraInfoDialog(summary, manual, about);
    infoDlg->show();
}

// Helper functions -------------------------------------------------

void CameraUIView::downloadOneItem(const QString& item, const QString& folder,
                                   const QString& downloadDir, bool& proceedFurther,
                                   bool& overwriteAll)
{
    proceedFurther = true;

    QString saveFile(downloadDir);
    if (!downloadDir.endsWith("/"))
        saveFile += "/";
    saveFile += item;

    while (QFile::exists(saveFile) && !(overwriteAll)) {

        bool overwrite=false;

        SavefileDialog *dlg = new SavefileDialog(saveFile);
        if (dlg->exec()== QDialog::Rejected) {
            delete dlg;
            proceedFurther = false;
            return;
        }

        switch(dlg->saveFileOperation()) {
        case (SavefileDialog::Skip): {
            delete dlg;
            return;
        }
        case (SavefileDialog::SkipAll): {
            delete dlg;
            proceedFurther = false;
            return;
        }
        case (SavefileDialog::Overwrite): {
            overwrite = true;
            delete dlg;
            break;
        }
        case (SavefileDialog::OverwriteAll): {
            overwriteAll = true;
            delete dlg;
            break;
        }
        case (SavefileDialog::Rename): {
            saveFile = downloadDir+"/"+dlg->renameFile();
            delete dlg;
            break;
        }
        default:  {
            delete dlg;
            proceedFurther = false;
            return;
        }

        }

        if (overwrite) break;
    }

    controller_->requestDownloadItem(folder, item, saveFile);

}

bool CameraUIView::cameraReadyForUpload(QString& reason)
{
    bool result = false;

    if (!cameraConnected_) {
        reason = i18n("Camera Not Initialized");
        return result;
    }

    /*
      if (!controller_->cameraSupportsUpload()) {
      reason = i18n("Camera does not support Uploads");
      return result;
      }
    */

    if (!folderView_->selectedItem() ||
        folderView_->selectedItem() == folderView_->firstChild()) {
        reason = i18n("Please Select a Folder on Camera to Upload");
        return result;
    }

    result = true;
    return result;
}


#include "camerauiview.moc"
