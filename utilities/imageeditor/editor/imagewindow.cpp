/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-02-12
 * Description : digiKam image editor GUI
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2004-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// C++ includes.

#include <cstdio>

// Qt includes.

#include <qcursor.h>
#include <qtimer.h>
#include <qlabel.h>
#include <qimage.h>
#include <qsplitter.h>
#include <qpainter.h>
#include <qpixmap.h>

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

#include "constants.h"
#include "ddebug.h"
#include "dlogoaction.h"
#include "dpopupmenu.h"
#include "dragobjects.h"
#include "canvas.h"
#include "dimginterface.h"
#include "dimg.h"
#include "dmetadata.h"
#include "imageplugin.h"
#include "imagepluginloader.h"
#include "imageprint.h"
#include "albummanager.h"
#include "album.h"
#include "albumdb.h"
#include "albumsettings.h"
#include "syncjob.h"
#include "imageinfo.h"
#include "imagepropertiessidebardb.h"
#include "tagspopupmenu.h"
#include "ratingpopupmenu.h"
#include "slideshow.h"
#include "setup.h"
#include "iccsettingscontainer.h"
#include "iofilesettingscontainer.h"
#include "loadingcacheinterface.h"
#include "savingcontextcontainer.h"
#include "statusprogressbar.h"
#include "imageattributeswatch.h"
#include "deletedialog.h"
#include "metadatahub.h"
#include "themeengine.h"
#include "editorstackview.h"
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
        imageInfoCurrent                    = 0;
        rightSidebar                        = 0;
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

    ImageInfoList             imageInfoList;
    ImageInfo                *imageInfoCurrent;

    ImagePropertiesSideBarDB *rightSidebar;
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
    setAcceptDrops(true);

    // -- Build the GUI -------------------------------

    setupUserArea();
    setupStatusBar();
    setupActions();

    // Load image plugins to GUI

    m_imagePluginLoader = ImagePluginLoader::instance();
    loadImagePlugins();

    // Create context menu.

    setupContextMenu();

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

Sidebar* ImageWindow::rightSideBar() const
{
    return dynamic_cast<Sidebar*>(d->rightSidebar);
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

    connect(this, SIGNAL(signalSelectionChanged( const QRect &)),
            d->rightSidebar, SLOT(slotImageSelectionChanged( const QRect &)));

    connect(this, SIGNAL(signalNoCurrentItem()),
            d->rightSidebar, SLOT(slotNoCurrentItem()));

    ImageAttributesWatch *watch = ImageAttributesWatch::instance();

    connect(watch, SIGNAL(signalFileMetadataChanged(const KURL &)),
            this, SLOT(slotFileMetadataChanged(const KURL &)));
}

