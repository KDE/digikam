//////////////////////////////////////////////////////////////////////////////
//
//    Copyright (C) 2002-2004 Renchi Raju <renchi at pooh.tam.uiuc.edu>
//                            Gilles Caulier <caulier dot gilles at free.fr>
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
//////////////////////////////////////////////////////////////////////////////

// Qt Includes.

#include <qstring.h>
#include <qstringlist.h>
#include <qstrlist.h>
#include <qfileinfo.h>
#include <qdir.h>
#include <qimage.h>
#include <qevent.h>
#include <qapplication.h>

// KDE includes.

#include <kurl.h>
#include <kfiledialog.h>
#include <kpushbutton.h>
#include <klocale.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <krun.h>

// Local includes.

#include "albummanager.h"
#include "album.h"

#include "albumfolderview.h"
#include "albumfolderitem.h"
#include "albumiconview.h"
#include "albumiconitem.h"
#include "albumsettings.h"
#include "albumhistory.h"
#include "thumbnailsize.h"

#include "digikamapp.h"
#include "digikamview.h"

DigikamView::DigikamView(QWidget *parent)
    : QSplitter(Qt::Horizontal, parent)
{
    mParent = static_cast<DigikamApp *>(parent);

    mAlbumMan = AlbumManager::instance();

    mFolderView = new AlbumFolderView(this);
    mIconView = new AlbumIconView(this);

    QSizePolicy leftSzPolicy(QSizePolicy::Preferred,
                             QSizePolicy::Expanding,
                             1, 1);
    QSizePolicy rightSzPolicy(QSizePolicy::Preferred,
                              QSizePolicy::Expanding,
                              2, 1);

    setOpaqueResize(false);

    setupConnections();

    mAlbumMan->setItemHandler(mIconView);

    mFolderView->setInFocus(true);
    mIconView->setInFocus(false);

    mAlbumHistory = new AlbumHistory();    
    
    KConfig *config = kapp->config();
    config->setGroup("MainWindow");
    if(config->hasKey("SplitterSizes"))
        setSizes(config->readIntListEntry("SplitterSizes"));
    else {
        mFolderView->setSizePolicy(leftSzPolicy);
        mIconView->setSizePolicy(rightSzPolicy);
    }

}

DigikamView::~DigikamView()
{
    KConfig *config = kapp->config();
    config->setGroup("MainWindow");
    config->writeEntry("SplitterSizes", sizes());

    delete mAlbumHistory;
    mAlbumMan->setItemHandler(0);
}

void DigikamView::applySettings(const AlbumSettings* settings)
{
    mIconView->applySettings(settings);
    mFolderView->applySettings();
}

void DigikamView::setupConnections()
{
    // -- AlbumManager connections --------------------------------

    connect(mAlbumMan, SIGNAL(signalAlbumCurrentChanged(Album*)),
            this, SLOT(slot_albumSelected(Album*)));
    connect(mAlbumMan, SIGNAL(signalAlbumsCleared()),
            this, SLOT(slot_albumsCleared()));
    connect(mAlbumMan, SIGNAL(signalAlbumDeleted(Album*)),
            this, SLOT(slotAlbumDeleted(Album*)));
    
    // -- IconView Connections -------------------------------------

    connect(mIconView,  SIGNAL(signalSelectionChanged()),
            this, SLOT(slot_imageSelected()));

    connect(mIconView,  SIGNAL(signalItemsAdded()),
            this, SLOT(slot_albumHighlight()));

    connect(mIconView, SIGNAL(signalInFocus()),
            SLOT(slotIconViewInFocus()));

    connect(mFolderView, SIGNAL(signalTagsAssigned()),
            mIconView->viewport(), SLOT(update()));

    connect(mFolderView, SIGNAL(signalInFocus()),
            SLOT(slotFolderViewInFocus()));

    connect(mFolderView, SIGNAL(signalAlbumModified()),
	    mIconView, SLOT(slotAlbumModified()));
}

void DigikamView::slot_sortAlbums(int order)
{
    AlbumSettings* settings = AlbumSettings::instance();
    if (!settings) return;
    settings->setAlbumSortOrder(
        (AlbumSettings::AlbumSortOrder) order);
    mFolderView->applySettings();
}

void DigikamView::slot_newAlbum()
{
    mFolderView->albumNew();
}

void DigikamView::slot_deleteAlbum()
{
    mFolderView->albumDelete();
}

void DigikamView::slotNewTag()
{
    mFolderView->tagNew();
}

void DigikamView::slotDeleteTag()
{
    mFolderView->tagDelete();
}

void DigikamView::slotEditTag()
{
    mFolderView->tagEdit();
}

// ----------------------------------------------------------------

