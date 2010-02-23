/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-12-06
 * Description : Black and White conversion tool.
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2006-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "bwsepiatool.moc"

// Qt includes

#include <QFile>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QTextStream>

// KDE includes

#include <kapplication.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kfiledialog.h>
#include <kglobalsettings.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <ktabwidget.h>

// LibKDcraw includes

#include <libkdcraw/rnuminput.h>

// Local includes

#include "colorgradientwidget.h"
#include "curvesbox.h"
#include "curveswidget.h"
#include "dimg.h"
#include "editortoolsettings.h"
#include "histogramwidget.h"
#include "histogrambox.h"
#include "imageiface.h"
#include "imageguidewidget.h"
#include "bwsepiafilter.h"
#include "previewlist.h"

using namespace KDcrawIface;

namespace DigikamImagesPluginCore
{

class BWSepiaToolPriv
{

public:

    BWSepiaToolPriv() :
        configGroupName("convertbw Tool"),
        configSettingsTabEntry("Settings Tab"),
        configBWFilterEntry("BW Filter"),
        configBWFilmEntry("BW Film"),
        configBWToneEntry("BW Tone"),
        configContrastAdjustmentEntry("ContrastAdjustment"),
        configStrengthAdjustmentEntry("StrengthAdjustment"),
        configHistogramChannelEntry("Histogram Channel"),
        configHistogramScaleEntry("Histogram Scale"),
        configCurveEntry("BWSepiaCurve"),

        destinationPreviewData(0),
        bwFilters(0),
        bwFilm(0),
        bwTone(0),
        tab(0),
        cInput(0),
        strengthInput(0),
        previewWidget(0),
        curvesBox(0),
        gboxSettings(0),
        originalImage(0)
        {}

    const QString              configGroupName;
    const QString              configSettingsTabEntry;
    const QString              configBWFilterEntry;
    const QString              configBWFilmEntry;
    const QString              configBWToneEntry;
    const QString              configContrastAdjustmentEntry;
    const QString              configStrengthAdjustmentEntry;
    const QString              configHistogramChannelEntry;
    const QString              configHistogramScaleEntry;
    const QString              configCurveEntry;

    uchar*                     destinationPreviewData;

    PreviewList*               bwFilters;
    PreviewList*               bwFilm;
    PreviewList*               bwTone;

    KTabWidget*                tab;

    KDcrawIface::RIntNumInput* cInput;
    KDcrawIface::RIntNumInput* strengthInput;

    ImageGuideWidget*          previewWidget;

    CurvesBox*                 curvesBox;

    EditorToolSettings*        gboxSettings;

