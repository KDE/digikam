/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-03-26
 * Description : a digiKam image editor plugin to restore
 *               a photograph
 *
 * Copyright (C) 2005-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes

#include <QFile>
#include <QGridLayout>
#include <QImage>
#include <QLabel>
#include <QPixmap>

// KDE includes

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

// Local includes

#include "editortoolsettings.h"
#include "greycstorationiface.h"
#include "greycstorationsettings.h"
#include "greycstorationwidget.h"
#include "imageiface.h"
#include "imagepanelwidget.h"
#include "version.h"

using namespace Digikam;

namespace DigikamRestorationImagesPlugin
{

class RestorationToolPriv
{
public:

    RestorationToolPriv()
    {
        mainTab           = 0;
        restorationTypeCB = 0;
        settingsWidget    = 0;
        previewWidget     = 0;
        gboxSettings      = 0;
    }


    KTabWidget*           mainTab;

    KComboBox*            restorationTypeCB;

    GreycstorationWidget* settingsWidget;
    ImagePanelWidget*     previewWidget;
    EditorToolSettings*   gboxSettings;
};

RestorationTool::RestorationTool(QObject* parent)
               : EditorToolThreaded(parent),
                 d(new RestorationToolPriv)
{
    setObjectName("restoration");
    setToolName(i18n("Restoration"));
    setToolIcon(SmallIcon("restoration"));

    // -------------------------------------------------------------

    d->gboxSettings = new EditorToolSettings(EditorToolSettings::Default|
                                             EditorToolSettings::Ok|
                                             EditorToolSettings::Cancel|
                                             EditorToolSettings::Load|
                                             EditorToolSettings::SaveAs|
                                             EditorToolSettings::Try,
                                             EditorToolSettings::PanIcon);

    QGridLayout* gridSettings = new QGridLayout(d->gboxSettings->plainPage());
    d->mainTab = new KTabWidget( d->gboxSettings->plainPage() );

    QWidget* firstPage = new QWidget( d->mainTab );
    QGridLayout* grid  = new QGridLayout(firstPage);
    d->mainTab->addTab( firstPage, i18n("Preset") );

    KUrlLabel *cimgLogoLabel = new KUrlLabel(firstPage);
    cimgLogoLabel->setText(QString());
    cimgLogoLabel->setUrl("http://cimg.sourceforge.net");
    cimgLogoLabel->setPixmap(QPixmap(KStandardDirs::locate("data", "digikam/data/logo-cimg.png")));
    cimgLogoLabel->setToolTip( i18n("Visit CImg library website"));

    QLabel *typeLabel   = new QLabel(i18n("Filtering type:"), firstPage);
    typeLabel->setAlignment ( Qt::AlignRight | Qt::AlignVCenter);
    d->restorationTypeCB = new KComboBox(firstPage);
    d->restorationTypeCB->addItem( i18nc("no restoration preset", "None") );
    d->restorationTypeCB->addItem( i18n("Reduce Uniform Noise") );
    d->restorationTypeCB->addItem( i18n("Reduce JPEG Artifacts") );
    d->restorationTypeCB->addItem( i18n("Reduce Texturing") );
    d->restorationTypeCB->setWhatsThis(i18n("<p>Select the filter preset to use for photograph restoration here:</p>"
                                            "<p><b>None</b>: Most common values. Puts settings to default.<br/>"
                                            "<b>Reduce Uniform Noise</b>: reduce small image artifacts such as sensor noise.<br/>"
                                            "<b>Reduce JPEG Artifacts</b>: reduce large image artifacts, such as a JPEG compression mosaic.<br/>"
                                            "<b>Reduce Texturing</b>: reduce image artifacts, such as paper texture, or Moire patterns "
                                            "on scanned images.</p>"));

    grid->addWidget(cimgLogoLabel,        0, 1, 1, 1);
    grid->addWidget(typeLabel,            1, 0, 1, 1);
    grid->addWidget(d->restorationTypeCB, 1, 1, 1, 1);
    grid->setRowStretch(1, 10);
    grid->setMargin(d->gboxSettings->spacingHint());
    grid->setSpacing(0);

    // -------------------------------------------------------------

    d->settingsWidget = new GreycstorationWidget( d->mainTab );
    gridSettings->addWidget(d->mainTab,                               0, 1, 1, 1);
    gridSettings->addWidget(new QLabel(d->gboxSettings->plainPage()), 1, 1, 1, 1);
    gridSettings->setMargin(d->gboxSettings->spacingHint());
    gridSettings->setSpacing(d->gboxSettings->spacingHint());
    gridSettings->setRowStretch(2, 10);

    setToolSettings(d->gboxSettings);

    // -------------------------------------------------------------

    d->previewWidget = new ImagePanelWidget(470, 350, "restoration Tool", d->gboxSettings->panIconView());

    setToolView(d->previewWidget);
    init();

    // -------------------------------------------------------------

    connect(cimgLogoLabel, SIGNAL(leftClickedUrl(const QString&)),
            this, SLOT(processCImgUrl(const QString&)));

    connect(d->restorationTypeCB, SIGNAL(activated(int)),
            this, SLOT(slotResetValues(int)));

    // -------------------------------------------------------------

    GreycstorationSettings defaults;
    defaults.setRestorationDefaultSettings();
    d->settingsWidget->setDefaultSettings(defaults);
}

RestorationTool::~RestorationTool()
{
    delete d;
}

void RestorationTool::renderingFinished()
{
    d->previewWidget->setEnable(true);
    d->mainTab->setEnabled(true);
}

void RestorationTool::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("restoration Tool");

