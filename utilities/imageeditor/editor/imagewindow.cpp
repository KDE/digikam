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
#include <kdeversion.h>
#include <kmenubar.h>
#include <ktoolbar.h>
#include <kaccel.h>
#include <kaction.h>
#include <kstdaccel.h>
#include <kstdaction.h>
#include <kstdguiitem.h>
#include <kstatusbar.h>
#include <kprogress.h>
#include <kwin.h>

// Local includes.

#include "ddebug.h"
#include "dpopupmenu.h"
#include "canvas.h"
#include "dimginterface.h"
#include "themeengine.h"
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
#include "imageattributeswatch.h"
#include "deletedialog.h"
#include "imagewindow.h"
#include "imagewindow.moc"

namespace Digikam
{

class ImageWindowPriv
{

public:

    ImageWindowPriv()
    {
        allowSaving                         = true;
        star0                               = 0;
        star1                               = 0;
        star2                               = 0;
        star3                               = 0;
        star4                               = 0;
        star5                               = 0;
        fileDeletePermanentlyAction         = 0;
        fileDeletePermanentlyDirectlyAction = 0;
        fileTrashDirectlyAction             = 0;
        view                                = 0;
        imageInfoCurrent                    = 0;
        rightSidebar                        = 0;
        contextMenu                         = 0;
    }

    // If image editor is launched by camera interface, current
    // image cannot be saved.
    bool                      allowSaving;

    KURL::List                urlList;
    KURL                      urlCurrent;

    // Rating actions.
    KAction                  *star0;
    KAction                  *star1;
    KAction                  *star2;
    KAction                  *star3;
    KAction                  *star4;
    KAction                  *star5;

    // Delete actions
    KAction                  *fileDeletePermanentlyAction;
    KAction                  *fileDeletePermanentlyDirectlyAction;
    KAction                  *fileTrashDirectlyAction;

    AlbumIconView            *view;
    ImageInfoList             imageInfoList;
    ImageInfo                *imageInfoCurrent;

    ImagePropertiesSideBarDB *rightSidebar;

    DPopupMenu               *contextMenu;
};

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
    d = new ImageWindowPriv;
    m_instance = this;

    // -- Build the GUI -------------------------------

    setupUserArea();
    setupStatusBar();
    setupActions();

    // Load image plugins to GUI

    m_imagePluginLoader = ImagePluginLoader::instance();
    loadImagePlugins();

    // Create context menu.

    d->contextMenu = new DPopupMenu(this);
    KActionCollection *ac = actionCollection();
    if( ac->action("editorwindow_backward") ) ac->action("editorwindow_backward")->plug(d->contextMenu);
    if( ac->action("editorwindow_forward") ) ac->action("editorwindow_forward")->plug(d->contextMenu);
    d->contextMenu->insertSeparator();
    if( ac->action("editorwindow_rotate") ) ac->action("editorwindow_rotate")->plug(d->contextMenu);
    if( ac->action("editorwindow_crop") ) ac->action("editorwindow_crop")->plug(d->contextMenu);
    d->contextMenu->insertSeparator();
    if( ac->action("editorwindow_delete") ) ac->action("editorwindow_delete")->plug(d->contextMenu);

    // Make signals/slots connections

    setupConnections();

    // -- Read settings --------------------------------

    readSettings();
    applySettings();
    setAutoSaveSettings("ImageViewer Settings");

    //-------------------------------------------------------------

    d->rightSidebar->loadViewState();
    d->rightSidebar->populateTags();
}

ImageWindow::~ImageWindow()
{
    m_instance = 0;

    unLoadImagePlugins();

    // No need to delete m_imagePluginLoader instance here, it will be done by main interface.

    delete d->rightSidebar;
    delete d;
}

void ImageWindow::closeEvent(QCloseEvent* e)
{
    if (!e)
        return;

    if (!queryClose())
        return;

    // put right side bar in a defined state
    emit signalNoCurrentItem();

    m_canvas->resetImage();

    saveSettings();

    e->accept();
}

