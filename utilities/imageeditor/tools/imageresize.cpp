/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at gmail dot com>
 * Date   : 2005-04-07
 * Description : a tool to resize a picture
 *
 * Copyright 2005-2007 by Gilles Caulier 
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

// C++ include.

#include <cmath>

// Qt includes.

#include <qvgroupbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qlayout.h>
#include <qframe.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qtabwidget.h>
#include <qtimer.h>
#include <qevent.h>
#include <qpixmap.h>
#include <qbrush.h>
#include <qfile.h>
#include <qimage.h>

// KDE includes.

#include <kseparator.h>
#include <kcursor.h>
#include <kurllabel.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kfiledialog.h>
#include <kstandarddirs.h>
#include <kprogress.h>
#include <kmessagebox.h>
#include <knuminput.h>
#include <kglobalsettings.h>

// Digikam includes.

#include "dimg.h"
#include "ddebug.h"
#include "imageiface.h"
#include "dimgthreadedfilter.h"
#include "greycstorationiface.h"
#include "greycstorationwidget.h"
#include "greycstorationsettings.h"

// Local includes.

#include "imageresize.h"
#include "imageresize.moc"

namespace Digikam
{

ImageResize::ImageResize(QWidget* parent)
           : KDialogBase(Plain, i18n("Resize Image"),
                         Help|Default|User2|User3|Ok|Cancel, Ok,
                         parent, 0, true, false,
                         QString(),
                         i18n("&Save As..."),
                         i18n("&Load...")),
             m_parent(parent)
{
    setHelp("resizetool.anchor", "digikam");
    QString whatsThis;
    setButtonWhatsThis ( Default, i18n("<p>Reset all filter parameters to their default values.") );
    setButtonWhatsThis ( User3, i18n("<p>Load all filter parameters from settings text file.") );
    setButtonWhatsThis ( User2, i18n("<p>Save all filter parameters to settings text file.") );
    enableButton(Ok, false);

    m_currentRenderingMode = NoneRendering;
    m_cimgInterface        = 0L;

    ImageIface iface(0, 0);
    m_orgWidth    = iface.originalWidth();
    m_orgHeight   = iface.originalHeight();
    m_prevW       = m_orgWidth;
    m_prevH       = m_orgHeight;
    m_prevWP      = 100.0;
    m_prevHP      = 100.0;

    // -------------------------------------------------------------

    QVBoxLayout *vlay  = new QVBoxLayout(plainPage(), 0, spacingHint());
    m_mainTab          = new QTabWidget( plainPage() );

    QWidget* firstPage = new QWidget( m_mainTab );
    QGridLayout* grid  = new QGridLayout( firstPage, 8, 2, spacingHint());
    m_mainTab->addTab( firstPage, i18n("New Size") );

    QLabel *label1 = new QLabel(i18n("Width:"), firstPage);
    m_wInput = new KIntNumInput(firstPage);
    m_wInput->setRange(1, QMAX(m_orgWidth * 10, 9999), 1, true);
    m_wInput->setName("m_wInput");
    QWhatsThis::add( m_wInput, i18n("<p>Set here the new image width in pixels."));

    QLabel *label2 = new QLabel(i18n("Height:"), firstPage);
    m_hInput = new KIntNumInput(firstPage);
    m_hInput->setRange(1, QMAX(m_orgHeight * 10, 9999), 1, true);
    m_hInput->setName("m_hInput");
    QWhatsThis::add( m_hInput, i18n("<p>Set here the new image height in pixels."));

    QLabel *label3 = new QLabel(i18n("Width (%):"), firstPage);
    m_wpInput = new KDoubleNumInput(firstPage);
    m_wpInput->setRange(1.0, 999.0, 1.0, true);
    m_wpInput->setName("m_wpInput");
    QWhatsThis::add( m_wpInput, i18n("<p>Set here the new image width in percents."));

    QLabel *label4 = new QLabel(i18n("Height (%):"), firstPage);
    m_hpInput = new KDoubleNumInput(firstPage);
    m_hpInput->setRange(1.0, 999.0, 1.0, true);
    m_hpInput->setName("m_hpInput");
    QWhatsThis::add( m_hpInput, i18n("<p>Set here the new image height in percents."));

    m_preserveRatioBox = new QCheckBox(i18n("Maintain aspect ratio"), firstPage);
    QWhatsThis::add( m_preserveRatioBox, i18n("<p>Enable this option to maintain aspect "
                                              "ratio with new image sizes."));

    KURLLabel *cimgLogoLabel = new KURLLabel(firstPage);
    cimgLogoLabel->setText(QString());
    cimgLogoLabel->setURL("http://cimg.sourceforge.net");
    KGlobal::dirs()->addResourceType("logo-cimg", KGlobal::dirs()->kde_default("data") +
                                     "digikam/data");
    QString directory = KGlobal::dirs()->findResourceDir("logo-cimg", "logo-cimg.png");
    cimgLogoLabel->setPixmap( QPixmap( directory + "logo-cimg.png" ) );
    QToolTip::add(cimgLogoLabel, i18n("Visit CImg library website"));

    m_useGreycstorationBox = new QCheckBox(i18n("Restore photograph (slow)"), firstPage);
    QWhatsThis::add( m_useGreycstorationBox, i18n("<p>Enable this option to restore photograph content. "
                                                  "Warning: this process can take a while."));

    m_progressBar = new KProgress(100, firstPage);
    m_progressBar->setValue(0);
    QWhatsThis::add(m_progressBar, i18n("<p>This is the current progress when you use Restoration mode."));

    grid->addMultiCellWidget(m_preserveRatioBox, 0, 0, 0, 2);
    grid->addMultiCellWidget(label1, 1, 1, 0, 0);
    grid->addMultiCellWidget(m_wInput, 1, 1, 1, 2);
    grid->addMultiCellWidget(label2, 2, 2, 0, 0);
    grid->addMultiCellWidget(m_hInput, 2, 2, 1, 2);
    grid->addMultiCellWidget(label3, 3, 3, 0, 0);
    grid->addMultiCellWidget(m_wpInput, 3, 3, 1, 2);
    grid->addMultiCellWidget(label4, 4, 4, 0, 0);
    grid->addMultiCellWidget(m_hpInput, 4, 4, 1, 2);
    grid->addMultiCellWidget(new KSeparator(firstPage), 5, 5, 0, 2);
    grid->addMultiCellWidget(cimgLogoLabel, 6, 7, 0, 0);
    grid->addMultiCellWidget(m_useGreycstorationBox, 6, 6, 1, 2);
    grid->addMultiCellWidget(m_progressBar, 7, 7, 1, 2);
    grid->setRowStretch(8, 10);

    // -------------------------------------------------------------

    m_settingsWidget = new GreycstorationWidget(m_mainTab);
    vlay->addWidget(m_mainTab);

    // -------------------------------------------------------------

    adjustSize();
    disableResize();
    QTimer::singleShot(0, this, SLOT(readUserSettings()));

    // -------------------------------------------------------------

    connect(cimgLogoLabel, SIGNAL(leftClickedURL(const QString&)),
            this, SLOT(processCImgURL(const QString&)));

    connect(m_wInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotValuesChanged()));
            