    DImg*                      originalImage;
};

// -----------------------------------------------------------------------------------

BWSepiaTool::BWSepiaTool(QObject* parent)
           : EditorTool(parent), d(new BWSepiaToolPriv)
{
    setObjectName("convertbw");
    setToolName(i18n("Black && White"));
    setToolIcon(SmallIcon("bwtonal"));
    setToolHelp("blackandwhitetool.anchor");

    // -------------------------------------------------------------

    d->previewWidget = new ImageGuideWidget;
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

    QGridLayout* gridSettings = new QGridLayout(d->gboxSettings->plainPage());

    // -------------------------------------------------------------

    d->tab = new KTabWidget(d->gboxSettings->plainPage());

    ImageIface iface(0, 0);
    PreviewListItem* item = 0;
    d->originalImage      = iface.getOriginalImg();
    DImg thumbImage       = d->originalImage->smoothScale(128, 128, Qt::KeepAspectRatio);
    d->bwFilm             = new PreviewList(parent);
    int type              = BWSepiaContainer::BWGeneric;

    item = d->bwFilm->addItem(new BWSepiaFilter(&thumbImage, 0, BWSepiaContainer(type)), i18nc("generic black and white film", "Generic"), type);
    item->setWhatsThis(0, i18n("<b>Generic</b>:"
                               "<p>Simulate a generic black and white film.</p>"));

    ++type;
    item = d->bwFilm->addItem(new BWSepiaFilter(&thumbImage, 0, BWSepiaContainer(type)), i18n("Agfa 200X"), type);
    item->setWhatsThis(0, i18n("<b>Agfa 200X</b>:"
                               "<p>Simulate the Agfa 200X black and white film at 200 ISO.</p>"));

    ++type;
    item = d->bwFilm->addItem(new BWSepiaFilter(&thumbImage, 0, BWSepiaContainer(type)), i18n("Agfa Pan 25"), type);
    item->setWhatsThis(0, i18n("<b>Agfa Pan 25</b>:"
                               "<p>Simulate the Agfa Pan black and white film at 25 ISO.</p>"));

    ++type;
    item = d->bwFilm->addItem(new BWSepiaFilter(&thumbImage, 0, BWSepiaContainer(type)), i18n("Agfa Pan 100"), type);
    item->setWhatsThis(0, i18n("<b>Agfa Pan 100</b>:"
                               "<p>Simulate the Agfa Pan black and white film at 100 ISO.</p>"));

    ++type;
    item = d->bwFilm->addItem(new BWSepiaFilter(&thumbImage, 0, BWSepiaContainer(type)), i18n("Agfa Pan 400"), type);
    item->setWhatsThis(0, i18n("<b>Agfa Pan 400</b>:"
                               "<p>Simulate the Agfa Pan black and white film at 400 ISO.</p>"));

    ++type;
    item = d->bwFilm->addItem(new BWSepiaFilter(&thumbImage, 0, BWSepiaContainer(type)), i18n("Ilford Delta 100"), type);
    item->setWhatsThis(0, i18n("<b>Ilford Delta 100</b>:"
                               "<p>Simulate the Ilford Delta black and white film at 100 ISO.</p>"));

    ++type;
    item = d->bwFilm->addItem(new BWSepiaFilter(&thumbImage, 0, BWSepiaContainer(type)), i18n("Ilford Delta 400"), type);
    item->setWhatsThis(0, i18n("<b>Ilford Delta 400</b>:"
                               "<p>Simulate the Ilford Delta black and white film at 400 ISO.</p>"));

    ++type;
    item = d->bwFilm->addItem(new BWSepiaFilter(&thumbImage, 0, BWSepiaContainer(type)), i18n("Ilford Delta 400 Pro 3200"), type);
    item->setWhatsThis(0, i18n("<b>Ilford Delta 400 Pro 3200</b>:"
                               "<p>Simulate the Ilford Delta 400 Pro black and white film at 3200 ISO.</p>"));

    ++type;
    item = d->bwFilm->addItem(new BWSepiaFilter(&thumbImage, 0, BWSepiaContainer(type)), i18n("Ilford FP4 Plus"), type);
    item->setWhatsThis(0, i18n("<b>Ilford FP4 Plus</b>:"
                               "<p>Simulate the Ilford FP4 Plus black and white film at 125 ISO.</p>"));

    ++type;
    item = d->bwFilm->addItem(new BWSepiaFilter(&thumbImage, 0, BWSepiaContainer(type)), i18n("Ilford HP5 Plus"), type);
    item->setWhatsThis(0, i18n("<b>Ilford HP5 Plus</b>:"
                               "<p>Simulate the Ilford HP5 Plus black and white film at 400 ISO.</p>"));

    ++type;
    item = d->bwFilm->addItem(new BWSepiaFilter(&thumbImage, 0, BWSepiaContainer(type)), i18n("Ilford PanF Plus"), type);
    item->setWhatsThis(0, i18n("<b>Ilford PanF Plus</b>:"
                               "<p>Simulate the Ilford PanF Plus black and white film at 50 ISO.</p>"));

    ++type;
    item = d->bwFilm->addItem(new BWSepiaFilter(&thumbImage, 0, BWSepiaContainer(type)), i18n("Ilford XP2 Super"), type);
    item->setWhatsThis(0, i18n("<b>Ilford XP2 Super</b>:"
                               "<p>Simulate the Ilford XP2 Super black and white film at 400 ISO.</p>"));

    ++type;
    item = d->bwFilm->addItem(new BWSepiaFilter(&thumbImage, 0, BWSepiaContainer(type)), i18n("Kodak Tmax 100"), type);
    item->setWhatsThis(0, i18n("<b>Kodak Tmax 100</b>:"
                               "<p>Simulate the Kodak Tmax black and white film at 100 ISO.</p>"));

    ++type;
    item = d->bwFilm->addItem(new BWSepiaFilter(&thumbImage, 0, BWSepiaContainer(type)), i18n("Kodak Tmax 400"), type);
    item->setWhatsThis(0, i18n("<b>Kodak Tmax 400</b>:"
                               "<p>Simulate the Kodak Tmax black and white film at 400 ISO.</p>"));

    ++type;
    item = d->bwFilm->addItem(new BWSepiaFilter(&thumbImage, 0, BWSepiaContainer(type)), i18n("Kodak TriX"), type);
    item->setWhatsThis(0, i18n("<b>Kodak TriX</b>:"
                               "<p>Simulate the Kodak TriX black and white film at 400 ISO.</p>"));

    // -------------------------------------------------------------

    type = BWSepiaContainer::BWIlfordSFX200;

    item = d->bwFilm->addItem(new BWSepiaFilter(&thumbImage, 0, BWSepiaContainer(type)), i18n("Ilford SPX 200"), type);
    item->setWhatsThis(0, i18n("<b>Ilford SPX 200</b>:"
                               "<p>Simulate the Ilford SPX infrared film at 200 ISO.</p>"));

    ++type;
    item = d->bwFilm->addItem(new BWSepiaFilter(&thumbImage, 0, BWSepiaContainer(type)), i18n("Ilford SPX 400"), type);
    item->setWhatsThis(0, i18n("<b>Ilford SPX 400</b>:"
                               "<p>Simulate the Ilford SPX infrared film at 400 ISO.</p>"));

    ++type;
    item = d->bwFilm->addItem(new BWSepiaFilter(&thumbImage, 0, BWSepiaContainer(type)), i18n("Ilford SPX 800"), type);
    item->setWhatsThis(0, i18n("<b>Ilford SPX 800</b>:"
                               "<p>Simulate the Ilford SPX infrared film at 800 ISO.</p>"));

    // -------------------------------------------------------------

    QWidget* vbox     = new QWidget(d->tab);
    QVBoxLayout* vlay = new QVBoxLayout(vbox);
    d->bwFilters      = new PreviewList(parent);

    type = BWSepiaContainer::BWNoFilter;
    item = d->bwFilters->addItem(new BWSepiaFilter(&thumbImage, 0, BWSepiaContainer(type)), i18n("No Lens Filter"), type);
    item->setWhatsThis(0, i18n("<b>No Lens Filter</b>:"
                               "<p>Do not apply a lens filter when rendering the image.</p>"));

    ++type;
    item = d->bwFilters->addItem(new BWSepiaFilter(&thumbImage, 0, BWSepiaContainer(type)), i18n("Green Filter"), type);
    item->setWhatsThis(0, i18n("<b>Black & White with Green Filter</b>:"
                               "<p>Simulate black and white film exposure using a green filter. "
                               "This is useful for all scenic shoots, especially "
                               "portraits photographed against the sky.</p>"));

    ++type;
    item = d->bwFilters->addItem(new BWSepiaFilter(&thumbImage, 0, BWSepiaContainer(type)), i18n("Orange Filter"), type);
    item->setWhatsThis(0, i18n("<b>Black & White with Orange Filter</b>:"
                               "<p>Simulate black and white film exposure using an orange filter. "
                               "This will enhance landscapes, marine scenes and aerial "
                               "photography.</p>"));

    ++type;
    item = d->bwFilters->addItem(new BWSepiaFilter(&thumbImage, 0, BWSepiaContainer(type)), i18n("Red Filter"), type);
    item->setWhatsThis(0, i18n("<b>Black & White with Red Filter</b>:"
                               "<p>Simulate black and white film exposure using a red filter. "
                               "This creates dramatic sky effects, and simulates moonlight scenes "
                               "in the daytime.</p>"));

    ++type;
    item = d->bwFilters->addItem(new BWSepiaFilter(&thumbImage, 0, BWSepiaContainer(type)), i18n("Yellow Filter"), type);
    item->setWhatsThis(0, i18n("<b>Black & White with Yellow Filter</b>:"
                               "<p>Simulate black and white film exposure using a yellow filter. "
                               "This has the most natural tonal correction, and improves contrast. Ideal for "
                               "landscapes.</p>"));

    ++type;
    item = d->bwFilters->addItem(new BWSepiaFilter(&thumbImage, 0, BWSepiaContainer(type)), i18n("Yellow-Green Filter"), type);
    item->setWhatsThis(0, i18n("<b>Black & White with Yellow-Green Filter</b>:"
                               "<p>Simulate black and white film exposure using a yellow-green filter. "
                               "A yellow-green filter is highly effective for outdoor portraits because "
                               "red is rendered dark while green appears lighter. Great for correcting skin tones, "
                               "bringing out facial expressions in close-ups and emphasizing the feeling of liveliness. "
                               "This filter is highly effective for indoor portraits under tungsten lighting.</p>"));

    ++type;
    item = d->bwFilters->addItem(new BWSepiaFilter(&thumbImage, 0, BWSepiaContainer(type)), i18n("Blue Filter"), type);
    item->setWhatsThis(0, i18n("<b>Black & White with Blue Filter</b>:"
                               "<p>Simulate black and white film exposure using a blue filter. "
                               "This accentuates haze and fog. Used for dye transfer and contrast effects.</p>"));

    d->strengthInput = new RIntNumInput();
    d->strengthInput->input()->setLabel(i18n("Strength:"), Qt::AlignLeft | Qt::AlignVCenter);
    d->strengthInput->setRange(1, 5, 1);
    d->strengthInput->setSliderEnabled(true);
    d->strengthInput->setDefaultValue(1);
    d->strengthInput->setWhatsThis(i18n("Here, set the strength adjustment of the lens filter."));

    vlay->addWidget(d->bwFilters);
    vlay->addWidget(d->strengthInput);
    vlay->setSpacing(d->gboxSettings->spacingHint());
    vlay->setMargin(0);

    // -------------------------------------------------------------

    d->bwTone = new PreviewList(parent);

    type = BWSepiaContainer::BWNoTone;
    item = d->bwTone->addItem(new BWSepiaFilter(&thumbImage, 0, BWSepiaContainer(type)), i18n("No Tone Filter"), type);
    item->setWhatsThis(0, i18n("<b>No Tone Filter</b>:"
                               "<p>Do not apply a tone filter to the image.</p>"));

    ++type;
    item = d->bwTone->addItem(new BWSepiaFilter(&thumbImage, 0, BWSepiaContainer(type)), i18n("Sepia Filter"), type);
    item->setWhatsThis(0, i18n("<b>Black & White with Sepia Tone</b>:"
                               "<p>Gives a warm highlight and mid-tone while adding a bit of coolness to "
                               "the shadows - very similar to the process of bleaching a print and "
                               "re-developing in a sepia toner.</p>"));

    ++type;
    item = d->bwTone->addItem(new BWSepiaFilter(&thumbImage, 0, BWSepiaContainer(type)), i18n("Brown Filter"), type);
    item->setWhatsThis(0, i18n("<b>Black & White with Brown Tone</b>:"
                               "<p>This filter is more neutral than the Sepia Tone "
                               "filter.</p>"));

    ++type;
    item = d->bwTone->addItem(new BWSepiaFilter(&thumbImage, 0, BWSepiaContainer(type)), i18n("Cold Filter"), type);
    item->setWhatsThis(0, i18n("<b>Black & White with Cold Tone</b>:"
                               "<p>Start subtly and replicate printing on a cold tone black and white "
                               "paper such as a bromide enlarging "
                               "paper.</p>"));

    ++type;
    item = d->bwTone->addItem(new BWSepiaFilter(&thumbImage, 0, BWSepiaContainer(type)), i18n("Selenium Filter"), type);
    item->setWhatsThis(0, i18n("<b>Black & White with Selenium Tone</b>:"
                               "<p>This effect replicates traditional selenium chemical toning done "
                               "in the darkroom.</p>"));

    ++type;
    item = d->bwTone->addItem(new BWSepiaFilter(&thumbImage, 0, BWSepiaContainer(type)), i18n("Platinum Filter"), type);
    item->setWhatsThis(0, i18n("<b>Black & White with Platinum Tone</b>:"
                               "<p>This effect replicates traditional platinum chemical toning done "
                               "in the darkroom.</p>"));

    ++type;
    item = d->bwTone->addItem(new BWSepiaFilter(&thumbImage, 0, BWSepiaContainer(type)), i18n("Green Filter"), type);
    item->setWhatsThis(0, i18n("<b>Black & White with greenish tint</b>:"
                               "<p>This effect is also known as Verdante.</p>"));

    // -------------------------------------------------------------

    QWidget* curveBox = new QWidget();
    d->curvesBox      = new CurvesBox(256, 256, d->originalImage->bits(), d->originalImage->width(),
                                      d->originalImage->height(), d->originalImage->sixteenBit());
    d->curvesBox->enableCurveTypes(true);
    d->curvesBox->enableResetButton(true);
    d->curvesBox->setWhatsThis( i18n("This is the curve adjustment of the image luminosity"));

    // -------------------------------------------------------------

    d->cInput = new RIntNumInput(curveBox);
    d->cInput->input()->setLabel(i18n("Contrast:"), Qt::AlignLeft | Qt::AlignVCenter);
    d->cInput->setRange(-100, 100, 1);
    d->cInput->setSliderEnabled(true);
    d->cInput->setDefaultValue(0);
    d->cInput->setWhatsThis(i18n("Set here the contrast adjustment of the image."));

    QGridLayout* gridTab2 = new QGridLayout();
    gridTab2->addWidget(d->curvesBox, 0, 0, 1, 1);
    gridTab2->addWidget(d->cInput,    1, 0, 1, 1);
    gridTab2->setRowStretch(2, 10);
    gridTab2->setMargin(d->gboxSettings->spacingHint());
    gridTab2->setSpacing(0);
    curveBox->setLayout(gridTab2);

    // -------------------------------------------------------------

    d->tab->insertTab(FilmTab,       d->bwFilm, i18n("Film"));
    d->tab->insertTab(BWFiltersTab,  vbox,      i18n("Lens Filters"));
    d->tab->insertTab(ToneTab,       d->bwTone, i18n("Tone"));
    d->tab->insertTab(LuminosityTab, curveBox,  i18n("Lightness"));

    gridSettings->addWidget(d->tab, 0, 0, 1, 5);
    gridSettings->setRowStretch(0, 10);
    gridSettings->setMargin(d->gboxSettings->spacingHint());
    gridSettings->setSpacing(d->gboxSettings->spacingHint());

    setToolSettings(d->gboxSettings);
    init();

    // -------------------------------------------------------------

    connect(d->previewWidget, SIGNAL(spotPositionChangedFromOriginal(const Digikam::DColor&, const QPoint&)),
            this, SLOT(slotSpotColorChanged(const Digikam::DColor&)));

    connect(d->previewWidget, SIGNAL(spotPositionChangedFromTarget(const Digikam::DColor&, const QPoint& )),
            this, SLOT(slotColorSelectedFromTarget(const Digikam::DColor&)));

    connect(d->bwFilters, SIGNAL(itemSelectionChanged()),
            this, SLOT(slotFilterSelected()));

    connect(d->strengthInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimer()));

