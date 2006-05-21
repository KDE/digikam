/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *         Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2004-02-12
 * Description : digiKam image editor GUI
 *
 * Copyright 2004-2005 by Renchi Raju, Gilles Caulier
 * Copyright 2006 by Gilles Caulier
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

// C++ Includes.

#include <cstdio>

// Qt includes.

#include <qpopupmenu.h>
#include <qcursor.h>
#include <qtimer.h>
#include <qlabel.h>
#include <qimage.h>
#include <qsplitter.h>

// KDE includes.

#include <kcursor.h>
#include <klocale.h>
#include <kconfig.h>
#include <kstandarddirs.h>
#include <kapplication.h>
#include <kmessagebox.h>
#include <ktempfile.h>
#include <kimageio.h>
#include <kfiledialog.h>
#include <kdebug.h>
#include <kdeversion.h>
#include <kmenubar.h>
#include <ktoolbar.h>
#include <kaccel.h>
#include <kaction.h>
#include <kstdaccel.h>
#include <kstdaction.h>
#include <kstdguiitem.h>
#include <kstatusbar.h>
#include <kpopupmenu.h>
#include <kprogress.h>
#include <kwin.h>

// Local includes.

#include "canvas.h"
#include "dimginterface.h"
#include "dimg.h"
#include "dmetadata.h"
#include "imageplugin.h"
#include "imagepluginloader.h"
#include "imageresizedlg.h"
#include "imageprint.h"
#include "albummanager.h"
#include "album.h"
#include "albumdb.h"
#include "albumsettings.h"
#include "syncjob.h"
#include "albumiconview.h"
#include "albumiconitem.h"
#include "imageinfo.h"
#include "imagepropertiessidebardb.h"
#include "tagspopupmenu.h"
#include "setup.h"
#include "setupimgplugins.h"
#include "iofileprogressbar.h"
#include "iccsettingscontainer.h"
#include "iofilesettingscontainer.h"
#include "loadingcacheinterface.h"
#include "savingcontextcontainer.h"
#include "imagewindow.h"
#include "imageattributeswatch.h"

