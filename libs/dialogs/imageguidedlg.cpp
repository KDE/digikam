/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-07
 * Description : A threaded filter plugin dialog with a preview
 *               image guide widget and a settings user area
 *
 * Copyright (C) 2005-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <QGroupBox>
#include <QLabel>
#include <QTimer>
#include <QFrame>
#include <QSpinBox>
#include <QSplitter>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QProgressBar>

// KDE includes.

#include <kpushbutton.h>
#include <kcursor.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <khelpmenu.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kmenu.h>
#include <kstandarddirs.h>
#include <kglobalsettings.h>
#include <kcolorbutton.h>
#include <kconfig.h>
#include <kseparator.h>
#include <kglobal.h>
#include <ktoolinvocation.h>
#include <kvbox.h>

// Local includes.

#include "ddebug.h"
#include "sidebar.h"
#include "dimgthreadedfilter.h"
#include "dimginterface.h"
#include "imageguidedlg.h"
#include "imageguidedlg.moc"

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
        tryAction            = false;
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

    bool          tryAction;
    bool          progress;

    int           currentRenderingMode;

    QWidget      *parent;
    QWidget      *settings;

    QTimer       *timer;

    QString       name;

    QGridLayout  *mainLayout;
    QGridLayout  *settingsLayout;

    QSpinBox     *guideSize;

    KHBox        *hbox;

    QSplitter    *splitter;

    QProgressBar *progressBar;

    KColorButton *guideColorBt;

    KAboutData   *aboutData;

    Sidebar      *settingsSideBar;
};

