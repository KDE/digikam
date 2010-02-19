/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-02-26
 * Description : image channels mixer.
 *
 * Copyright (C) 2005-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "channelmixertool.moc"

// C++ includes

#include <cstdio>
#include <cmath>
#include <cstring>
#include <cstdlib>
#include <cerrno>

// Qt includes

#include <QCheckBox>
#include <QColor>
#include <QFile>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPixmap>
#include <QPushButton>
#include <QSpinBox>
#include <QTimer>
#include <QToolButton>

// KDE includes

#include <kaboutdata.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kcursor.h>
#include <kfiledialog.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <khelpmenu.h>
#include <kicon.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmenu.h>
#include <kmessagebox.h>
#include <kselector.h>
#include <kstandarddirs.h>
#include <kvbox.h>

// LibKDcraw includes

#include <libkdcraw/rnuminput.h>

// Local includes

#include "colorgradientwidget.h"
#include "daboutdata.h"
#include "dimg.h"
#include "mixerfilter.h"
#include "editortoolsettings.h"
#include "histogrambox.h"
#include "histogramwidget.h"
#include "imagehistogram.h"
#include "imageiface.h"
#include "imageguidewidget.h"
#include "version.h"

using namespace KDcrawIface;

namespace DigikamChannelMixerImagesPlugin
{

class ChannelMixerToolPriv
{
public:

    ChannelMixerToolPriv() :
        configGroupName("channelmixer Tool"),
        configMonochromeEntry("Monochrome"),
        configPreserveLuminosityEntry("PreserveLuminosity"),
        configRedRedGainEntry("RedRedGain"),
        configRedGreenGainEntry("RedGreenGain"),
        configRedBlueGainEntry("RedBlueGain"),
        configGreenRedGainEntry("GreenRedGain"),
        configGreenGreenGainEntry("GreenGreenGain"),
        configGreenBlueGainEntry("GreenBlueGain"),
        configBlueRedGainEntry("BlueRedGain"),
        configBlueGreenGainEntry("BlueGreenGain"),
        configBlueBlueGainEntry("BlueBlueGain"),
        configBlackRedGainEntry("BlackRedGain"),
        configBlackGreenGainEntry("BlackGreenGain"),
        configBlackBlueGainEntry("BlackBlueGain"),
        configHistogramChannelEntry("Histogram Channel"),
        configHistogramScaleEntry("Histogram Scale"),

        destinationPreviewData(0),
        resetButton(0),
        preserveLuminosity(0),
        monochrome(0),
        redGain(0),
        greenGain(0),
        blueGain(0),
        previewWidget(0),
        gboxSettings(0)
        {}

    const QString       configGroupName;
    const QString       configMonochromeEntry;
    const QString       configPreserveLuminosityEntry;
    const QString       configRedRedGainEntry;
    const QString       configRedGreenGainEntry;
    const QString       configRedBlueGainEntry;
    const QString       configGreenRedGainEntry;
    const QString       configGreenGreenGainEntry;
    const QString       configGreenBlueGainEntry;
    const QString       configBlueRedGainEntry;
    const QString       configBlueGreenGainEntry;
    const QString       configBlueBlueGainEntry;
    const QString       configBlackRedGainEntry;
    const QString       configBlackGreenGainEntry;
    const QString       configBlackBlueGainEntry;
    const QString       configHistogramChannelEntry;
    const QString       configHistogramScaleEntry;

    uchar*              destinationPreviewData;
  
    MixerContainer      mixerSettings;
    
    QPushButton*        resetButton;

    QCheckBox*          preserveLuminosity;
    QCheckBox*          monochrome;

    RDoubleNumInput*    redGain;
    RDoubleNumInput*    greenGain;
    RDoubleNumInput*    blueGain;