    GreycstorationSettings settings;
    GreycstorationSettings defaults;
    defaults.setRestorationDefaultSettings();

    settings.fastApprox = group.readEntry("FastApprox",    defaults.fastApprox);
    settings.interp     = group.readEntry("Interpolation", defaults.interp);
    settings.amplitude  = group.readEntry("Amplitude",     (double)defaults.amplitude);
    settings.sharpness  = group.readEntry("Sharpness",     (double)defaults.sharpness);
    settings.anisotropy = group.readEntry("Anisotropy",    (double)defaults.anisotropy);
    settings.alpha      = group.readEntry("Alpha",         (double)defaults.alpha);
    settings.sigma      = group.readEntry("Sigma",         (double)defaults.sigma);
    settings.gaussPrec  = group.readEntry("GaussPrec",     (double)defaults.gaussPrec);
    settings.dl         = group.readEntry("Dl",            (double)defaults.dl);
    settings.da         = group.readEntry("Da",            (double)defaults.da);
    settings.nbIter     = group.readEntry("Iteration",     defaults.nbIter);
    settings.tile       = group.readEntry("Tile",          defaults.tile);
    settings.btile      = group.readEntry("BTile",         defaults.btile);
    d->settingsWidget->setSettings(settings);

    int p = group.readEntry("Preset", (int)NoPreset);
    d->restorationTypeCB->setCurrentIndex(p);
    if (p == NoPreset)
        d->settingsWidget->setEnabled(true);
    else
        d->settingsWidget->setEnabled(false);
}

void RestorationTool::writeSettings()
{
    GreycstorationSettings settings = d->settingsWidget->getSettings();
    KSharedConfig::Ptr config       = KGlobal::config();
    KConfigGroup group              = config->group("restoration Tool");
    group.writeEntry("Preset",        d->restorationTypeCB->currentIndex());
    group.writeEntry("FastApprox",    settings.fastApprox);
    group.writeEntry("Interpolation", settings.interp);
    group.writeEntry("Amplitude",     (double)settings.amplitude);
    group.writeEntry("Sharpness",     (double)settings.sharpness);
    group.writeEntry("Anisotropy",    (double)settings.anisotropy);
    group.writeEntry("Alpha",         (double)settings.alpha);
    group.writeEntry("Sigma",         (double)settings.sigma);
    group.writeEntry("GaussPrec",     (double)settings.gaussPrec);
    group.writeEntry("Dl",            (double)settings.dl);
    group.writeEntry("Da",            (double)settings.da);
    group.writeEntry("Iteration",     settings.nbIter);
    group.writeEntry("Tile",          settings.tile);
    group.writeEntry("BTile",         settings.btile);
    d->previewWidget->writeSettings();
    group.sync();
}