ImageGuideDlg::ImageGuideDlg(QWidget* parent, QString title, QString name,
                             bool loadFileSettings, bool progress,
                             bool guideVisible, int guideMode,
                             bool prevModeOptions, bool useImageSelection,
                             bool tryAction)
             : KDialog(parent)
{
    kapp->setOverrideCursor( Qt::WaitCursor );
    setButtons(Help|Default|User1|User2|User3|Try|Ok|Cancel);
    setDefaultButton(Ok);
    setModal(true);
    setButtonText(User1, i18n("&Abort"));
    setButtonText(User2, i18n("&Save As..."));
    setButtonText(User3, i18n("&Load..."));
    setCaption(DImgInterface::defaultInterface()->getImageFileName() + QString(" - ") + title);

    d = new ImageGuideDlgPriv;
    d->parent        = parent;
    d->name          = name;
    d->progress      = progress;
    d->tryAction     = tryAction;
    m_threadedFilter = 0;
    QString whatsThis;

    setButtonWhatsThis( Default, i18n("<p>Reset all filter parameters to their default values.") );
    setButtonWhatsThis( User1, i18n("<p>Abort the current image rendering.") );
    setButtonWhatsThis( User3, i18n("<p>Load all filter parameters from settings text file.") );
    setButtonWhatsThis( User2, i18n("<p>Save all filter parameters to settings text file.") );
    showButton(User2, loadFileSettings);
    showButton(User3, loadFileSettings);
    showButton(Try, tryAction);

    restoreDialogSize(KGlobal::config()->group(name + QString(" Tool Dialog")));

    // -------------------------------------------------------------

    setMainWidget(new QWidget(this));
    d->mainLayout = new QGridLayout(mainWidget());

    // -------------------------------------------------------------

    QString desc;

    if (guideVisible)
        desc = i18n("<p>This is the image filter effect preview. "
                    "If you move the mouse cursor on this area, "
                    "a vertical and horizontal dashed line will be draw "
                    "to guide you in adjusting the filter settings. "
                    "Press the left mouse button to freeze the dashed "
                    "line's position.");
    else
        desc = i18n("<p>This is the image filter effect preview.");

    d->hbox              = new KHBox(mainWidget());
    d->splitter          = new QSplitter(d->hbox);
    m_imagePreviewWidget = new ImageWidget(d->name, d->splitter, desc, prevModeOptions, 
                                           guideMode, guideVisible, useImageSelection);

    d->splitter->setFrameStyle( QFrame::NoFrame );
    d->splitter->setFrameShadow( QFrame::Plain );
    d->splitter->setFrameShape( QFrame::NoFrame );
    d->splitter->setOpaqueResize(false);
    d->splitter->setStretchFactor(0, 10);      // set imagewidget default size to max.

    QString sbName(d->name + QString(" Image Plugin Sidebar"));
    d->settingsSideBar = new Sidebar(d->hbox, KMultiTabBar::Right);
    d->settingsSideBar->setObjectName(sbName.toAscii());
    d->settingsSideBar->setSplitter(d->splitter);

    d->mainLayout->addWidget(d->hbox, 1, 0, 2, 2);
    d->mainLayout->setColumnStretch(0, 10);
    d->mainLayout->setRowStretch(2, 10);
    d->mainLayout->setMargin(0);
    d->mainLayout->setSpacing(spacingHint());

    // -------------------------------------------------------------

    d->settings          = new QWidget(mainWidget());
    d->settingsLayout    = new QGridLayout(d->settings);
    QVBoxLayout *vLayout = new QVBoxLayout();

    // -------------------------------------------------------------

    QWidget *gboxGuideSettings = new QWidget(d->settings);
    QGridLayout* grid          = new QGridLayout( gboxGuideSettings );
    KSeparator *line           = new KSeparator(Qt::Horizontal, gboxGuideSettings);

    QLabel *label5  = new QLabel(i18n("Guide color:"), gboxGuideSettings);
    d->guideColorBt = new KColorButton( QColor( Qt::red ), gboxGuideSettings );
    d->guideColorBt->setWhatsThis(i18n("<p>Set here the color used to draw guides dashed-lines."));

    QLabel *label6 = new QLabel(i18n("Guide width:"), gboxGuideSettings);
    d->guideSize   = new QSpinBox(gboxGuideSettings);
    d->guideSize->setRange(1, 5);
    d->guideSize->setSingleStep(1);
    d->guideSize->setWhatsThis(i18n("<p>Set here the width in pixels used to draw guides dashed-lines."));

    grid->addWidget(line, 0, 0, 1, 3);
    grid->addWidget(label5, 1, 0, 1, 1);
    grid->addWidget(d->guideColorBt, 1, 2, 1, 1);
    grid->addWidget(label6, 2, 0, 1, 1);
    grid->addWidget(d->guideSize, 2, 2, 1, 1);
    grid->setColumnStretch(1, 10);
    grid->setMargin(spacingHint());
    grid->setSpacing(spacingHint());

    if (guideVisible) gboxGuideSettings->show();
    else gboxGuideSettings->hide();

    // -------------------------------------------------------------

    KHBox *hbox    = new KHBox(d->settings);
    QLabel *space1 = new QLabel(hbox);
    space1->setFixedWidth(spacingHint());

    d->progressBar = new QProgressBar(hbox);
    d->progressBar->setMaximum(100);
    d->progressBar->setMaximumHeight(fontMetrics().height() +4);
    d->progressBar->setWhatsThis(i18n("<p>This is the percentage of the task which has completed up to this point."));
    d->progressBar->setValue(0);
    setProgressVisible(false);

    QLabel *space2 = new QLabel(hbox);
    space2->setFixedWidth(spacingHint());

    // -------------------------------------------------------------

    vLayout->addWidget(gboxGuideSettings);
    vLayout->addWidget(hbox);
    vLayout->addStretch(10);
    vLayout->setMargin(0);
    vLayout->setSpacing(spacingHint());

    d->settingsLayout->addLayout(vLayout, 1, 0, 1, 1);
    d->settingsLayout->setMargin(spacingHint());
    d->settingsLayout->setSpacing(spacingHint());
    d->settingsLayout->setRowStretch(2, 10);

    d->settingsSideBar->appendTab(d->settings, SmallIcon("configure"), i18n("Settings"));
    d->settingsSideBar->loadViewState();

    // Reading splitter sizes here prevent flicker effect in dialog.

    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->name + QString(" Tool Dialog"));
    if (group.hasKey("SplitterState")) 
    {
        QByteArray state;
        state = group.readEntry("SplitterState", state);
        d->splitter->restoreState(QByteArray::fromBase64(state));
    }

    // -------------------------------------------------------------

    connect(this, SIGNAL(cancelClicked()),
            this, SLOT(slotCancel()));

    connect(this, SIGNAL(tryClicked()),
            this, SLOT(slotTry()));

    connect(this, SIGNAL(defaultClicked()),
            this, SLOT(slotDefault()));

    connect(this, SIGNAL(user1Clicked()),
            this, SLOT(slotUser1()));

    connect(this, SIGNAL(user2Clicked()),
            this, SLOT(slotUser2()));

    connect(this, SIGNAL(user3Clicked()),
            this, SLOT(slotUser3()));

    connect(this, SIGNAL(helpClicked()),
            this, SLOT(slotHelp()));

    // -------------------------------------------------------------

    QTimer::singleShot(0, this, SLOT(slotInit()));
    kapp->restoreOverrideCursor();
}

