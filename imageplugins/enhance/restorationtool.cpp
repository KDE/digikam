/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-03-26
 * Description : a digiKam image editor plugin to restore
 *               a photograph
 *
 * Copyright (C) 2005-2013 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "restorationtool.moc"

// Qt includes

#include <QFile>
#include <QGridLayout>
#include <QImage>
#include <QLabel>
#include <QPixmap>

// KDE includes

#include <kapplication.h>
#include <kcombobox.h>
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
#include "greycstorationfilter.h"
#include "greycstorationsettings.h"
#include "imageiface.h"
#include "imageregionwidget.h"

namespace DigikamEnhanceImagePlugin
{

class RestorationTool::Private
{
public:

    enum RestorationFilteringPreset
    {
        ReduceUniformNoise=0,
        ReduceJPEGArtefacts,
        ReduceTexturing,
        Custom
    };

public:

    Private() :
        mainTab(0),
        restorationTypeCB(0),
        settingsWidget(0),
        previewWidget(0),
        gboxSettings(0)
    {}

    static const QString    configGroupName;
    static const QString    configPresetEntry;
    static const QString    configFastApproxEntry;
    static const QString    configInterpolationEntry;
    static const QString    configAmplitudeEntry;
    static const QString    configSharpnessEntry;
    static const QString    configAnisotropyEntry;
    static const QString    configAlphaEntry;
    static const QString    configSigmaEntry;
    static const QString    configGaussPrecEntry;
    static const QString    configDlEntry;
    static const QString    configDaEntry;
    static const QString    configIterationEntry;
    static const QString    configTileEntry;
    static const QString    configBTileEntry;

    KTabWidget*             mainTab;

    KComboBox*              restorationTypeCB;