void DigikamView::slotAlbumDeleted(Album *album)
{
    Album *nextAlbum = mAlbumHistory->deleteAlbum(album);
    
    if(nextAlbum && nextAlbum->getViewItem())
    {
        AlbumFolderItem *item;    
        item = static_cast<AlbumFolderItem*>(nextAlbum->getViewItem());
        mFolderView->setSelected(item);
        mParent->enableAlbumBackwardHistory(!mAlbumHistory->isBackwardEmpty());
        mParent->enableAlbumForwardHistory(!mAlbumHistory->isForwardEmpty());            
    }
}

void DigikamView::slotAlbumHistoryBack(int steps)
{
    Album *album = mAlbumHistory->back(steps);
    
    if(album && album->getViewItem())
    {
        AlbumFolderItem *item;    
        item = static_cast<AlbumFolderItem*>(album->getViewItem());
        mFolderView->setSelected(item);
        mParent->enableAlbumBackwardHistory(!mAlbumHistory->isBackwardEmpty());
        mParent->enableAlbumForwardHistory(!mAlbumHistory->isForwardEmpty());            
    }
    return;
}

void DigikamView::slotAlbumHistoryForward(int steps)
{
    Album *album = mAlbumHistory->forward(steps);
    
    if(album && album->getViewItem())
    {
        AlbumFolderItem *item;
        item = static_cast<AlbumFolderItem*>(album->getViewItem());
        mFolderView->setSelected(item);
        mParent->enableAlbumBackwardHistory(!mAlbumHistory->isBackwardEmpty());
        mParent->enableAlbumForwardHistory(!mAlbumHistory->isForwardEmpty());
    }
    return;    
}

void DigikamView::getBackwardHistory(QStringList &titles)
{
    mAlbumHistory->getBackwardHistory(titles);
}

void DigikamView::getForwardHistory(QStringList &titles)
{
    mAlbumHistory->getForwardHistory(titles);
}

void DigikamView::slotSelectAlbum(const KURL &url)
{
    if(url.isEmpty())
        return;
    
    Album *album = mAlbumMan->findPAlbum(url);
    if(album && album->getViewItem())
    {
        AlbumFolderItem *item;
        item = static_cast<AlbumFolderItem*>(album->getViewItem());
        mFolderView->setSelected(item);
        mParent->enableAlbumBackwardHistory(!mAlbumHistory->isBackwardEmpty());
        mParent->enableAlbumForwardHistory(!mAlbumHistory->isForwardEmpty());
    }
}

// ----------------------------------------------------------------

void DigikamView::slot_albumSelected(Album* album)
{
    if (!album) {
        mIconView->setAlbum(0);
        emit signal_albumSelected(false);
        emit signal_tagSelected(false);
        return;
    }

    if (album->type() == Album::PHYSICAL)
    {
        emit signal_albumSelected(true);
        emit signal_tagSelected(false);
    }
    else if (album->type() == Album::TAG)
    {
        emit signal_albumSelected(false);
        emit signal_tagSelected(true);
    }
    
    mAlbumHistory->addAlbum(album);
    mParent->enableAlbumBackwardHistory(!mAlbumHistory->isBackwardEmpty());
    mParent->enableAlbumForwardHistory(!mAlbumHistory->isForwardEmpty());    
    
    mIconView->setAlbum(album);
}

void DigikamView::slot_albumOpenInKonqui()
{
    Album *album = mAlbumMan->currentAlbum();
    if (!album || album->type() != Album::PHYSICAL)
        return;

    PAlbum* palbum = dynamic_cast<PAlbum*>(album);
       
    new KRun(palbum->getKURL()); // KRun will delete itself.
}

void DigikamView::slot_imageSelected()
{
    int count = 0;

    ThumbItem* item = 0;
    ThumbItem* selectedItem = 0;

    for (item=mIconView->firstItem();
         item; item=item->nextItem()) {
        if (item->isSelected()) {
            count++;
            selectedItem = item;
            break;
        }
    }

    if (!count) {
        emit signal_imageSelected(false);
    }
    else {
        emit signal_imageSelected(true);
    }
}


void DigikamView::slot_albumsCleared()
{
    mIconView->clear();
    emit signal_albumSelected(false);
}

// ----------------------------------------------------------------

void DigikamView::slot_thumbSizePlus()
{

    ThumbnailSize thumbSize;

    switch(mIconView->thumbnailSize().size()) {

    case (ThumbnailSize::Small): {
        thumbSize = ThumbnailSize(ThumbnailSize::Medium);
        break;
    }
    case (ThumbnailSize::Medium): {
        thumbSize = ThumbnailSize(ThumbnailSize::Large);
        break;
    }
    case (ThumbnailSize::Large): {
        thumbSize = ThumbnailSize(ThumbnailSize::Huge);
        break;
    }
    case (ThumbnailSize::Huge): {
        thumbSize = ThumbnailSize(ThumbnailSize::Huge);
        break;
    }
    default:
        return;
    }

    if (thumbSize.size() == ThumbnailSize::Huge) {
        mParent->enableThumbSizePlusAction(false);
    }
    mParent->enableThumbSizeMinusAction(true);

    mIconView->setThumbnailSize(thumbSize);
}