ImageGuideDlg::~ImageGuideDlg()
{
    delete d->aboutData;
    delete d->timer;
    delete d->settingsSideBar;
    delete m_threadedFilter;
    delete d;
}

void ImageGuideDlg::readSettings()
{
    QColor defaultGuideColor(Qt::red);
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group(d->name + QString(" Tool Dialog"));
    d->guideColorBt->setColor(group.readEntry("Guide Color", defaultGuideColor));
    d->guideSize->setValue(group.readEntry("Guide Width", 1));
    m_imagePreviewWidget->slotChangeGuideSize(d->guideSize->value());
    m_imagePreviewWidget->slotChangeGuideColor(d->guideColorBt->color());
}

void ImageGuideDlg::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group(d->name + QString(" Tool Dialog"));
    group.writeEntry( "Guide Color", d->guideColorBt->color() );
    group.writeEntry( "Guide Width", d->guideSize->value() );
    group.writeEntry("SplitterState", d->splitter->saveState().toBase64());
    saveDialogSize(group);
    config->sync();
}

void ImageGuideDlg::slotInit()
{
    readSettings();
    // Reset values to defaults.
    QTimer::singleShot(0, this, SLOT(readUserSettings()));

    if (!d->tryAction)
    {
        connect(m_imagePreviewWidget, SIGNAL(signalResized()),
                this, SLOT(slotResized()));
    }

    connect(d->guideColorBt, SIGNAL(changed(const QColor &)),
            m_imagePreviewWidget, SLOT(slotChangeGuideColor(const QColor &)));

    connect(d->guideSize, SIGNAL(valueChanged(int)),
            m_imagePreviewWidget, SLOT(slotChangeGuideSize(int)));
}

void ImageGuideDlg::setUserAreaWidget(QWidget *w)
{
    w->setParent( d->splitter );
    QVBoxLayout *vLayout = new QVBoxLayout();
    vLayout->setSpacing(spacingHint());
    vLayout->addWidget(w);
    d->settingsLayout->addLayout(vLayout, 0, 0, 1, 1);
}

void ImageGuideDlg::setAboutData(KAboutData *about)
{
    disconnect(this, SIGNAL(helpClicked()),
               this, SLOT(slotHelp()));

    d->aboutData            = about;
    KPushButton *helpButton = button( Help );
    KHelpMenu* helpMenu     = new KHelpMenu(this, d->aboutData, false);
    helpMenu->menu()->removeAction(helpMenu->menu()->actions().first());
    QAction *handbook       = new QAction(i18n("digiKam Handbook"), this);
    connect(handbook, SIGNAL(triggered(bool)),
            this, SLOT(slotHelp()));
    helpMenu->menu()->insertAction(helpMenu->menu()->actions().first(), handbook);
    helpButton->setDelayedMenu( helpMenu->menu() );
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
    enableButton(Ok,      true);
    enableButton(User1,   false);
    enableButton(User2,   true);
    enableButton(User3,   true);
    enableButton(Try,     true);
    enableButton(Default, true);
    renderingFinished();
}

void ImageGuideDlg::slotButtonClicked(int button)
{
    // KDialog calls QDialog::accept() for Ok.
    // We need to override this, we can only accept() when the thread has finished.

    if (button == Ok)
        slotOk();
    else
        KDialog::slotButtonClicked(button);
}

void ImageGuideDlg::slotTry()
{
    slotEffect();
}

void ImageGuideDlg::slotResized()
{
    if (d->currentRenderingMode == ImageGuideDlgPriv::FinalRendering)
    {
       m_imagePreviewWidget->update();
       return;
    }
    else if (d->currentRenderingMode == ImageGuideDlgPriv::PreviewRendering)
    {
       if (m_threadedFilter)
          m_threadedFilter->cancelFilter();
    }

    QTimer::singleShot(0, this, SLOT(slotEffect()));
}