    ImageGuideWidget*   previewWidget;
    EditorToolSettings* gboxSettings;
};

ChannelMixerTool::ChannelMixerTool(QObject* parent)
                : EditorTool(parent),
                  d(new ChannelMixerToolPriv)
{
    setObjectName("channelmixer");
    setToolName(i18n("Channel Mixer"));
    setToolIcon(SmallIcon("channelmixer"));

    // -------------------------------------------------------------

    d->previewWidget = new ImageGuideWidget;
    d->previewWidget->setWhatsThis(i18n("You can see here the image's color channels' "
                                        "gain adjustments preview. You can pick a color on the image "
                                        "to see the corresponding color level on the histogram."));
    setToolView(d->previewWidget);
    setPreviewModeMask(PreviewToolBar::AllPreviewModes);
    
    // -------------------------------------------------------------

    d->gboxSettings = new EditorToolSettings;
    d->gboxSettings->setButtons(EditorToolSettings::Default|
                                EditorToolSettings::Load|
                                EditorToolSettings::SaveAs|
                                EditorToolSettings::Ok|
                                EditorToolSettings::Cancel);

    d->gboxSettings->setTools(EditorToolSettings::Histogram);
    d->gboxSettings->setHistogramType(Digikam::RGB);

    QGridLayout* grid = new QGridLayout(d->gboxSettings->plainPage());

    // -------------------------------------------------------------

    QLabel *redLabel  = new QLabel(i18n("Red:"), d->gboxSettings->plainPage());
    d->redGain        = new RDoubleNumInput(d->gboxSettings->plainPage());
    d->redGain->setDecimals(0);
    d->redGain->setRange(-200.0, 200.0, 1);
    d->redGain->setDefaultValue(0);
    d->redGain->setWhatsThis(i18n("Select the red color gain, as a percentage, "
                                 "for the current channel."));

    QLabel *greenLabel = new QLabel(i18n("Green:"), d->gboxSettings->plainPage());
    d->greenGain       = new RDoubleNumInput(d->gboxSettings->plainPage());
    d->greenGain->setDecimals(0);
    d->greenGain->setRange(-200.0, 200.0, 1);
    d->greenGain->setDefaultValue(0);
    d->greenGain->setWhatsThis(i18n("Select the green color gain, as a percentage, "
                                    "for the current channel."));

    QLabel *blueLabel = new QLabel(i18n("Blue:"), d->gboxSettings->plainPage());
    d->blueGain       = new RDoubleNumInput(d->gboxSettings->plainPage());
    d->blueGain->setDecimals(0);
    d->blueGain->setRange(-200.0, 200.0, 1);
    d->blueGain->setDefaultValue(0);
    d->blueGain->setWhatsThis(i18n("Select the blue color gain, as a percentage, "
                                   "for the current channel."));

    d->resetButton = new QPushButton(i18n("&Reset"), d->gboxSettings->plainPage());
    d->resetButton->setIcon(KIconLoader::global()->loadIcon("document-revert", KIconLoader::Toolbar));
    d->resetButton->setWhatsThis(i18n("Reset color channels' gains settings from "
                                      "the currently selected channel."));

    // -------------------------------------------------------------

    d->monochrome = new QCheckBox(i18n("Monochrome"), d->gboxSettings->plainPage());
    d->monochrome->setWhatsThis(i18n("Enable this option if you want the image rendered "
                                     "in monochrome mode. "
                                     "In this mode, the histogram will display only luminosity values."));

    d->preserveLuminosity = new QCheckBox(i18n("Preserve luminosity"), d->gboxSettings->plainPage());
    d->preserveLuminosity->setWhatsThis(i18n("Enable this option is you want preserve "
                                             "the image luminosity."));

    // -------------------------------------------------------------

    grid->addWidget(redLabel,              0, 0, 1, 1);
    grid->addWidget(d->redGain,            0, 1, 1, 4);
    grid->addWidget(greenLabel,            1, 0, 1, 1);
    grid->addWidget(d->greenGain,          1, 1, 1, 4);
    grid->addWidget(blueLabel,             2, 0, 1, 1);
    grid->addWidget(d->blueGain,           2, 1, 1, 4);
    grid->addWidget(d->resetButton,        3, 0, 1, 2);
    grid->addWidget(d->monochrome,         4, 0, 1, 5);
    grid->addWidget(d->preserveLuminosity, 5, 0, 1, 5);
    grid->setRowStretch(6, 10);
    grid->setMargin(d->gboxSettings->spacingHint());
    grid->setSpacing(d->gboxSettings->spacingHint());

    setToolSettings(d->gboxSettings);
    init();

    // -------------------------------------------------------------
    // Channels and scale selection slots.

    connect(d->previewWidget, SIGNAL(spotPositionChangedFromTarget(const Digikam::DColor&, const QPoint&)),
            this, SLOT(slotColorSelectedFromTarget(const Digikam::DColor&)));

    connect(d->previewWidget, SIGNAL(signalResized()),
            this, SLOT(slotEffect()));

    // -------------------------------------------------------------
    // Gains settings slots.

    connect(d->redGain, SIGNAL(valueChanged(double)),
            this, SLOT(slotGainsChanged()));

    connect(d->greenGain, SIGNAL(valueChanged(double)),
            this, SLOT(slotGainsChanged()));

    connect(d->blueGain, SIGNAL(valueChanged(double)),
            this, SLOT(slotGainsChanged()));

    connect(d->preserveLuminosity, SIGNAL(toggled (bool)),
            this, SLOT(slotEffect()));

    connect(d->monochrome, SIGNAL(toggled (bool)),
            this, SLOT(slotMonochromeActived(bool)));

    // -------------------------------------------------------------
    // Buttons slots.

    connect(d->resetButton, SIGNAL(clicked()),
            this, SLOT(slotResetCurrentChannel()));
}

ChannelMixerTool::~ChannelMixerTool()
{
    if (d->destinationPreviewData)
       delete [] d->destinationPreviewData;

    delete d;
}

void ChannelMixerTool::slotResetCurrentChannel()
{
    switch (d->gboxSettings->histogramBox()->channel())
    {
        case GreenChannel:
        {
            d->mixerSettings.greenRedGain     = 0.0;
            d->mixerSettings.greenGreenGain   = 1.0;
            d->mixerSettings.greenBlueGain    = 0.0;
            break;
        }
        
        case BlueChannel:
        {
            d->mixerSettings.blueRedGain      = 0.0;
            d->mixerSettings.blueGreenGain    = 0.0;
            d->mixerSettings.blueBlueGain     = 1.0;
            break;
        }
            
        default:                        // Red or monochrome.
        {
            if (d->monochrome->isChecked())
            {
                d->mixerSettings.blackRedGain   = 1.0;
                d->mixerSettings.blackGreenGain = 0.0;
                d->mixerSettings.blackBlueGain  = 0.0;
            }
            else
            {
                d->mixerSettings.redRedGain   = 1.0;
                d->mixerSettings.redGreenGain = 0.0;
                d->mixerSettings.redBlueGain  = 0.0;
            }
            break;
        }
    }

    adjustSliders();
    slotEffect();
    d->gboxSettings->histogramBox()->histogram()->reset();
}

void ChannelMixerTool::slotColorSelectedFromTarget( const DColor& color )
{
    d->gboxSettings->histogramBox()->histogram()->setHistogramGuideByColor(color);
}

void ChannelMixerTool::slotGainsChanged()
{
    switch(d->gboxSettings->histogramBox()->channel())
    {
        case GreenChannel:
        {
            d->mixerSettings.greenRedGain   = d->redGain->value()   / 100.0;
            d->mixerSettings.greenGreenGain = d->greenGain->value() / 100.0;
            d->mixerSettings.greenBlueGain  = d->blueGain->value()  / 100.0;
            break;
        }

        case BlueChannel:
        {
            d->mixerSettings.blueRedGain   = d->redGain->value()   / 100.0;
            d->mixerSettings.blueGreenGain = d->greenGain->value() / 100.0;
            d->mixerSettings.blueBlueGain  = d->blueGain->value()  / 100.0;
            break;
        }

        default:                         // Red or monochrome.
        {
            if ( d->monochrome->isChecked() )
            {
              d->mixerSettings.blackRedGain   = d->redGain->value()   / 100.0;
              d->mixerSettings.blackGreenGain = d->greenGain->value() / 100.0;
              d->mixerSettings.blackBlueGain  = d->blueGain->value()  / 100.0;
            }
            else
            {
              d->mixerSettings.redRedGain   = d->redGain->value()   / 100.0;
              d->mixerSettings.redGreenGain = d->greenGain->value() / 100.0;
              d->mixerSettings.redBlueGain  = d->blueGain->value()  / 100.0;
            }
            break;
        }
    }

    slotTimer();
}

void ChannelMixerTool::adjustSliders()
{
    d->redGain->blockSignals(true);
    d->greenGain->blockSignals(true);
    d->blueGain->blockSignals(true);

    switch(d->gboxSettings->histogramBox()->channel())
    {
        case GreenChannel:
        {
            d->redGain->setDefaultValue(0);
            d->greenGain->setDefaultValue(100);
            d->blueGain->setDefaultValue(0);
            d->redGain->setValue(d->mixerSettings.greenRedGain     * 100.0);
            d->greenGain->setValue(d->mixerSettings.greenGreenGain * 100.0);
            d->blueGain->setValue(d->mixerSettings.greenBlueGain   * 100.0);
            break;
        }

        case BlueChannel:
        {
            d->redGain->setDefaultValue(0);
            d->greenGain->setDefaultValue(0);
            d->blueGain->setDefaultValue(100);
            d->redGain->setValue(d->mixerSettings.blueRedGain     * 100.0);
            d->greenGain->setValue(d->mixerSettings.blueGreenGain * 100.0);
            d->blueGain->setValue(d->mixerSettings.blueBlueGain   * 100.0);
            break;
        }

       default:          // Red or monochrome.
       {
            if ( d->monochrome->isChecked() )
            {
              d->redGain->setDefaultValue(100);
              d->greenGain->setDefaultValue(0);
              d->blueGain->setDefaultValue(0);
              d->redGain->setValue(d->mixerSettings.blackRedGain     * 100.0);
              d->greenGain->setValue(d->mixerSettings.blackGreenGain * 100.0);
              d->blueGain->setValue(d->mixerSettings.blackBlueGain   * 100.0);
            }
            else
            {
              d->redGain->setDefaultValue(100);
              d->greenGain->setDefaultValue(0);
              d->blueGain->setDefaultValue(0);
              d->redGain->setValue(d->mixerSettings.redRedGain     * 100.0);
              d->greenGain->setValue(d->mixerSettings.redGreenGain * 100.0);
              d->blueGain->setValue(d->mixerSettings.redBlueGain   * 100.0);
            }
            break;
        }
    }

    d->redGain->blockSignals(false);
    d->greenGain->blockSignals(false);
    d->blueGain->blockSignals(false);
}

void ChannelMixerTool::slotMonochromeActived(bool mono)
{
    d->gboxSettings->histogramBox()->setChannelEnabled(!mono);
    d->gboxSettings->histogramBox()->setChannel(RedChannel);
}

void ChannelMixerTool::slotEffect()
{
    ImageIface* iface = d->previewWidget->imageIface();
    uchar *data       = iface->getPreviewImage();
    int w             = iface->previewWidth();
    int h             = iface->previewHeight();
    bool sb           = iface->previewSixteenBit();

    // Create the new empty destination image data space.
    d->gboxSettings->histogramBox()->histogram()->stopHistogramComputation();

    if (d->destinationPreviewData)
       delete [] d->destinationPreviewData;

    d->destinationPreviewData     = new uchar[w*h*(sb ? 8 : 4)];
    d->mixerSettings.bPreserveLum = d->preserveLuminosity->isChecked();
    d->mixerSettings.bMonochrome  = d->monochrome->isChecked();
    MixerFilter mixer(data, w, h, sb, d->mixerSettings);

    iface->putPreviewImage(data);
    d->previewWidget->updatePreview();

    // Update histogram.
    memcpy (d->destinationPreviewData, data, w*h*(sb ? 8 : 4));
    d->gboxSettings->histogramBox()->histogram()->updateData(d->destinationPreviewData, w, h, sb, 0, 0, 0, false);
    delete [] data;
}

void ChannelMixerTool::finalRendering()
{
    kapp->setOverrideCursor( Qt::WaitCursor );
    ImageIface* iface             = d->previewWidget->imageIface();
    uchar *data                   = iface->getOriginalImage();
    int w                         = iface->originalWidth();
    int h                         = iface->originalHeight();
    bool sb                       = iface->originalSixteenBit();
    d->mixerSettings.bPreserveLum = d->preserveLuminosity->isChecked();
    d->mixerSettings.bMonochrome  = d->monochrome->isChecked();
    MixerFilter mixer(data, w, h, sb, d->mixerSettings);

    iface->putOriginalImage(i18n("Channel Mixer"), data);
    delete [] data;
    kapp->restoreOverrideCursor();
}

void ChannelMixerTool::slotChannelChanged()
{
    if (d->monochrome->isChecked())
        d->gboxSettings->histogramBox()->setGradientColors(QColor("black"), QColor("white"));

    adjustSliders();
    slotEffect();
}

void ChannelMixerTool::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);

    d->monochrome->setChecked(group.readEntry(d->configMonochromeEntry,                 false));
    d->preserveLuminosity->setChecked(group.readEntry(d->configPreserveLuminosityEntry, false));

    d->mixerSettings.redRedGain     = group.readEntry(d->configRedRedGainEntry,   1.0);
    d->mixerSettings.redGreenGain   = group.readEntry(d->configRedGreenGainEntry, 0.0);
    d->mixerSettings.redBlueGain    = group.readEntry(d->configRedBlueGainEntry,  0.0);

    d->mixerSettings.greenRedGain   = group.readEntry(d->configGreenRedGainEntry,   0.0);
    d->mixerSettings.greenGreenGain = group.readEntry(d->configGreenGreenGainEntry, 1.0);
    d->mixerSettings.greenBlueGain  = group.readEntry(d->configGreenBlueGainEntry,  0.0);

    d->mixerSettings.blueRedGain    = group.readEntry(d->configBlueRedGainEntry,   0.0);
    d->mixerSettings.blueGreenGain  = group.readEntry(d->configBlueGreenGainEntry, 0.0);
    d->mixerSettings.blueBlueGain   = group.readEntry(d->configBlueBlueGainEntry,  1.0);

    d->mixerSettings.blackRedGain   = group.readEntry(d->configBlackRedGainEntry,   1.0);
    d->mixerSettings.blackGreenGain = group.readEntry(d->configBlackGreenGainEntry, 0.0);
    d->mixerSettings.blackBlueGain  = group.readEntry(d->configBlackBlueGainEntry,  0.0);

    adjustSliders();

    // we need to call the set methods here, otherwise the histogram will not be updated correctly
    d->gboxSettings->histogramBox()->setChannel((ChannelType)group.readEntry(d->configHistogramChannelEntry,
                                                (int)LuminosityChannel));
    d->gboxSettings->histogramBox()->setScale((HistogramScale)group.readEntry(d->configHistogramScaleEntry,
                                              (int)LogScaleHistogram));

    d->gboxSettings->histogramBox()->histogram()->reset();

    slotEffect();
}