    connect(d->bwFilm, SIGNAL(itemSelectionChanged()),
            this, SLOT(slotEffect()));

    connect(d->bwTone, SIGNAL(itemSelectionChanged()),
            this, SLOT(slotEffect()));

    connect(d->cInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimer()));

    connect(d->previewWidget, SIGNAL(signalResized()),
            this, SLOT(slotEffect()));

    connect(d->curvesBox, SIGNAL(signalCurvesChanged()),
            this, SLOT(slotTimer()));

    connect(d->curvesBox, SIGNAL(signalChannelReset(int)),
            this, SLOT(slotEffect()));
}

BWSepiaTool::~BWSepiaTool()
{
    if (d->destinationPreviewData)
       delete [] d->destinationPreviewData;

    delete d;
}

void BWSepiaTool::slotInit()
{
    EditorTool::slotInit();
    d->bwFilters->startFilters();
    d->bwFilm->startFilters();
    d->bwTone->startFilters();
}

void BWSepiaTool::slotFilterSelected()
{
    int filter = d->bwFilters->currentId();
    if (filter == BWSepiaContainer::BWNoFilter)
        d->strengthInput->setEnabled(false);
    else
        d->strengthInput->setEnabled(true);

    slotEffect();
}

void BWSepiaTool::slotScaleChanged()
{
    d->curvesBox->setScale(d->gboxSettings->histogramBox()->scale());
}

