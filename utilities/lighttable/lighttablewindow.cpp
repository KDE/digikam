/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-02-12
 * Description : digiKam light table GUI
 *
 * Copyright (C) 2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes.

#include <qsplitter.h>
#include <qdockarea.h>

// KDE includes.

#include <kkeydialog.h>
#include <kedittoolbar.h>
#include <kaction.h>
#include <kaccel.h>
#include <kdeversion.h>
#include <klocale.h>
#include <kwin.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kstatusbar.h>
#include <kmenubar.h>

// Local includes.

#include "ddebug.h"
#include "themeengine.h"
#include "dimg.h"
#include "dmetadata.h"
#include "albumsettings.h"
#include "imagepropertiessidebardb.h"
#include "imageattributeswatch.h"
#include "slideshow.h"
#include "setup.h"
#include "statusprogressbar.h"
#include "statuszoombar.h"
#include "thumbnailsize.h"
#include "lighttablepreview.h"
#include "lighttableview.h"
#include "lighttablebar.h"
#include "lighttablewindow.h"
#include "lighttablewindow.moc"

namespace Digikam
{

class LightTableWindowPriv
{

public:

    LightTableWindowPriv()
    {
        fullScreenHideToolBar               = true;
        fullScreen                          = false;
        removeFullScreenButton              = false;
        cancelSlideShow                     = false;
        accelerators                        = 0;
        leftSidebar                         = 0;
        rightSidebar                        = 0;
        previewView                         = 0;
        barView                             = 0;
        hSplitter                           = 0;
        vSplitter                           = 0;
        fileDeleteAction                    = 0;
        slideShowAction                     = 0;
        fullScreenAction                    = 0;
        fileDeletePermanentlyAction         = 0;
        fileDeletePermanentlyDirectlyAction = 0;
        fileTrashDirectlyAction             = 0;
        donateMoneyAction                   = 0;
        star0                               = 0;
        star1                               = 0;
        star2                               = 0;
        star3                               = 0;
        star4                               = 0;
        star5                               = 0;
        zoomFitToWindowAction               = 0;
        zoomPlusAction                      = 0;
        zoomMinusAction                     = 0;
        zoomTo100percents                   = 0;
        nameLabel                           = 0;
        leftZoomBar                         = 0;  
        rightZoomBar                        = 0;  
    }

    bool                      fullScreenHideToolBar;
    bool                      fullScreen;
    bool                      removeFullScreenButton;
    bool                      cancelSlideShow;

    QSplitter                *hSplitter;
    QSplitter                *vSplitter;

    KAction                  *fileDeleteAction;
    KAction                  *slideShowAction;
    KAction                  *donateMoneyAction;
    KAction                  *fileDeletePermanentlyAction;
    KAction                  *fileDeletePermanentlyDirectlyAction;
    KAction                  *fileTrashDirectlyAction;
    KAction                  *star0;
    KAction                  *star1;
    KAction                  *star2;
    KAction                  *star3;
    KAction                  *star4;
    KAction                  *star5;
    KAction                  *zoomPlusAction;
    KAction                  *zoomMinusAction;
    KAction                  *zoomTo100percents;
    KAction                  *zoomFitToWindowAction;

    KToggleAction            *fullScreenAction;

    KAccel                   *accelerators;

    LightTableBar            *barView;

    LightTableView           *previewView;

    StatusZoomBar            *leftZoomBar;
    StatusZoomBar            *rightZoomBar;

    StatusProgressBar        *nameLabel;