void ChannelMixerTool::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);

    group.writeEntry(d->configHistogramChannelEntry, (int)d->gboxSettings->histogramBox()->channel());
    group.writeEntry(d->configHistogramScaleEntry,   (int)d->gboxSettings->histogramBox()->scale());

    group.writeEntry(d->configMonochromeEntry,         d->monochrome->isChecked());
    group.writeEntry(d->configPreserveLuminosityEntry, d->preserveLuminosity->isChecked());

    group.writeEntry(d->configRedRedGainEntry,     d->mixerSettings.redRedGain);
    group.writeEntry(d->configRedGreenGainEntry,   d->mixerSettings.redGreenGain);
    group.writeEntry(d->configRedBlueGainEntry,    d->mixerSettings.redBlueGain);

    group.writeEntry(d->configGreenRedGainEntry,   d->mixerSettings.greenRedGain);
    group.writeEntry(d->configGreenGreenGainEntry, d->mixerSettings.greenGreenGain);
    group.writeEntry(d->configGreenBlueGainEntry,  d->mixerSettings.greenBlueGain);

    group.writeEntry(d->configBlueRedGainEntry,    d->mixerSettings.blueRedGain);
    group.writeEntry(d->configBlueGreenGainEntry,  d->mixerSettings.blueGreenGain);
    group.writeEntry(d->configBlueBlueGainEntry,   d->mixerSettings.blueBlueGain);

    group.writeEntry(d->configBlackRedGainEntry,   d->mixerSettings.blackRedGain);
    group.writeEntry(d->configBlackGreenGainEntry, d->mixerSettings.blackGreenGain);
    group.writeEntry(d->configBlackBlueGainEntry,  d->mixerSettings.blackBlueGain);

    config->sync();
}