void BWSepiaTool::slotSpotColorChanged(const DColor& color)
{
    d->curvesBox->setCurveGuide(color);
}

void BWSepiaTool::slotColorSelectedFromTarget(const DColor& color)
{
    d->gboxSettings->histogramBox()->histogram()->setHistogramGuideByColor(color);
}

void BWSepiaTool::blockWidgetSignals(bool b)
{
    d->bwFilm->blockSignals(b);
    d->bwFilters->blockSignals(b);
    d->bwTone->blockSignals(b);
    d->cInput->blockSignals(b);
    d->strengthInput->blockSignals(b);
}

void BWSepiaTool::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);

    d->tab->setCurrentIndex(group.readEntry(d->configSettingsTabEntry,           (int)BWFiltersTab));
    d->bwFilters->setCurrentId(group.readEntry(d->configBWFilterEntry,           0));
    d->bwFilm->setCurrentId(group.readEntry(d->configBWFilmEntry,                0));
    d->bwTone->setCurrentId(group.readEntry(d->configBWToneEntry,                0));
    d->cInput->setValue(group.readEntry(d->configContrastAdjustmentEntry,        d->cInput->defaultValue()));
    d->strengthInput->setValue(group.readEntry(d->configStrengthAdjustmentEntry, d->strengthInput->defaultValue()));

    d->curvesBox->readCurveSettings(group, d->configCurveEntry);

    // we need to call the set methods here, otherwise the curve will not be updated correctly
    d->gboxSettings->histogramBox()->setChannel((ChannelType)group.readEntry(d->configHistogramChannelEntry,
                    (int)LuminosityChannel));
    d->gboxSettings->histogramBox()->setScale((HistogramScale)group.readEntry(d->configHistogramScaleEntry,
                    (int)LogScaleHistogram));

    slotFilterSelected();
}