void RestorationTool::slotResetValues(int i)
{
    if (i == NoPreset)
        d->settingsWidget->setEnabled(true);
    else
        d->settingsWidget->setEnabled(false);

    slotResetSettings();
}

void RestorationTool::slotResetSettings()
{
    GreycstorationSettings settings;
    settings.setRestorationDefaultSettings();

    switch(d->restorationTypeCB->currentIndex())
    {
        case ReduceUniformNoise:
        {
            settings.amplitude = 40.0;
            break;
        }

        case ReduceJPEGArtefacts:
        {
            settings.sharpness = 0.3F;
            settings.sigma     = 1.0;
            settings.amplitude = 100.0;
            settings.nbIter    = 2;
            break;
        }

        case ReduceTexturing:
        {
            settings.sharpness = 0.5F;
            settings.sigma     = 1.5;
            settings.amplitude = 100.0;
            settings.nbIter    = 2;
            break;
        }
    }

    d->settingsWidget->setSettings(settings);
}

void RestorationTool::processCImgUrl(const QString& url)
{
    KToolInvocation::invokeBrowser(url);
}

void RestorationTool::prepareEffect()
{
    d->mainTab->setEnabled(false);

    DImg previewImage = d->previewWidget->getOriginalRegionImage();

    setFilter(dynamic_cast<DImgThreadedFilter*>(new GreycstorationIface(&previewImage,
                                                d->settingsWidget->getSettings(), GreycstorationIface::Restore,
                                                0, 0, QImage(), this)));
}

void RestorationTool::prepareFinal()
{
    d->mainTab->setEnabled(false);

    ImageIface iface(0, 0);
    uchar *data = iface.getOriginalImage();
    DImg originalImage(iface.originalWidth(), iface.originalHeight(),
                       iface.originalSixteenBit(), iface.originalHasAlpha(), data);

    setFilter(dynamic_cast<DImgThreadedFilter*>(new GreycstorationIface(&originalImage,
                                                d->settingsWidget->getSettings(), GreycstorationIface::Restore,
                                                0, 0, QImage(), this)));

    delete [] data;
}

void RestorationTool::putPreviewData()
{
    DImg imDest = filter()->getTargetImage();
    d->previewWidget->setPreviewImage(imDest);
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
    if ( loadRestorationFile.isEmpty() )
       return;

    QFile file(loadRestorationFile.path());

    if ( file.open(QIODevice::ReadOnly) )
    {
        if (!d->settingsWidget->loadSettings(file, QString("# Photograph Restoration Configuration File V2")))
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
    d->restorationTypeCB->blockSignals(true);
    d->restorationTypeCB->setCurrentIndex((int)NoPreset);
    d->restorationTypeCB->blockSignals(false);
    d->settingsWidget->setEnabled(true);
}

void RestorationTool::slotSaveAsSettings()
{
    KUrl saveRestorationFile = KFileDialog::getSaveUrl(KGlobalSettings::documentPath(),
                                            QString( "*" ), kapp->activeWindow(),
                                            QString( i18n("Photograph Restoration Settings File to Save")) );
    if ( saveRestorationFile.isEmpty() )
       return;

    QFile file(saveRestorationFile.path());

    if ( file.open(QIODevice::WriteOnly) )
        d->settingsWidget->saveSettings(file, QString("# Photograph Restoration Configuration File V2"));
    else
        KMessageBox::error(kapp->activeWindow(), i18n("Cannot save settings to the Photograph Restoration text file."));

    file.close();
}

}  // namespace DigikamRestorationImagesPlugin
