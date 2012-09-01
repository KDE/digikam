/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-12-01
 * Description : image histogram adjust curves.
 *
 * Copyright (C) 2004-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "adjustcurvestool.moc"

// C++ includes

#include <cmath>

// Qt includes

#include <QGroupBox>
#include <QVBoxLayout>
#include <QLabel>

// KDE includes

#include <kapplication.h>
#include <kconfig.h>
#include <kcursor.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kicon.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kstandarddirs.h>

// Local includes

#include "dimg.h"
#include "editortoolsettings.h"
#include "histogrambox.h"
#include "histogramwidget.h"
#include "curvesfilter.h"
#include "curvessettings.h"
#include "imagehistogram.h"
#include "imagecurves.h"
#include "imageiface.h"
#include "imageregionwidget.h"

namespace DigikamColorImagePlugin
{

class AdjustCurvesTool::AdjustCurvesToolPriv
{
public:

    AdjustCurvesToolPriv() :
        destinationPreviewData(0),
        settingsView(0),
        previewWidget(0),
        gboxSettings(0)
    {}

    static const QString configGroupName;
    static const QString configHistogramChannelEntry;
    static const QString configHistogramScaleEntry;

    uchar*               destinationPreviewData;

    CurvesSettings*      settingsView;
    ImageRegionWidget*   previewWidget;

    EditorToolSettings*  gboxSettings;
};
const QString AdjustCurvesTool::AdjustCurvesToolPriv::configGroupName("adjustcurves Tool");
const QString AdjustCurvesTool::AdjustCurvesToolPriv::configHistogramChannelEntry("Histogram Channel");
const QString AdjustCurvesTool::AdjustCurvesToolPriv::configHistogramScaleEntry("Histogram Scale");

// --------------------------------------------------------

AdjustCurvesTool::AdjustCurvesTool(QObject* parent)
    : EditorToolThreaded(parent),
      d(new AdjustCurvesToolPriv)
{
    setObjectName("adjustcurves");
    setToolName(i18n("Adjust Curves"));
    setToolIcon(SmallIcon("adjustcurves"));

    // -------------------------------------------------------------

    d->previewWidget = new ImageRegionWidget;
    setToolView(d->previewWidget);
    setPreviewModeMask(PreviewToolBar::AllPreviewModes);

    // -------------------------------------------------------------

    d->gboxSettings = new EditorToolSettings;
    d->gboxSettings->setTools(EditorToolSettings::Histogram);
    d->gboxSettings->setHistogramType(Digikam::LRGBA);
    d->gboxSettings->setButtons(EditorToolSettings::Default|
                                EditorToolSettings::Load|
                                EditorToolSettings::SaveAs|
                                EditorToolSettings::Ok|
                                EditorToolSettings::Cancel);

    // we don't need to use the Gradient widget in this tool
    d->gboxSettings->histogramBox()->setGradientVisible(false);

    // -------------------------------------------------------------

    ImageIface iface(0, 0);
    QVBoxLayout* vbox = new QVBoxLayout(d->gboxSettings->plainPage());
    d->settingsView   = new CurvesSettings(d->gboxSettings->plainPage(), iface.getOriginal());
    d->gboxSettings->histogramBox()->setContentsMargins(d->settingsView->curvesLeftOffset(), 0, 0, 0);
    vbox->addWidget(d->settingsView);
    vbox->addStretch(10);
    vbox->setMargin(0);
    vbox->setSpacing(0);

    setToolSettings(d->gboxSettings);
    init();

    // -------------------------------------------------------------

    connect(d->settingsView, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotTimer()));

    connect(d->gboxSettings, SIGNAL(signalChannelChanged()),
            this, SLOT(slotChannelChanged()));

    connect(d->gboxSettings, SIGNAL(signalScaleChanged()),
            this, SLOT(slotScaleChanged()));

    connect(d->previewWidget, SIGNAL(signalResized()),
            this, SLOT(slotEffect()));

    connect(d->previewWidget, SIGNAL(signalCapturedPointFromOriginal(Digikam::DColor,QPoint)),
            d->settingsView, SLOT(slotSpotColorChanged(Digikam::DColor)));

    connect(d->settingsView, SIGNAL(signalSpotColorChanged()),
            this, SLOT(slotSpotColorChanged()));

    connect(d->settingsView, SIGNAL(signalChannelReset(int)),
            this, SLOT(slotResetCurrentChannel()));

