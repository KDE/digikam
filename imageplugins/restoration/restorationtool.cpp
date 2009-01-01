/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-03-26
 * Description : a digiKam image editor plugin to restore
 *               a photograph
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


#include "restorationtool.h"
#include "restorationtool.moc"

// Qt includes.

#include <QFile>
#include <QGridLayout>
#include <QImage>
#include <QLabel>
#include <QPixmap>

// KDE includes.

#include <kaboutdata.h>
#include <kapplication.h>
#include <kcombobox.h>
#include <kdebug.h>
#include <kfiledialog.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <ktabwidget.h>
#include <ktoolinvocation.h>
#include <kurllabel.h>

// Local includes.

#include "version.h"
#include "imageiface.h"
#include "imagepanelwidget.h"
#include "editortoolsettings.h"
#include "greycstorationsettings.h"
#include "greycstorationwidget.h"
#include "greycstorationiface.h"

using namespace Digikam;

namespace DigikamRestorationImagesPlugin
{

RestorationTool::RestorationTool(QObject* parent)
               : EditorToolThreaded(parent)
{
    setObjectName("restoration");
    setToolName(i18n("Restoration"));
    setToolIcon(SmallIcon("restoration"));

    // -------------------------------------------------------------

    m_gboxSettings = new EditorToolSettings(EditorToolSettings::Default|
                                            EditorToolSettings::Ok|
                                            EditorToolSettings::Cancel|
                                            EditorToolSettings::Load|
                                            EditorToolSettings::SaveAs|
                                            EditorToolSettings::Try,
                                            EditorToolSettings::PanIcon);

    QGridLayout* gridSettings = new QGridLayout(m_gboxSettings->plainPage());
    m_mainTab = new KTabWidget( m_gboxSettings->plainPage() );

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
    m_restorationTypeCB = new KComboBox(firstPage);
    m_restorationTypeCB->addItem( i18nc("no restoration preset", "None") );
    m_restorationTypeCB->addItem( i18n("Reduce Uniform Noise") );
    m_restorationTypeCB->addItem( i18n("Reduce JPEG Artifacts") );
    m_restorationTypeCB->addItem( i18n("Reduce Texturing") );
    m_restorationTypeCB->setWhatsThis( i18n("<p>Select the filter preset to use for photograph restoration here:</p>"
                                            "<p><b>None</b>: Most common values. Puts settings to default.<br/>"
                                            "<b>Reduce Uniform Noise</b>: reduce small image artifacts like sensor noise.<br/>"
                                            "<b>Reduce JPEG Artifacts</b>: reduce large image artifacts like JPEG compression mosaic.<br/>"
                                            "<b>Reduce Texturing</b>: reduce image artifacts like paper texture or Moire patterns "
                                            "of a scanned image.</p>"));

    grid->addWidget(cimgLogoLabel,       0, 1, 1, 1);
    grid->addWidget(typeLabel,           1, 0, 1, 1);
    grid->addWidget(m_restorationTypeCB, 1, 1, 1, 1);
    grid->setRowStretch(1, 10);
    grid->setMargin(m_gboxSettings->spacingHint());
    grid->setSpacing(0);

    // -------------------------------------------------------------

    m_settingsWidget = new GreycstorationWidget( m_mainTab );
    gridSettings->addWidget(m_mainTab,                               0, 1, 1, 1);
    gridSettings->addWidget(new QLabel(m_gboxSettings->plainPage()), 1, 1, 1, 1);
    gridSettings->setMargin(m_gboxSettings->spacingHint());
    gridSettings->setSpacing(m_gboxSettings->spacingHint());
    gridSettings->setRowStretch(2, 10);

    setToolSettings(m_gboxSettings);

    // -------------------------------------------------------------

    m_previewWidget = new ImagePanelWidget(470, 350, "restoration Tool", m_gboxSettings->panIconView());

    setToolView(m_previewWidget);
    init();

    // -------------------------------------------------------------

    connect(cimgLogoLabel, SIGNAL(leftClickedUrl(const QString&)),
            this, SLOT(processCImgUrl(const QString&)));

    connect(m_restorationTypeCB, SIGNAL(activated(int)),
            this, SLOT(slotResetValues(int)));

    // -------------------------------------------------------------

    GreycstorationSettings defaults;
    defaults.setRestorationDefaultSettings();
    m_settingsWidget->setDefaultSettings(defaults);
}

RestorationTool::~RestorationTool()
{
}

void RestorationTool::renderingFinished()
{
    m_previewWidget->setEnable(true);
    m_mainTab->setEnabled(true);
}

void RestorationTool::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("restoration Tool");

