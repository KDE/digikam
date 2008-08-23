/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-03-26
 * Description : a digiKam image editor plugin to restore
 *               a photograph
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

#include <QComboBox>
#include <QFile>
#include <QGridLayout>
#include <QImage>
#include <QLabel>
#include <QPixmap>
#include <QTabWidget>

// KDE includes.

#include <kaboutdata.h>
#include <kapplication.h>
#include <kfiledialog.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <ktoolinvocation.h>
#include <kurllabel.h>

// Local includes.

#include "version.h"
#include "daboutdata.h"
#include "ddebug.h"
#include "imageiface.h"
#include "greycstorationsettings.h"
#include "greycstorationwidget.h"
#include "greycstorationiface.h"
#include "imageeffect_restoration.h"
#include "imageeffect_restoration.moc"

namespace DigikamRestorationImagesPlugin
{

ImageEffect_Restoration::ImageEffect_Restoration(QWidget* parent)
                       : Digikam::CtrlPanelDlg(parent, i18n("Photograph Restoration"),
                                               "restoration", true, true, true,
                                               Digikam::ImagePannelWidget::SeparateViewAll)
{
    QString whatsThis;

    // About data and help button.

    KAboutData* about = new KAboutData("digikam", 0,
                                       ki18n("Photograph Restoration"),
                                       digiKamVersion().toAscii(),
                                       ki18n("A digiKam image plugin to restore a photograph."),
                                       KAboutData::License_GPL,
                                       ki18n("(c) 2005-2008, Gilles Caulier"),
                                       KLocalizedString(),
                                       Digikam::webProjectUrl().url().toUtf8());

    about->addAuthor(ki18n("Gilles Caulier"), ki18n("Author and maintainer"),
                     "caulier dot gilles at gmail dot com");

    about->addAuthor(ki18n("David Tschumperle"), ki18n("CImg library"), 0,
                     "http://cimg.sourceforge.net");

    about->addAuthor(ki18n("Gerhard Kulzer"), ki18n("Feedback and plugin polishing"),
                     "gerhard at kulzer.net");

    setAboutData(about);

    // -------------------------------------------------------------

    m_mainTab = new QTabWidget( m_imagePreviewWidget );

    QWidget* firstPage = new QWidget( m_mainTab );
    QGridLayout* grid  = new QGridLayout(firstPage);
    m_mainTab->addTab( firstPage, i18n("Preset") );

    KUrlLabel *cimgLogoLabel = new KUrlLabel(firstPage);
    cimgLogoLabel->setText(QString());
    cimgLogoLabel->setUrl("http://cimg.sourceforge.net");
    cimgLogoLabel->setPixmap(QPixmap(KStandardDirs::locate("data", "digikam/data/logo-cimg.png")));
    cimgLogoLabel->setToolTip( i18n("Visit CImg library website"));

    QLabel *typeLabel   = new QLabel(i18n("Filtering type:"), firstPage);
    typeLabel->setAlignment ( Qt::AlignRight | Qt::AlignVCenter);
    m_restorationTypeCB = new QComboBox(firstPage);
    m_restorationTypeCB->addItem( i18n("None") );
    m_restorationTypeCB->addItem( i18n("Reduce Uniform Noise") );
    m_restorationTypeCB->addItem( i18n("Reduce JPEG Artefacts") );
    m_restorationTypeCB->addItem( i18n("Reduce Texturing") );
    m_restorationTypeCB->setWhatsThis( i18n("<p>Select the filter preset to use for photograph restoration here:<p>"
                                            "<b>None</b>: Most common values. Puts settings to default.<p>"
                                            "<b>Reduce Uniform Noise</b>: reduce small image artifacts like sensor noise.<p>"
                                            "<b>Reduce JPEG Artefacts</b>: reduce large image artifacts like JPEG compression mosaic.<p>"
                                            "<b>Reduce Texturing</b>: reduce image artifacts like paper texture or Moire patterns "
                                            "of a scanned image.<p>"));

    grid->setMargin(spacingHint());
    grid->setSpacing(0);
    grid->addWidget(cimgLogoLabel, 0, 1, 1, 1);
    grid->addWidget(typeLabel, 1, 0, 1, 1);
    grid->addWidget(m_restorationTypeCB, 1, 1, 1, 1);
    grid->setRowStretch(1, 10);

    // -------------------------------------------------------------

    m_settingsWidget = new Digikam::GreycstorationWidget( m_mainTab );
    m_imagePreviewWidget->setUserAreaWidget(m_mainTab);

    // -------------------------------------------------------------

    connect(cimgLogoLabel, SIGNAL(leftClickedUrl(const QString&)),
            this, SLOT(processCImgUrl(const QString&)));

    connect(m_restorationTypeCB, SIGNAL(activated(int)),
            this, SLOT(slotResetValues(int)));

    // -------------------------------------------------------------

    Digikam::GreycstorationSettings defaults;
    defaults.setRestorationDefaultSettings();
    m_settingsWidget->setDefaultSettings(defaults);
}

ImageEffect_Restoration::~ImageEffect_Restoration()
{
}

void ImageEffect_Restoration::renderingFinished()
{
    m_imagePreviewWidget->setEnable(true);
    m_mainTab->setEnabled(true);
}

void ImageEffect_Restoration::readUserSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group("restoration Tool Dialog");

