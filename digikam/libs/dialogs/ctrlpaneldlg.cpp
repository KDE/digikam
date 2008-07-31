/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-07
 * Description : A threaded filter control panel dialog for
 *               image editor plugins using DImg
 *
 * Copyright (C) 2005-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include "dimgthreadedfilter.h"
#include "dimginterface.h"
#include "ctrlpaneldlg.h"
#include "ctrlpaneldlg.moc"

namespace Digikam
{

class CtrlPanelDlgPriv
{
public:

    enum RunningMode
    {
        NoneRendering=0,
        PreviewRendering,
        FinalRendering
    };

    CtrlPanelDlgPriv()
    {
        parent               = 0;
        timer                = 0;
        aboutData            = 0;
        progressBar          = true;
        tryAction            = false;
        currentRenderingMode = NoneRendering;
    }

    bool         tryAction;
    bool         progressBar;

    int          currentRenderingMode;

    QWidget     *parent;

    QTimer      *timer;

    QString      name;

    KAboutData  *aboutData;
};

CtrlPanelDlg::CtrlPanelDlg(QWidget* parent, QString title, QString name,
                           bool loadFileSettings, 
                           bool tryAction, 
                           bool progressBar,
                           int separateViewMode)
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

    d = new CtrlPanelDlgPriv;
    d->parent        = parent;
    d->name          = name;
    d->tryAction     = tryAction;
    d->progressBar   = progressBar;
    m_threadedFilter = 0;
    QString whatsThis;

    setButtonWhatsThis( Default, i18n("<p>Reset all filter parameters to their default values.") );
    setButtonWhatsThis( User1, i18n("<p>Abort the current image rendering.") );
    setButtonWhatsThis( User3, i18n("<p>Load all filter parameters from settings text file.") );
    setButtonWhatsThis( User2, i18n("<p>Save all filter parameters to settings text file.") );
    showButton(User2, loadFileSettings);
    showButton(User3, loadFileSettings);
    showButton(Try, tryAction);

    // disable Abort button on startup
    enableButton(User1, false);

    restoreDialogSize(KGlobal::config()->group(name + QString(" Tool Dialog")));

    // -------------------------------------------------------------

    setMainWidget(new QWidget(this));
    QVBoxLayout *topLayout = new QVBoxLayout(mainWidget());

    // -------------------------------------------------------------

    m_imagePreviewWidget = new ImagePannelWidget(470, 350, 
                                                 name + QString(" Tool Dialog"),
                                                 mainWidget(), separateViewMode);
    topLayout->addWidget(m_imagePreviewWidget);
    topLayout->setMargin(0);
    topLayout->setSpacing(spacingHint());

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

CtrlPanelDlg::~CtrlPanelDlg()
{
    delete d->aboutData;
    delete d->timer;
    delete m_threadedFilter;
    delete d;
}

void CtrlPanelDlg::slotInit()
{
    // Reset values to defaults.
    QTimer::singleShot(0, this, SLOT(readUserSettings()));

    if (!d->tryAction)
    {
       connect(m_imagePreviewWidget, SIGNAL(signalOriginalClipFocusChanged()),
               this, SLOT(slotFocusChanged()));
    }
    else
    {
       connect(m_imagePreviewWidget, SIGNAL(signalResized()),
               this, SLOT(slotFocusChanged()));
    }
}

void CtrlPanelDlg::setAboutData(KAboutData *about)
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

void CtrlPanelDlg::abortPreview()
{
    d->currentRenderingMode = CtrlPanelDlgPriv::NoneRendering;
    m_imagePreviewWidget->setProgress(0);
    m_imagePreviewWidget->setPreviewImageWaitCursor(false);
    m_imagePreviewWidget->setEnable(true);
    m_imagePreviewWidget->setProgressVisible(false);
    enableButton(Ok,      true);
    enableButton(User1,   false);
    enableButton(User2,   true);
    enableButton(User3,   true);
    enableButton(Try,     true);
    enableButton(Default, true);
    renderingFinished();
}

void CtrlPanelDlg::slotButtonClicked(int button)
{
    // KDialog calls QDialog::accept() for Ok.
    // We need to override this, we can only accept() when the thread has finished.

    if (button == Ok)
        slotOk();
    else
        KDialog::slotButtonClicked(button);
}

void CtrlPanelDlg::slotTry()
{
    slotEffect();
}

void CtrlPanelDlg::slotUser1()
{
    if (d->currentRenderingMode != CtrlPanelDlgPriv::NoneRendering)
        if (m_threadedFilter)
           m_threadedFilter->cancelFilter();
}

void CtrlPanelDlg::slotDefault()
{
   resetValues();
   slotEffect();
}

void CtrlPanelDlg::slotCancel()
{
    if (d->currentRenderingMode != CtrlPanelDlgPriv::NoneRendering)
    {
       if (m_threadedFilter)
          m_threadedFilter->cancelFilter();

       kapp->restoreOverrideCursor();
    }

    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group(d->name + QString(" Tool Dialog"));
    saveDialogSize(group);
    done(Cancel);
}

void CtrlPanelDlg::closeEvent(QCloseEvent *e)
{
    if (d->currentRenderingMode != CtrlPanelDlgPriv::NoneRendering)
    {
       if (m_threadedFilter)
          m_threadedFilter->cancelFilter();

       kapp->restoreOverrideCursor();
    }

    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->name + QString(" Tool Dialog"));
    saveDialogSize(group);
    e->accept();
}