void BWSepiaTool::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);

    group.writeEntry(d->configSettingsTabEntry,        d->tab->currentIndex());
    group.writeEntry(d->configHistogramChannelEntry,   (int)d->gboxSettings->histogramBox()->channel());
    group.writeEntry(d->configHistogramScaleEntry,     (int)d->gboxSettings->histogramBox()->scale());
    group.writeEntry(d->configBWFilterEntry,           d->bwFilters->currentId());
    group.writeEntry(d->configBWFilmEntry,             d->bwFilm->currentId());
    group.writeEntry(d->configBWToneEntry,             d->bwTone->currentId());
    group.writeEntry(d->configContrastAdjustmentEntry, d->cInput->value());
    group.writeEntry(d->configStrengthAdjustmentEntry, d->strengthInput->value());

    d->curvesBox->writeCurveSettings(group, d->configCurveEntry);

    group.sync();
}

void BWSepiaTool::slotResetSettings()
{
    blockWidgetSignals(true);

    d->bwFilters->setCurrentId(BWSepiaContainer::BWNoFilter);
    d->bwFilm->setCurrentId(BWSepiaContainer::BWGeneric);
    d->bwTone->setCurrentId(BWSepiaContainer::BWNoTone);

    d->cInput->slotReset();
    d->strengthInput->slotReset();

    for (int channel = 0 ; channel < 5 ; ++channel)
       d->curvesBox->curves()->curvesChannelReset(channel);

    d->curvesBox->reset();

    blockWidgetSignals(false);

    d->gboxSettings->histogramBox()->histogram()->reset();
    d->bwFilters->update();
    d->bwTone->update();

    slotEffect();
}