    ImagePropertiesSideBarDB *leftSidebar;
    ImagePropertiesSideBarDB *rightSidebar;
};

LightTableWindow* LightTableWindow::m_instance = 0;

LightTableWindow* LightTableWindow::lightTableWindow()
{
    if (!m_instance)
        new LightTableWindow();

    return m_instance;
}

bool LightTableWindow::lightTableWindowCreated()
{
    return m_instance;
}

LightTableWindow::LightTableWindow()
                : KMainWindow(0, "lighttable", WType_TopLevel)
{
    d = new LightTableWindowPriv;
    m_instance = this;

    setCaption(i18n("digiKam Light Table"));

    // -- Build the GUI -------------------------------

    setupUserArea();
    setupStatusBar();
    setupActions();
    setupAccelerators();

    // Make signals/slots connections

    setupConnections();

    //-------------------------------------------------------------

    d->leftSidebar->loadViewState();
    d->rightSidebar->loadViewState();
    d->leftSidebar->populateTags();
    d->rightSidebar->populateTags();

    KConfig* config = kapp->config();
    config->setGroup("LightTable Settings");

    if(config->hasKey("Vertical Splitter Sizes"))
        d->vSplitter->setSizes(config->readIntListEntry("Vertical Splitter Sizes"));

    if(config->hasKey("Horizontal Splitter Sizes"))
        d->hSplitter->setSizes(config->readIntListEntry("Horizontal Splitter Sizes"));

    setAutoSaveSettings("LightTable Settings");
}

LightTableWindow::~LightTableWindow()
{
    m_instance = 0;

    delete d->barView;
    delete d->rightSidebar;
    delete d->leftSidebar;
    delete d;
}

void LightTableWindow::closeEvent(QCloseEvent* e)
{
    if (!e) return;

    KConfig* config = kapp->config();
    config->setGroup("LightTable Settings");
    config->writeEntry("Vertical Splitter Sizes", d->vSplitter->sizes());
    config->writeEntry("Horizontal Splitter Sizes", d->hSplitter->sizes());
    config->sync();

    e->accept();
}

void LightTableWindow::setupUserArea()
{
    QWidget* mainW    = new QWidget(this);
    d->hSplitter      = new QSplitter(Qt::Horizontal, mainW);
    QHBoxLayout *hlay = new QHBoxLayout(mainW);
    d->leftSidebar    = new ImagePropertiesSideBarDB(mainW, "LightTable Left Sidebar", d->hSplitter,
                                                     Sidebar::Left, true);

    QWidget* centralW = new QWidget(d->hSplitter);
    QVBoxLayout *vlay = new QVBoxLayout(centralW);
    d->vSplitter      = new QSplitter(Qt::Vertical, centralW);
    d->barView        = new LightTableBar(d->vSplitter, ThumbBarView::Horizontal);
    d->previewView    = new LightTableView(d->vSplitter);
    vlay->addWidget(d->vSplitter);

    d->rightSidebar   = new ImagePropertiesSideBarDB(mainW, "LightTable Right Sidebar", d->hSplitter,
                                                     Sidebar::Right, true);

    hlay->addWidget(d->leftSidebar);
    hlay->addWidget(d->hSplitter);
    hlay->addWidget(d->rightSidebar);

    d->hSplitter->setFrameStyle( QFrame::NoFrame );
    d->hSplitter->setFrameShadow( QFrame::Plain );
    d->hSplitter->setFrameShape( QFrame::NoFrame );
    d->hSplitter->setOpaqueResize(false);
    d->vSplitter->setFrameStyle( QFrame::NoFrame );
    d->vSplitter->setFrameShadow( QFrame::Plain );
    d->vSplitter->setFrameShape( QFrame::NoFrame );
    d->vSplitter->setOpaqueResize(false);

    setCentralWidget(mainW);
}

void LightTableWindow::setupStatusBar()
{
    d->leftZoomBar = new StatusZoomBar(statusBar());
    statusBar()->addWidget(d->leftZoomBar, 1);

    d->nameLabel = new StatusProgressBar(statusBar());
    d->nameLabel->setAlignment(Qt::AlignCenter);
    d->nameLabel->setMaximumHeight(fontMetrics().height()+2);    
    statusBar()->addWidget(d->nameLabel, 100);
 
    d->rightZoomBar = new StatusZoomBar(statusBar());
    statusBar()->addWidget(d->rightZoomBar, 1);
}

void LightTableWindow::setupConnections()
{
    connect(d->barView, SIGNAL(setLeftPanelInfo(ImageInfo*)),
            this, SLOT(slotSetLeftPanelInfo(ImageInfo*)));

    connect(d->barView, SIGNAL(setRightPanelInfo(ImageInfo*)),
            this, SLOT(slotSetRightPanelInfo(ImageInfo*)));

    connect(d->nameLabel, SIGNAL(signalCancelButtonPressed()),
            this, SLOT(slotNameLabelCancelButtonPressed()));

    connect(d->leftZoomBar, SIGNAL(signalZoomMinusClicked()),
            d->previewView, SLOT(slotDecreaseLeftZoom()));

    connect(d->leftZoomBar, SIGNAL(signalZoomPlusClicked()),
            d->previewView, SLOT(slotIncreaseLeftZoom()));

    connect(d->leftZoomBar, SIGNAL(signalZoomSliderChanged(int)),
            d->previewView, SLOT(slotLeftZoomSliderChanged(int)));

    connect(d->rightZoomBar, SIGNAL(signalZoomMinusClicked()),
            d->previewView, SLOT(slotDecreaseRightZoom()));

    connect(d->rightZoomBar, SIGNAL(signalZoomPlusClicked()),
            d->previewView, SLOT(slotIncreaseRightZoom()));

    connect(d->rightZoomBar, SIGNAL(signalZoomSliderChanged(int)),
            d->previewView, SLOT(slotRightZoomSliderChanged(int)));

    connect(d->previewView, SIGNAL(signalLeftZoomFactorChanged(double)),
            this, SLOT(slotLeftZoomFactorChanged(double)));

    connect(d->previewView, SIGNAL(signalRightZoomFactorChanged(double)),
            this, SLOT(slotRightZoomFactorChanged(double)));

    connect(d->previewView, SIGNAL(signalSlideShow()),
            this, SLOT(slotToggleSlideShow()));

    ImageAttributesWatch *watch = ImageAttributesWatch::instance();

    connect(watch, SIGNAL(signalFileMetadataChanged(const KURL &)),
            this, SLOT(slotFileMetadataChanged(const KURL &)));
}

void LightTableWindow::setupActions()
{
    // -- Standard 'File' menu actions ---------------------------------------------

    KStdAction::quit(this, SLOT(close()), actionCollection(), "lighttable_exit");

    // -- Standard 'View' menu actions ---------------------------------------------

    d->zoomPlusAction = KStdAction::zoomIn(d->previewView, SLOT(slotIncreaseZoom()),
                                           actionCollection(), "lighttable_zoomplus");


    d->zoomMinusAction = KStdAction::zoomOut(d->previewView, SLOT(slotDecreaseZoom()),
                                             actionCollection(), "lighttable_zoomminus");

    d->zoomTo100percents = new KAction(i18n("Zoom to 1:1"), "viewmag1",
                                       CTRL+SHIFT+Key_Z, this, SLOT(slotZoomTo100Percents()),
                                       actionCollection(), "lighttable_zoomto100percents");


    d->zoomFitToWindowAction = new KAction(i18n("Fit to &Window"), "view_fit_window",
                                           CTRL+SHIFT+Key_A, this, SLOT(slotFitToWindow()),
                                           actionCollection(), "lighttable_zoomfit2window");

#if KDE_IS_VERSION(3,2,0)
    d->fullScreenAction = KStdAction::fullScreen(this, SLOT(slotToggleFullScreen()),
                                                 actionCollection(), this, "lighttable_fullscreen");
#else
    d->fullScreenAction = new KToggleAction(i18n("Fullscreen"), "window_fullscreen",
                                            CTRL+SHIFT+Key_F, this,
                                            SLOT(slotToggleFullScreen()),
                                            actionCollection(), "lighttable_fullscreen");
#endif

    d->slideShowAction = new KAction(i18n("Slide Show"), "slideshow", Key_F9,
                                     this, SLOT(slotToggleSlideShow()),
                                     actionCollection(),"lighttable_slideshow");

    // -- Standard 'Configure' menu actions ----------------------------------------

    KStdAction::keyBindings(this, SLOT(slotEditKeys()),           actionCollection());
    KStdAction::configureToolbars(this, SLOT(slotConfToolbars()), actionCollection());
    KStdAction::preferences(this, SLOT(slotSetup()),              actionCollection());

    // -- Standard 'Help' menu actions ---------------------------------------------


    d->donateMoneyAction = new KAction(i18n("Donate Money..."),
                                       0, 0, 
                                       this, SLOT(slotDonateMoney()),
                                       actionCollection(),
                                       "lighttable_donatemoney");    

    // Provides a menu entry that allows showing/hiding the toolbar(s)
    setStandardToolBarMenuEnabled(true);

    // Provides a menu entry that allows showing/hiding the statusbar
    createStandardStatusBarAction();

    // -- Rating actions ---------------------------------------------------------------

    d->star0 = new KAction(i18n("Assign Rating \"No Star\""), CTRL+Key_0,
                          d->rightSidebar, SLOT(slotAssignRatingNoStar()),
                          actionCollection(), "lighttable_ratenostar");
    d->star1 = new KAction(i18n("Assign Rating \"One Star\""), CTRL+Key_1,
                          d->rightSidebar, SLOT(slotAssignRatingOneStar()),
                          actionCollection(), "lighttable_rateonestar");
    d->star2 = new KAction(i18n("Assign Rating \"Two Stars\""), CTRL+Key_2,
                          d->rightSidebar, SLOT(slotAssignRatingTwoStar()),
                          actionCollection(), "lighttable_ratetwostar");
    d->star3 = new KAction(i18n("Assign Rating \"Three Stars\""), CTRL+Key_3,
                          d->rightSidebar, SLOT(slotAssignRatingThreeStar()),
                          actionCollection(), "lighttable_ratethreestar");
    d->star4 = new KAction(i18n("Assign Rating \"Four Stars\""), CTRL+Key_4,
                          d->rightSidebar, SLOT(slotAssignRatingFourStar()),
                          actionCollection(), "lighttable_ratefourstar");
    d->star5 = new KAction(i18n("Assign Rating \"Five Stars\""), CTRL+Key_5,
                          d->rightSidebar, SLOT(slotAssignRatingFiveStar()),
                          actionCollection(), "lighttable_ratefivestar");

    // ---------------------------------------------------------------------------------

    createGUI("lighttablewindowui.rc", false);
}

void LightTableWindow::setupAccelerators()
{
    d->accelerators = new KAccel(this);

    d->accelerators->insert("Exit fullscreen", i18n("Exit Fullscreen mode"),
                    i18n("Exit out of the fullscreen mode"),
                    Key_Escape, this, SLOT(slotEscapePressed()),
                    false, true);

    d->accelerators->insert("Zoom Plus Key_Plus", i18n("Zoom In"),
                    i18n("Zoom in on Image"),
                    Key_Plus, d->previewView, SLOT(slotIncreaseZoom()),
                    false, true);
    
    d->accelerators->insert("Zoom Plus Key_Minus", i18n("Zoom Out"),
                    i18n("Zoom out of Image"),
                    Key_Minus, d->previewView, SLOT(slotDecreaseZoom()),
                    false, true);
}

void LightTableWindow::loadImageInfos(const ImageInfoList &list, ImageInfo *imageInfoCurrent)
{
    for (QPtrList<ImageInfo>::const_iterator it = list.begin(); it != list.end(); ++it)
    {
        if (!d->barView->findItemByInfo(*it))
        {
            LightTableBarItem *item = new LightTableBarItem(d->barView, *it);
            if (*it == imageInfoCurrent)
                d->barView->setSelected(item);
        }
    }   

    // if window is iconified, show it
    if (isMinimized())
    {
        KWin::deIconifyWindow(winId());
    }

    d->nameLabel->progressBarMode(StatusProgressBar::TextMode, 
                  i18n("1 item inserted in Light Table"/*, "%1 items inserted in Light Table"*/)
                  .arg(d->barView->countItems()));   
}   

void LightTableWindow::slotSetLeftPanelInfo(ImageInfo* info)
{
    d->previewView->setLeftImageInfo(info);
    d->leftSidebar->itemChanged(info);
}

void LightTableWindow::slotSetRightPanelInfo(ImageInfo* info)
{
    d->previewView->setRightImageInfo(info);
    d->rightSidebar->itemChanged(info);
}

void LightTableWindow::slotZoomTo100Percents()
{
    d->previewView->setLeftZoomFactor(1.0);
    d->previewView->setRightZoomFactor(1.0);
}

void LightTableWindow::slotFitToWindow()
{
    d->previewView->fitToWindow();
}

void LightTableWindow::slotToggleSlideShow()
{
    KConfig* config = kapp->config();
    config->setGroup("ImageViewer Settings");
    bool startWithCurrent = config->readBoolEntry("SlideShowStartCurrent", false);

    SlideShowSettings settings;
    settings.delay                = config->readNumEntry("SlideShowDelay", 5) * 1000;
    settings.printName            = config->readBoolEntry("SlideShowPrintName", true);
    settings.printDate            = config->readBoolEntry("SlideShowPrintDate", false);
    settings.printApertureFocal   = config->readBoolEntry("SlideShowPrintApertureFocal", false);
    settings.printExpoSensitivity = config->readBoolEntry("SlideShowPrintExpoSensitivity", false);
    settings.printMakeModel       = config->readBoolEntry("SlideShowPrintMakeModel", false);
    settings.printComment         = config->readBoolEntry("SlideShowPrintComment", false);
    settings.loop                 = config->readBoolEntry("SlideShowLoop", false);
    slideShow(startWithCurrent, settings);
}

void LightTableWindow::slideShow(bool startWithCurrent, SlideShowSettings& settings)
{
    int       i = 0;
    DMetadata meta;
    d->cancelSlideShow = false;

    d->nameLabel->progressBarMode(StatusProgressBar::CancelProgressBarMode, 
                                  i18n("Prepare slideshow. Please wait..."));

    ImageInfoList list = d->barView->itemsImageInfoList();

    for (ImageInfo *info = list.first() ; !d->cancelSlideShow && info ; info = list.next())
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
        settings.fileList.append(info->kurl());

        d->nameLabel->setProgressValue((int)((i++/(float)list.count())*100.0));
        kapp->processEvents();
    }