    connect(m_hInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotValuesChanged()));
            
    connect(m_wpInput, SIGNAL(valueChanged(double)),
            this, SLOT(slotValuesChanged()));
            
    connect(m_hpInput, SIGNAL(valueChanged(double)),
            this, SLOT(slotValuesChanged()));

    connect(m_useGreycstorationBox, SIGNAL(toggled(bool)),
             this, SLOT(slotRestorationToggled(bool)) );
}

ImageResize::~ImageResize()
{
    if (m_cimgInterface)
       delete m_cimgInterface;
}

void ImageResize::slotRestorationToggled(bool b)
{
    m_settingsWidget->setEnabled(b);
    m_progressBar->setEnabled(b);
    enableButton(User2, b);
    enableButton(User3, b);
}    

void ImageResize::readUserSettings()
{
    KConfig* config = kapp->config();
    config->setGroup("resize Tool Dialog");

    GreycstorationSettings settings;
    settings.fastApprox = config->readBoolEntry("FastApprox", true);
    settings.interp     = config->readNumEntry("Interpolation",
                          GreycstorationSettings::NearestNeighbor);
    settings.amplitude  = config->readDoubleNumEntry("Amplitude", 20.0);
    settings.sharpness  = config->readDoubleNumEntry("Sharpness", 0.2);
    settings.anisotropy = config->readDoubleNumEntry("Anisotropy", 0.9);
    settings.alpha      = config->readDoubleNumEntry("Alpha", 0.1);
    settings.sigma      = config->readDoubleNumEntry("Sigma", 1.5);
    settings.gaussPrec  = config->readDoubleNumEntry("GaussPrec", 2.0);
    settings.dl         = config->readDoubleNumEntry("Dl", 0.8);
    settings.da         = config->readDoubleNumEntry("Da", 30.0);
    settings.nbIter     = config->readNumEntry("Iteration", 3);
    settings.tile       = config->readNumEntry("Tile", 512);
    settings.btile      = config->readNumEntry("BTile", 4);
    m_settingsWidget->setSettings(settings);
    m_useGreycstorationBox->setChecked(config->readBoolEntry("RestorePhotograph", false));
    slotRestorationToggled(m_useGreycstorationBox->isChecked());

    m_preserveRatioBox->blockSignals(true);
    m_wInput->blockSignals(true);
    m_hInput->blockSignals(true);
    m_wpInput->blockSignals(true);
    m_hpInput->blockSignals(true);
    m_preserveRatioBox->setChecked(true);
    m_wInput->setValue(m_orgWidth);
    m_hInput->setValue(m_orgHeight);
    m_wpInput->setValue(100);
    m_hpInput->setValue(100);
    m_preserveRatioBox->blockSignals(false);
    m_wInput->blockSignals(false);
    m_hInput->blockSignals(false);
    m_wpInput->blockSignals(false);
    m_hpInput->blockSignals(false);
}