    GreycstorationSettings* settingsWidget;
    ImageRegionWidget*      previewWidget;
    EditorToolSettings*     gboxSettings;
};

const QString RestorationTool::Private::configGroupName("restoration Tool");
const QString RestorationTool::Private::configPresetEntry("Preset");
const QString RestorationTool::Private::configFastApproxEntry("FastApprox");
const QString RestorationTool::Private::configInterpolationEntry("Interpolation");
const QString RestorationTool::Private::configAmplitudeEntry("Amplitude");
const QString RestorationTool::Private::configSharpnessEntry("Sharpness");
const QString RestorationTool::Private::configAnisotropyEntry("Anisotropy");
const QString RestorationTool::Private::configAlphaEntry("Alpha");
const QString RestorationTool::Private::configSigmaEntry("Sigma");
const QString RestorationTool::Private::configGaussPrecEntry("GaussPrec");
const QString RestorationTool::Private::configDlEntry("Dl");
const QString RestorationTool::Private::configDaEntry("Da");
const QString RestorationTool::Private::configIterationEntry("Iteration");
const QString RestorationTool::Private::configTileEntry("Tile");
const QString RestorationTool::Private::configBTileEntry("BTile");

// --------------------------------------------------------

RestorationTool::RestorationTool(QObject* const parent)
    : EditorToolThreaded(parent),
      d(new Private)
{
    setObjectName("restoration");
    setToolName(i18n("Restoration"));
    setToolIcon(SmallIcon("restoration"));

    // -------------------------------------------------------------

    d->gboxSettings = new EditorToolSettings;
    d->gboxSettings->setButtons(EditorToolSettings::Default|
                                EditorToolSettings::Ok|
                                EditorToolSettings::Cancel|
                                EditorToolSettings::Load|
                                EditorToolSettings::SaveAs|
                                EditorToolSettings::Try);

    QGridLayout* const gridSettings = new QGridLayout(d->gboxSettings->plainPage());
    d->mainTab                      = new KTabWidget( d->gboxSettings->plainPage() );

    QWidget* const firstPage = new QWidget( d->mainTab );
    QGridLayout* const grid  = new QGridLayout(firstPage);
    d->mainTab->addTab( firstPage, i18n("Preset") );

    KUrlLabel* const cimgLogoLabel = new KUrlLabel(firstPage);
    cimgLogoLabel->setText(QString());
    cimgLogoLabel->setUrl("http://cimg.sourceforge.net");
    cimgLogoLabel->setPixmap(QPixmap(KStandardDirs::locate("data", "digikam/data/logo-cimg.png")));
    cimgLogoLabel->setToolTip( i18n("Visit CImg library website"));

    QLabel* const typeLabel = new QLabel(i18n("Filtering type:"), firstPage);
    typeLabel->setAlignment ( Qt::AlignRight | Qt::AlignVCenter);
    d->restorationTypeCB    = new KComboBox(firstPage);
    d->restorationTypeCB->addItem( i18n("Reduce Uniform Noise") );
    d->restorationTypeCB->addItem( i18n("Reduce JPEG Artifacts") );
    d->restorationTypeCB->addItem( i18n("Reduce Texturing") );
    d->restorationTypeCB->addItem( i18nc("custom restoration settings", "Custom") );
    d->restorationTypeCB->setWhatsThis(i18n("<p>Select the filter preset to use for photograph restoration here:</p>"
                                            "<p><b>Reduce Uniform Noise</b>: reduce small image artifacts such as sensor noise.<br/>"
                                            "<b>Reduce JPEG Artifacts</b>: reduce large image artifacts, such as a JPEG compression mosaic.<br/>"
                                            "<b>Reduce Texturing</b>: reduce image artifacts, such as paper texture, or Moire patterns on scanned images.<br/>"
                                            "<b>Custom</b>: Puts settings to most common values, fully customizable.</p>"));

    grid->addWidget(cimgLogoLabel,        0, 1, 1, 1);
    grid->addWidget(typeLabel,            1, 0, 1, 1);
    grid->addWidget(d->restorationTypeCB, 1, 1, 1, 1);
    grid->setRowStretch(1, 10);
    grid->setMargin(d->gboxSettings->spacingHint());
    grid->setSpacing(0);

    // -------------------------------------------------------------

    d->settingsWidget = new GreycstorationSettings( d->mainTab );
    gridSettings->addWidget(d->mainTab,                               0, 1, 1, 1);
    gridSettings->addWidget(new QLabel(d->gboxSettings->plainPage()), 1, 1, 1, 1);
    gridSettings->setMargin(d->gboxSettings->spacingHint());
    gridSettings->setSpacing(d->gboxSettings->spacingHint());
    gridSettings->setRowStretch(2, 10);

    // -------------------------------------------------------------

    d->previewWidget = new ImageRegionWidget;

    setToolSettings(d->gboxSettings);
    setToolView(d->previewWidget);
    setPreviewModeMask(PreviewToolBar::AllPreviewModes);

    // -------------------------------------------------------------

    connect(cimgLogoLabel, SIGNAL(leftClickedUrl(QString)),
            this, SLOT(processCImgUrl(QString)));

    connect(d->restorationTypeCB, SIGNAL(activated(int)),
            this, SLOT(slotResetValues(int)));

    // -------------------------------------------------------------

    GreycstorationContainer defaults;
    defaults.setRestorationDefaultSettings();
    d->settingsWidget->setDefaultSettings(defaults);
}

RestorationTool::~RestorationTool()
{
    delete d;
}

void RestorationTool::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);

    GreycstorationContainer prm;
    GreycstorationContainer defaults;
    defaults.setRestorationDefaultSettings();

    prm.fastApprox = group.readEntry(d->configFastApproxEntry,    defaults.fastApprox);
    prm.interp     = group.readEntry(d->configInterpolationEntry, defaults.interp);
    prm.amplitude  = group.readEntry(d->configAmplitudeEntry,     (double)defaults.amplitude);
    prm.sharpness  = group.readEntry(d->configSharpnessEntry,     (double)defaults.sharpness);
    prm.anisotropy = group.readEntry(d->configAnisotropyEntry,    (double)defaults.anisotropy);
    prm.alpha      = group.readEntry(d->configAlphaEntry,         (double)defaults.alpha);
    prm.sigma      = group.readEntry(d->configSigmaEntry,         (double)defaults.sigma);
    prm.gaussPrec  = group.readEntry(d->configGaussPrecEntry,     (double)defaults.gaussPrec);
    prm.dl         = group.readEntry(d->configDlEntry,            (double)defaults.dl);
    prm.da         = group.readEntry(d->configDaEntry,            (double)defaults.da);
    prm.nbIter     = group.readEntry(d->configIterationEntry,     defaults.nbIter);
    prm.tile       = group.readEntry(d->configTileEntry,          defaults.tile);
    prm.btile      = group.readEntry(d->configBTileEntry,         defaults.btile);
    d->settingsWidget->setSettings(prm);

    int p = group.readEntry(d->configPresetEntry, (int)Private::ReduceUniformNoise);
    d->restorationTypeCB->setCurrentIndex(p);

    if (p == Private::Custom)
    {
        d->settingsWidget->setEnabled(true);
    }
    else
    {
        d->settingsWidget->setEnabled(false);
    }
}