    d->nameLabel->progressBarMode(StatusProgressBar::TextMode, QString());   

    if (!d->cancelSlideShow)
    {
        settings.exifRotate = AlbumSettings::instance()->getExifRotate();
    
        SlideShow *slide = new SlideShow(settings);
        if (startWithCurrent)
            slide->setCurrent(d->barView->currentItemImageInfo()->kurl());
    
        slide->show();
    }
}

void LightTableWindow::slotNameLabelCancelButtonPressed()
{
    d->cancelSlideShow = true;
}

void LightTableWindow::slotToggleFullScreen()
{
    if (d->fullScreen) // out of fullscreen
    {

#if QT_VERSION >= 0x030300
        setWindowState( windowState() & ~WindowFullScreen );
#else
        showNormal();
#endif
        menuBar()->show();
        statusBar()->show();
        leftDock()->show();
        rightDock()->show();
        topDock()->show();
        bottomDock()->show();
        
        QObject* obj = child("ToolBar","KToolBar");
        
        if (obj)
        {
            KToolBar* toolBar = static_cast<KToolBar*>(obj);
            
            if (d->fullScreenAction->isPlugged(toolBar) && d->removeFullScreenButton)
                d->fullScreenAction->unplug(toolBar);
                
            if (toolBar->isHidden())
                showToolBars();
        }

        // -- remove the gui action accels ----

        unplugActionAccel(d->zoomPlusAction);
        unplugActionAccel(d->zoomMinusAction);
        unplugActionAccel(d->zoomFitToWindowAction);

        if (d->fullScreen) d->rightSidebar->restore();
        else               d->rightSidebar->backup();        
        
        d->fullScreen = false;
    }
    else  // go to fullscreen
    {
        // hide the menubar and the statusbar
        menuBar()->hide();
        statusBar()->hide();
        topDock()->hide();
        leftDock()->hide();
        rightDock()->hide();
        bottomDock()->hide();
        
        QObject* obj = child("ToolBar","KToolBar");
        
        if (obj)
        {
            KToolBar* toolBar = static_cast<KToolBar*>(obj);
            
            if (d->fullScreenHideToolBar)
            {
                hideToolBars();
            }
            else
            {   
                showToolBars();

                if ( !d->fullScreenAction->isPlugged(toolBar) )
                {
                    d->fullScreenAction->plug(toolBar);
                    d->removeFullScreenButton=true;
                }
                else    
                {
                    // If FullScreen button is enable in toolbar settings
                    // We don't remove it when we out of fullscreen mode.
                    d->removeFullScreenButton=false;
                }
            }
        }

        // -- Insert all the gui actions into the accel --

        plugActionAccel(d->zoomPlusAction);
        plugActionAccel(d->zoomMinusAction);
        plugActionAccel(d->zoomFitToWindowAction);

        if (d->fullScreen) d->rightSidebar->restore();
        else               d->rightSidebar->backup();        

        showFullScreen();
        d->fullScreen = true;
    }
}