void BWSepiaTool::slotEffect()
{
    kapp->setOverrideCursor(Qt::WaitCursor);

    d->gboxSettings->histogramBox()->histogram()->stopHistogramComputation();

    delete [] d->destinationPreviewData;

    ImageIface* iface         = d->previewWidget->imageIface();
    d->destinationPreviewData = iface->getPreviewImage();
    int w                     = iface->previewWidth();
    int h                     = iface->previewHeight();
    bool sb                   = iface->previewSixteenBit();

    BWSepiaContainer prm;
    prm.filmType        = d->bwFilm->currentId();
    prm.filterType      = d->bwFilters->currentId();
    prm.toneType        = d->bwTone->currentId();
    prm.bcgPrm.contrast = ((double)(d->cInput->value()/100.0) + 1.00);
    prm.strength        = 1.0 + ((double)d->strengthInput->value() - 1.0) * (1.0 / 3.0);
    prm.curves->fillFromOtherCurvers(d->curvesBox->curves());
    BWSepiaFilter bw(d->destinationPreviewData, w, h, sb, prm);
    DImg preview        = bw.getTargetImage();

    iface->putPreviewImage(preview.bits());
    d->previewWidget->updatePreview();

    // Update histogram.

    memcpy(d->destinationPreviewData, preview.bits(), preview.numBytes());
    d->gboxSettings->histogramBox()->histogram()->updateData(d->destinationPreviewData, w, h, sb, 0, 0, 0, false);

    kapp->restoreOverrideCursor();
}

