/* ============================================================
 * File  : ctrlpaneldialog.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-05-07
 * Description : A threaded filter control panel dialog for
 *               image editor plugins
 *
 * Copyright 2005 by Gilles Caulier
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
#include <kdebug.h>

// Local includes.

#include "version.h"
#include "bannerwidget.h"
#include "ctrlpaneldialog.h"

namespace DigikamImagePlugins
{

CtrlPanelDialog::CtrlPanelDialog(QWidget* parent, QString title, QString name,
                                 bool loadFileSettings, bool tryAction, bool progressBar,
                                 int separateViewMode)
               : KDialogBase(Plain, title,
                             Help|Default|User1|User2|User3|Try|Ok|Cancel, Ok,
                             parent, 0, true, true,
                             i18n("&Abort"),
                             i18n("&Save As..."),
                             i18n("&Load...")),
                 m_parent(parent), m_name(name), m_tryAction(tryAction)
{
    m_currentRenderingMode = NoneRendering;
    m_timer                = 0L;
    m_threadedFilter       = 0L;
    QString whatsThis;

    setButtonWhatsThis ( Default, i18n("<p>Reset all filter parameters to their default values.") );
    setButtonWhatsThis ( User1, i18n("<p>Abort the current image rendering.") );
    setButtonWhatsThis ( User3, i18n("<p>Load all filter parameters from settings text file.") );
    setButtonWhatsThis ( User2, i18n("<p>Save all filter parameters to settings text file.") );
    showButton(User2, loadFileSettings);
    showButton(User3, loadFileSettings);
    showButton(Try, tryAction);

    resize(configDialogSize(name + QString::QString(" Tool Dialog")));

    // -------------------------------------------------------------

    QVBoxLayout *topLayout = new QVBoxLayout( plainPage(), 0, spacingHint());

    QFrame *headerFrame = new DigikamImagePlugins::BannerWidget(plainPage(), title);

    topLayout->addWidget(headerFrame);

    // -------------------------------------------------------------

    QHBoxLayout *hlay1 = new QHBoxLayout(topLayout);

    m_imagePreviewWidget = new Digikam::ImagePannelWidget(240, 160,
                                        name + QString::QString(" Tool Dialog"),
                                        plainPage(),  progressBar, separateViewMode);
    hlay1->addWidget(m_imagePreviewWidget);

    // -------------------------------------------------------------

    QTimer::singleShot(0, this, SLOT(slotInit()));
}

CtrlPanelDialog::~CtrlPanelDialog()
{
    saveDialogSize(m_name + QString::QString(" Tool Dialog"));

    if (m_timer)
       delete m_timer;

    if (m_threadedFilter)
       delete m_threadedFilter;
}

void CtrlPanelDialog::slotInit()
{
    // Reset values to defaults.
    QTimer::singleShot(0, this, SLOT(readUserSettings()));

    if (!m_tryAction)
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

void CtrlPanelDialog::setAboutData(KAboutData *about)
{
    QPushButton *helpButton = actionButton( Help );
    KHelpMenu* helpMenu = new KHelpMenu(this, about, false);
    helpMenu->menu()->removeItemAt(0);
    helpMenu->menu()->insertItem(i18n("Plugin Handbook"), this, SLOT(slotHelp()), 0, -1, 0);
    helpButton->setPopup( helpMenu->menu() );
}

void CtrlPanelDialog::abortPreview()
{
    m_currentRenderingMode = NoneRendering;
    m_imagePreviewWidget->setProgress(0);
    m_imagePreviewWidget->setPreviewImageWaitCursor(false);
    m_imagePreviewWidget->setEnable(true);
    enableButton(Ok, true);
    enableButton(User1, false);
    enableButton(User2, true);
    enableButton(User3, true);
    enableButton(Try,   true);
    enableButton(Default, true);
    renderingFinished();
}

void CtrlPanelDialog::slotTry()
{
    slotEffect();
}

void CtrlPanelDialog::slotUser1()
{
    if (m_currentRenderingMode != NoneRendering)
        if (m_threadedFilter)
           m_threadedFilter->stopComputation();
}

void CtrlPanelDialog::slotDefault()
{
   resetValues();
   slotEffect();
}

void CtrlPanelDialog::slotCancel()
{
    if (m_currentRenderingMode != NoneRendering)
       {
       if (m_threadedFilter)
          m_threadedFilter->stopComputation();

       kapp->restoreOverrideCursor();
       }

    done(Cancel);
}

void CtrlPanelDialog::closeEvent(QCloseEvent *e)
{
    if (m_currentRenderingMode != NoneRendering)
       {
       if (m_threadedFilter)
          m_threadedFilter->stopComputation();

       kapp->restoreOverrideCursor();
       }

    e->accept();
}

void CtrlPanelDialog::slotFocusChanged(void)
{
    if (m_currentRenderingMode == FinalRendering)
       {
       m_imagePreviewWidget->update();
       return;
       }
    else if (m_currentRenderingMode == PreviewRendering)
       {
       if (m_threadedFilter)
          m_threadedFilter->stopComputation();
       }

    QTimer::singleShot(0, this, SLOT(slotEffect()));
}

void CtrlPanelDialog::slotHelp()
{
    KApplication::kApplication()->invokeHelp(m_name, "digikamimageplugins");
}

void CtrlPanelDialog::slotTimer()
{
    if (m_timer)
       {
       m_timer->stop();
       delete m_timer;
       }

    m_timer = new QTimer( this );
    connect( m_timer, SIGNAL(timeout()),
             this, SLOT(slotEffect()) );
    m_timer->start(500, true);
}

void CtrlPanelDialog::slotEffect()
{
    // Computation already in process.
    if (m_currentRenderingMode == PreviewRendering) return;

    m_currentRenderingMode = PreviewRendering;

    m_imagePreviewWidget->setEnable(false);
    enableButton(Ok,    false);
    enableButton(User1, true);
    enableButton(User2, false);
    enableButton(User3, false);
    enableButton(Try,   false);
    enableButton(Default, false);
    m_imagePreviewWidget->setPreviewImageWaitCursor(true);
    m_imagePreviewWidget->setProgress(0);

    if (m_threadedFilter)
       delete m_threadedFilter;

    prepareEffect();
}

void CtrlPanelDialog::slotOk()
{
    writeUserSettings();
    m_currentRenderingMode = FinalRendering;

    m_imagePreviewWidget->setEnable(false);
    enableButton(Ok,    false);
    enableButton(User1, false);
    enableButton(User2, false);
    enableButton(User3, false);
    enableButton(Try,   false);
    enableButton(Default, false);
    kapp->setOverrideCursor( KCursor::waitCursor() );
    m_imagePreviewWidget->setProgress(0);

    if (m_threadedFilter)
       delete m_threadedFilter;

    prepareFinal();
}

void CtrlPanelDialog::customEvent(QCustomEvent *event)
{
    if (!event) return;

    Digikam::ThreadedFilter::EventData *d = (Digikam::ThreadedFilter::EventData*) event->data();

    if (!d) return;

    if (d->starting)           // Computation in progress !
        {
        m_imagePreviewWidget->setProgress(d->progress);
        }
    else
        {
        if (d->success)        // Computation Completed !
            {
            switch (m_currentRenderingMode)
              {
              case PreviewRendering:
                 {
                 kdDebug() << "Preview " << m_name << " completed..." << endl;
                 putPreviewData();
                 abortPreview();
                 break;
                 }

              case FinalRendering:
                 {
                 kdDebug() << "Final" << m_name << " completed..." << endl;
                 putFinalData();
                 kapp->restoreOverrideCursor();
                 accept();
                 break;
                 }
              }
            }
        else                   // Computation Failed !
            {
            switch (m_currentRenderingMode)
                {
                case PreviewRendering:
                    {
                    kdDebug() << "Preview " << m_name << " failed..." << endl;
                    // abortPreview() must be call here for set progress bar to 0 properly.
                    abortPreview();
                    break;
                    }

                case FinalRendering:
                    break;
                }
            }
        }

    delete d;
}

// Backport KDialog::keyPressEvent() implementation from KDELibs to ignore Enter/Return Key events 
// to prevent any conflicts between dialog keys events and SpinBox keys events.

void CtrlPanelDialog::keyPressEvent(QKeyEvent *e)
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

}  // NameSpace DigikamImagePlugins

#include "ctrlpaneldialog.moc"
