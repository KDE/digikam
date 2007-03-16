/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at gmail dot com>
 * Date   : 2005-04-07
 * Description : a digiKam image editor plugin to blowup
 *               a photograph
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

//#include <cmath>

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
#include <kaboutdata.h>
#include <khelpmenu.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kpopupmenu.h>
#include <kfiledialog.h>
#include <kstandarddirs.h>
#include <kprogress.h>
#include <kmessagebox.h>
#include <knuminput.h>
#include <kglobalsettings.h>

// Digikam includes.

#include <digikamheaders.h>

// Local includes.

#include "version.h"
#include "bannerwidget.h"
#include "imageeffect_blowup.h"
#include "imageeffect_blowup.moc"

namespace DigikamBlowUpImagesPlugin
{

ImageEffect_BlowUp::ImageEffect_BlowUp(QWidget* parent)
                  : KDialogBase(Plain, i18n("Blowup Photograph"),
                                Help|Default|User2|User3|Ok|Cancel, Ok,
                                parent, 0, true, false,
                                QString(),
                                i18n("&Save As..."),
                                i18n("&Load...")),
                    m_parent(parent)
{
    QString whatsThis;
    setButtonWhatsThis ( Default, i18n("<p>Reset all filter parameters to their default values.") );
    setButtonWhatsThis ( User3, i18n("<p>Load all filter parameters from settings text file.") );
    setButtonWhatsThis ( User2, i18n("<p>Save all filter parameters to settings text file.") );
    enableButton(Ok, false);

    m_currentRenderingMode = NoneRendering;
    m_cimgInterface        = 0L;

    Digikam::ImageIface iface(0, 0);
    m_orgWidth    = iface.originalWidth();
    m_orgHeight   = iface.originalHeight();
    m_prevW       = m_orgWidth;
    m_prevH       = m_orgHeight;
    m_prevWP      = 100.0;
    m_prevHP      = 100.0;

    // About data and help button.

    m_about = new KAboutData("digikamimageplugins",
                             I18N_NOOP("Blowup Photograph"),
                             digikamimageplugins_version,
                             I18N_NOOP("A digiKam image plugin to blowup a photograph."),
                             KAboutData::License_GPL,
                             "(c) 2005-2007, Gilles Caulier",
                             0,
                             "http://extragear.kde.org/apps/digikamimageplugins");

    m_about->addAuthor("Gilles Caulier", I18N_NOOP("Author and maintainer"),
                       "caulier dot gilles at gmail dot com");

    m_about->addAuthor("David Tschumperle", I18N_NOOP("CImg library"), 0,
                       "http://cimg.sourceforge.net");

    m_about->addAuthor("Gerhard Kulzer", I18N_NOOP("Feedback and plugin polishing"),
                       "gerhard at kulzer.net");

    KHelpMenu* helpMenu = new KHelpMenu(this, m_about, false);
    helpMenu->menu()->removeItemAt(0);
    helpMenu->menu()->insertItem(i18n("Plugin Handbook"), this, SLOT(slotHelp()), 0, -1, 0);
    actionButton(Help)->setPopup( helpMenu->menu() );

    // -------------------------------------------------------------

    QVBoxLayout *topLayout = new QVBoxLayout( plainPage(), 0, spacingHint());
    QFrame *headerFrame    = new DigikamImagePlugins::BannerWidget(plainPage(), i18n("Blowup Photograph"));
    topLayout->addWidget(headerFrame);

    // -------------------------------------------------------------

    QVBoxLayout *vlay  = new QVBoxLayout(topLayout);
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

    m_settingsWidget = new Digikam::GreycstorationWidget(m_mainTab);
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

ImageEffect_BlowUp::~ImageEffect_BlowUp()
{
    delete m_about;
    
    if (m_cimgInterface)
       delete m_cimgInterface;
}

void ImageEffect_BlowUp::slotRestorationToggled(bool b)
{
    m_settingsWidget->setEnabled(b);
    m_progressBar->setEnabled(b);
    enableButton(User2, b);
    enableButton(User3, b);
}    

void ImageEffect_BlowUp::readUserSettings()
{
    KConfig* config = kapp->config();
    config->setGroup("blowup Tool Dialog");

    Digikam::GreycstorationSettings settings;
    settings.fastApprox = config->readBoolEntry("FastApprox", true);
    settings.interp     = config->readNumEntry("Interpolation",
                          Digikam::GreycstorationSettings::NearestNeighbor);
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

void ImageEffect_BlowUp::writeUserSettings()
{
    Digikam::GreycstorationSettings settings = m_settingsWidget->getSettings();
    KConfig* config = kapp->config();
    config->setGroup("blowup Tool Dialog");
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

void ImageEffect_BlowUp::slotDefault()
{
    Digikam::GreycstorationSettings settings;
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

void ImageEffect_BlowUp::slotValuesChanged()
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

void ImageEffect_BlowUp::slotCancel()
{
    if (m_currentRenderingMode != NoneRendering)
    {
        m_cimgInterface->stopComputation();
        m_parent->unsetCursor();
    }

    done(Cancel);
}

void ImageEffect_BlowUp::slotHelp()
{
    KApplication::kApplication()->invokeHelp("blowup", "digikamimageplugins");
}

void ImageEffect_BlowUp::processCImgURL(const QString& url)
{
    KApplication::kApplication()->invokeBrowser(url);
}

void ImageEffect_BlowUp::closeEvent(QCloseEvent *e)
{
    if (m_currentRenderingMode != NoneRendering)
    {
        m_cimgInterface->stopComputation();
        m_parent->unsetCursor();
    }

    e->accept();
}

void ImageEffect_BlowUp::slotOk()
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

    Digikam::ImageIface iface(0, 0);
    uchar *data = iface.getOriginalImage();
    Digikam::DImg originalImage = Digikam::DImg(iface.originalWidth(), iface.originalHeight(),
                                  iface.originalSixteenBit(), iface.originalHasAlpha(), data);
    delete [] data;

    if (m_cimgInterface)
    {
        delete m_cimgInterface;
        m_cimgInterface = 0;
    }

    int mode = m_useGreycstorationBox->isChecked() ? Digikam::GreycstorationIface::Resize
                                                   : Digikam::GreycstorationIface::SimpleResize;

    m_cimgInterface = new Digikam::GreycstorationIface(
                                    &originalImage, m_settingsWidget->getSettings(),
                                    mode, 
                                    m_wInput->value(),
                                    m_hInput->value(),
                                    0, this);
}

void ImageEffect_BlowUp::customEvent(QCustomEvent *event)
{
    if (!event) return;

    Digikam::GreycstorationIface::EventData *d =
                        (Digikam::GreycstorationIface::EventData*) event->data();

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
                    DDebug() << "Final BlowUp completed..." << endl;
                    
                    Digikam::ImageIface iface(0, 0);
                    Digikam::DImg resizedImage = m_cimgInterface->getTargetImage();

                    iface.putOriginalImage(i18n("BlowUp"), resizedImage.bits(),
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

void ImageEffect_BlowUp::slotUser3()
{
    KURL loadBlowupFile = KFileDialog::getOpenURL(KGlobalSettings::documentPath(),
                                       QString( "*" ), this,
                                       QString( i18n("Photograph Blowup Settings File to Load")) );
    if( loadBlowupFile.isEmpty() )
       return;

    QFile file(loadBlowupFile.path());

    if ( file.open(IO_ReadOnly) )   
    {
        if (!m_settingsWidget->loadSettings(file, QString("# Photograph Blowup Configuration File V2")))
        {
           KMessageBox::error(this, 
                        i18n("\"%1\" is not a Photograph Blowup settings text file.")
                        .arg(loadBlowupFile.fileName()));
           file.close();            
           return;
        }
    }
    else
        KMessageBox::error(this, i18n("Cannot load settings from the Photograph Blowup text file."));

    file.close();
}

void ImageEffect_BlowUp::slotUser2()
{
    KURL saveBlowupFile = KFileDialog::getSaveURL(KGlobalSettings::documentPath(),
                                       QString( "*" ), this,
                                       QString( i18n("Photograph Blowup Settings File to Save")) );
    if( saveBlowupFile.isEmpty() )
       return;

    QFile file(saveBlowupFile.path());

    if ( file.open(IO_WriteOnly) )   
        m_settingsWidget->saveSettings(file, QString("# Photograph Blowup Configuration File V2"));
    else
        KMessageBox::error(this, i18n("Cannot save settings to the Photograph Blowup text file."));

    file.close();
}

}  // NameSpace DigikamBlowUpImagesPlugin