    GreycstorationSettings settings;
    GreycstorationSettings defaults;
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

void RestorationTool::writeSettings()
{
    GreycstorationSettings settings = m_settingsWidget->getSettings();
    KSharedConfig::Ptr config       = KGlobal::config();
    KConfigGroup group = config->group("restoration Tool");
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

void RestorationTool::slotResetValues(int i)
{
    if (i == NoPreset)
        m_settingsWidget->setEnabled(true);
    else
        m_settingsWidget->setEnabled(false);

    slotResetSettings();
}

void RestorationTool::slotResetSettings()
{
    GreycstorationSettings settings;
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

void RestorationTool::processCImgUrl(const QString& url)
{
    KToolInvocation::invokeBrowser(url);
}

void RestorationTool::prepareEffect()
{
    m_mainTab->setEnabled(false);

    DImg previewImage = m_previewWidget->getOriginalRegionImage();

    setFilter(dynamic_cast<DImgThreadedFilter*>(new GreycstorationIface(&previewImage,
                                                m_settingsWidget->getSettings(), GreycstorationIface::Restore,
                                                0, 0, QImage(), this)));
}

void RestorationTool::prepareFinal()
{
    m_mainTab->setEnabled(false);

    ImageIface iface(0, 0);
    uchar *data = iface.getOriginalImage();
    DImg originalImage(iface.originalWidth(), iface.originalHeight(),
                       iface.originalSixteenBit(), iface.originalHasAlpha(), data);

    setFilter(dynamic_cast<DImgThreadedFilter*>(new GreycstorationIface(&originalImage,
                                                m_settingsWidget->getSettings(), GreycstorationIface::Restore,
                                                0, 0, QImage(), this)));

    delete [] data;
}

void RestorationTool::putPreviewData()
{
    DImg imDest = filter()->getTargetImage();
    m_previewWidget->setPreviewImage(imDest);
}

void RestorationTool::putFinalData()
{
    ImageIface iface(0, 0);
    iface.putOriginalImage(i18n("Restoration"), filter()->getTargetImage().bits());
}

void RestorationTool::slotLoadSettings()
{
    KUrl loadRestorationFile = KFileDialog::getOpenUrl(KGlobalSettings::documentPath(),
                                            QString( "*" ), kapp->activeWindow(),
                                            QString( i18n("Photograph Restoration Settings File to Load")) );
    if( loadRestorationFile.isEmpty() )
       return;

    QFile file(loadRestorationFile.path());

    if ( file.open(QIODevice::ReadOnly) )
    {
        if (!m_settingsWidget->loadSettings(file, QString("# Photograph Restoration Configuration File V2")))
        {
           KMessageBox::error(kapp->activeWindow(),
                        i18n("\"%1\" is not a Photograph Restoration settings text file.",
                             loadRestorationFile.fileName()));
           file.close();
           return;
        }

        slotEffect();
    }
    else
        KMessageBox::error(kapp->activeWindow(), i18n("Cannot load settings from the Photograph Restoration text file."));

    file.close();
    m_restorationTypeCB->blockSignals(true);
    m_restorationTypeCB->setCurrentIndex((int)NoPreset);
    m_restorationTypeCB->blockSignals(false);
    m_settingsWidget->setEnabled(true);
}

void RestorationTool::slotSaveAsSettings()
{
    KUrl saveRestorationFile = KFileDialog::getSaveUrl(KGlobalSettings::documentPath(),
                                            QString( "*" ), kapp->activeWindow(),
                                            QString( i18n("Photograph Restoration Settings File to Save")) );
    if( saveRestorationFile.isEmpty() )
       return;

    QFile file(saveRestorationFile.path());

    if ( file.open(QIODevice::WriteOnly) )
        m_settingsWidget->saveSettings(file, QString("# Photograph Restoration Configuration File V2"));
    else
        KMessageBox::error(kapp->activeWindow(), i18n("Cannot save settings to the Photograph Restoration text file."));

    file.close();
}

}  // namespace DigikamRestorationImagesPlugin