    Digikam::GreycstorationSettings settings;
    Digikam::GreycstorationSettings defaults;
    defaults.setRestorationDefaultSettings();

    settings.fastApprox = group.readEntry("FastApprox", defaults.fastApprox);
    settings.interp     = group.readEntry("Interpolation", defaults.interp);
    settings.amplitude  = group.readEntry("Amplitude", defaults.amplitude);
    settings.sharpness  = group.readEntry("Sharpness", defaults.sharpness);
    settings.anisotropy = group.readEntry("Anisotropy", defaults.anisotropy);
    settings.alpha      = group.readEntry("Alpha", defaults.alpha);
    settings.sigma      = group.readEntry("Sigma", defaults.sigma);
    settings.gaussPrec  = group.readEntry("GaussPrec", defaults.gaussPrec);
    settings.dl         = group.readEntry("Dl", defaults.dl);
    settings.da         = group.readEntry("Iteration", defaults.nbIter);
    settings.tile       = group.readEntry("Tile", defaults.tile);
    settings.btile      = group.readEntry("BTile", defaults.btile);
    m_settingsWidget->setSettings(settings);

    int p = group.readEntry("Preset", (int)NoPreset);
    m_restorationTypeCB->setCurrentIndex(p);
    if (p == NoPreset)
        m_settingsWidget->setEnabled(true);
    else
        m_settingsWidget->setEnabled(false);
}

void ImageEffect_Restoration::writeUserSettings()
{
    Digikam::GreycstorationSettings settings = m_settingsWidget->getSettings();
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group("restoration Tool Dialog");
    group.writeEntry("Preset", m_restorationTypeCB->currentIndex());
    group.writeEntry("FastApprox", settings.fastApprox);
    group.writeEntry("Interpolation", settings.interp);
    group.writeEntry("Amplitude", (double)settings.amplitude);
    group.writeEntry("Sharpness", (double)settings.sharpness);
    group.writeEntry("Anisotropy", (double)settings.anisotropy);
    group.writeEntry("Alpha", (double)settings.alpha);
    group.writeEntry("Sigma", (double)settings.sigma);
    group.writeEntry("GaussPrec", (double)settings.gaussPrec);
    group.writeEntry("Dl", (double)settings.dl);
    group.writeEntry("Da", (double)settings.da);
    group.writeEntry("Iteration", settings.nbIter);
    group.writeEntry("Tile", settings.tile);
    group.writeEntry("BTile", settings.btile);
    group.sync();
}

void ImageEffect_Restoration::slotResetValues(int i)
{
    if (i == NoPreset)
        m_settingsWidget->setEnabled(true);
    else
        m_settingsWidget->setEnabled(false);

    resetValues();
}