void ImageResize::writeUserSettings()
{
    GreycstorationSettings settings = m_settingsWidget->getSettings();
    KConfig* config = kapp->config();
    config->setGroup("resize Tool Dialog");
    config->writeEntry("FastApprox", settings.fastApprox);
    config->writeEntry("Interpolation", settings.interp);
    config->writeEntry("Amplitude", settings.amplitude);
    config->writeEntry("Sharpness", settings.sharpness);
    config->writeEntry("Anisotropy", settings.anisotropy);
    config->writeEntry("Alpha", settings.alpha);
    config->writeEntry("Sigma", settings.sigma);
    config->writeEntry("GaussPrec", settings.gaussPrec);
    config->writeEntry("Dl", settings.dl);
    config->writeEntry("Da", settings.da);
    config->writeEntry("Iteration", settings.nbIter);
    config->writeEntry("Tile", settings.tile);
    config->writeEntry("BTile", settings.btile);
    config->writeEntry("RestorePhotograph", m_useGreycstorationBox->isChecked());
    config->sync();
}

void ImageResize::slotDefault()
{
    GreycstorationSettings settings;
    settings.setResizeDefaultSettings();   
    m_settingsWidget->setSettings(settings);
    m_useGreycstorationBox->setChecked(false);
    slotRestorationToggled(m_useGreycstorationBox->isChecked());

    m_preserveRatioBox->blockSignals(true);
    m_wInput->blockSignals(true);
    m_hInput->blockSignals(true);
    m_wpInput->blockSignals(true);
    m_hpInput->blockSignals(true);
    m_preserveRatioBox->setChecked(true);
    m_wInput->setValue(m_orgWidth);
    m_hInput->setValue(m_orgHeight);
    m_wpInput->setValue(100.0);
    m_hpInput->setValue(100.0);
    m_preserveRatioBox->blockSignals(false);
    m_wInput->blockSignals(false);
    m_hInput->blockSignals(false);
    m_wpInput->blockSignals(false);
    m_hpInput->blockSignals(false);
}

void ImageResize::slotValuesChanged()
{
    enableButton(Ok, true);
    m_wInput->blockSignals(true);
    m_hInput->blockSignals(true);
    m_wpInput->blockSignals(true);
    m_hpInput->blockSignals(true);
    
    QString s(sender()->name());
    
    if (s == "m_wInput")
    {
        double val = m_wInput->value();
        double wp  = val/(double)(m_orgWidth) * 100.0;
        m_wpInput->setValue(wp);

        if (m_preserveRatioBox->isChecked())
        {
            m_hpInput->setValue(wp);
            int h = (int)(wp*m_orgHeight/100);
            m_hInput->setValue(h);
        }
    }
    else if (s == "m_hInput")
    {
        double val = m_hInput->value();
        double hp  = val/(double)(m_orgHeight) * 100.0;
        m_hpInput->setValue(hp);

        if (m_preserveRatioBox->isChecked())
        {
            m_wpInput->setValue(hp);
            int w = (int)(hp*m_orgWidth/100);
            m_wInput->setValue(w);
        }
    }
    else if (s == "m_wpInput")
    {
        double val = m_wpInput->value();
        int w      = (int)(val*m_orgWidth/100);
        m_wInput->setValue(w);

        if (m_preserveRatioBox->isChecked())
        {
            m_hpInput->setValue(val);
            int h = (int)(val*m_orgHeight/100);
            m_hInput->setValue(h);
        }
    }
    else if (s == "m_hpInput")
    {
        double val = m_hpInput->value();
        int h      = (int)(val*m_orgHeight/100);
        m_hInput->setValue(h);

        if (m_preserveRatioBox->isChecked())
        {
            m_wpInput->setValue(val);
            int w = (int)(val*m_orgWidth/100);
            m_wInput->setValue(w);
        }
    }

    m_prevW  = m_wInput->value();
    m_prevH  = m_hInput->value();
    m_prevWP = m_wpInput->value();
    m_prevHP = m_hpInput->value();
    
    m_wInput->blockSignals(false);
    m_hInput->blockSignals(false);
    m_wpInput->blockSignals(false);
    m_hpInput->blockSignals(false);
}

