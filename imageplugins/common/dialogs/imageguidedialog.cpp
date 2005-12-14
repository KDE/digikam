/* ============================================================
 * File  : imageguidedialog.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-05-07
 * Description : A threaded filter plugin dialog with a preview
 *               image guide widget and a settings user area
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
#include <qspinbox.h>

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

#include "version.h"
#include "bannerwidget.h"
#include "imageguidedialog.h"

namespace DigikamImagePlugins
{

ImageGuideDialog::ImageGuideDialog(QWidget* parent, QString title, QString name,
                                   bool loadFileSettings, bool progress,
                                   bool guideVisible, int guideMode)
                  : KDialogBase(Plain, title,
                                Help|Default|User1|User2|User3|Ok|Cancel, Ok,
                                parent, 0, true, true,
                                i18n("&Abort"),
                                i18n("&Save As..."),
                                i18n("&Load...")),
                    m_parent(parent), m_name(name)
{
    m_currentRenderingMode = NoneRendering;
    m_timer                = 0L;
    m_threadedFilter       = 0L;
    m_about                = 0L;    
    QString whatsThis;

    setButtonWhatsThis ( Default, i18n("<p>Reset all filter parameters to their default values.") );
    setButtonWhatsThis ( User1, i18n("<p>Abort the current image rendering.") );
    setButtonWhatsThis ( User3, i18n("<p>Load all filter parameters from settings text file.") );
    setButtonWhatsThis ( User2, i18n("<p>Save all filter parameters to settings text file.") );
    showButton(User2, loadFileSettings);
    showButton(User3, loadFileSettings);

    resize(configDialogSize(name + QString::QString(" Tool Dialog")));

    // -------------------------------------------------------------

    m_mainLayout = new QGridLayout( plainPage(), 2, 1 , marginHint(), spacingHint());

    QFrame *headerFrame = new DigikamImagePlugins::BannerWidget(plainPage(), title);
    m_mainLayout->addMultiCellWidget(headerFrame, 0, 0, 0, 1);

    // -------------------------------------------------------------

    QFrame *frame = new QFrame(plainPage());
    frame->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QVBoxLayout* l = new QVBoxLayout(frame, 5, 0);
    m_imagePreviewWidget = new Digikam::ImageGuideWidget(240, 160, frame, guideVisible, guideMode);

    if (guideVisible)
        QWhatsThis::add( m_imagePreviewWidget, i18n("<p>This is the the image filter effect preview. "
                                                    "If you move the mouse cursor on this area, "
                                                    "a vertical and horizontal dashed line will be draw "
                                                    "to guide you in adjusting the filter settings. "
                                                    "Press the left mouse button to freeze the dashed "
                                                    "line's position."));
    else
        QWhatsThis::add( m_imagePreviewWidget, i18n("<p>This is the image filter effect preview."));

    l->addWidget(m_imagePreviewWidget, 0);
    m_mainLayout->addMultiCellWidget(frame, 1, 2, 0, 0);
    m_mainLayout->setColStretch(0, 10);
    m_mainLayout->setRowStretch(2, 10);

    // -------------------------------------------------------------

    QVBoxLayout *vLayout = new QVBoxLayout( spacingHint() );
    m_progressBar = new KProgress(100, plainPage());
    QWhatsThis::add(m_progressBar ,i18n("<p>This is the current percentage of the task completed."));
    m_progressBar->setValue(0);

    if (progress) m_progressBar->show();
    else m_progressBar->hide();

    vLayout->addWidget(m_progressBar);

    // -------------------------------------------------------------

    QWidget *gboxGuideSettings = new QWidget(plainPage());
    QGridLayout* grid = new QGridLayout( gboxGuideSettings, 2, 2, marginHint(), spacingHint());
    KSeparator *line = new KSeparator (Horizontal, gboxGuideSettings);
    grid->addMultiCellWidget(line, 0, 0, 0, 2);

    QLabel *label5 = new QLabel(i18n("Guide color:"), gboxGuideSettings);
    m_guideColorBt = new KColorButton( QColor( Qt::red ), gboxGuideSettings );
    QWhatsThis::add( m_guideColorBt, i18n("<p>Set here the color used to draw guides dashed-lines."));
    grid->addMultiCellWidget(label5, 1, 1, 0, 0);
    grid->addMultiCellWidget(m_guideColorBt, 1, 1, 1, 2);

    QLabel *label6 = new QLabel(i18n("Guide width:"), gboxGuideSettings);
    m_guideSize = new QSpinBox( 1, 5, 1, gboxGuideSettings);
    QWhatsThis::add( m_guideSize, i18n("<p>Set here the width in pixels used to draw guides dashed-lines."));
    grid->addMultiCellWidget(label6, 2, 2, 0, 0);
    grid->addMultiCellWidget(m_guideSize, 2, 2, 1, 2);

    if (guideVisible) gboxGuideSettings->show();
    else gboxGuideSettings->hide();

    vLayout->addWidget(gboxGuideSettings);
    vLayout->addStretch(10);
    m_mainLayout->addMultiCellLayout(vLayout, 2, 2, 1, 1);

    // -------------------------------------------------------------

    QTimer::singleShot(0, this, SLOT(slotInit()));
}

ImageGuideDialog::~ImageGuideDialog()
{
    saveDialogSize(m_name + QString::QString(" Tool Dialog"));

    if (m_timer)
       delete m_timer;

    if (m_threadedFilter)
       delete m_threadedFilter;

    if (m_about)
       delete m_about;                    
}

void ImageGuideDialog::readSettings(void)
{
    QColor *defaultGuideColor = new QColor( Qt::red );
    KConfig *config = kapp->config();
    config->setGroup(m_name + QString::QString(" Tool Dialog"));
    m_guideColorBt->setColor(config->readColorEntry("Guide Color", defaultGuideColor));
    m_guideSize->setValue(config->readNumEntry("Guide Width", 1));
    m_imagePreviewWidget->slotChangeGuideSize(m_guideSize->value());
    m_imagePreviewWidget->slotChangeGuideColor(m_guideColorBt->color());
    delete defaultGuideColor;
}

void ImageGuideDialog::writeSettings(void)
{
    KConfig *config = kapp->config();
    config->setGroup(m_name + QString::QString(" Tool Dialog"));
    config->writeEntry( "Guide Color", m_guideColorBt->color() );
    config->writeEntry( "Guide Width", m_guideSize->value() );
    config->sync();
}

void ImageGuideDialog::slotInit()
{
    readSettings();
    // Reset values to defaults.
    QTimer::singleShot(0, this, SLOT(readUserSettings()));

    connect(m_imagePreviewWidget, SIGNAL(signalResized()),
            this, SLOT(slotResized()));

    connect(m_guideColorBt, SIGNAL(changed(const QColor &)),
            m_imagePreviewWidget, SLOT(slotChangeGuideColor(const QColor &)));

    connect(m_guideSize, SIGNAL(valueChanged(int)),
            m_imagePreviewWidget, SLOT(slotChangeGuideSize(int)));
}

void ImageGuideDialog::setUserAreaWidget(QWidget *w)
{
    QVBoxLayout *vLayout = new QVBoxLayout( spacingHint() );
    vLayout->addWidget(w);
    m_mainLayout->addMultiCellLayout(vLayout, 1, 1, 1, 1);
}

void ImageGuideDialog::setAboutData(KAboutData *about)
{
    m_about = about;
    QPushButton *helpButton = actionButton( Help );
    KHelpMenu* helpMenu = new KHelpMenu(this, m_about, false);
    helpMenu->menu()->removeItemAt(0);
    helpMenu->menu()->insertItem(i18n("Plugin Handbook"), this, SLOT(slotHelp()), 0, -1, 0);
    helpButton->setPopup( helpMenu->menu() );
}

void ImageGuideDialog::abortPreview()
{
    m_currentRenderingMode = NoneRendering;
    m_progressBar->setValue(0);
    enableButton(Ok, true);
    enableButton(User1, false);
    enableButton(User2, true);
    enableButton(User3, true);
    enableButton(Default, true);
    renderingFinished();
}

void ImageGuideDialog::slotResized(void)
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

void ImageGuideDialog::slotUser1()
{
    if (m_currentRenderingMode != NoneRendering)
        if (m_threadedFilter)
            m_threadedFilter->stopComputation();
}

void ImageGuideDialog::slotDefault()
{
    resetValues();
    slotEffect();
}

void ImageGuideDialog::slotCancel()
{
    if (m_currentRenderingMode != NoneRendering)
    {
       if (m_threadedFilter)
          m_threadedFilter->stopComputation();

       kapp->restoreOverrideCursor();
    }
    
    done(Cancel);
}

void ImageGuideDialog::closeEvent(QCloseEvent *e)
{
    if (m_currentRenderingMode != NoneRendering)
    {
       if (m_threadedFilter)
          m_threadedFilter->stopComputation();

       kapp->restoreOverrideCursor();
    }

    e->accept();
}

void ImageGuideDialog::slotHelp()
{
    KApplication::kApplication()->invokeHelp(m_name, "digikamimageplugins");
}

void ImageGuideDialog::slotTimer()
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

void ImageGuideDialog::slotEffect()
{
    // Computation already in process.
    if (m_currentRenderingMode == PreviewRendering) return;

    m_currentRenderingMode = PreviewRendering;

    enableButton(Ok,    false);
    enableButton(User1, true);
    enableButton(User2, false);
    enableButton(User3, false);
    enableButton(Default, false);
    m_progressBar->setValue(0);

    if (m_threadedFilter)
       delete m_threadedFilter;

    prepareEffect();
}

void ImageGuideDialog::slotOk()
{
    writeUserSettings();
    m_currentRenderingMode = FinalRendering;

    enableButton(Ok,    false);
    enableButton(User1, false);
    enableButton(User2, false);
    enableButton(User3, false);
    enableButton(Default, false);
    kapp->setOverrideCursor( KCursor::waitCursor() );
    m_progressBar->setValue(0);

    if (m_threadedFilter)
       delete m_threadedFilter;

    prepareFinal();
}

void ImageGuideDialog::customEvent(QCustomEvent *event)
{
    if (!event) return;

    Digikam::ThreadedFilter::EventData *d = (Digikam::ThreadedFilter::EventData*) event->data();

    if (!d) return;

    if (d->starting)           // Computation in progress !
    {
        m_progressBar->setValue(d->progress);
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

void ImageGuideDialog::keyPressEvent(QKeyEvent *e)
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

#include "imageguidedialog.moc"