bool ImageWindow::queryClose()
{
    // Note: we reimplement closeEvent above for this window.
    // Additionally, queryClose is called from DigikamApp.

    // wait if a save operation is currently running
    if (!waitForSavingToComplete())
        return false;

    return promptUserSave(d->urlCurrent);
}

void ImageWindow::setupConnections()
{
    setupStandardConnections();

    // To toggle properly keyboards shortcuts from comments & tags side bar tab.

    connect(d->rightSidebar, SIGNAL(signalNextItem()),
            this, SLOT(slotForward()));

    connect(d->rightSidebar, SIGNAL(signalPrevItem()),
            this, SLOT(slotBackward()));

    connect(this, SIGNAL(signalSelectionChanged( QRect* )),
            d->rightSidebar, SLOT(slotImageSelectionChanged( QRect * )));

    connect(this, SIGNAL(signalNoCurrentItem()),
            d->rightSidebar, SLOT(slotNoCurrentItem()));

    ImageAttributesWatch *watch = ImageAttributesWatch::instance();

    connect(watch, SIGNAL(signalFileMetadataChanged(const KURL &)),
            this, SLOT(slotFileMetadataChanged(const KURL &)));

    connect(ThemeEngine::instance(), SIGNAL(signalThemeChanged()),
            this, SLOT(slotThemeChanged()));
}

void ImageWindow::setupUserArea()
{
    QWidget* widget  = new QWidget(this);
    QHBoxLayout *lay = new QHBoxLayout(widget);

    m_splitter       = new QSplitter(widget);
    m_canvas         = new Canvas(m_splitter);

    QSizePolicy rightSzPolicy(QSizePolicy::Preferred, QSizePolicy::Expanding, 2, 1);
    m_canvas->setSizePolicy(rightSzPolicy);

    d->rightSidebar  = new ImagePropertiesSideBarDB(widget, "ImageEditor Right Sidebar", m_splitter,
                                                    Sidebar::Right, true, false);
    lay->addWidget(m_splitter);
    lay->addWidget(d->rightSidebar);

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

    d->star0 = new KAction(i18n("Assign Rating \"No Star\""), CTRL+Key_0,
                          d->rightSidebar, SLOT(slotAssignRatingNoStar()),
                          actionCollection(), "imageview_ratenostar");
    d->star1 = new KAction(i18n("Assign Rating \"One Star\""), CTRL+Key_1,
                          d->rightSidebar, SLOT(slotAssignRatingOneStar()),
                          actionCollection(), "imageview_rateonestar");
    d->star2 = new KAction(i18n("Assign Rating \"Two Stars\""), CTRL+Key_2,
                          d->rightSidebar, SLOT(slotAssignRatingTwoStar()),
                          actionCollection(), "imageview_ratetwostar");
    d->star3 = new KAction(i18n("Assign Rating \"Three Stars\""), CTRL+Key_3,
                          d->rightSidebar, SLOT(slotAssignRatingThreeStar()),
                          actionCollection(), "imageview_ratethreestar");
    d->star4 = new KAction(i18n("Assign Rating \"Four Stars\""), CTRL+Key_4,
                          d->rightSidebar, SLOT(slotAssignRatingFourStar()),
                          actionCollection(), "imageview_ratefourstar");
    d->star5 = new KAction(i18n("Assign Rating \"Five Stars\""), CTRL+Key_5,
                          d->rightSidebar, SLOT(slotAssignRatingFiveStar()),
                          actionCollection(), "imageview_ratefivestar");

    // -- Special Delete actions ---------------------------------------------------------------

    // Pop up dialog to ask user whether to permanently delete
    d->fileDeletePermanentlyAction = new KAction(i18n("Delete File Permanently"),
            "editdelete",
            SHIFT+Key_Delete,
            this,
            SLOT(slotDeleteCurrentItemPermanently()),
            actionCollection(),
            "image_delete_permanently");

    // These two actions are hidden, no menu entry, no toolbar entry, no shortcut.
    // Power users may add them.
    d->fileDeletePermanentlyDirectlyAction = new KAction(i18n("Delete Permanently without Confirmation"),
            "editdelete",
            0,
            this,
            SLOT(slotDeleteCurrentItemPermanentlyDirectly()),
            actionCollection(),
            "image_delete_permanently_directly");

    d->fileTrashDirectlyAction             = new KAction(i18n("Move to Trash without Confirmation"),
            "edittrash",
            0,
            this,
            SLOT(slotTrashCurrentItemDirectly()),
            actionCollection(),
            "image_trash_directly");

    // ---------------------------------------------------------------------------------

    createGUI("digikamimagewindowui.rc", false);

    setupStandardAccelerators();
}

