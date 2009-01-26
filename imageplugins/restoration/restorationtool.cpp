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

// Qt includes.

#include <qlabel.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qlayout.h>
#include <qcombobox.h>
#include <qtabwidget.h>
#include <qfile.h>
#include <qimage.h>

// KDE includes.

#include <kurllabel.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kfiledialog.h>
#include <kglobalsettings.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>

// Local includes.

#include "daboutdata.h"
#include "ddebug.h"
#include "imageiface.h"
#include "imagepanelwidget.h"
#include "editortoolsettings.h"
#include "greycstorationsettings.h"
#include "greycstorationwidget.h"
#include "greycstorationiface.h"
#include "restorationtool.h"
#include "restorationtool.moc"

using namespace Digikam;

namespace DigikamRestorationImagesPlugin
{

RestorationTool::RestorationTool(QObject* parent)
               : EditorToolThreaded(parent)
{
    setName("restoration");
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

    QGridLayout* gridSettings = new QGridLayout(m_gboxSettings->plainPage(), 2, 1);
    m_mainTab                 = new QTabWidget( m_gboxSettings->plainPage() );

    QWidget* firstPage = new QWidget( m_mainTab );
    QGridLayout* grid  = new QGridLayout(firstPage, 2, 2);
    m_mainTab->addTab( firstPage, i18n("Preset") );

    KURLLabel *cimgLogoLabel = new KURLLabel(firstPage);
    cimgLogoLabel->setText(QString());
    cimgLogoLabel->setURL("http://cimg.sourceforge.net");
    KGlobal::dirs()->addResourceType("logo-cimg", KGlobal::dirs()->kde_default("data") + "digikam/data");
    QString directory = KGlobal::dirs()->findResourceDir("logo-cimg", "logo-cimg.png");
    cimgLogoLabel->setPixmap( QPixmap( directory + "logo-cimg.png" ) );
    QToolTip::add(cimgLogoLabel, i18n("Visit CImg library website"));

    QLabel *typeLabel   = new QLabel(i18n("Filtering type:"), firstPage);
    typeLabel->setAlignment ( Qt::AlignRight | Qt::AlignVCenter);
    m_restorationTypeCB = new QComboBox(false, firstPage);
    m_restorationTypeCB->insertItem( i18n("None") );
    m_restorationTypeCB->insertItem( i18n("Reduce Uniform Noise") );
    m_restorationTypeCB->insertItem( i18n("Reduce JPEG Artefacts") );
    m_restorationTypeCB->insertItem( i18n("Reduce Texturing") );
    QWhatsThis::add( m_restorationTypeCB, i18n("<p>Select the filter preset to use for photograph restoration:<p>"
                                               "<b>None</b>: Most common values. Puts settings to default.<p>"
                                               "<b>Reduce Uniform Noise</b>: reduce small image artifacts like sensor noise.<p>"
                                               "<b>Reduce JPEG Artefacts</b>: reduce large image artifacts like JPEG compression mosaic.<p>"
                                               "<b>Reduce Texturing</b>: reduce image artifacts like paper texture or Moire patterns "
                                               "of a scanned image.<p>"));

    grid->addMultiCellWidget(cimgLogoLabel,       0, 0, 1, 1);
    grid->addMultiCellWidget(typeLabel,           1, 1, 0, 0);
    grid->addMultiCellWidget(m_restorationTypeCB, 1, 1, 1, 1);
    grid->setRowStretch(1, 10);
    grid->setMargin(m_gboxSettings->spacingHint());
    grid->setSpacing(m_gboxSettings->spacingHint());

    m_settingsWidget = new GreycstorationWidget( m_mainTab );
    gridSettings->addMultiCellWidget(m_mainTab,                               0, 0, 1, 1);
    gridSettings->addMultiCellWidget(new QLabel(m_gboxSettings->plainPage()), 1, 1, 1, 1);
    gridSettings->setMargin(m_gboxSettings->spacingHint());
    gridSettings->setSpacing(m_gboxSettings->spacingHint());
    gridSettings->setRowStretch(2, 10);

    setToolSettings(m_gboxSettings);

    // -------------------------------------------------------------

    m_previewWidget = new ImagePanelWidget(470, 350, "restoration Tool", m_gboxSettings->panIconView());

    setToolView(m_previewWidget);
    init();

    // -------------------------------------------------------------

    connect(cimgLogoLabel, SIGNAL(leftClickedURL(const QString&)),
            this, SLOT(processCImgURL(const QString&)));

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
    KConfig* config = kapp->config();
    config->setGroup("restoration Tool");

    GreycstorationSettings settings;
    GreycstorationSettings defaults;
    defaults.setRestorationDefaultSettings();

    settings.fastApprox = config->readBoolEntry("FastApprox", defaults.fastApprox);
    settings.interp     = config->readNumEntry("Interpolation", defaults.interp);
    settings.amplitude  = config->readDoubleNumEntry("Amplitude", defaults.amplitude);
    settings.sharpness  = config->readDoubleNumEntry("Sharpness", defaults.sharpness);
    settings.anisotropy = config->readDoubleNumEntry("Anisotropy", defaults.anisotropy);
    settings.alpha      = config->readDoubleNumEntry("Alpha", defaults.alpha);
    settings.sigma      = config->readDoubleNumEntry("Sigma", defaults.sigma);
    settings.gaussPrec  = config->readDoubleNumEntry("GaussPrec", defaults.gaussPrec);
    settings.dl         = config->readDoubleNumEntry("Dl", defaults.dl);
    settings.da         = config->readDoubleNumEntry("Da", defaults.da);
    settings.nbIter     = config->readNumEntry("Iteration", defaults.nbIter);
    settings.tile       = config->readNumEntry("Tile", defaults.tile);
    settings.btile      = config->readNumEntry("BTile", defaults.btile);
    m_settingsWidget->setSettings(settings);

    int p = config->readNumEntry("Preset", NoPreset);
    m_restorationTypeCB->setCurrentItem(p);
    if (p == NoPreset)
        m_settingsWidget->setEnabled(true);
    else
        m_settingsWidget->setEnabled(false);
}

void RestorationTool::writeSettings()
{
    GreycstorationSettings settings = m_settingsWidget->getSettings();
    KConfig* config = kapp->config();
    config->setGroup("restoration Tool");
    config->writeEntry("Preset", m_restorationTypeCB->currentItem());
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
    m_previewWidget->writeSettings();
    config->sync();
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

    switch(m_restorationTypeCB->currentItem())
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

void RestorationTool::processCImgURL(const QString& url)
{
    KApplication::kApplication()->invokeBrowser(url);
}

void RestorationTool::prepareEffect()
{
    m_mainTab->setEnabled(false);

    DImg previewImage = m_previewWidget->getOriginalRegionImage();

    setFilter(dynamic_cast<DImgThreadedFilter*>(new GreycstorationIface(&previewImage,
                                                m_settingsWidget->getSettings(), GreycstorationIface::Restore,
                                                0, 0, 0, this)));
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
                                                0, 0, 0, this)));

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
    KURL loadRestorationFile = KFileDialog::getOpenURL(KGlobalSettings::documentPath(),
                                            QString( "*" ), kapp->activeWindow(),
                                            QString( i18n("Photograph Restoration Settings File to Load")) );
    if( loadRestorationFile.isEmpty() )
       return;

    QFile file(loadRestorationFile.path());

    if ( file.open(IO_ReadOnly) )
    {
        if (!m_settingsWidget->loadSettings(file, QString("# Photograph Restoration Configuration File V2")))
        {
           KMessageBox::error(kapp->activeWindow(),
                        i18n("\"%1\" is not a Photograph Restoration settings text file.")
                        .arg(loadRestorationFile.fileName()));
           file.close();
           return;
        }

        slotEffect();
    }
    else
        KMessageBox::error(kapp->activeWindow(), i18n("Cannot load settings from the Photograph Restoration text file."));

    file.close();
    m_restorationTypeCB->blockSignals(true);
    m_restorationTypeCB->setCurrentItem(NoPreset);
    m_restorationTypeCB->blockSignals(false);
    m_settingsWidget->setEnabled(true);
}

void RestorationTool::slotSaveAsSettings()
{
    KURL saveRestorationFile = KFileDialog::getSaveURL(KGlobalSettings::documentPath(),
                                            QString( "*" ), kapp->activeWindow(),
                                            QString( i18n("Photograph Restoration Settings File to Save")) );
    if( saveRestorationFile.isEmpty() )
       return;

    QFile file(saveRestorationFile.path());

    if ( file.open(IO_WriteOnly) )
        m_settingsWidget->saveSettings(file, QString("# Photograph Restoration Configuration File V2"));
    else
        KMessageBox::error(kapp->activeWindow(), i18n("Cannot save settings to the Photograph Restoration text file."));

    file.close();
}

}  // NameSpace DigikamRestorationImagesPlugin