void ImageGuideDlg::slotUser1()
{
    if (d->currentRenderingMode != ImageGuideDlgPriv::NoneRendering)
        if (m_threadedFilter)
            m_threadedFilter->cancelFilter();
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
          m_threadedFilter->cancelFilter();

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
          m_threadedFilter->cancelFilter();

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
        KToolInvocation::invokeHelp(d->name, "digikam");
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
    d->timer->setSingleShot(true);
    d->timer->start(500);
}

void ImageGuideDlg::slotEffect()
{
    // Computation already in process.
    if (d->currentRenderingMode != ImageGuideDlgPriv::NoneRendering)
        return;

    d->currentRenderingMode = ImageGuideDlgPriv::PreviewRendering;
    DDebug() << "Preview " << d->name << " started..." << endl;

    enableButton(Ok,      false);
    enableButton(User1,   true);
    enableButton(User2,   false);
    enableButton(User3,   false);
    enableButton(Default, false);
    enableButton(Try,     false);
    d->progressBar->setValue(0);
    if (d->progress) setProgressVisible(true);

    if (m_threadedFilter)
    {
        delete m_threadedFilter;
        m_threadedFilter = 0;
    }

    prepareEffect();

    if (m_threadedFilter)
    {
        connect(m_threadedFilter, SIGNAL(started()),
                this, SLOT(slotFilterStarted()));
        connect(m_threadedFilter, SIGNAL(finished(bool)),
                this, SLOT(slotFilterFinished(bool)));
        connect(m_threadedFilter, SIGNAL(progress(int)),
                this, SLOT(slotFilterProgress(int)));

        m_threadedFilter->startFilter();
    }
}

void ImageGuideDlg::slotOk()
{
    d->currentRenderingMode = ImageGuideDlgPriv::FinalRendering;
    DDebug() << "Final " << d->name << " started..." << endl;
    writeSettings();
    writeUserSettings();

    enableButton(Ok,      false);
    enableButton(User1,   false);
    enableButton(User2,   false);
    enableButton(User3,   false);
    enableButton(Default, false);
    enableButton(Try,     false);
    kapp->setOverrideCursor( Qt::WaitCursor );
    d->progressBar->setValue(0);
    if (d->progress)
        setProgressVisible(true);

    if (m_threadedFilter)
    {
        delete m_threadedFilter;
        m_threadedFilter = 0;
    }

    prepareFinal();

    if (m_threadedFilter)
    {
        connect(m_threadedFilter, SIGNAL(started()),
                this, SLOT(slotFilterStarted()));
        connect(m_threadedFilter, SIGNAL(finished(bool)),
                this, SLOT(slotFilterFinished(bool)));
        connect(m_threadedFilter, SIGNAL(progress(int)),
                this, SLOT(slotFilterProgress(int)));

        m_threadedFilter->startFilter();
    }
}

void ImageGuideDlg::slotFilterStarted()
{
}

void ImageGuideDlg::slotFilterFinished(bool success)
{
    if (success)        // Computation Completed !
    {
        switch (d->currentRenderingMode)
        {
            case ImageGuideDlgPriv::PreviewRendering:
            {
                DDebug() << "Preview " << d->name << " completed..." << endl;
                putPreviewData();
                abortPreview();
                break;
            }

            case ImageGuideDlgPriv::FinalRendering:
            {
                DDebug() << "Final" << d->name << " completed..." << endl;
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
                DDebug() << "Preview " << d->name << " failed..." << endl;
                    // abortPreview() must be call here for set progress bar to 0 properly.
                abortPreview();
                break;
            }

            case ImageGuideDlgPriv::FinalRendering:
                break;
        }
    }
}

void ImageGuideDlg::slotFilterProgress(int progress)
{
    d->progressBar->setValue(progress);
}

// Backport KDialog::keyPressEvent() implementation from KDELibs to ignore Enter/Return Key events
// to prevent any conflicts between dialog keys events and SpinBox keys events.

// TODO: KDE4PORT: Check if this code work fine with KDE4::KDialog()

void ImageGuideDlg::keyPressEvent(QKeyEvent *e)
{
    if ( e->modifiers() == 0 )
    {
        switch ( e->key() )
        {
            case Qt::Key_Escape:
                e->accept();
                reject();
            break;
            case Qt::Key_Enter:
            case Qt::Key_Return:
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
        if ( e->modifiers() == Qt::ControlModifier &&
            (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) )
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