void BWSepiaTool::finalRendering()
{
    kapp->setOverrideCursor(Qt::WaitCursor);
    ImageIface iface(0, 0);
    uchar* data = iface.getOriginalImage();
    int w       = iface.originalWidth();
    int h       = iface.originalHeight();
    bool sb     = iface.originalSixteenBit();

    BWSepiaContainer prm;
    prm.filmType        = d->bwFilm->currentId();
    prm.filterType      = d->bwFilters->currentId();
    prm.toneType        = d->bwTone->currentId();
    prm.bcgPrm.contrast = ((double)(d->cInput->value()/100.0) + 1.00);
    prm.strength        = 1.0 + ((double)d->strengthInput->value() - 1.0) * (1.0 / 3.0);
    prm.curves->fillFromOtherCurvers(d->curvesBox->curves());
    BWSepiaFilter bw(data, w, h, sb, prm);

    iface.putOriginalImage(i18n("Convert to Black && White"), bw.getTargetImage().bits());

    kapp->restoreOverrideCursor();
}

//-- Load all settings from file --------------------------------------

void BWSepiaTool::slotLoadSettings()
{
    KUrl loadFile = KFileDialog::getOpenUrl(KGlobalSettings::documentPath(),
                                            QString( "*" ), kapp->activeWindow(),
                                            QString( i18n("Black & White Settings File to Load")) );
    if ( loadFile.isEmpty() )
       return;

    QFile file(loadFile.toLocalFile());

    if ( file.open(QIODevice::ReadOnly) )
    {
        QTextStream stream( &file );

        if ( stream.readLine() != "# Black & White Configuration File" )
        {
           KMessageBox::error(kapp->activeWindow(),
                        i18n("\"%1\" is not a Black & White settings text file.",
                             loadFile.fileName()));
           file.close();
           return;
        }

        blockWidgetSignals(true);

        d->bwFilm->setCurrentId(stream.readLine().toInt());
        d->bwFilters->setCurrentId(stream.readLine().toInt());
        d->bwTone->setCurrentId(stream.readLine().toInt());
        d->cInput->setValue(stream.readLine().toInt());

        for (int i = 0 ; i < 5 ; ++i)
            d->curvesBox->curves()->curvesChannelReset(i);

        d->curvesBox->curves()->setCurveType(d->curvesBox->channel(), ImageCurves::CURVE_SMOOTH);
        d->curvesBox->reset();

        // TODO cant we use the kconfig mechanisms provided by CurveWidget here?
        QPoint disable = ImageCurves::getDisabledValue();
        QPoint p;
        for (int j = 0 ; j < ImageCurves::NUM_POINTS ; ++j)
        {
            p.setX( stream.readLine().toInt() );
            p.setY( stream.readLine().toInt() );

            if (d->originalImage->sixteenBit() && p != disable)
            {
                p.setX(p.x()*ImageCurves::MULTIPLIER_16BIT);
                p.setY(p.y()*ImageCurves::MULTIPLIER_16BIT);
            }

            d->curvesBox->curves()->setCurvePoint(LuminosityChannel, j, p);
        }

        for (int i = 0 ; i < ImageCurves::NUM_CHANNELS ; ++i)
            d->curvesBox->curves()->curvesCalculateCurve(i);

        blockWidgetSignals(false);

        d->gboxSettings->histogramBox()->histogram()->reset();
        d->bwFilters->update();
        d->bwTone->update();

        slotEffect();
    }
    else
    {
        KMessageBox::error(kapp->activeWindow(), i18n("Cannot load settings from the Black & White text file."));
    }

    file.close();
}