void CtrlPanelDlg::slotFocusChanged(void)
{
    if (d->currentRenderingMode == CtrlPanelDlgPriv::FinalRendering)
    {
       m_imagePreviewWidget->update();
       return;
    }
    else if (d->currentRenderingMode == CtrlPanelDlgPriv::PreviewRendering)
    {
       if (m_threadedFilter)
          m_threadedFilter->cancelFilter();
    }

    QTimer::singleShot(0, this, SLOT(slotEffect()));
}

void CtrlPanelDlg::slotHelp()
{
    // If setAboutData() is called by plugin, well DigikamImagePlugins help is lauched, 
    // else digiKam help. In this case, setHelp() method must be used to set anchor and handbook name.

    if (d->aboutData)
        KToolInvocation::invokeHelp(d->name, "digikam");
}

void CtrlPanelDlg::slotTimer()
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

void CtrlPanelDlg::slotEffect()
{
    // Computation already in process.
    if (d->currentRenderingMode != CtrlPanelDlgPriv::NoneRendering)
        return;

    d->currentRenderingMode = CtrlPanelDlgPriv::PreviewRendering;
    DDebug() << "Preview " << d->name << " started..." << endl;

    m_imagePreviewWidget->setEnable(false);
    m_imagePreviewWidget->setProgressVisible(true);
    enableButton(Ok,      false);
    enableButton(User1,   true);
    enableButton(User2,   false);
    enableButton(User3,   false);
    enableButton(Try,     false);
    enableButton(Default, false);
    m_imagePreviewWidget->setPreviewImageWaitCursor(true);
    m_imagePreviewWidget->setProgress(0);

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

void CtrlPanelDlg::slotOk()
{
    d->currentRenderingMode = CtrlPanelDlgPriv::FinalRendering;
    DDebug() << "Final " << d->name << " started..." << endl;

    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group(d->name + QString(" Tool Dialog"));
    saveDialogSize(group);
    writeUserSettings();

    m_imagePreviewWidget->setEnable(false);
    m_imagePreviewWidget->setProgressVisible(true);
    enableButton(Ok,      false);
    enableButton(User1,   false);
    enableButton(User2,   false);
    enableButton(User3,   false);
    enableButton(Try,     false);
    enableButton(Default, false);
    kapp->setOverrideCursor( Qt::WaitCursor );
    m_imagePreviewWidget->setProgress(0);

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

void CtrlPanelDlg::slotFilterStarted()
{
}

void CtrlPanelDlg::slotFilterFinished(bool success)
{
    if (success)        // Computation Completed !
    {
        switch (d->currentRenderingMode)
        {
            case CtrlPanelDlgPriv::PreviewRendering:
            {
                DDebug() << "Preview " << d->name << " completed..." << endl;
                putPreviewData();
                abortPreview();
                break;
            }

            case CtrlPanelDlgPriv::FinalRendering:
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
            case CtrlPanelDlgPriv::PreviewRendering:
            {
                DDebug() << "Preview " << d->name << " failed..." << endl;
                    // abortPreview() must be call here for set progress bar to 0 properly.
                abortPreview();
                break;
            }

            case CtrlPanelDlgPriv::FinalRendering:
                break;
        }
    }
}

void CtrlPanelDlg::slotFilterProgress(int progress)
{
    m_imagePreviewWidget->setProgress(progress);
}

// Backport KDialog::keyPressEvent() implementation from KDELibs to ignore Enter/Return Key events
// to prevent any conflicts between dialog keys events and SpinBox keys events.

// TODO: KDE4PORT: Check if this code work fine with KDE4::KDialog()

void CtrlPanelDlg::keyPressEvent(QKeyEvent *e)
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
