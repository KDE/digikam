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
#include "greycstorationsettings.h"
#include "greycstorationwidget.h"
#include "greycstorationiface.h"
#include "bannerwidget.h"
#include "imageeffect_blowup.h"
#include "imageeffect_blowup.moc"

namespace DigikamBlowUpImagesPlugin
{

ImageEffect_BlowUp::ImageEffect_BlowUp(QWidget* parent)
                  : KDialogBase(Plain, i18n("Blowup Photograph"),
                                Help|Default|User2|User3|Ok|Cancel, Ok,
                                parent, 0, true, true,
                                QString(),
                                i18n("&Save As..."),
                                i18n("&Load...")),
                    m_parent(parent)
{
    QString whatsThis;
    setButtonWhatsThis ( Default, i18n("<p>Reset all filter parameters to their default values.") );
    setButtonWhatsThis ( User3, i18n("<p>Load all filter parameters from settings text file.") );
    setButtonWhatsThis ( User2, i18n("<p>Save all filter parameters to settings text file.") );

    m_currentRenderingMode = NoneRendering;
    m_cimgInterface        = 0L;

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

    QVBoxLayout *vlay = new QVBoxLayout(topLayout);
    m_mainTab         = new QTabWidget( plainPage() );

    QWidget* firstPage = new QWidget( m_mainTab );
    QGridLayout* grid = new QGridLayout( firstPage, 4, 2, spacingHint());
    m_mainTab->addTab( firstPage, i18n("New Size") );

    KURLLabel *cimgLogoLabel = new KURLLabel(firstPage);
    cimgLogoLabel->setText(QString());
    cimgLogoLabel->setURL("http://cimg.sourceforge.net");
    KGlobal::dirs()->addResourceType("cimg-logo", KGlobal::dirs()->kde_default("data") + "digikamimageplugins/data");
    QString directory = KGlobal::dirs()->findResourceDir("cimg-logo", "cimg-logo.png");
    cimgLogoLabel->setPixmap( QPixmap( directory + "cimg-logo.png" ) );
    QToolTip::add(cimgLogoLabel, i18n("Visit CImg library website"));

    QLabel *label1 = new QLabel(i18n("Width:"), firstPage);
    label1->setAlignment ( Qt::AlignRight | Qt::AlignVCenter);
    m_newWidth = new KIntNumInput(firstPage);
    QWhatsThis::add( m_newWidth, i18n("<p>Set here the new imager width in pixels."));

    QLabel *label2 = new QLabel(i18n("Height:"), firstPage);
    label2->setAlignment ( Qt::AlignRight | Qt::AlignVCenter);
    m_newHeight = new KIntNumInput(firstPage);
    QWhatsThis::add( m_newHeight, i18n("<p>Set here the new image height in pixels."));

    m_preserveRatioBox = new QCheckBox(i18n("Maintain aspect ratio"), firstPage);
    QWhatsThis::add( m_preserveRatioBox, i18n("<p>Enable this option to maintain aspect ratio with new image sizes."));

    grid->addMultiCellWidget(cimgLogoLabel, 0, 2, 0, 0);
    grid->addMultiCellWidget(m_preserveRatioBox, 0, 0, 2, 2);
    grid->addMultiCellWidget(label1, 1, 1, 1, 1);
    grid->addMultiCellWidget(m_newWidth, 1, 1, 2, 2);
    grid->addMultiCellWidget(label2, 2, 2, 1, 1);
    grid->addMultiCellWidget(m_newHeight, 2, 2, 2, 2);

    m_progressBar = new KProgress(100, firstPage);
    m_progressBar->setValue(0);
    QWhatsThis::add( m_progressBar, i18n("<p>This is the current percentage of the task completed.") );
    grid->addMultiCellWidget(m_progressBar, 3, 3, 0, 2);
    grid->setRowStretch(4, 10);

    // -------------------------------------------------------------

    m_settingsWidget = new DigikamImagePlugins::GreycstorationWidget(m_mainTab);
    vlay->addWidget(m_mainTab);

    // -------------------------------------------------------------

    adjustSize();
    disableResize();
    QTimer::singleShot(0, this, SLOT(readUserSettings()));

    // -------------------------------------------------------------

    connect(cimgLogoLabel, SIGNAL(leftClickedURL(const QString&)),
            this, SLOT(processCImgURL(const QString&)));

    connect(m_newWidth, SIGNAL(valueChanged (int)),
            this, SLOT(slotAdjustRatioFromWidth(int)));

    connect(m_newHeight, SIGNAL(valueChanged (int)),
            this, SLOT(slotAdjustRatioFromHeight(int)));
}

ImageEffect_BlowUp::~ImageEffect_BlowUp()
{
    delete m_about;
    
    if (m_cimgInterface)
       delete m_cimgInterface;
}

void ImageEffect_BlowUp::readUserSettings()
{
    KConfig* config = kapp->config();
    config->setGroup("blowup Tool Dialog");

    DigikamImagePlugins::GreycstorationSettings settings;
    settings.fastApprox = config->readBoolEntry("FastApprox", true);
    settings.interp     = config->readNumEntry("Interpolation",
                          DigikamImagePlugins::GreycstorationSettings::NearestNeighbor);
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

    m_newWidth->setValue(config->readNumEntry("NewWidth", 1024));
    m_newHeight->setValue(config->readNumEntry("NewHeight", 768));
    m_preserveRatioBox->setChecked(config->readBoolEntry("AspectRatio", true));
}

void ImageEffect_BlowUp::writeUserSettings()
{
    DigikamImagePlugins::GreycstorationSettings settings = m_settingsWidget->getSettings();
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
    config->writeEntry("NewWidth", m_newWidth->value());
    config->writeEntry("NewHeight", m_newHeight->value());
    config->writeEntry("AspectRatio", m_preserveRatioBox->isChecked());
    config->sync();
}

void ImageEffect_BlowUp::slotDefault()
{
    DigikamImagePlugins::GreycstorationSettings settings;
    settings.setResizeDefaultSettings();   
    m_settingsWidget->setSettings(settings);

    Digikam::ImageIface iface(0, 0);
    m_aspectRatio = (double)iface.originalWidth() / (double)iface.originalHeight();
    m_preserveRatioBox->setChecked(true);
    m_newWidth->blockSignals(true);
    m_newHeight->blockSignals(true);
    m_newWidth->setValue(iface.originalWidth());
    m_newHeight->setValue(iface.originalHeight());
    m_newWidth->blockSignals(false);
    m_newHeight->blockSignals(false);
}

void ImageEffect_BlowUp::slotAdjustRatioFromWidth(int w)
{
    if ( m_preserveRatioBox->isChecked() )
    {
        m_newHeight->blockSignals(true);
        m_newHeight->setValue( (int) (w / m_aspectRatio) );
        m_newHeight->blockSignals(false);
    }
}

void ImageEffect_BlowUp::slotAdjustRatioFromHeight(int h)
{
    if ( m_preserveRatioBox->isChecked() )
    {
        m_newWidth->blockSignals(true);
        m_newWidth->setValue( (int)(m_aspectRatio * h) );
        m_newWidth->blockSignals(false);
    }
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
    m_currentRenderingMode = FinalRendering;
    m_mainTab->setCurrentPage(0);
    m_settingsWidget->setEnabled(false);
    m_preserveRatioBox->setEnabled(false);
    m_newWidth->setEnabled(false);
    m_newHeight->setEnabled(false);
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

    m_cimgInterface = new DigikamImagePlugins::GreycstorationIface(
                                    &originalImage, m_settingsWidget->getSettings(),
                                    DigikamImagePlugins::GreycstorationIface::Resize, 
                                    m_newWidth->value(),
                                    m_newHeight->value(),
                                    0, this);
}

void ImageEffect_BlowUp::customEvent(QCustomEvent *event)
{
    if (!event) return;

    DigikamImagePlugins::GreycstorationIface::EventData *d =
                        (DigikamImagePlugins::GreycstorationIface::EventData*) event->data();

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