void ImageWindow::applySettings()
{
    applyStandardSettings();

    KConfig* config = kapp->config();
    config->setGroup("ImageViewer Settings");

    if (!config->readBoolEntry("UseThemeBackgroundColor", true))
        m_bgColor = config->readColorEntry("BackgroundColor", &Qt::black);
    else
        m_bgColor = ThemeEngine::instance()->baseColor();

    m_canvas->setBackgroundColor(m_bgColor);

    AlbumSettings *settings = AlbumSettings::instance();
    m_canvas->setExifOrient(settings->getExifRotate());
    m_setExifOrientationTag = settings->getExifSetOrientation();
}

void ImageWindow::loadURL(const KURL::List& urlList, const KURL& urlCurrent,
                          const QString& caption, bool allowSaving, AlbumIconView* view)
{
    d->urlList          = urlList;
    d->urlCurrent       = urlCurrent;
    d->imageInfoList    = ImageInfoList();
    d->imageInfoCurrent = 0;

    loadCurrentList(caption, allowSaving, view);
}

void ImageWindow::loadImageInfos(const ImageInfoList &imageInfoList, ImageInfo *imageInfoCurrent,
                                 const QString& caption, bool allowSaving, AlbumIconView* view)
{
    // the ownership of the list's objects is passed to us
    d->imageInfoList    = imageInfoList;
    d->imageInfoCurrent = imageInfoCurrent;

    d->imageInfoList.setAutoDelete(true);

    // create URL list
    d->urlList = KURL::List();

    ImageInfoListIterator it(d->imageInfoList);
    ImageInfo *info;
    for (; (info = it.current()); ++it)
    {
        d->urlList.append(info->kurl());
    }

    d->urlCurrent  = d->imageInfoCurrent->kurl();

    loadCurrentList(caption, allowSaving, view);
}

void ImageWindow::loadCurrentList(const QString& caption, bool allowSaving, AlbumIconView* view)
{
    // this method contains the code shared by loadURL and loadImageInfos

    // if window is iconified, show it
    if (isMinimized())
    {
        KWin::deIconifyWindow(winId());
    }

    if (!promptUserSave(d->urlCurrent))
        return;

    setCaption(i18n("digiKam Image Editor - %1").arg(caption));

    d->view        = view;
    d->allowSaving = allowSaving;

    m_saveAction->setEnabled(false);
    m_revertAction->setEnabled(false);
    m_undoAction->setEnabled(false);
    m_redoAction->setEnabled(false);

    QTimer::singleShot(0, this, SLOT(slotLoadCurrent()));
}

void ImageWindow::slotLoadCurrent()
{
    KURL::List::iterator it = d->urlList.find(d->urlCurrent);

    if (it != d->urlList.end())
    {
        m_canvas->load(d->urlCurrent.path(), m_IOFileSettings);

        ++it;
        if (it != d->urlList.end())
            m_canvas->preload((*it).path());
    }

    // Do this _after_ the canvas->load(), so that the main view histogram does not load
    // a smaller version if a raw image, and after that the DImgInterface loads the full version.
    // So first let DImgInterface create its loading task, only then any external objects.
    setViewToURL(d->urlCurrent);
}

void ImageWindow::setViewToURL(const KURL &url)
{
    if (d->view)
    {
        IconItem* item = d->view->findItem(url.url());
        d->view->clearSelection();
        d->view->updateContents();
        if (item)
            d->view->setCurrentItem(item);
    }
}