void RestorationTool::writeSettings()
{
    GreycstorationContainer prm = d->settingsWidget->settings();
    KSharedConfig::Ptr config   = KGlobal::config();
    KConfigGroup group          = config->group(d->configGroupName);

    group.writeEntry(d->configPresetEntry,        d->restorationTypeCB->currentIndex());
    group.writeEntry(d->configFastApproxEntry,    prm.fastApprox);
    group.writeEntry(d->configInterpolationEntry, prm.interp);
    group.writeEntry(d->configAmplitudeEntry,     (double)prm.amplitude);
    group.writeEntry(d->configSharpnessEntry,     (double)prm.sharpness);
    group.writeEntry(d->configAnisotropyEntry,    (double)prm.anisotropy);
    group.writeEntry(d->configAlphaEntry,         (double)prm.alpha);
    group.writeEntry(d->configSigmaEntry,         (double)prm.sigma);
    group.writeEntry(d->configGaussPrecEntry,     (double)prm.gaussPrec);
    group.writeEntry(d->configDlEntry,            (double)prm.dl);
    group.writeEntry(d->configDaEntry,            (double)prm.da);
    group.writeEntry(d->configIterationEntry,     prm.nbIter);
    group.writeEntry(d->configTileEntry,          prm.tile);
    group.writeEntry(d->configBTileEntry,         prm.btile);
    group.sync();
}

void RestorationTool::slotResetValues(int i)
{
    if (i == Private::Custom)
    {
        d->settingsWidget->setEnabled(true);
    }
    else
    {
        d->settingsWidget->setEnabled(false);
    }

    slotResetSettings();
}

void RestorationTool::slotResetSettings()
{
    GreycstorationContainer settings;
    settings.setRestorationDefaultSettings();

    switch (d->restorationTypeCB->currentIndex())
    {
        case Private::ReduceUniformNoise:
        {
            settings.amplitude = 40.0;
            break;
        }

        case Private::ReduceJPEGArtefacts:
        {
            settings.sharpness = 0.3F;
            settings.sigma     = 1.0;
            settings.amplitude = 100.0;
            settings.nbIter    = 2;
            break;
        }

        case Private::ReduceTexturing:
        {
            settings.sharpness = 0.5F;
            settings.sigma     = 1.5;
            settings.amplitude = 100.0;
            settings.nbIter    = 2;
            break;
        }
        
        default: // Custom
            break;
    }

    d->settingsWidget->setSettings(settings);
}

void RestorationTool::processCImgUrl(const QString& url)
{
    KToolInvocation::invokeBrowser(url);
}

void RestorationTool::preparePreview()
{
    DImg previewImage = d->previewWidget->getOriginalRegionImage();

    setFilter(new GreycstorationFilter(&previewImage,
                                       d->settingsWidget->settings(), GreycstorationFilter::Restore,
                                       0, 0, QImage(), this));
}

void RestorationTool::prepareFinal()
{
    ImageIface iface;
    DImg originalImage = iface.original()->copy();

    setFilter(new GreycstorationFilter(&originalImage,
                                       d->settingsWidget->settings(), GreycstorationFilter::Restore,
                                       0, 0, QImage(), this));
}

void RestorationTool::setPreviewImage()
{
    DImg imDest = filter()->getTargetImage();
    d->previewWidget->setPreviewImage(imDest);
}

void RestorationTool::setFinalImage()
{
    ImageIface iface;
    iface.setOriginal(i18n("Restoration"), filter()->filterAction(), filter()->getTargetImage());
}

void RestorationTool::slotLoadSettings()
{
    KUrl loadRestorationFile = KFileDialog::getOpenUrl(KGlobalSettings::documentPath(),
                               QString( "*" ), kapp->activeWindow(),
                               QString( i18n("Photograph Restoration Settings File to Load")) );

    if ( loadRestorationFile.isEmpty() )
    {
        return;
    }

    QFile file(loadRestorationFile.toLocalFile());

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

        slotPreview();
    }
    else
    {
        KMessageBox::error(kapp->activeWindow(), i18n("Cannot load settings from the Photograph Restoration text file."));
    }

    file.close();
    d->restorationTypeCB->blockSignals(true);
    d->restorationTypeCB->setCurrentIndex((int)Private::Custom);
    d->restorationTypeCB->blockSignals(false);
    d->settingsWidget->setEnabled(true);
}

void RestorationTool::slotSaveAsSettings()
{
    KUrl saveRestorationFile = KFileDialog::getSaveUrl(KGlobalSettings::documentPath(),
                               QString( "*" ), kapp->activeWindow(),
                               QString( i18n("Photograph Restoration Settings File to Save")) );

    if ( saveRestorationFile.isEmpty() )
    {
        return;
    }

    QFile file(saveRestorationFile.toLocalFile());

    if ( file.open(QIODevice::WriteOnly) )
    {
        d->settingsWidget->saveSettings(file, QString("# Photograph Restoration Configuration File V2"));
    }
    else
    {
        KMessageBox::error(kapp->activeWindow(), i18n("Cannot save settings to the Photograph Restoration text file."));
    }

    file.close();
}

}  // namespace DigikamEnhanceImagePlugin