//-- Save all settings to file ---------------------------------------

void BWSepiaTool::slotSaveAsSettings()
{
    KUrl saveFile = KFileDialog::getSaveUrl(KGlobalSettings::documentPath(),
                                            QString( "*" ), kapp->activeWindow(),
                                            QString( i18n("Black & White Settings File to Save")) );
    if ( saveFile.isEmpty() )
       return;

    QFile file(saveFile.toLocalFile());

    if ( file.open(QIODevice::WriteOnly) )
    {
        QTextStream stream( &file );
        stream << "# Black & White Configuration File\n";
        stream << d->bwFilm->currentId() << "\n";
        stream << d->bwFilters->currentId() << "\n";
        stream << d->bwTone->currentId() << "\n";
        stream << d->cInput->value() << "\n";

        // TODO cant we use the kconfig mechanisms provided by CurveWidget here?
        for (int j = 0 ; j < ImageCurves::NUM_POINTS ; ++j)
        {
            QPoint p = d->curvesBox->curves()->getCurvePoint(LuminosityChannel, j);
            if (d->originalImage->sixteenBit())
            {
                p.setX(p.x()/ImageCurves::MULTIPLIER_16BIT);
                p.setY(p.y()/ImageCurves::MULTIPLIER_16BIT);
            }
            stream << p.x() << "\n";
            stream << p.y() << "\n";
        }
    }
    else
    {
        KMessageBox::error(kapp->activeWindow(),
                           i18n("Cannot save settings to the Black & White text file."));
    }

    file.close();
}

}  // namespace DigikamImagesPluginCore