void ImageWindow::setupUserArea()
{
    QWidget* widget  = new QWidget(this);
    QHBoxLayout *lay = new QHBoxLayout(widget);

    m_splitter       = new QSplitter(widget);
    m_stackView      = new EditorStackView(m_splitter);
    m_canvas         = new Canvas(m_stackView);
    m_stackView->setCanvas(m_canvas);
    m_stackView->setViewMode(EditorStackView::CanvasMode);

    m_canvas->makeDefaultEditingCanvas();

    QSizePolicy rightSzPolicy(QSizePolicy::Preferred, QSizePolicy::Expanding, 2, 1);
    m_canvas->setSizePolicy(rightSzPolicy);

    d->rightSidebar  = new ImagePropertiesSideBarDB(widget, "ImageEditor Right Sidebar", m_splitter,
                                                    Sidebar::Right, true);
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

    // Provides a menu entry that allows showing/hiding the toolbar(s)
    setStandardToolBarMenuEnabled(true);

    // Provides a menu entry that allows showing/hiding the statusbar
    createStandardStatusBarAction();

    // -- Rating actions ---------------------------------------------------------------

    d->star0 = new KAction(i18n("Assign Rating \"No Stars\""), CTRL+Key_0,
                          this, SLOT(slotAssignRatingNoStar()),
                          actionCollection(), "imageview_ratenostar");
    d->star1 = new KAction(i18n("Assign Rating \"One Star\""), CTRL+Key_1,
                          this, SLOT(slotAssignRatingOneStar()),
                          actionCollection(), "imageview_rateonestar");
    d->star2 = new KAction(i18n("Assign Rating \"Two Stars\""), CTRL+Key_2,
                          this, SLOT(slotAssignRatingTwoStar()),
                          actionCollection(), "imageview_ratetwostar");
    d->star3 = new KAction(i18n("Assign Rating \"Three Stars\""), CTRL+Key_3,
                          this, SLOT(slotAssignRatingThreeStar()),
                          actionCollection(), "imageview_ratethreestar");
    d->star4 = new KAction(i18n("Assign Rating \"Four Stars\""), CTRL+Key_4,
                          this, SLOT(slotAssignRatingFourStar()),
                          actionCollection(), "imageview_ratefourstar");
    d->star5 = new KAction(i18n("Assign Rating \"Five Stars\""), CTRL+Key_5,
                          this, SLOT(slotAssignRatingFiveStar()),
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

    new DLogoAction(actionCollection(), "logo_action");

    createGUI("digikamimagewindowui.rc", false);

    setupStandardAccelerators();
}

void ImageWindow::applySettings()
{
    applyStandardSettings();

    AlbumSettings *settings = AlbumSettings::instance();
    m_canvas->setExifOrient(settings->getExifRotate());
    m_setExifOrientationTag = settings->getExifSetOrientation();
    refreshView();
}

void ImageWindow::refreshView()
{
    d->rightSidebar->refreshTagsView();
}

void ImageWindow::loadURL(const KURL::List& urlList, const KURL& urlCurrent,
                          const QString& caption, bool allowSaving)
{
    if (!promptUserSave(d->urlCurrent))
        return;

    d->urlList          = urlList;
    d->urlCurrent       = urlCurrent;
    d->imageInfoList    = ImageInfoList();
    d->imageInfoCurrent = 0;

    loadCurrentList(caption, allowSaving);
}

void ImageWindow::loadImageInfos(const ImageInfoList &imageInfoList, ImageInfo *imageInfoCurrent,
                                 const QString& caption, bool allowSaving)
{
    // The ownership of objects of imageInfoList is passed to us.
    // imageInfoCurrent is contained in imageInfoList.

    // Very first thing is to check for changes, user may choose to cancel operation
    if (!promptUserSave(d->urlCurrent))
    {
        // delete objects from list
        for (ImageInfoList::iterator it = imageInfoList.begin(); it != imageInfoList.end(); ++it)
            delete *it;
        return;
    }

    // take over ImageInfo list
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

    loadCurrentList(caption, allowSaving);
}

void ImageWindow::loadCurrentList(const QString& caption, bool allowSaving)
{
    // this method contains the code shared by loadURL and loadImageInfos

    // if window is iconified, show it
    if (isMinimized())
    {
        KWin::deIconifyWindow(winId());
    }

    if (!caption.isEmpty())
        setCaption(i18n("Image Editor - %1").arg(caption));
    else
        setCaption(i18n("Image Editor"));

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
    emit signalURLChanged(url);
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
    d->imageInfoCurrent = d->imageInfoList.last();
    slotLoadCurrent();
}

void ImageWindow::slotContextMenu()
{
    if (m_contextMenu)
    {
        RatingPopupMenu *ratingMenu     = 0;
        TagsPopupMenu   *assignTagsMenu = 0;
        TagsPopupMenu   *removeTagsMenu = 0;
        int separatorID1 = -1;
        int separatorID2 = -1;

        if (d->imageInfoCurrent)
        {
            // Bulk assignment/removal of tags --------------------------

            Q_LLONG id = d->imageInfoCurrent->id();
            QValueList<Q_LLONG> idList;
            idList.append(id);

            assignTagsMenu = new TagsPopupMenu(idList, 1000, TagsPopupMenu::ASSIGN);
            removeTagsMenu = new TagsPopupMenu(idList, 2000, TagsPopupMenu::REMOVE);

            separatorID1 = m_contextMenu->insertSeparator();

            m_contextMenu->insertItem(i18n("Assign Tag"), assignTagsMenu);
            int i = m_contextMenu->insertItem(i18n("Remove Tag"), removeTagsMenu);

            connect(assignTagsMenu, SIGNAL(signalTagActivated(int)),
                    this, SLOT(slotAssignTag(int)));

            connect(removeTagsMenu, SIGNAL(signalTagActivated(int)),
                    this, SLOT(slotRemoveTag(int)));

            AlbumDB* db = AlbumManager::instance()->albumDB();
            if (!db->hasTags( idList ))
                m_contextMenu->setItemEnabled(i, false);

            separatorID2 = m_contextMenu->insertSeparator();

            // Assign Star Rating -------------------------------------------

            ratingMenu = new RatingPopupMenu();

            connect(ratingMenu, SIGNAL(activated(int)),
                    this, SLOT(slotAssignRating(int)));

            m_contextMenu->insertItem(i18n("Assign Rating"), ratingMenu);
        }

        m_contextMenu->exec(QCursor::pos());

        if (separatorID1 != -1)
            m_contextMenu->removeItem(separatorID1);
        if (separatorID2 != -1)
            m_contextMenu->removeItem(separatorID2);

        delete assignTagsMenu;
        delete removeTagsMenu;
        delete ratingMenu;
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

        DImg* img = m_canvas->interface()->getImg();

        if (d->imageInfoCurrent)
        {
            d->rightSidebar->itemChanged(d->imageInfoCurrent,
                                         m_canvas->getSelectedArea(), img);
        }
        else
        {
            d->rightSidebar->itemChanged(d->urlCurrent, m_canvas->getSelectedArea(), img);
        }
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
        MetadataHub hub;
        hub.load(d->imageInfoCurrent);
        hub.setTag(tagID, true);
        hub.write(d->imageInfoCurrent, MetadataHub::PartialWrite);
        hub.write(d->imageInfoCurrent->filePath(), MetadataHub::FullWriteIfChanged);
    }
}

void ImageWindow::slotRemoveTag(int tagID)
{
    if (d->imageInfoCurrent)
    {
        MetadataHub hub;
        hub.load(d->imageInfoCurrent);
        hub.setTag(tagID, false);
        hub.write(d->imageInfoCurrent, MetadataHub::PartialWrite);
        hub.write(d->imageInfoCurrent->filePath(), MetadataHub::FullWriteIfChanged);
    }
}

void ImageWindow::slotAssignRatingNoStar()
{
    slotAssignRating(0);
}

void ImageWindow::slotAssignRatingOneStar()
{
    slotAssignRating(1);
}

void ImageWindow::slotAssignRatingTwoStar()
{
    slotAssignRating(2);
}

void ImageWindow::slotAssignRatingThreeStar()
{
    slotAssignRating(3);
}

void ImageWindow::slotAssignRatingFourStar()
{
    slotAssignRating(4);
}

void ImageWindow::slotAssignRatingFiveStar()
{
    slotAssignRating(5);
}

void ImageWindow::slotAssignRating(int rating)
{
    rating = QMIN(RatingMax, QMAX(RatingMin, rating));
    if (d->imageInfoCurrent)
    {
        MetadataHub hub;
        hub.load(d->imageInfoCurrent);
        hub.setRating(rating);
        hub.write(d->imageInfoCurrent, MetadataHub::PartialWrite);
        hub.write(d->imageInfoCurrent->filePath(), MetadataHub::FullWriteIfChanged);
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
    // is not include in the digiKam Albums library database.
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

    kapp->config()->sync();

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

        // If the DImg is put in the cache under the new name, this means the new file will not be reloaded.
        // This may irritate users who want to check for quality loss in lossy formats.
        // In any case, only do that if the format did not change - too many assumptions otherwise (see bug #138949).
        if (m_savingContext->originalFormat == m_savingContext->format)
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
    // Sanity check. Just to be homogenous with SaveAs.
    if (d->imageInfoCurrent)
    {
        // Write metadata from database to DImg
        MetadataHub hub;
        hub.load(d->imageInfoCurrent);
        DImg image(m_canvas->currentImage());
        hub.write(image, MetadataHub::FullWrite);
    }

    startingSave(d->urlCurrent);
    return true;
}

bool ImageWindow::saveAs()
{
    // If image editor is started from CameraGUI, there is no ImageInfo instance to use.
    if (d->imageInfoCurrent)
    {
        // Write metadata from database to DImg
        MetadataHub hub;
        hub.load(d->imageInfoCurrent);
        DImg image(m_canvas->currentImage());
        hub.write(image, MetadataHub::FullWrite);
    }

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

    // if available, provide a digikamalbums:// URL to KIO
    KURL kioURL;
    if (d->imageInfoCurrent)
        kioURL = d->imageInfoCurrent->kurlForKIO();
    else
        kioURL = d->urlCurrent;
    KURL fileURL = d->urlCurrent;

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

    // trash does not like non-local URLs, put is not implemented
    if (useTrash)
        kioURL = fileURL;

    if (!SyncJob::del(kioURL, useTrash))
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

void ImageWindow::slotFilePrint()
{
    printImage(d->urlCurrent);
};

void ImageWindow::slideShow(bool startWithCurrent, SlideShowSettings& settings)
{
    float     cnt;
    DMetadata meta;
    int i               = 0;
    m_cancelSlideShow   = false;
    settings.exifRotate = AlbumSettings::instance()->getExifRotate();

    if (!d->imageInfoList.isEmpty())
    {
        // We have started image editor from Album GUI. we get picture comments from database.

        m_nameLabel->progressBarMode(StatusProgressBar::CancelProgressBarMode,
                                    i18n("Preparing slideshow. Please wait..."));

        cnt = (float)d->imageInfoList.count();

        for (ImageInfo *info = d->imageInfoList.first() ;
             !m_cancelSlideShow && info ; info = d->imageInfoList.next())
        {
            SlidePictureInfo pictInfo;
            pictInfo.comment = info->caption();

            // Perform optimizations: only read pictures metadata if necessary.
            if (settings.printApertureFocal || settings.printExpoSensitivity || settings.printMakeModel)
            {
                meta.load(info->kurl().path());
                pictInfo.photoInfo = meta.getPhotographInformations();
            }

            // In case of dateTime extraction from metadata failed
            pictInfo.photoInfo.dateTime = info->dateTime();
            settings.pictInfoMap.insert(info->kurl(), pictInfo);

            m_nameLabel->setProgressValue((int)((i++/cnt)*100.0));
            kapp->processEvents();
        }
    }
    else
    {
        // We have started image editor from Camera GUI. we get picture comments from metadata.

        m_nameLabel->progressBarMode(StatusProgressBar::CancelProgressBarMode,
                                    i18n("Preparing slideshow. Please wait..."));

        cnt = (float)d->urlList.count();

        for (KURL::List::Iterator it = d->urlList.begin() ;
             !m_cancelSlideShow && (it != d->urlList.end()) ; ++it)
        {
            SlidePictureInfo pictInfo;
            meta.load((*it).path());
            pictInfo.comment   = meta.getImageComment();
            pictInfo.photoInfo = meta.getPhotographInformations();
            settings.pictInfoMap.insert(*it, pictInfo);

            m_nameLabel->setProgressValue((int)((i++/cnt)*100.0));
            kapp->processEvents();
        }
    }

    m_nameLabel->progressBarMode(StatusProgressBar::TextMode, QString());

    if (!m_cancelSlideShow)
    {
        settings.exifRotate = AlbumSettings::instance()->getExifRotate();
        settings.fileList   = d->urlList;

        SlideShow *slide = new SlideShow(settings);
        if (startWithCurrent)
            slide->setCurrent(d->urlCurrent);

        slide->show();
    }
}

void ImageWindow::dragMoveEvent(QDragMoveEvent *e)
{
    int             albumID;
    QValueList<int> albumIDs;
    QValueList<int> imageIDs;
    KURL::List      urls;
    KURL::List      kioURLs;

    if (ItemDrag::decode(e, urls, kioURLs, albumIDs, imageIDs) ||
        AlbumDrag::decode(e, urls, albumID) ||
        TagDrag::canDecode(e))
    {
        e->accept();
        return;
    }

    e->ignore();
}

void ImageWindow::dropEvent(QDropEvent *e)
{
    int             albumID;
    QValueList<int> albumIDs;
    QValueList<int> imageIDs;
    KURL::List      urls;
    KURL::List      kioURLs;

    if (ItemDrag::decode(e, urls, kioURLs, albumIDs, imageIDs))
    {
        ImageInfoList imageInfoList;

        for (QValueList<int>::const_iterator it = imageIDs.begin();
             it != imageIDs.end(); ++it)
        {
            ImageInfo *info = new ImageInfo(*it);
            imageInfoList.append(info);
        }

        if (imageInfoList.isEmpty())
        {
            e->ignore();
            return;
        }

        QString ATitle;
        AlbumManager* man  = AlbumManager::instance();
        PAlbum* palbum     = man->findPAlbum(albumIDs.first());
        if (palbum) ATitle = palbum->title();

        TAlbum* talbum     = man->findTAlbum(albumIDs.first());
        if (talbum) ATitle = talbum->title();

        loadImageInfos(imageInfoList, imageInfoList.first(),
                       i18n("Album \"%1\"").arg(ATitle), true);
        e->accept();
    }
    else if (AlbumDrag::decode(e, urls, albumID))
    {
        AlbumManager* man           = AlbumManager::instance();
        QValueList<Q_LLONG> itemIDs = man->albumDB()->getItemIDsInAlbum(albumID);
        ImageInfoList imageInfoList;

        for (QValueList<Q_LLONG>::const_iterator it = itemIDs.begin();
             it != itemIDs.end(); ++it)
        {
            ImageInfo *info = new ImageInfo(*it);
            imageInfoList.append(info);
        }

        if (imageInfoList.isEmpty())
        {
            e->ignore();
            return;
        }

        QString ATitle;
        PAlbum* palbum     = man->findPAlbum(albumIDs.first());
        if (palbum) ATitle = palbum->title();

        loadImageInfos(imageInfoList, imageInfoList.first(),
                       i18n("Album \"%1\"").arg(ATitle), true);
        e->accept();
    }
    else if(TagDrag::canDecode(e))
    {
        QByteArray ba = e->encodedData("digikam/tag-id");
        QDataStream ds(ba, IO_ReadOnly);
        int tagID;
        ds >> tagID;

        AlbumManager* man           = AlbumManager::instance();
        QValueList<Q_LLONG> itemIDs = man->albumDB()->getItemIDsInTag(tagID, true);
        ImageInfoList imageInfoList;

        for (QValueList<Q_LLONG>::const_iterator it = itemIDs.begin();
             it != itemIDs.end(); ++it)
        {
            ImageInfo *info = new ImageInfo(*it);
            imageInfoList.append(info);
        }

        if (imageInfoList.isEmpty())
        {
            e->ignore();
            return;
        }

        QString ATitle;
        TAlbum* talbum     = man->findTAlbum(tagID);
        if (talbum) ATitle = talbum->title();

        loadImageInfos(imageInfoList, imageInfoList.first(),
                       i18n("Album \"%1\"").arg(ATitle), true);
        e->accept();
    }
    else
    {
        e->ignore();
    }
}

void ImageWindow::slotRevert()
{
    if(!promptUserSave(d->urlCurrent))
        return;

    m_canvas->slotRestore();
}

void ImageWindow::slotChangeTheme(const QString& theme)
{
    AlbumSettings::instance()->setCurrentTheme(theme);
    ThemeEngine::instance()->slotChangeTheme(theme);
}

}  // namespace Digikam