void ChannelMixerTool::slotResetSettings()
{
    d->monochrome->blockSignals(true);
    d->preserveLuminosity->blockSignals(true);

    d->mixerSettings.redRedGain     = 1.0;
    d->mixerSettings.redGreenGain   = 0.0;
    d->mixerSettings.redBlueGain    = 0.0;

    d->mixerSettings.greenRedGain   = 0.0;
    d->mixerSettings.greenGreenGain = 1.0;
    d->mixerSettings.greenBlueGain  = 0.0;

    d->mixerSettings.blueRedGain    = 0.0;
    d->mixerSettings.blueGreenGain  = 0.0;
    d->mixerSettings.blueBlueGain   = 1.0;

    d->mixerSettings.blackRedGain   = 1.0;
    d->mixerSettings.blackGreenGain = 0.0;
    d->mixerSettings.blackBlueGain  = 0.0;

    adjustSliders();

    d->monochrome->blockSignals(false);
    d->preserveLuminosity->blockSignals(false);
    d->gboxSettings->histogramBox()->histogram()->reset();
    d->gboxSettings->histogramBox()->setChannel(RedChannel);
}

// Load all gains.
void ChannelMixerTool::slotLoadSettings()
{
    KUrl loadGainsFileUrl;
    FILE* fp = 0L;

    loadGainsFileUrl = KFileDialog::getOpenUrl(KGlobalSettings::documentPath(),
                                            QString( "*" ), kapp->activeWindow(),
                                            QString( i18n("Select Gimp Gains Mixer File to Load")) );
    if ( loadGainsFileUrl.isEmpty() )
       return;

    fp = fopen(QFile::encodeName(loadGainsFileUrl.toLocalFile()), "r");

    if ( fp )
    {
        bool monochrome;
        ChannelType currentOutputChannel = RedChannel;
        char buf1[1024];
        char buf2[1024];
        char buf3[1024];

        buf1[0] = '\0';

        fgets(buf1, 1023, fp);

        fscanf (fp, "%*s %s", buf1);

        // Get the current output channel in dialog.

        if (strcmp (buf1, "RED") == 0)
            currentOutputChannel = RedChannel;
        else if (strcmp (buf1, "GREEN") == 0)
            currentOutputChannel = GreenChannel;
        else if (strcmp (buf1, "BLUE") == 0)
            currentOutputChannel = BlueChannel;

        fscanf (fp, "%*s %s", buf1); // preview flag, preserved for compatibility

        fscanf (fp, "%*s %s", buf1);

        if (strcmp (buf1, "true") == 0)
            monochrome = true;
        else
            monochrome = false;

        fscanf (fp, "%*s %s", buf1);

        if (strcmp (buf1, "true") == 0)
            d->preserveLuminosity->setChecked(true);
        else
            d->preserveLuminosity->setChecked(false);

        fscanf (fp, "%*s %s %s %s", buf1, buf2, buf3);
        d->mixerSettings.redRedGain   = atof(buf1);
        d->mixerSettings.redGreenGain = atof(buf2);
        d->mixerSettings.redBlueGain  = atof(buf3);

        fscanf (fp, "%*s %s %s %s", buf1, buf2, buf3);
        d->mixerSettings.greenRedGain   = atof(buf1);
        d->mixerSettings.greenGreenGain = atof(buf2);
        d->mixerSettings.greenBlueGain  = atof(buf3);

        fscanf (fp, "%*s %s %s %s", buf1, buf2, buf3);
        d->mixerSettings.blueRedGain   = atof(buf1);
        d->mixerSettings.blueGreenGain = atof(buf2);
        d->mixerSettings.blueBlueGain  = atof(buf3);

        fscanf (fp, "%*s %s %s %s", buf1, buf2, buf3);
        d->mixerSettings.blackRedGain   = atof(buf1);
        d->mixerSettings.blackGreenGain = atof(buf2);
        d->mixerSettings.blackBlueGain  = atof(buf3);

        fclose(fp);

        // Refresh settings.
        d->monochrome->setChecked(monochrome);
        d->gboxSettings->histogramBox()->setChannel(currentOutputChannel);
    }
    else
    {
        KMessageBox::error(kapp->activeWindow(),
                           i18n("Cannot load settings from the Gains Mixer text file."));
        return;
    }
}