void ImageResize::slotCancel()
{
    if (m_currentRenderingMode != NoneRendering)
    {
        m_cimgInterface->stopComputation();
        m_parent->unsetCursor();
    }

    done(Cancel);
}

void ImageResize::processCImgURL(const QString& url)
{
    KApplication::kApplication()->invokeBrowser(url);
}

void ImageResize::closeEvent(QCloseEvent *e)
{
    if (m_currentRenderingMode != NoneRendering)
    {
        m_cimgInterface->stopComputation();
        m_parent->unsetCursor();
    }

    e->accept();
}

void ImageResize::slotOk()
{
    if (m_prevW  != m_wInput->value()  || m_prevH  != m_hInput->value() ||
        m_prevWP != m_wpInput->value() || m_prevHP != m_hpInput->value())
        slotValuesChanged();

    m_currentRenderingMode = FinalRendering;
    m_mainTab->setCurrentPage(0);
    m_settingsWidget->setEnabled(false);
    m_preserveRatioBox->setEnabled(false);
    m_useGreycstorationBox->setEnabled(false);
    m_wInput->setEnabled(false);
    m_hInput->setEnabled(false);
    m_wpInput->setEnabled(false);
    m_hpInput->setEnabled(false);
    enableButton(Ok, false);
    enableButton(Default, false);
    enableButton(User2, false);
    enableButton(User3, false);

    m_parent->setCursor( KCursor::waitCursor() );
    m_progressBar->setValue(0);
    m_progressBar->setEnabled(true);
    writeUserSettings();

    ImageIface iface(0, 0);
    uchar *data = iface.getOriginalImage();
    DImg originalImage = DImg(iface.originalWidth(), iface.originalHeight(),
                                  iface.originalSixteenBit(), iface.originalHasAlpha(), data);
    delete [] data;

    if (m_cimgInterface)
    {
        delete m_cimgInterface;
        m_cimgInterface = 0;
    }

    int mode = m_useGreycstorationBox->isChecked() ? GreycstorationIface::Resize
                                                   : GreycstorationIface::SimpleResize;

    m_cimgInterface = new GreycstorationIface(
                          &originalImage, m_settingsWidget->getSettings(),
                          mode, 
                          m_wInput->value(),
                          m_hInput->value(),
                          0, this);
}

void ImageResize::customEvent(QCustomEvent *event)
{
    if (!event) return;

    GreycstorationIface::EventData *d = (GreycstorationIface::EventData*) event->data();

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
                case FinalRendering:
                {
                    DDebug() << "Final resizing completed..." << endl;
                    
                    ImageIface iface(0, 0);
                    DImg resizedImage = m_cimgInterface->getTargetImage();

                    iface.putOriginalImage(i18n("Resize"), resizedImage.bits(),
                                           resizedImage.width(), resizedImage.height());
                    m_parent->unsetCursor();
                    accept();
                    break;
                }
            }
        }
        else                   // Computation Failed !
        {
            switch (m_currentRenderingMode)
            {
                case FinalRendering:
                    break;
            }
        }
    }

    delete d;
}

void ImageResize::slotUser3()
{
    KURL loadBlowupFile = KFileDialog::getOpenURL(KGlobalSettings::documentPath(),
                                       QString( "*" ), this,
                                       QString( i18n("Photograph Resizing Settings File to Load")) );
    if( loadBlowupFile.isEmpty() )
       return;

    QFile file(loadBlowupFile.path());

    if ( file.open(IO_ReadOnly) )   
    {
        if (!m_settingsWidget->loadSettings(file, QString("# Photograph Resizing Configuration File")))
        {
           KMessageBox::error(this, 
                        i18n("\"%1\" is not a Photograph Resizing settings text file.")
                        .arg(loadBlowupFile.fileName()));
           file.close();            
           return;
        }
    }
    else
        KMessageBox::error(this, i18n("Cannot load settings from the Photograph Resizing text file."));

    file.close();
}

void ImageResize::slotUser2()
{
    KURL saveBlowupFile = KFileDialog::getSaveURL(KGlobalSettings::documentPath(),
                                       QString( "*" ), this,
                                       QString( i18n("Photograph Resizing Settings File to Save")) );
    if( saveBlowupFile.isEmpty() )
       return;

    QFile file(saveBlowupFile.path());

    if ( file.open(IO_WriteOnly) )   
        m_settingsWidget->saveSettings(file, QString("# Photograph Resizing Configuration File"));
    else
        KMessageBox::error(this, i18n("Cannot save settings to the Photograph Resizing text file."));

    file.close();
}

}  // NameSpace Digikam