    connect(d->settingsView, SIGNAL(signalPickerChanged(int)),
            this, SLOT(slotPickerColorButtonActived(int)));

    /*
        connect(d->previewWidget, SIGNAL(spotPositionChangedFromTarget(Digikam::DColor,QPoint)),
                this, SLOT(slotColorSelectedFromTarget(Digikam::DColor)));
    */
}

AdjustCurvesTool::~AdjustCurvesTool()
{
    if (d->destinationPreviewData)
    {
        delete [] d->destinationPreviewData;
    }

    delete d;
}

void AdjustCurvesTool::slotPickerColorButtonActived(int type)
{
    if (type == CurvesBox::NoPicker)
    {
        return;
    }

    d->previewWidget->setCapturePointMode(true);
}

void AdjustCurvesTool::slotSpotColorChanged()
{
    d->previewWidget->setCapturePointMode(false);
    slotEffect();
}

void AdjustCurvesTool::slotColorSelectedFromTarget(const DColor& color)
{
    d->gboxSettings->histogramBox()->histogram()->setHistogramGuideByColor(color);
}

void AdjustCurvesTool::slotResetCurrentChannel()
{
    d->gboxSettings->histogramBox()->histogram()->reset();
    slotEffect();
}

void AdjustCurvesTool::slotChannelChanged()
{
    d->settingsView->setCurrentChannel(d->gboxSettings->histogramBox()->channel());
}

void AdjustCurvesTool::slotScaleChanged()
{
    d->settingsView->setScale(d->gboxSettings->histogramBox()->scale());
}

void AdjustCurvesTool::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);

    // we need to call the set methods here, otherwise the curve will not be updated correctly
    d->gboxSettings->histogramBox()->setChannel((ChannelType)group.readEntry(d->configHistogramChannelEntry,
            (int)LuminosityChannel));
    d->gboxSettings->histogramBox()->setScale((HistogramScale)group.readEntry(d->configHistogramScaleEntry,
            (int)LogScaleHistogram));

    d->settingsView->readSettings(group);

    slotScaleChanged();
    slotChannelChanged();

    slotEffect();
}

void AdjustCurvesTool::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);
    group.writeEntry(d->configHistogramChannelEntry, (int)d->gboxSettings->histogramBox()->channel());
    group.writeEntry(d->configHistogramScaleEntry,   (int)d->gboxSettings->histogramBox()->scale());

    d->settingsView->writeSettings(group);

    config->sync();
}

void AdjustCurvesTool::slotResetSettings()
{
    d->settingsView->resetToDefault();
    d->gboxSettings->histogramBox()->histogram()->reset();
    slotEffect();
}

void AdjustCurvesTool::prepareEffect()
{
    CurvesContainer settings = d->settingsView->settings();

    d->gboxSettings->histogramBox()->histogram()->stopHistogramComputation();

    DImg preview = d->previewWidget->getOriginalRegionImage(true);
    setFilter(new CurvesFilter(&preview, this, settings));
}

void AdjustCurvesTool::putPreviewData()
{
    DImg preview = filter()->getTargetImage();
    d->previewWidget->setPreviewImage(preview);

    // Update histogram.

    if (d->destinationPreviewData)
    {
        delete [] d->destinationPreviewData;
    }

    d->destinationPreviewData = preview.copyBits();
    d->gboxSettings->histogramBox()->histogram()->updateData(d->destinationPreviewData,
            preview.width(), preview.height(), preview.sixteenBit(),
            0, 0, 0, false);
}

void AdjustCurvesTool::prepareFinal()
{
    CurvesContainer settings = d->settingsView->settings();

    ImageIface iface(0, 0);
    setFilter(new CurvesFilter(iface.getOriginal(), this, settings));
}

void AdjustCurvesTool::putFinalData()
{
    ImageIface iface(0, 0);
    iface.putOriginal(i18n("Adjust Curve"), filter()->filterAction(), filter()->getTargetImage());
}

void AdjustCurvesTool::slotLoadSettings()
{
    d->settingsView->loadSettings();

    // Refresh the current curves config.
    slotChannelChanged();
    slotEffect();
}

void AdjustCurvesTool::slotSaveAsSettings()
{
    d->settingsView->saveAsSettings();
}

}  // namespace DigikamColorImagePlugin