// Save all gains.
void ChannelMixerTool::slotSaveAsSettings()
{
    KUrl saveGainsFileUrl;
    FILE* fp = 0L;

    saveGainsFileUrl = KFileDialog::getSaveUrl(KGlobalSettings::documentPath(),
                                               QString( "*" ), kapp->activeWindow(),
                                               QString( i18n("Gimp Gains Mixer File to Save")) );
    if ( saveGainsFileUrl.isEmpty() )
       return;

    fp = fopen(QFile::encodeName(saveGainsFileUrl.toLocalFile()), "w");

    if ( fp )
    {
        const char* str = 0L;
        char        buf1[256];
        char        buf2[256];
        char        buf3[256];

        switch (d->gboxSettings->histogramBox()->channel())
        {
           case RedChannel:
              str = "RED";
              break;
           case GreenChannel:
              str = "GREEN";
              break;
           case BlueChannel:
              str = "BLUE";
              break;
           default:
              kWarning() <<  "Unknown Color channel gains";
              break;
        }

        fprintf (fp, "# Channel Mixer Configuration File\n");

        fprintf (fp, "CHANNEL: %s\n", str);
        fprintf (fp, "PREVIEW: %s\n", "true"); // preserved for compatibility
        fprintf (fp, "MONOCHROME: %s\n",
                 d->monochrome->isChecked() ? "true" : "false");
        fprintf (fp, "PRESERVE_LUMINOSITY: %s\n",
                 d->preserveLuminosity->isChecked() ? "true" : "false");

        sprintf (buf1, "%5.3f", d->mixerSettings.redRedGain);
        sprintf (buf2, "%5.3f", d->mixerSettings.redGreenGain);
        sprintf (buf3, "%5.3f", d->mixerSettings.redBlueGain);
        fprintf (fp, "RED: %s %s %s\n", buf1, buf2,buf3);

        sprintf (buf1, "%5.3f", d->mixerSettings.greenRedGain);
        sprintf (buf2, "%5.3f", d->mixerSettings.greenGreenGain);
        sprintf (buf3, "%5.3f", d->mixerSettings.greenBlueGain);
        fprintf (fp, "GREEN: %s %s %s\n", buf1, buf2,buf3);

        sprintf (buf1, "%5.3f", d->mixerSettings.blueRedGain);
        sprintf (buf2, "%5.3f", d->mixerSettings.blueGreenGain);
        sprintf (buf3, "%5.3f", d->mixerSettings.blueBlueGain);
        fprintf (fp, "BLUE: %s %s %s\n", buf1, buf2,buf3);

        sprintf (buf1, "%5.3f", d->mixerSettings.blackRedGain);
        sprintf (buf2, "%5.3f", d->mixerSettings.blackGreenGain);
        sprintf (buf3, "%5.3f", d->mixerSettings.blackBlueGain);
        fprintf (fp, "BLACK: %s %s %s\n", buf1, buf2,buf3);

        fclose (fp);
    }
    else
    {
        KMessageBox::error(kapp->activeWindow(),
                           i18n("Cannot save settings to the Gains Mixer text file."));
        return;
    }
}

}  // namespace DigikamChannelMixerImagesPlugin