void LightTableWindow::slotEscapePressed()
{
    if (d->fullScreen)
        d->fullScreenAction->activate();
}

void LightTableWindow::showToolBars()
{
    QPtrListIterator<KToolBar> it = toolBarIterator();
    KToolBar* bar;

    for( ; it.current()!=0L ; ++it)
    {
        bar=it.current();
        
        if (bar->area())
            bar->area()->show();
        else
            bar->show();
    }
}

void LightTableWindow::hideToolBars()
{
    QPtrListIterator<KToolBar> it = toolBarIterator();
    KToolBar* bar;

    for( ; it.current()!=0L ; ++it)
    {
        bar=it.current();
        
        if (bar->area()) 
            bar->area()->hide();
        else 
            bar->hide();
    }
}

void LightTableWindow::plugActionAccel(KAction* action)
{
    if (!action)
        return;

    d->accelerators->insert(action->text(),
                    action->text(),
                    action->whatsThis(),
                    action->shortcut(),
                    action,
                    SLOT(activate()));
}

void LightTableWindow::unplugActionAccel(KAction* action)
{
    d->accelerators->remove(action->text());
}

void LightTableWindow::slotDonateMoney()
{
    KApplication::kApplication()->invokeBrowser("http://www.digikam.org/?q=donation");
}