void ImageWindow::slotForward()
{
    if(!promptUserSave(d->urlCurrent))
        return;

    KURL::List::iterator it = d->urlList.find(d->urlCurrent);
    int index = d->imageInfoList.find(d->imageInfoCurrent);

    if (it != d->urlList.end()) 
    {
        if (d->urlCurrent != d->urlList.last())
        {
           KURL urlNext = *(++it);
           d->imageInfoCurrent = d->imageInfoList.at(index + 1);
           d->urlCurrent = urlNext;
           slotLoadCurrent();
        }
    }
}

void ImageWindow::slotBackward()
{
    if(!promptUserSave(d->urlCurrent))
        return;

    KURL::List::iterator it = d->urlList.find(d->urlCurrent);
    int index = d->imageInfoList.find(d->imageInfoCurrent);

    if (it != d->urlList.begin()) 
    {
        if (d->urlCurrent != d->urlList.first())
        {
            KURL urlPrev = *(--it);
            d->imageInfoCurrent = d->imageInfoList.at(index - 1);
            d->urlCurrent = urlPrev;
            slotLoadCurrent();
        }
    }
}

void ImageWindow::slotFirst()
{
    if(!promptUserSave(d->urlCurrent))
        return;

    d->urlCurrent = d->urlList.first();
    d->imageInfoCurrent = d->imageInfoList.first();
    slotLoadCurrent();
}

void ImageWindow::slotLast()
{
    if(!promptUserSave(d->urlCurrent))
        return;

    d->urlCurrent = d->urlList.last();
    d->imageInfoCurrent = d->imageInfoList.first();
    slotLoadCurrent();
}