void DigikamView::slot_thumbSizeMinus()
{
    ThumbnailSize thumbSize;

    switch(mIconView->thumbnailSize().size()) {

    case (ThumbnailSize::Small): {
        thumbSize = ThumbnailSize(ThumbnailSize::Small);
        break;
    }
    case (ThumbnailSize::Medium): {
        thumbSize = ThumbnailSize(ThumbnailSize::Small);
        break;
    }
    case (ThumbnailSize::Large): {
        thumbSize = ThumbnailSize(ThumbnailSize::Medium);
        break;
    }
    case (ThumbnailSize::Huge): {
        thumbSize = ThumbnailSize(ThumbnailSize::Large);
        break;
    }
    default:
        return;
    }

    if (thumbSize.size() == ThumbnailSize::Small) {
        mParent->enableThumbSizeMinusAction(false);
    }
    mParent->enableThumbSizePlusAction(true);

    mIconView->setThumbnailSize(thumbSize);
}

void DigikamView::slot_albumPropsEdit()
{
    Album *album = mAlbumMan->currentAlbum();
    if (!album || album->type() != Album::PHYSICAL)
        return;

    mFolderView->albumEdit(dynamic_cast<PAlbum*>(album));
}

void DigikamView::slot_albumAddImages()
{
    Album *album = mAlbumMan->currentAlbum();
    if (!album || album->type() != Album::PHYSICAL)
        return;

    PAlbum* palbum = dynamic_cast<PAlbum*>(album);

    KFileDialog dlg(QString::null,
                    AlbumSettings::instance()->getImageFileFilter(),
                    this, "filedialog", true);
    dlg.setOperationMode(KFileDialog::Opening );
    dlg.setCaption(i18n("Add Images"));
    dlg.setMode(KFile::Files);
    dlg.okButton()->setText(i18n("&Add"));
    dlg.exec();          

    KURL::List urls = dlg.selectedURLs();
    
    if (!urls.isEmpty())
    {
        KIO::CopyJob* job =
            KIO::copy(urls, palbum->getKURL(), true);
        connect(job, SIGNAL(result(KIO::Job *) ),
                this, SLOT(slot_imageCopyResult(KIO::Job *)));
    }
}

void DigikamView::slotAlbumImportFolder()
{
    mFolderView->albumImportFolder();
}

void DigikamView::slot_albumHighlight()
{
    Album *album = mAlbumMan->currentAlbum();
    if (!album || !album->type() == Album::PHYSICAL)
        return;

    mFolderView->albumHighlight(dynamic_cast<PAlbum*>(album));
}

void DigikamView::slot_imageCopyResult(KIO::Job* job)
{
    if (job->error())
        job->showErrorDialog(this);
}

// ----------------------------------------------------------------

void DigikamView::slot_imageView(AlbumIconItem *iconItem)
{
    AlbumIconItem *item;

    if (!iconItem) {
        item = mIconView->firstSelectedItem();
        if (!item) return;
    }
    else {
        item = iconItem;
    }

    mIconView->slotDisplayItem(item);
}

void DigikamView::slot_imageCommentsEdit(AlbumIconItem *iconItem)
{
    AlbumIconItem *item;

    if (!iconItem) {
        item = mIconView->firstSelectedItem();
        if (!item) return;
    }
    else {
        item = iconItem;
    }

    mIconView->slotEditImageComments(item);
}

void DigikamView::slot_imageExifOrientation(int orientation)
{
    mIconView->slotSetExifOrientation(orientation);
}

void DigikamView::slot_imageRename(AlbumIconItem *iconItem)
{
    AlbumIconItem *item;

    if (!iconItem) {
        item = mIconView->firstSelectedItem();
        if (!item) return;
    }
    else {
        item = iconItem;
    }

    mIconView->slotRename(item);
}

void DigikamView::slot_imageDelete()
{
    mIconView->slotDeleteSelectedItems();
}

void DigikamView::slotImageProperties()
{
    AlbumIconItem *iconItem =
        mIconView->firstSelectedItem();
    if (!iconItem) return;

    mIconView->slotProperties(iconItem);
}

void DigikamView::slotSelectAll()
{
    mIconView->selectAll();
}

void DigikamView::slotSelectNone()
{
    mIconView->clearSelection();
}

void DigikamView::slotSelectInvert()
{
    mIconView->invertSelection();
}

void DigikamView::slotSortImages(int order)
{
    AlbumSettings* settings = AlbumSettings::instance();
    if (!settings)
        return;
    settings->setImageSortOder((AlbumSettings::ImageSortOrder) order);
    mIconView->slotUpdate();
}

void DigikamView::slotFolderViewInFocus()
{
    mFolderView->setInFocus(true);
    mIconView->setInFocus(false);
}

void DigikamView::slotIconViewInFocus()
{
    mFolderView->setInFocus(false);
    mIconView->setInFocus(true);
}

#include "digikamview.moc"
