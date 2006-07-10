/* ============================================================
 * File  : imageguidedlg.cpp
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2005-05-07
 * Description : A threaded filter plugin dialog with a preview
 *               image guide widget and a settings user area
 *
 * Copyright 2005-2006 by Gilles Caulier
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

#include <qvgroupbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qwhatsthis.h>
#include <qtooltip.h>
#include <qlayout.h>
#include <qframe.h>
#include <qtimer.h>
#include <qspinbox.h>
#include <qsplitter.h>
#include <qhbox.h>

// KDE includes.

#include <kcursor.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <khelpmenu.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kpopupmenu.h>
#include <kstandarddirs.h>
#include <kglobalsettings.h>
#include <kprogress.h>
#include <kdebug.h>
#include <kcolorbutton.h>
#include <kconfig.h>
#include <kseparator.h>

// Local includes.

#include "sidebar.h"
#include "dimgthreadedfilter.h"
#include "dimginterface.h"
#include "imageguidedlg.h"

namespace Digikam
{

class ImageGuideDlgPriv
{
public:

    enum RunningMode
    {
        NoneRendering=0,
        PreviewRendering,
        FinalRendering
    };
    
    ImageGuideDlgPriv()
    {
        progress             = true;
        currentRenderingMode = NoneRendering;
        parent               = 0;
        settings             = 0;
        timer                = 0;
        aboutData            = 0;
        guideColorBt         = 0;
        progressBar          = 0;
        guideSize            = 0;
        mainLayout           = 0;
        settingsLayout       = 0;
        hbox                 = 0;
        settingsSideBar      = 0;
        splitter             = 0;
    }

    bool          progress;
    
    int           currentRenderingMode;

    QWidget      *parent;
    QWidget      *settings;
    
    QTimer       *timer;
    
    QString       name;

    QGridLayout  *mainLayout;
    QGridLayout  *settingsLayout;
    
    QSpinBox     *guideSize;

    QHBox        *hbox;

    QSplitter    *splitter;

    KProgress    *progressBar;
        
    KColorButton *guideColorBt;

    KAboutData   *aboutData;

    Sidebar      *settingsSideBar;
};

ImageGuideDlg::ImageGuideDlg(QWidget* parent, QString title, QString name,
                             bool loadFileSettings, bool progress,
                             bool guideVisible, int guideMode, QFrame* bannerFrame,
                             bool prevModeOptions)
             : KDialogBase(Plain, 0,
                           Help|Default|User1|User2|User3|Ok|Cancel, Ok,
                           parent, 0, true, true,
                           i18n("&Abort"),
                           i18n("&Save As..."),
                           i18n("&Load..."))
{
    kapp->setOverrideCursor( KCursor::waitCursor() );
    setCaption(DImgInterface::instance()->getImageFileName() + QString(" - ") + title);
    
    d = new ImageGuideDlgPriv;
    d->parent        = parent;
    d->name          = name;
    d->progress      = progress;
    m_threadedFilter = 0;
    QString whatsThis;

    setButtonWhatsThis ( Default, i18n("<p>Reset all filter parameters to their default values.") );
    setButtonWhatsThis ( User1, i18n("<p>Abort the current image rendering.") );
    setButtonWhatsThis ( User3, i18n("<p>Load all filter parameters from settings text file.") );
    setButtonWhatsThis ( User2, i18n("<p>Save all filter parameters to settings text file.") );
    showButton(User2, loadFileSettings);
    showButton(User3, loadFileSettings);

    resize(configDialogSize(name + QString(" Tool Dialog")));

    // -------------------------------------------------------------

    d->mainLayout = new QGridLayout( plainPage(), 2, 1);

    if (bannerFrame)
    {
        bannerFrame->reparent( plainPage(), QPoint(0, 0) );
        d->mainLayout->addMultiCellWidget(bannerFrame, 0, 0, 0, 1);
    }

    // -------------------------------------------------------------

    QString desc;

    if (guideVisible)
        desc = i18n("<p>This is the the image filter effect preview. "
                    "If you move the mouse cursor on this area, "
                    "a vertical and horizontal dashed line will be draw "
                    "to guide you in adjusting the filter settings. "
                    "Press the left mouse button to freeze the dashed "
                    "line's position.");
    else
        desc = i18n("<p>This is the image filter effect preview.");

    d->hbox              = new QHBox(plainPage());
    d->splitter          = new QSplitter(d->hbox);
    m_imagePreviewWidget = new ImageWidget(d->name, d->splitter, desc, prevModeOptions, 
                                           guideMode, guideVisible);
    
    d->splitter->setFrameStyle( QFrame::NoFrame );
    d->splitter->setFrameShadow( QFrame::Plain );
    d->splitter->setFrameShape( QFrame::NoFrame );    
    d->splitter->setOpaqueResize(false);
    
    QSizePolicy rightSzPolicy(QSizePolicy::Preferred, QSizePolicy::Expanding, 2, 1);
    m_imagePreviewWidget->setSizePolicy(rightSzPolicy);

    QString sbName(d->name + QString(" Image Plugin Sidebar"));
    d->settingsSideBar = new Sidebar(d->hbox, sbName.ascii(), Sidebar::Right);
    d->settingsSideBar->setSplitter(d->splitter);

    d->mainLayout->addMultiCellWidget(d->hbox, 1, 2, 0, 1);
    d->mainLayout->setColStretch(0, 10);
    d->mainLayout->setRowStretch(2, 10);

    // -------------------------------------------------------------

    d->settings       = new QWidget(plainPage());
    d->settingsLayout = new QGridLayout( d->settings, 1, 0);
    QVBoxLayout *vLayout = new QVBoxLayout( spacingHint() );
    
    // -------------------------------------------------------------

    QWidget *gboxGuideSettings = new QWidget(d->settings);
    QGridLayout* grid          = new QGridLayout( gboxGuideSettings, 2, 2, 0, spacingHint());
    KSeparator *line           = new KSeparator (Horizontal, gboxGuideSettings);
    grid->addMultiCellWidget(line, 0, 0, 0, 2);

    QLabel *label5  = new QLabel(i18n("Guide color:"), gboxGuideSettings);
    d->guideColorBt = new KColorButton( QColor( Qt::red ), gboxGuideSettings );
    QWhatsThis::add( d->guideColorBt, i18n("<p>Set here the color used to draw guides dashed-lines."));
    grid->addMultiCellWidget(label5, 1, 1, 0, 0);
    grid->addMultiCellWidget(d->guideColorBt, 1, 1, 1, 2);

    QLabel *label6 = new QLabel(i18n("Guide width:"), gboxGuideSettings);
    d->guideSize   = new QSpinBox( 1, 5, 1, gboxGuideSettings);
    QWhatsThis::add( d->guideSize, i18n("<p>Set here the width in pixels used to draw guides dashed-lines."));
    grid->addMultiCellWidget(label6, 2, 2, 0, 0);
    grid->addMultiCellWidget(d->guideSize, 2, 2, 1, 2);

    if (guideVisible) gboxGuideSettings->show();
    else gboxGuideSettings->hide();

    vLayout->addWidget(gboxGuideSettings);

    d->progressBar = new KProgress(100, d->settings);
    d->progressBar->setMaximumHeight( fontMetrics().height() );
    QWhatsThis::add(d->progressBar ,i18n("<p>This is the current percentage of the task completed."));
    d->progressBar->setValue(0);
    setProgressVisible(false);
    vLayout->addWidget(d->progressBar);

    vLayout->addStretch(10);
    d->settingsLayout->addMultiCellLayout(vLayout, 1, 1, 0, 0);

    d->settingsSideBar->appendTab(d->settings, SmallIcon("configure"), i18n("Settings"));    
    d->settingsSideBar->loadViewState();
    
    // Reading splitter sizes here prevent flicker effect in dialog.
    KConfig *config = kapp->config();
    config->setGroup(d->name + QString(" Tool Dialog"));
    if(config->hasKey("SplitterSizes"))
        d->splitter->setSizes(config->readIntListEntry("SplitterSizes"));

    // -------------------------------------------------------------

    QTimer::singleShot(0, this, SLOT(slotInit()));
    kapp->restoreOverrideCursor();
}

ImageGuideDlg::~ImageGuideDlg()
{
    if (d->timer)
       delete d->timer;

    if (m_threadedFilter)
       delete m_threadedFilter;

    if (d->aboutData)
       delete d->aboutData;
    
    delete d->settingsSideBar;            
    delete d;            
}

void ImageGuideDlg::readSettings(void)
{
    QColor *defaultGuideColor = new QColor( Qt::red );
    KConfig *config = kapp->config();
    config->setGroup(d->name + QString(" Tool Dialog"));
    d->guideColorBt->setColor(config->readColorEntry("Guide Color", defaultGuideColor));
    d->guideSize->setValue(config->readNumEntry("Guide Width", 1));
    m_imagePreviewWidget->slotChangeGuideSize(d->guideSize->value());
    m_imagePreviewWidget->slotChangeGuideColor(d->guideColorBt->color());
    delete defaultGuideColor;
}

void ImageGuideDlg::writeSettings(void)
{
    KConfig *config = kapp->config();
    config->setGroup(d->name + QString(" Tool Dialog"));
    config->writeEntry( "Guide Color", d->guideColorBt->color() );
    config->writeEntry( "Guide Width", d->guideSize->value() );
    config->writeEntry( "SplitterSizes", d->splitter->sizes() );
    config->sync();
    saveDialogSize(d->name + QString(" Tool Dialog"));
}

void ImageGuideDlg::slotInit()
{
    readSettings();
    // Reset values to defaults.
    QTimer::singleShot(0, this, SLOT(readUserSettings()));

    connect(m_imagePreviewWidget, SIGNAL(signalResized()),
            this, SLOT(slotResized()));

    connect(d->guideColorBt, SIGNAL(changed(const QColor &)),
            m_imagePreviewWidget, SLOT(slotChangeGuideColor(const QColor &)));

    connect(d->guideSize, SIGNAL(valueChanged(int)),
            m_imagePreviewWidget, SLOT(slotChangeGuideSize(int)));
}

void ImageGuideDlg::setUserAreaWidget(QWidget *w)
{
    w->reparent( d->settings, QPoint(0, 0) );
    QVBoxLayout *vLayout = new QVBoxLayout( spacingHint() );
    vLayout->addWidget(w);
    d->settingsLayout->addMultiCellLayout(vLayout, 0, 0, 0, 0);
}

void ImageGuideDlg::setAboutData(KAboutData *about)
{
    d->aboutData = about;
    QPushButton *helpButton = actionButton( Help );
    KHelpMenu* helpMenu = new KHelpMenu(this, d->aboutData, false);
    helpMenu->menu()->removeItemAt(0);
    helpMenu->menu()->insertItem(i18n("Plugin Handbook"), this, SLOT(slotHelp()), 0, -1, 0);
    helpButton->setPopup( helpMenu->menu() );
}

void ImageGuideDlg::setProgressVisible(bool v)
{
    if (v)
        d->progressBar->show();
    else
        d->progressBar->hide();
}

void ImageGuideDlg::abortPreview()
{
    d->currentRenderingMode = ImageGuideDlgPriv::NoneRendering;
    d->progressBar->setValue(0);
    setProgressVisible(false);
    enableButton(Ok, true);
    enableButton(User1, false);
    enableButton(User2, true);
    enableButton(User3, true);
    enableButton(Default, true);
    renderingFinished();
}

void ImageGuideDlg::slotResized(void)
{
    if (d->currentRenderingMode == ImageGuideDlgPriv::FinalRendering)
    {
       m_imagePreviewWidget->update();
       return;
    }
    else if (d->currentRenderingMode == ImageGuideDlgPriv::PreviewRendering)
    {
       if (m_threadedFilter)
          m_threadedFilter->stopComputation();
    }

    QTimer::singleShot(0, this, SLOT(slotEffect()));
}

void ImageGuideDlg::slotUser1()
{
    if (d->currentRenderingMode != ImageGuideDlgPriv::NoneRendering)
        if (m_threadedFilter)
            m_threadedFilter->stopComputation();
}

void ImageGuideDlg::slotDefault()
{
    resetValues();
    slotEffect();
}

void ImageGuideDlg::slotCancel()
{
    if (d->currentRenderingMode != ImageGuideDlgPriv::NoneRendering)
    {
       if (m_threadedFilter)
          m_threadedFilter->stopComputation();

       kapp->restoreOverrideCursor();
    }
    
    writeSettings();
    done(Cancel);
}

void ImageGuideDlg::closeEvent(QCloseEvent *e)
{
    if (d->currentRenderingMode != ImageGuideDlgPriv::NoneRendering)
    {
       if (m_threadedFilter)
          m_threadedFilter->stopComputation();

       kapp->restoreOverrideCursor();
    }

    writeSettings();
    e->accept();
}

void ImageGuideDlg::slotHelp()
{
    // If setAboutData() is called by plugin, well DigikamImagePlugins help is lauched, 
    // else digiKam help. In this case, setHelp() method must be used to set anchor and handbook name.

    if (d->aboutData)
        KApplication::kApplication()->invokeHelp(d->name, "digikamimageplugins");
    else
        KDialogBase::slotHelp();
}

void ImageGuideDlg::slotTimer()
{
    if (d->timer)
    {
       d->timer->stop();
       delete d->timer;
    }

    d->timer = new QTimer( this );
    connect( d->timer, SIGNAL(timeout()),
             this, SLOT(slotEffect()) );
    d->timer->start(500, true);
}

void ImageGuideDlg::slotEffect()
{
    // Computation already in process.
    if (d->currentRenderingMode == ImageGuideDlgPriv::PreviewRendering ||
        d->currentRenderingMode == ImageGuideDlgPriv::FinalRendering)
        return;

    d->currentRenderingMode = ImageGuideDlgPriv::PreviewRendering;
    kdDebug() << "Preview " << d->name << " started..." << endl;

    enableButton(Ok,      false);
    enableButton(User1,   true);
    enableButton(User2,   false);
    enableButton(User3,   false);
    enableButton(Default, false);
    d->progressBar->setValue(0);
    if (d->progress) setProgressVisible(true);

    if (m_threadedFilter)
       delete m_threadedFilter;

    prepareEffect();
}

void ImageGuideDlg::slotOk()
{
    d->currentRenderingMode = ImageGuideDlgPriv::FinalRendering;
    kdDebug() << "Final " << d->name << " started..." << endl;
    writeSettings();
    writeUserSettings();

    enableButton(Ok,      false);
    enableButton(User1,   false);
    enableButton(User2,   false);
    enableButton(User3,   false);
    enableButton(Default, false);
    kapp->setOverrideCursor( KCursor::waitCursor() );
    d->progressBar->setValue(0);

    if (m_threadedFilter)
       delete m_threadedFilter;

    prepareFinal();
}

void ImageGuideDlg::customEvent(QCustomEvent *event)
{
    if (!event) return;

    DImgThreadedFilter::EventData *ed = (DImgThreadedFilter::EventData*) event->data();

    if (!ed) return;

    if (ed->starting)           // Computation in progress !
    {
        d->progressBar->setValue(ed->progress);
    }
    else
    {
        if (ed->success)        // Computation Completed !
        {
            switch (d->currentRenderingMode)
            {
                case ImageGuideDlgPriv::PreviewRendering:
                {
                    kdDebug() << "Preview " << d->name << " completed..." << endl;
                    putPreviewData();
                    abortPreview();
                    break;
                }

                case ImageGuideDlgPriv::FinalRendering:
                {
                    kdDebug() << "Final" << d->name << " completed..." << endl;
                    putFinalData();
                    kapp->restoreOverrideCursor();
                    accept();
                    break;
                }
            }
        }
        else                   // Computation Failed !
        {
            switch (d->currentRenderingMode)
            {
                case ImageGuideDlgPriv::PreviewRendering:
                {
                    kdDebug() << "Preview " << d->name << " failed..." << endl;
                    // abortPreview() must be call here for set progress bar to 0 properly.
                    abortPreview();
                    break;
                }

                case ImageGuideDlgPriv::FinalRendering:
                    break;
            }
        }
    }

    delete ed;
}

// Backport KDialog::keyPressEvent() implementation from KDELibs to ignore Enter/Return Key events 
// to prevent any conflicts between dialog keys events and SpinBox keys events.

void ImageGuideDlg::keyPressEvent(QKeyEvent *e)
{
    if ( e->state() == 0 )
    {
        switch ( e->key() )
        {
        case Key_Escape:
            e->accept();
            reject();
        break;
        case Key_Enter:            
        case Key_Return:     
            e->ignore();              
        break;
        default:
            e->ignore();
            return;
        }
    }
    else
    {
        // accept the dialog when Ctrl-Return is pressed
        if ( e->state() == ControlButton &&
            (e->key() == Key_Return || e->key() == Key_Enter) )
        {
            e->accept();
            accept();
        }
        else
        {
            e->ignore();
        }
    }
}

}  // NameSpace Digikam

#include "imageguidedlg.moc"