void ImageEffect_Restoration::resetValues()
{
    Digikam::GreycstorationSettings settings;
    settings.setRestorationDefaultSettings();

    switch(m_restorationTypeCB->currentIndex())
    {
        case ReduceUniformNoise:
        {
            settings.amplitude = 40.0;
            break;
        }

        case ReduceJPEGArtefacts:
        {
            settings.sharpness = 0.3;
            settings.sigma     = 1.0;
            settings.amplitude = 100.0;
            settings.nbIter    = 2;
            break;
        }

        case ReduceTexturing:
        {
            settings.sharpness = 0.5;
            settings.sigma     = 1.5;
            settings.amplitude = 100.0;
            settings.nbIter    = 2;
            break;
        }
    }

    m_settingsWidget->setSettings(settings);
}

void ImageEffect_Restoration::processCImgUrl(const QString& url)
{
    KToolInvocation::invokeBrowser(url);
}

void ImageEffect_Restoration::prepareEffect()
{
    m_mainTab->setEnabled(false);

    Digikam::DImg previewImage = m_imagePreviewWidget->getOriginalRegionImage();

    m_threadedFilter = dynamic_cast<Digikam::DImgThreadedFilter *>(
                       new Digikam::GreycstorationIface(
                                    &previewImage, m_settingsWidget->getSettings(),
                                    Digikam::GreycstorationIface::Restore,
                                    0, 0, QImage(), this));
}

void ImageEffect_Restoration::prepareFinal()
{
    m_mainTab->setEnabled(false);

    Digikam::ImageIface iface(0, 0);
    uchar *data = iface.getOriginalImage();
    Digikam::DImg originalImage(iface.originalWidth(), iface.originalHeight(),
                                iface.originalSixteenBit(), iface.originalHasAlpha(), data);

    m_threadedFilter = dynamic_cast<Digikam::DImgThreadedFilter *>(
                       new Digikam::GreycstorationIface(
                                    &originalImage, m_settingsWidget->getSettings(),
                                    Digikam::GreycstorationIface::Restore,
                                    0, 0, QImage(), this));

    delete [] data;
}

void ImageEffect_Restoration::putPreviewData(void)
{
    Digikam::DImg imDest = m_threadedFilter->getTargetImage();
    m_imagePreviewWidget->setPreviewImage(imDest);
}

void ImageEffect_Restoration::putFinalData(void)
{
    Digikam::ImageIface iface(0, 0);

    iface.putOriginalImage(i18n("Restoration"),
                           m_threadedFilter->getTargetImage().bits());
}

void ImageEffect_Restoration::slotUser3()
{
    KUrl loadRestorationFile = KFileDialog::getOpenUrl(KGlobalSettings::documentPath(),
                                            QString( "*" ), this,
                                            QString( i18n("Photograph Restoration Settings File to Load")) );
    if( loadRestorationFile.isEmpty() )
       return;

    QFile file(loadRestorationFile.path());

    if ( file.open(QIODevice::ReadOnly) )
    {
        if (!m_settingsWidget->loadSettings(file, QString("# Photograph Restoration Configuration File V2")))
        {
           KMessageBox::error(this,
                        i18n("\"%1\" is not a Photograph Restoration settings text file.",
                             loadRestorationFile.fileName()));
           file.close();
           return;
        }

        slotEffect();
    }
    else
        KMessageBox::error(this, i18n("Cannot load settings from the Photograph Restoration text file."));

    file.close();
    m_restorationTypeCB->blockSignals(true);
    m_restorationTypeCB->setCurrentIndex((int)NoPreset);
    m_restorationTypeCB->blockSignals(false);
    m_settingsWidget->setEnabled(true);
}

void ImageEffect_Restoration::slotUser2()
{
    KUrl saveRestorationFile = KFileDialog::getSaveUrl(KGlobalSettings::documentPath(),
                                            QString( "*" ), this,
                                            QString( i18n("Photograph Restoration Settings File to Save")) );
    if( saveRestorationFile.isEmpty() )
       return;

    QFile file(saveRestorationFile.path());

    if ( file.open(QIODevice::WriteOnly) )
        m_settingsWidget->saveSettings(file, QString("# Photograph Restoration Configuration File V2"));
    else
        KMessageBox::error(this, i18n("Cannot save settings to the Photograph Restoration text file."));

    file.close();
}

}  // NameSpace DigikamRestorationImagesPlugin
