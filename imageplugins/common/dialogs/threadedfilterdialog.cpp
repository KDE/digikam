/* ============================================================
 * File  : threadedfilterdialog.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-05-07
 * Description : A basic template of threaded filter plugin
 *               dialog without widgets.
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

#include <qlabel.h>
#include <qpushbutton.h>
#include <qwhatsthis.h>
#include <qtooltip.h>
#include <qtimer.h>

// KDE includes.

#include <kcursor.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <khelpmenu.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kpopupmenu.h>
#include <kprogress.h>
#include <kstandarddirs.h>
#include <kglobalsettings.h>
#include <kdebug.h>
#include <kconfig.h>

// Local includes.

#include "version.h"
#include "bannerwidget.h"
#include "threadedfilterdialog.h"

namespace DigikamImagePlugins
{

ThreadedFilterDialog::ThreadedFilterDialog(QWidget* parent, QString title, QString name,
                                           bool loadFileSettings)
                    : KDialogBase(Plain, title,
                                  Help|Default|User1|User2|User3|Try|Ok|Cancel, Ok,
                                  parent, 0, true, true,
                                  i18n("&Abort"),
                                  i18n("&Save As..."),
                                  i18n("&Load...")),
                      m_parent(parent), m_name(name)
{
    m_currentRenderingMode = NoneRendering;
    m_timer                = 0L;
    m_threadedFilter       = 0L;
    m_progressBar          = 0L;
    QString whatsThis;

    setButtonWhatsThis ( Default, i18n("<p>Reset all filter parameters to their default values.") );
    setButtonWhatsThis ( User1, i18n("<p>Abort the current image rendering.") );
    setButtonWhatsThis ( User3, i18n("<p>Load all filter parameters from settings text file.") );
    setButtonWhatsThis ( User2, i18n("<p>Save all filter parameters to settings text file.") );
    showButton(User2, loadFileSettings);
    showButton(User3, loadFileSettings);
    showButton(Try, false);

    resize(configDialogSize(name + QString::QString(" Tool Dialog")));
}

ThreadedFilterDialog::~ThreadedFilterDialog()
{
    saveDialogSize(m_name + QString::QString(" Tool Dialog"));

    if (m_timer)
       delete m_timer;

    if (m_threadedFilter)
       delete m_threadedFilter;
}

void ThreadedFilterDialog::setAboutData(KAboutData *about)
{
    QPushButton *helpButton = actionButton( Help );
    KHelpMenu* helpMenu = new KHelpMenu(this, about, false);
    helpMenu->menu()->removeItemAt(0);
    helpMenu->menu()->insertItem(i18n("Plugin Handbook"), this, SLOT(slotHelp()), 0, -1, 0);
    helpButton->setPopup( helpMenu->menu() );
}

void ThreadedFilterDialog::abortPreview()
{
    m_currentRenderingMode = NoneRendering;
    if (m_progressBar) m_progressBar->setValue(0);
    enableButton(Ok,      true);
    enableButton(User1,   false);
    enableButton(User2,   true);
    enableButton(User3,   true);
    enableButton(Try,     true);
    enableButton(Default, true);
    renderingFinished();
}

void ThreadedFilterDialog::slotUser1()
{
    if (m_currentRenderingMode != NoneRendering)
        if (m_threadedFilter)
            m_threadedFilter->stopComputation();
}

void ThreadedFilterDialog::slotDefault()
{
    resetValues();
    slotEffect();
}

void ThreadedFilterDialog::slotCancel()
{
    if (m_currentRenderingMode != NoneRendering)
       {
       if (m_threadedFilter)
          m_threadedFilter->stopComputation();

       kapp->restoreOverrideCursor();
       }

    done(Cancel);
}

void ThreadedFilterDialog::closeEvent(QCloseEvent *e)
{
    if (m_currentRenderingMode != NoneRendering)
       {
       if (m_threadedFilter)
          m_threadedFilter->stopComputation();

       kapp->restoreOverrideCursor();
       }

    e->accept();
}

void ThreadedFilterDialog::slotHelp()
{
    KApplication::kApplication()->invokeHelp(m_name, "digikamimageplugins");
}

void ThreadedFilterDialog::slotTimer()
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

void ThreadedFilterDialog::slotEffect()
{
    // Computation already in process.
    if (m_currentRenderingMode == PreviewRendering) return;

    m_currentRenderingMode = PreviewRendering;

    enableButton(Ok,      false);
    enableButton(User1,   true);
    enableButton(User2,   false);
    enableButton(User3,   false);
    enableButton(Try,     false);
    enableButton(Default, false);
    if (m_progressBar) m_progressBar->setValue(0);

    if (m_threadedFilter)
       delete m_threadedFilter;

    prepareEffect();
}

void ThreadedFilterDialog::slotOk()
{
    m_currentRenderingMode = FinalRendering;

    enableButton(Ok,      false);
    enableButton(User1,   false);
    enableButton(User2,   false);
    enableButton(User3,   false);
    enableButton(Try,     false);
    enableButton(Default, false);
    kapp->setOverrideCursor( KCursor::waitCursor() );
    if (m_progressBar) m_progressBar->setValue(0);

    if (m_threadedFilter)
       delete m_threadedFilter;

    prepareFinal();
}

void ThreadedFilterDialog::customEvent(QCustomEvent *event)
{
    if (!event) return;

    Digikam::ThreadedFilter::EventData *d = (Digikam::ThreadedFilter::EventData*) event->data();

    if (!d) return;

    if (d->starting)           // Computation in progress !
        {
        if (m_progressBar) m_progressBar->setValue(d->progress);
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

}  // NameSpace DigikamImagePlugins

#include "threadedfilterdialog.moc"