namespace Digikam
{

ImageWindow* ImageWindow::m_instance = 0;

ImageWindow* ImageWindow::imagewindow()
{
    if (!m_instance)
        new ImageWindow();

    return m_instance;
}

bool ImageWindow::imagewindowCreated()
{
    return m_instance;
}

ImageWindow::ImageWindow()
           : EditorWindow( "Image Editor" )
{
    m_instance    = this;
    m_allowSaving = true;
    m_view        = 0;

    // -- Build the GUI -------------------------------

    setupUserArea();
    setupStatusBar();
    setupActions();

    // Load image plugins to GUI

    m_imagePluginLoader = ImagePluginLoader::instance();
    loadImagePlugins();

    // Create context menu.

    m_contextMenu = static_cast<QPopupMenu*>(factory()->container("RMBMenu", this));

    // Make signals/slots connections

    setupConnections();

    // -- Read settings --------------------------------

    readSettings();
    applySettings();
    setAutoSaveSettings("ImageViewer Settings");

    //-------------------------------------------------------------
    
    m_rightSidebar->loadViewState();
    m_rightSidebar->populateTags();
}

ImageWindow::~ImageWindow()
{
    m_instance = 0;

    unLoadImagePlugins();

    // No need to delete m_imagePluginLoader instance here, it will be done by main interface.
    
    delete m_rightSidebar;
}

void ImageWindow::closeEvent(QCloseEvent* e)
{
    if (!e)
        return;

    if (!promptUserSave(m_urlCurrent))
        return;

    saveSettings();
    e->accept();
}

void ImageWindow::setupConnections()
{
    setupStandardConnections();

    // To toogle properly keyboards shortcuts from comments & tags side bar tab.
    
    connect(m_rightSidebar, SIGNAL(signalNextItem()),
            this, SLOT(slotForward()));
                
    connect(m_rightSidebar, SIGNAL(signalPrevItem()),
            this, SLOT(slotBackward()));

    connect(this, SIGNAL(signalSelectionChanged( QRect* )),
            m_rightSidebar, SLOT(slotImageSelectionChanged( QRect * )));

    connect(this, SIGNAL(signalNoCurrentItem()),
            m_rightSidebar, SLOT(slotNoCurrentItem()));

    ImageAttributesWatch *watch = ImageAttributesWatch::instance();

    connect(watch, SIGNAL(signalFileMetadataChanged(const KURL &)),
            this, SLOT(slotFileMetadataChanged(const KURL &)));
}

void ImageWindow::setupUserArea()
{
    QWidget* widget  = new QWidget(this);
    QHBoxLayout *lay = new QHBoxLayout(widget);
    
    m_splitter       = new QSplitter(widget);
    m_canvas         = new Canvas(m_splitter);
    
    QSizePolicy rightSzPolicy(QSizePolicy::Preferred, QSizePolicy::Expanding, 2, 1);
    m_canvas->setSizePolicy(rightSzPolicy);
        
    m_rightSidebar   = new ImagePropertiesSideBarDB(widget, "ImageEditor Right Sidebar", m_splitter,
                                                    Sidebar::Right, true, false);
    lay->addWidget(m_splitter);
    lay->addWidget(m_rightSidebar);
    
    m_splitter->setFrameStyle( QFrame::NoFrame );
    m_splitter->setFrameShadow( QFrame::Plain );
    m_splitter->setFrameShape( QFrame::NoFrame );
    m_splitter->setOpaqueResize(false);
    setCentralWidget(widget);
}

void ImageWindow::setupActions()
{
    setupStandardActions();

    // -- Rating actions ---------------------------------------------------------------

    m_0Star = new KAction(i18n("No Star"), CTRL+Key_0,
                          m_rightSidebar, SLOT(slotAssignRatingNoStar()),
                          actionCollection(), "imageview_ratenostar");
    m_1Star = new KAction(i18n("One Star"), CTRL+Key_1,
                          m_rightSidebar, SLOT(slotAssignRatingOneStar()),
                          actionCollection(), "imageview_rateonestar");
    m_2Star = new KAction(i18n("Two Star"), CTRL+Key_2, 
                          m_rightSidebar, SLOT(slotAssignRatingTwoStar()),
                          actionCollection(), "imageview_ratetwostar");
    m_3Star = new KAction(i18n("Three Star"), CTRL+Key_3, 
                          m_rightSidebar, SLOT(slotAssignRatingThreeStar()),
                          actionCollection(), "imageview_ratethreestar");
    m_4Star = new KAction(i18n("Four Star"), CTRL+Key_4, 
                          m_rightSidebar, SLOT(slotAssignRatingFourStar()),
                          actionCollection(), "imageview_ratefourstar");
    m_5Star = new KAction(i18n("Five Star"), CTRL+Key_5, 
                          m_rightSidebar, SLOT(slotAssignRatingFiveStar()),
                          actionCollection(), "imageview_ratefivestar");

    // ---------------------------------------------------------------------------------

    createGUI("digikamimagewindowui.rc", false);

    setupStandardAccelerators();
}

void ImageWindow::applySettings()
{
    applyStandardSettings();
    
    KConfig* config = kapp->config();
    config->setGroup("ImageViewer Settings");

    AlbumSettings *settings = AlbumSettings::instance();
    if (settings->getUseTrash())
    {
        m_fileDeleteAction->setIcon("edittrash");
        m_fileDeleteAction->setText(i18n("Move to Trash"));
    }
    else
    {
        m_fileDeleteAction->setIcon("editdelete");
        m_fileDeleteAction->setText(i18n("Delete File"));
    }

    m_canvas->setExifOrient(settings->getExifRotate());
    m_setExifOrientationTag = settings->getExifSetOrientation();
}

void ImageWindow::loadURL(const KURL::List& urlList, const KURL& urlCurrent,
                          const QString& caption, bool allowSaving, AlbumIconView* view)
{
    // if window is iconified, show it
    if (isMinimized())
    {
        KWin::deIconifyWindow(winId());
    }

    if (!promptUserSave(m_urlCurrent))
        return;

    setCaption(i18n("digiKam Image Editor - %1").arg(caption));

    m_view        = view;
    m_urlList     = urlList;
    m_urlCurrent  = urlCurrent;
    m_allowSaving = allowSaving;

    m_saveAction->setEnabled(false);
    m_revertAction->setEnabled(false);
    m_undoAction->setEnabled(false);
    m_redoAction->setEnabled(false);

    QTimer::singleShot(0, this, SLOT(slotLoadCurrent()));
}

void ImageWindow::loadImageInfos(const ImageInfoList &imageInfoList, ImageInfo *imageInfoCurrent,
                                 const QString& caption, bool allowSaving, AlbumIconView* view)
{
    m_imageInfoList    = imageInfoList;
    m_imageInfoCurrent = imageInfoCurrent;

    m_imageInfoList.setAutoDelete(true);

    // create URL list
    KURL::List urlList;

    ImageInfoListIterator it(m_imageInfoList);
    ImageInfo *info;
    for (; (info = it.current()); ++it)
    {
        urlList.append(info->kurl());
    }

    loadURL(urlList, imageInfoCurrent->kurl(), caption, allowSaving, view);
}

void ImageWindow::slotLoadCurrent()
{
    KURL::List::iterator it = m_urlList.find(m_urlCurrent);

    setViewToURL(*it);

    if (it != m_urlList.end())
    {
        m_canvas->load(m_urlCurrent.path(), m_IOFileSettings);
        
        ++it;
        if (it != m_urlList.end())
            m_canvas->preload((*it).path());
    }
}

void ImageWindow::setViewToURL(const KURL &url)
{
    if (m_view)
    {
        IconItem* item = m_view->findItem(url.url());
        m_view->clearSelection();
        m_view->updateContents();
        if (item)
            m_view->setCurrentItem(item);
    }
}

void ImageWindow::slotForward()
{
    if(!promptUserSave(m_urlCurrent))
        return;

    KURL::List::iterator it = m_urlList.find(m_urlCurrent);
    int index = m_imageInfoList.find(m_imageInfoCurrent);

    if (it != m_urlList.end()) 
    {
        if (m_urlCurrent != m_urlList.last())
        {
           KURL urlNext = *(++it);
           m_imageInfoCurrent = m_imageInfoList.at(index + 1);
           m_urlCurrent = urlNext;
           slotLoadCurrent();
        }
    }
}

void ImageWindow::slotBackward()
{
    if(!promptUserSave(m_urlCurrent))
        return;

    KURL::List::iterator it = m_urlList.find(m_urlCurrent);
    int index = m_imageInfoList.find(m_imageInfoCurrent);

    if (it != m_urlList.begin()) 
    {
        if (m_urlCurrent != m_urlList.first())
        {
            KURL urlPrev = *(--it);
            m_imageInfoCurrent = m_imageInfoList.at(index - 1);
            m_urlCurrent = urlPrev;
            slotLoadCurrent();
        }
    }
}

void ImageWindow::slotFirst()
{
    if(!promptUserSave(m_urlCurrent))
        return;

    m_urlCurrent = m_urlList.first();
    m_imageInfoCurrent = m_imageInfoList.first();
    slotLoadCurrent();
}

void ImageWindow::slotLast()
{
    if(!promptUserSave(m_urlCurrent))
        return;

    m_urlCurrent = m_urlList.last();
    m_imageInfoCurrent = m_imageInfoList.first();
    slotLoadCurrent();
}

void ImageWindow::slotContextMenu()
{
    if (m_contextMenu)
    {
        TagsPopupMenu* assignTagsMenu = 0;
        TagsPopupMenu* removeTagsMenu = 0;
        int separatorID = -1;

        if (m_imageInfoCurrent)
        {
            Q_LLONG id = m_imageInfoCurrent->id();
            QValueList<Q_LLONG> idList;
            idList.append(id);

            assignTagsMenu = new TagsPopupMenu(idList, 1000, TagsPopupMenu::ASSIGN);
            removeTagsMenu = new TagsPopupMenu(idList, 2000, TagsPopupMenu::REMOVE);

            separatorID = m_contextMenu->insertSeparator();

            m_contextMenu->insertItem(i18n("Assign Tag"), assignTagsMenu);
            int i = m_contextMenu->insertItem(i18n("Remove Tag"), removeTagsMenu);

            connect(assignTagsMenu, SIGNAL(signalTagActivated(int)),
                    this, SLOT(slotAssignTag(int)));
                    
            connect(removeTagsMenu, SIGNAL(signalTagActivated(int)),
                    this, SLOT(slotRemoveTag(int)));

            AlbumDB* db = AlbumManager::instance()->albumDB();
            if (!db->hasTags( idList ))
                m_contextMenu->setItemEnabled(i,false);
        }

        m_contextMenu->exec(QCursor::pos());

        if (separatorID != -1)
        {
            m_contextMenu->removeItem(separatorID);
        }

        delete assignTagsMenu;
        delete removeTagsMenu;
    }
}

void ImageWindow::slotChanged()
{
    QString mpixels;
    QSize dims(m_canvas->imageWidth(), m_canvas->imageHeight());
    mpixels.setNum(dims.width()*dims.height()/1000000.0, 'f', 2);
    QString str = (!dims.isValid()) ? i18n("Unknown") : i18n("%1x%2 (%3Mpx)")
                  .arg(dims.width()).arg(dims.height()).arg(mpixels);
    m_resLabel->setText(str);

    if (m_urlCurrent.isValid())
    {
        KURL u(m_urlCurrent.directory());

        QRect sel = m_canvas->getSelectedArea();
        DImg* img = DImgInterface::instance()->getImg();

        if (m_imageInfoCurrent)
        {
            KURL::List::iterator it = m_urlList.find(m_urlCurrent);
            bool hasPrevious = it != m_urlList.end();
            bool hasNext     = it != m_urlList.begin();

            m_rightSidebar->itemChanged(m_urlCurrent.url(), m_imageInfoCurrent,
                                        hasPrevious, hasNext, sel.isNull() ? 0 : &sel, img);
        }
        else
            m_rightSidebar->itemChanged(m_urlCurrent.url(), sel.isNull() ? 0 : &sel, img);
    }
}

void ImageWindow::slotUndoStateChanged(bool moreUndo, bool moreRedo, bool canSave)
{
    m_revertAction->setEnabled(canSave);
    m_undoAction->setEnabled(moreUndo);
    m_redoAction->setEnabled(moreRedo);

    if (m_allowSaving)
        m_saveAction->setEnabled(canSave);

    if (!moreUndo)
        m_rotatedOrFlipped = false;        
}

void ImageWindow::slotAssignTag(int tagID)
{
    if (m_imageInfoCurrent)
    {
        QStringList oldKeywords = m_imageInfoCurrent->tagNames();

        m_imageInfoCurrent->setTag(tagID);

        // Update Image Tags like Iptc keywords tags.

        if (AlbumSettings::instance())
        {
            if (AlbumSettings::instance()->getSaveIptcRating())
            {
                DMetadata metadata(m_imageInfoCurrent->filePath());
                metadata.setImageKeywords(oldKeywords, m_imageInfoCurrent->tagNames());
                metadata.applyChanges();
            }
        }
    }
}

void ImageWindow::slotRemoveTag(int tagID)
{
    if (m_imageInfoCurrent)
    {
        QStringList oldKeywords = m_imageInfoCurrent->tagNames();

        m_imageInfoCurrent->removeTag(tagID);

        // Update Image Tags like Iptc keywords tags.

        if (AlbumSettings::instance())
        {
            if (AlbumSettings::instance()->getSaveIptcRating())
            {
                DMetadata metadata(m_imageInfoCurrent->filePath());
                metadata.setImageKeywords(oldKeywords, m_imageInfoCurrent->tagNames());
                metadata.applyChanges();
            }
        }
    }
}

void ImageWindow::slotUpdateItemInfo()
{
    uint index = m_urlList.findIndex(m_urlCurrent);

    m_rotatedOrFlipped = false;
    
    QString text = m_urlCurrent.filename() + i18n(" (%2 of %3)")
                                             .arg(QString::number(index+1))
                                             .arg(QString::number(m_urlList.count()));
    m_nameLabel->setText(text);

    if (m_urlList.count() == 1) 
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

    if (index == 0) 
    {
        m_backwardAction->setEnabled(false);
        m_firstAction->setEnabled(false);
    }

    if (index == m_urlList.count()-1) 
    {
        m_forwardAction->setEnabled(false);
        m_lastAction->setEnabled(false);
    }

    // Disable some menu actions if the current root image URL
    // isn't include in the digiKam Albums library database.
    // This is necessary when ImageEditor is opened from cameraclient.

    KURL u(m_urlCurrent.directory());
    PAlbum *palbum = AlbumManager::instance()->findPAlbum(u);

    if (!palbum)
    {
        m_fileDeleteAction->setEnabled(false);
    }
    else
    {
        m_fileDeleteAction->setEnabled(true);
    }
}

void ImageWindow::setup(bool iccSetupPage)
{
    Setup setup(this, 0, iccSetupPage ? Setup::IccProfiles : Setup::Editor);
    
    if (setup.exec() != QDialog::Accepted)
        return;

    unLoadImagePlugins();
    m_imagePluginLoader->loadPluginsFromList(setup.imagePluginsPage()->getImagePluginsListEnable());
    kapp->config()->sync();
    loadImagePlugins();
    
    applySettings();
}

void ImageWindow::toggleGUI2FullScreen()
{
    if (m_fullScreen)
        m_rightSidebar->restore();
    else
        m_rightSidebar->backup();
}

void ImageWindow::toggleActions2SlideShow(bool val)
{
    toggleActions(val);
}

void ImageWindow::saveIsComplete()
{
    // With save(), we do not reload the image but just continue using the data.
    // This means that a saving operation does not lead to quality loss for
    // subsequent editing operations.

    // put image in cache, the LoadingCacheInterface cares for the details
    LoadingCacheInterface::putImage(m_savingContext->destinationURL.path(), m_canvas->currentImage());

    // notify main app that file changed
    emit signalFileModified(m_savingContext->destinationURL);

    // all that is done in slotLoadCurrent, except for loading
    KURL::List::iterator it = m_urlList.find(m_urlCurrent);
    setViewToURL(*it);

    if (++it != m_urlList.end()) 
    {
        m_canvas->preload((*it).path());
    }
    //slotLoadCurrent();
}

void ImageWindow::saveAsIsComplete()
{
    // Nothing to be done if operating without database
    if (!m_imageInfoCurrent)
        return;

    // Find the src and dest albums ------------------------------------------

    KURL srcDirURL(QDir::cleanDirPath(m_savingContext->srcURL.directory()));
    PAlbum* srcAlbum = AlbumManager::instance()->findPAlbum(srcDirURL);

    KURL dstDirURL(QDir::cleanDirPath(m_savingContext->destinationURL.directory()));
    PAlbum* dstAlbum = AlbumManager::instance()->findPAlbum(dstDirURL);

    if (dstAlbum && srcAlbum)
    {
        // Now copy the metadata of the original file to the new file ------------

        ImageInfo newInfo(m_imageInfoCurrent->copyItem(dstAlbum, m_savingContext->destinationURL.fileName()));

        if ( m_urlList.find(m_savingContext->destinationURL) == m_urlList.end() )
        {   // The image file did not exist in the list.
            KURL::List::iterator it = m_urlList.find(m_savingContext->srcURL);
            int index = m_urlList.findIndex(m_savingContext->srcURL);
            m_urlList.insert(it, m_savingContext->destinationURL);
            m_imageInfoCurrent = new ImageInfo(newInfo);
            m_imageInfoList.insert(index, m_imageInfoCurrent);
        }
        else if (m_urlCurrent != m_savingContext->destinationURL)
        {
            for (ImageInfo *info = m_imageInfoList.first(); info; info = m_imageInfoList.next())
            {
                if (info->kurl() == m_savingContext->destinationURL)
                {
                    m_imageInfoCurrent = new ImageInfo(newInfo);
                    // setAutoDelete is true
                    m_imageInfoList.replace(m_imageInfoList.at(), m_imageInfoCurrent);
                    break;
                }
            }
        }

        m_urlCurrent = m_savingContext->destinationURL;
        m_canvas->switchToLastSaved(m_savingContext->destinationURL.path());
        slotUpdateItemInfo();
        LoadingCacheInterface::putImage(m_savingContext->destinationURL.path(), m_canvas->currentImage());

        // notify main app that file changed or a file is added
        if(m_savingContext->destinationExisted)
            emit signalFileModified(m_savingContext->destinationURL);
        else
            emit signalFileAdded(m_savingContext->destinationURL);

        // all that is done in slotLoadCurrent, except for loading
        KURL::List::iterator it = m_urlList.find(m_urlCurrent);

        if (it != m_urlList.end()) 
        {
            setViewToURL(*it);
            m_canvas->preload((*++it).path());
        }
    }
    else
    {
        //TODO: make the user aware that the new path has not been used as new current filename
        //      because it is outside the digikam album hierachy
    }
}

bool ImageWindow::save()
{
    startingSave(m_urlCurrent);
    return true;
}

bool ImageWindow::saveAs()
{
    return ( startingSaveAs(m_urlCurrent) );
}

void ImageWindow::slotDeleteCurrentItem()
{
    KURL u(m_urlCurrent.directory());
    PAlbum *palbum = AlbumManager::instance()->findPAlbum(u);

    if (!palbum)
        return;

    AlbumSettings* settings = AlbumSettings::instance();

    if (!settings->getUseTrash())
    {
        QString warnMsg(i18n("About to Delete File \"%1\"\nAre you sure?")
                        .arg(m_urlCurrent.filename()));
        if (KMessageBox::warningContinueCancel(this,
                                               warnMsg,
                                               i18n("Warning"),
                                               i18n("Delete"))
            !=  KMessageBox::Continue)
        {
            return;
        }
    }

    if (!SyncJob::userDelete(m_urlCurrent))
    {
        QString errMsg(SyncJob::lastErrorMsg());
        KMessageBox::error(this, errMsg, errMsg);
        return;
    }

    emit signalFileDeleted(m_urlCurrent);

    KURL CurrentToRemove = m_urlCurrent;
    KURL::List::iterator it = m_urlList.find(m_urlCurrent);

    if (it != m_urlList.end())
    {
        if (m_urlCurrent != m_urlList.last())
        {
            // Try to get the next image in the current Album...

            KURL urlNext = *(++it);
            m_urlCurrent = urlNext;
            m_urlList.remove(CurrentToRemove);
            slotLoadCurrent();
            return;
        }
        else if (m_urlCurrent != m_urlList.first())
        {
            // Try to get the previous image in the current Album...

            KURL urlPrev = *(--it);
            m_urlCurrent = urlPrev;
            m_urlList.remove(CurrentToRemove);
            slotLoadCurrent();
            return;
        }
    }

    // No image in the current Album -> Quit ImageEditor...

    KMessageBox::information(this,
                             i18n("There is no image to show in the current album.\n"
                                  "The image editor will be closed."),
                             i18n("No Image in Current Album"));

    close();
}

void ImageWindow::slotFileMetadataChanged(const KURL &url)
{
    if (url == m_urlCurrent)
    {
        m_canvas->readMetadataFromFile(url.path());
    }
}

}  // namespace Digikam

#include "imagewindow.moc"