void LightTableWindow::slotEditKeys()
{
    KKeyDialog dialog(true, this);
    dialog.insert( actionCollection(), i18n( "General" ) );
    dialog.configure();
}

void LightTableWindow::slotConfToolbars()
{
    saveMainWindowSettings(KGlobal::config(), "LightTable Settings");
    KEditToolbar dlg(factory(), this);

    connect(&dlg, SIGNAL(newToolbarConfig()),
            this, SLOT(slotNewToolbarConfig()));

    dlg.exec();
}

void LightTableWindow::slotNewToolbarConfig()
{
    applyMainWindowSettings(KGlobal::config(), "LightTable Settings");
}

void LightTableWindow::slotSetup()
{
    Setup setup(this, 0);    
        
    if (setup.exec() != QDialog::Accepted)
        return;

    kapp->config()->sync();
    
    // TODO: Apply Settings here if necessary
}

void LightTableWindow::slotFileMetadataChanged(const KURL &url)
{
    if (url == d->barView->currentItemImageInfo()->kurl())
    {
        // TODO
    }
}

void LightTableWindow::slotLeftZoomFactorChanged(double zoom)
{
    double h    = (double)ThumbnailSize::Huge;
    double s    = (double)ThumbnailSize::Small;
    double zmin = d->previewView->leftZoomMin();
    double zmax = d->previewView->leftZoomMax();
    double b    = (zmin-(zmax*s/h))/(1-s/h);
    double a    = (zmax-b)/h;
    int size    = (int)((zoom - b) /a); 

    d->leftZoomBar->setZoomSliderValue(size);
    d->leftZoomBar->setZoomTrackerText(i18n("zoom: %1%").arg((int)(zoom*100.0)));

    d->leftZoomBar->setEnableZoomPlus(true);
    d->leftZoomBar->setEnableZoomMinus(true);

    if (d->previewView->leftMaxZoom())
        d->leftZoomBar->setEnableZoomPlus(false);

    if (d->previewView->leftMinZoom())
        d->leftZoomBar->setEnableZoomMinus(false);
}

void LightTableWindow::slotRightZoomFactorChanged(double zoom)
{
    double h    = (double)ThumbnailSize::Huge;
    double s    = (double)ThumbnailSize::Small;
    double zmin = d->previewView->rightZoomMin();
    double zmax = d->previewView->rightZoomMax();
    double b    = (zmin-(zmax*s/h))/(1-s/h);
    double a    = (zmax-b)/h;
    int size    = (int)((zoom - b) /a); 

    d->rightZoomBar->setZoomSliderValue(size);
    d->rightZoomBar->setZoomTrackerText(i18n("zoom: %1%").arg((int)(zoom*100.0)));

    d->rightZoomBar->setEnableZoomPlus(true);
    d->rightZoomBar->setEnableZoomMinus(true);

    if (d->previewView->rightMaxZoom())
        d->rightZoomBar->setEnableZoomPlus(false);

    if (d->previewView->rightMinZoom())
        d->rightZoomBar->setEnableZoomMinus(false);
}

}  // namespace Digikam