void ImageWindow::slotContextMenu()
{
    if (d->contextMenu)
    {
        TagsPopupMenu* assignTagsMenu = 0;
        TagsPopupMenu* removeTagsMenu = 0;
        int separatorID = -1;

        if (d->imageInfoCurrent)
        {
            Q_LLONG id = d->imageInfoCurrent->id();
            QValueList<Q_LLONG> idList;
            idList.append(id);

            assignTagsMenu = new TagsPopupMenu(idList, 1000, TagsPopupMenu::ASSIGN);
            removeTagsMenu = new TagsPopupMenu(idList, 2000, TagsPopupMenu::REMOVE);

            separatorID = d->contextMenu->insertSeparator();

            d->contextMenu->insertItem(i18n("Assign Tag"), assignTagsMenu);
            int i = d->contextMenu->insertItem(i18n("Remove Tag"), removeTagsMenu);

            connect(assignTagsMenu, SIGNAL(signalTagActivated(int)),
                    this, SLOT(slotAssignTag(int)));

            connect(removeTagsMenu, SIGNAL(signalTagActivated(int)),
                    this, SLOT(slotRemoveTag(int)));

            AlbumDB* db = AlbumManager::instance()->albumDB();
            if (!db->hasTags( idList ))
                d->contextMenu->setItemEnabled(i,false);
        }

        d->contextMenu->exec(QCursor::pos());

        if (separatorID != -1)
        {
            d->contextMenu->removeItem(separatorID);
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

    if (d->urlCurrent.isValid())
    {
        KURL u(d->urlCurrent.directory());

        QRect sel = m_canvas->getSelectedArea();
        DImg* img = DImgInterface::instance()->getImg();

        if (d->imageInfoCurrent)
        {
            KURL::List::iterator it = d->urlList.find(d->urlCurrent);
            bool hasPrevious = it != d->urlList.end();
            bool hasNext     = it != d->urlList.begin();

            d->rightSidebar->itemChanged(d->urlCurrent.url(), d->imageInfoCurrent,
                                        hasPrevious, hasNext, sel.isNull() ? 0 : &sel, img);
        }
        else
            d->rightSidebar->itemChanged(d->urlCurrent.url(), sel.isNull() ? 0 : &sel, img);
    }
}

void ImageWindow::slotUndoStateChanged(bool moreUndo, bool moreRedo, bool canSave)
{
    m_revertAction->setEnabled(canSave);
    m_undoAction->setEnabled(moreUndo);
    m_redoAction->setEnabled(moreRedo);

    if (d->allowSaving)
        m_saveAction->setEnabled(canSave);

    if (!moreUndo)
        m_rotatedOrFlipped = false;
}

void ImageWindow::slotAssignTag(int tagID)
{
    if (d->imageInfoCurrent)
    {
        QStringList oldKeywords = d->imageInfoCurrent->tagNames();

        d->imageInfoCurrent->setTag(tagID);

        // Update Image Tags like Iptc keywords tags.

        if (AlbumSettings::instance())
        {
            if (AlbumSettings::instance()->getSaveIptcRating())
            {
                DMetadata metadata(d->imageInfoCurrent->filePath());
                metadata.setImageKeywords(oldKeywords, d->imageInfoCurrent->tagNames());
                metadata.applyChanges();
            }
        }
    }
}

void ImageWindow::slotRemoveTag(int tagID)
{
    if (d->imageInfoCurrent)
    {
        QStringList oldKeywords = d->imageInfoCurrent->tagNames();

        d->imageInfoCurrent->removeTag(tagID);

        // Update Image Tags like Iptc keywords tags.

        if (AlbumSettings::instance())
        {
            if (AlbumSettings::instance()->getSaveIptcRating())
            {
                DMetadata metadata(d->imageInfoCurrent->filePath());
                metadata.setImageKeywords(oldKeywords, d->imageInfoCurrent->tagNames());
                metadata.applyChanges();
            }
        }
    }
}

void ImageWindow::slotUpdateItemInfo()
{
    uint index = d->urlList.findIndex(d->urlCurrent);

    m_rotatedOrFlipped = false;
    
    QString text = d->urlCurrent.filename() + i18n(" (%2 of %3)")
                                             .arg(QString::number(index+1))
                                             .arg(QString::number(d->urlList.count()));
    m_nameLabel->setText(text);

    if (d->urlList.count() == 1) 
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

    if (index == d->urlList.count()-1) 
    {
        m_forwardAction->setEnabled(false);
        m_lastAction->setEnabled(false);
    }

    // Disable some menu actions if the current root image URL
    // isn't include in the digiKam Albums library database.
    // This is necessary when ImageEditor is opened from cameraclient.

    KURL u(d->urlCurrent.directory());
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

bool ImageWindow::setup(bool iccSetupPage)
{
    Setup setup(this, 0, iccSetupPage ? Setup::IccProfiles : Setup::LastPageUsed);    
        
    if (setup.exec() != QDialog::Accepted)
        return false;

    unLoadImagePlugins();
    m_imagePluginLoader->loadPluginsFromList(setup.imagePluginsPage()->getImagePluginsListEnable());
    kapp->config()->sync();
    loadImagePlugins();
    
    applySettings();
    return true;
}

void ImageWindow::toggleGUI2FullScreen()
{
    if (m_fullScreen)
        d->rightSidebar->restore();
    else
        d->rightSidebar->backup();
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
    KURL::List::iterator it = d->urlList.find(d->urlCurrent);
    setViewToURL(*it);

    if (++it != d->urlList.end()) 
    {
        m_canvas->preload((*it).path());
    }
    //slotLoadCurrent();
}

void ImageWindow::saveAsIsComplete()
{
    // Nothing to be done if operating without database
    if (!d->imageInfoCurrent)
        return;

    // Find the src and dest albums ------------------------------------------

    KURL srcDirURL(QDir::cleanDirPath(m_savingContext->srcURL.directory()));
    PAlbum* srcAlbum = AlbumManager::instance()->findPAlbum(srcDirURL);

    KURL dstDirURL(QDir::cleanDirPath(m_savingContext->destinationURL.directory()));
    PAlbum* dstAlbum = AlbumManager::instance()->findPAlbum(dstDirURL);

    if (dstAlbum && srcAlbum)
    {
        // Now copy the metadata of the original file to the new file ------------

        ImageInfo newInfo(d->imageInfoCurrent->copyItem(dstAlbum, m_savingContext->destinationURL.fileName()));

        if ( d->urlList.find(m_savingContext->destinationURL) == d->urlList.end() )
        {   // The image file did not exist in the list.
            KURL::List::iterator it = d->urlList.find(m_savingContext->srcURL);
            int index = d->urlList.findIndex(m_savingContext->srcURL);
            d->urlList.insert(it, m_savingContext->destinationURL);
            d->imageInfoCurrent = new ImageInfo(newInfo);
            d->imageInfoList.insert(index, d->imageInfoCurrent);
        }
        else if (d->urlCurrent != m_savingContext->destinationURL)
        {
            for (ImageInfo *info = d->imageInfoList.first(); info; info = d->imageInfoList.next())
            {
                if (info->kurl() == m_savingContext->destinationURL)
                {
                    d->imageInfoCurrent = new ImageInfo(newInfo);
                    // setAutoDelete is true
                    d->imageInfoList.replace(d->imageInfoList.at(), d->imageInfoCurrent);
                    break;
                }
            }
        }

        d->urlCurrent = m_savingContext->destinationURL;
        m_canvas->switchToLastSaved(m_savingContext->destinationURL.path());
        slotUpdateItemInfo();
        LoadingCacheInterface::putImage(m_savingContext->destinationURL.path(), m_canvas->currentImage());

        // notify main app that file changed or a file is added
        if(m_savingContext->destinationExisted)
            emit signalFileModified(m_savingContext->destinationURL);
        else
            emit signalFileAdded(m_savingContext->destinationURL);

        // all that is done in slotLoadCurrent, except for loading
        KURL::List::iterator it = d->urlList.find(d->urlCurrent);

        if (it != d->urlList.end()) 
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
    startingSave(d->urlCurrent);
    return true;
}

bool ImageWindow::saveAs()
{
    return ( startingSaveAs(d->urlCurrent) );
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

    KURL u;
    u.setPath(d->urlCurrent.directory());
    PAlbum *palbum = AlbumManager::instance()->findPAlbum(u);

    if (!palbum)
        return;

    bool useTrash;

    if (ask)
    {
        bool preselectDeletePermanently = permanently;

        DeleteDialog dialog(this);

        KURL::List urlList;
        urlList.append(d->urlCurrent);
        if (!dialog.confirmDeleteList(urlList,
             DeleteDialogMode::Files,
             preselectDeletePermanently ?
                     DeleteDialogMode::NoChoiceDeletePermanently : DeleteDialogMode::NoChoiceTrash))
            return;

        useTrash = !dialog.shouldDelete();
    }
    else
    {
        useTrash = !permanently;
    }

    // bring all (sidebar) to a defined state without letting them sit on the deleted file
    emit signalNoCurrentItem();

    if (!SyncJob::del(d->urlCurrent, useTrash))
    {
        QString errMsg(SyncJob::lastErrorMsg());
        KMessageBox::error(this, errMsg, errMsg);
        return;
    }

    emit signalFileDeleted(d->urlCurrent);

    KURL CurrentToRemove = d->urlCurrent;
    KURL::List::iterator it = d->urlList.find(d->urlCurrent);
    int index = d->imageInfoList.find(d->imageInfoCurrent);

    if (it != d->urlList.end())
    {
        if (d->urlCurrent != d->urlList.last())
        {
            // Try to get the next image in the current Album...

            KURL urlNext = *(++it);
            d->urlCurrent = urlNext;
            d->imageInfoCurrent = d->imageInfoList.at(index + 1);
            d->urlList.remove(CurrentToRemove);
            d->imageInfoList.remove(index);
            slotLoadCurrent();
            return;
        }
        else if (d->urlCurrent != d->urlList.first())
        {
            // Try to get the previous image in the current Album.

            KURL urlPrev = *(--it);
            d->urlCurrent = urlPrev;
            d->imageInfoCurrent = d->imageInfoList.at(index - 1);
            d->urlList.remove(CurrentToRemove);
            d->imageInfoList.remove(index);
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
    if (url == d->urlCurrent)
    {
        m_canvas->readMetadataFromFile(url.path());
    }
}

void ImageWindow::slotThemeChanged()
{
    m_canvas->setBackgroundColor(ThemeEngine::instance()->baseColor());
}

void ImageWindow::slotFilePrint()
{
    printImage(d->urlCurrent); 
};

}  // namespace Digikam


