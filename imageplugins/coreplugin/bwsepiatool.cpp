/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-12-06
 * Description : Black and White conversion tool.
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2006-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "bwsepiatool.h"
#include "bwsepiatool.moc"

// Qt includes

#include <QFile>
#include <QGridLayout>
#include <QHash>
#include <QListWidget>
#include <QPixmap>
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
#include <kvbox.h>

// LibKDcraw includes

#include <libkdcraw/rnuminput.h>

// Local includes

#include "bcgmodifier.h"
#include "colorgradientwidget.h"
#include "curvesbox.h"
#include "curveswidget.h"
#include "dimg.h"
#include "dimgimagefilters.h"
#include "editortoolsettings.h"
#include "histogramwidget.h"
#include "histogrambox.h"
#include "imagecurves.h"
#include "imagehistogram.h"
#include "imageiface.h"
#include "imagewidget.h"

using namespace KDcrawIface;
using namespace Digikam;

namespace DigikamImagesPluginCore
{

enum BlackWhiteConversionType
{
    BWNoFilter=0,         // B&W filter to the front of lens.
    BWGreenFilter,
    BWOrangeFilter,
    BWRedFilter,
    BWYellowFilter,
    BWYellowGreenFilter,
    BWBlueFilter,

    BWGeneric,            // B&W film simulation.
    BWAgfa200X,
    BWAgfapan25,
    BWAgfapan100,
    BWAgfapan400,
    BWIlfordDelta100,
    BWIlfordDelta400,
    BWIlfordDelta400Pro3200,
    BWIlfordFP4,
    BWIlfordHP5,
    BWIlfordPanF,
    BWIlfordXP2Super,
    BWKodakTmax100,
    BWKodakTmax400,
    BWKodakTriX,

    BWNoTone,             // Chemical color tone filter.
    BWSepiaTone,
    BWBrownTone,
    BWColdTone,
    BWSeleniumTone,
    BWPlatinumTone,
    BWGreenTone
};

enum SettingsTab
{
    FilmTab=0,
    BWFiltersTab,
    ToneTab,
    LuminosityTab
};

// --------------------------------------------------------

class PreviewPixmapFactory : public QObject
{
public:

    PreviewPixmapFactory(BWSepiaTool* bwSepia, const QSize& previewSize);
    ~PreviewPixmapFactory();

    void invalidate() { m_previewPixmapMap.clear(); }

    const QSize previewSize() { return m_previewSize; };
    const QPixmap* pixmap(int id);

private:

    QPixmap makePixmap(int id);

private:

    QHash<int, QPixmap*> m_previewPixmapMap;
    QSize                m_previewSize;
    BWSepiaTool*         m_bwSepia;
};

PreviewPixmapFactory::PreviewPixmapFactory(BWSepiaTool* bwSepia, const QSize& previewSize)
                    : QObject(bwSepia)
{
    m_bwSepia     = bwSepia;
    m_previewSize = previewSize;
}

PreviewPixmapFactory::~PreviewPixmapFactory()
{
    // Delete all hash items
    foreach (QPixmap* pm, m_previewPixmapMap.values())
    {
        delete pm;
    }
    m_previewPixmapMap.clear();
}

const QPixmap* PreviewPixmapFactory::pixmap(int id)
{
    if (m_previewPixmapMap.find(id) == m_previewPixmapMap.end())
    {
        QPixmap pix = makePixmap(id);
        m_previewPixmapMap.insert(id, new QPixmap(pix));
    }

    QPixmap* res = m_previewPixmapMap[id];

    return res;
}

QPixmap PreviewPixmapFactory::makePixmap(int id)
{
    return m_bwSepia->getThumbnailForEffect(id);
}

// -----------------------------------------------------------------------------------

class ListWidgetBWPreviewItem : public QListWidgetItem
{

public:

    ListWidgetBWPreviewItem(QListWidget *parent, const QString& text,
                            PreviewPixmapFactory* factory, int id)
        : QListWidgetItem(QIcon(QPixmap(factory->previewSize())), text, parent)
    {
          m_previewPixmapFactory = factory;
          m_id                   = id;
    };

    void updatePreview()
    {
          setIcon(QIcon(*m_previewPixmapFactory->pixmap(m_id)));
    }

private:

    int                   m_id;
    PreviewPixmapFactory* m_previewPixmapFactory;
};

// -----------------------------------------------------------------------------------

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

        redAttn(0.0),
        greenAttn(0.0),
        blueAttn(0.0),
        redMult(0.0),
        greenMult(0.0),
        blueMult(0.0),
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
        originalImage(0),
        previewPixmapFactory(0)
        {}

    const QString                configGroupName;
    const QString                configSettingsTabEntry;
    const QString                configBWFilterEntry;
    const QString                configBWFilmEntry;
    const QString                configBWToneEntry;
    const QString                configContrastAdjustmentEntry;
    const QString                configStrengthAdjustmentEntry;
    const QString                configHistogramChannelEntry;
    const QString                configHistogramScaleEntry;

    // Color filter attenuation in percents.
    double                       redAttn;
    double                       greenAttn;
    double                       blueAttn;

    // Channel mixer color multiplier.
    double                       redMult;
    double                       greenMult;
    double                       blueMult;

    uchar*                       destinationPreviewData;

    QListWidget*                 bwFilters;
    QListWidget*                 bwFilm;
    QListWidget*                 bwTone;

    KTabWidget*                  tab;

    KDcrawIface::RIntNumInput*   cInput;
    KDcrawIface::RIntNumInput*   strengthInput;

    Digikam::ImageWidget*        previewWidget;

    Digikam::CurvesBox*          curvesBox;

    Digikam::EditorToolSettings* gboxSettings;

    Digikam::DImg*               originalImage;
    Digikam::DImg                thumbnailImage;

    PreviewPixmapFactory*        previewPixmapFactory;
};

// -----------------------------------------------------------------------------------

BWSepiaTool::BWSepiaTool(QObject* parent)
           : EditorTool(parent), d(new BWSepiaToolPriv)
{
    setObjectName("convertbw");
    setToolName(i18n("Black && White"));
    setToolIcon(SmallIcon("bwtonal"));
    setToolHelp("blackandwhitetool.anchor");

    d->destinationPreviewData = 0;

    ImageIface iface(0, 0);
    d->originalImage  = iface.getOriginalImg();
    d->thumbnailImage = d->originalImage->smoothScale(128, 128, Qt::KeepAspectRatio);

    // -------------------------------------------------------------

    d->previewWidget = new ImageWidget("convertbw Tool", 0,
                                      i18n("The black and white conversion tool preview is "
                                           "shown here. Picking a color on the image will "
                                           "show the corresponding color level on the histogram."));
    setToolView(d->previewWidget);

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

    d->bwFilm = new QListWidget(d->tab);
    d->bwFilm->setIconSize(d->thumbnailImage.size());
    d->previewPixmapFactory = new PreviewPixmapFactory(this, d->thumbnailImage.size());

    int type = BWGeneric;

    ListWidgetBWPreviewItem *item = new ListWidgetBWPreviewItem(d->bwFilm,
                                                                i18nc("generic black and white film",
                                                                      "Generic"), d->previewPixmapFactory, type);
    item->setWhatsThis(i18n("<b>Generic</b>:"
                            "<p>Simulate a generic black and white film.</p>"));

    ++type;
    item = new ListWidgetBWPreviewItem(d->bwFilm, i18n("Agfa 200X"), d->previewPixmapFactory, type);
    item->setWhatsThis(i18n("<b>Agfa 200X</b>:"
                            "<p>Simulate the Agfa 200X black and white film at 200 ISO.</p>"));

    ++type;
    item = new ListWidgetBWPreviewItem(d->bwFilm, i18n("Agfa Pan 25"), d->previewPixmapFactory, type);
    item->setWhatsThis(i18n("<b>Agfa Pan 25</b>:"
                            "<p>Simulate the Agfa Pan black and white film at 25 ISO.</p>"));

    ++type;
    item = new ListWidgetBWPreviewItem(d->bwFilm, i18n("Agfa Pan 100"), d->previewPixmapFactory, type);
    item->setWhatsThis(i18n("<b>Agfa Pan 100</b>:"
                            "<p>Simulate the Agfa Pan black and white film at 100 ISO.</p>"));

    ++type;
    item = new ListWidgetBWPreviewItem(d->bwFilm, i18n("Agfa Pan 400"), d->previewPixmapFactory, type);
    item->setWhatsThis(i18n("<b>Agfa Pan 400</b>:"
                            "<p>Simulate the Agfa Pan black and white film at 400 ISO.</p>"));

    ++type;
    item = new ListWidgetBWPreviewItem(d->bwFilm, i18n("Ilford Delta 100"), d->previewPixmapFactory, type);
    item->setWhatsThis(i18n("<b>Ilford Delta 100</b>:"
                            "<p>Simulate the Ilford Delta black and white film at 100 ISO.</p>"));

    ++type;
    item = new ListWidgetBWPreviewItem(d->bwFilm, i18n("Ilford Delta 400"), d->previewPixmapFactory, type);
    item->setWhatsThis(i18n("<b>Ilford Delta 400</b>:"
                            "<p>Simulate the Ilford Delta black and white film at 400 ISO.</p>"));

    ++type;
    item = new ListWidgetBWPreviewItem(d->bwFilm, i18n("Ilford Delta 400 Pro 3200"), d->previewPixmapFactory, type);
    item->setWhatsThis(i18n("<b>Ilford Delta 400 Pro 3200</b>:"
                            "<p>Simulate the Ilford Delta 400 Pro black and white film at 3200 ISO.</p>"));

    ++type;
    item = new ListWidgetBWPreviewItem(d->bwFilm, i18n("Ilford FP4 Plus"), d->previewPixmapFactory, type);
    item->setWhatsThis(i18n("<b>Ilford FP4 Plus</b>:"
                            "<p>Simulate the Ilford FP4 Plus black and white film at 125 ISO.</p>"));

    ++type;
    item = new ListWidgetBWPreviewItem(d->bwFilm, i18n("Ilford HP5 Plus"), d->previewPixmapFactory, type);
    item->setWhatsThis(i18n("<b>Ilford HP5 Plus</b>:"
                            "<p>Simulate the Ilford HP5 Plus black and white film at 400 ISO.</p>"));

    ++type;
    item = new ListWidgetBWPreviewItem(d->bwFilm, i18n("Ilford PanF Plus"), d->previewPixmapFactory, type);
    item->setWhatsThis(i18n("<b>Ilford PanF Plus</b>:"
                            "<p>Simulate the Ilford PanF Plus black and white film at 50 ISO.</p>"));

    ++type;
    item = new ListWidgetBWPreviewItem(d->bwFilm, i18n("Ilford XP2 Super"), d->previewPixmapFactory, type);
    item->setWhatsThis(i18n("<b>Ilford XP2 Super</b>:"
                            "<p>Simulate the Ilford XP2 Super black and white film at 400 ISO.</p>"));

    ++type;
    item = new ListWidgetBWPreviewItem(d->bwFilm, i18n("Kodak Tmax 100"), d->previewPixmapFactory, type);
    item->setWhatsThis(i18n("<b>Kodak Tmax 100</b>:"
                            "<p>Simulate the Kodak Tmax black and white film at 100 ISO.</p>"));

    ++type;
    item = new ListWidgetBWPreviewItem(d->bwFilm, i18n("Kodak Tmax 400"), d->previewPixmapFactory, type);
    item->setWhatsThis(i18n("<b>Kodak Tmax 400</b>:"
                            "<p>Simulate the Kodak Tmax black and white film at 400 ISO.</p>"));

    ++type;
    item = new ListWidgetBWPreviewItem(d->bwFilm, i18n("Kodak TriX"), d->previewPixmapFactory, type);
    item->setWhatsThis(i18n("<b>Kodak TriX</b>:"
                            "<p>Simulate the Kodak TriX black and white film at 400 ISO.</p>"));

    // -------------------------------------------------------------

    KVBox *vbox = new KVBox(d->tab);
    vbox->setSpacing(d->gboxSettings->spacingHint());

    d->bwFilters = new QListWidget(vbox);
    d->bwFilters->setIconSize(d->thumbnailImage.size());

    type = BWNoFilter;

    item = new ListWidgetBWPreviewItem(d->bwFilters, i18n("No Lens Filter"), d->previewPixmapFactory, type);
    item->setWhatsThis(i18n("<b>No Lens Filter</b>:"
                            "<p>Do not apply a lens filter when rendering the image.</p>"));

    ++type;
    item = new ListWidgetBWPreviewItem(d->bwFilters, i18n("Green Filter"), d->previewPixmapFactory, type);
    item->setWhatsThis(i18n("<b>Black & White with Green Filter</b>:"
                            "<p>Simulate black and white film exposure using a green filter. "
                            "This is useful for all scenic shoots, especially "
                            "portraits photographed against the sky.</p>"));

    ++type;
    item = new ListWidgetBWPreviewItem(d->bwFilters, i18n("Orange Filter"), d->previewPixmapFactory, type);
    item->setWhatsThis(i18n("<b>Black & White with Orange Filter</b>:"
                            "<p>Simulate black and white film exposure using an orange filter. "
                            "This will enhance landscapes, marine scenes and aerial "
                            "photography.</p>"));

    ++type;
    item = new ListWidgetBWPreviewItem(d->bwFilters, i18n("Red Filter"), d->previewPixmapFactory, type);
    item->setWhatsThis(i18n("<b>Black & White with Red Filter</b>:"
                            "<p>Simulate black and white film exposure using a red filter. "
                            "This creates dramatic sky effects, and simulates moonlight scenes "
                            "in the daytime.</p>"));

    ++type;
    item = new ListWidgetBWPreviewItem(d->bwFilters, i18n("Yellow Filter"), d->previewPixmapFactory, type);
    item->setWhatsThis(i18n("<b>Black & White with Yellow Filter</b>:"
                            "<p>Simulate black and white film exposure using a yellow filter. "
                            "This has the most natural tonal correction, and improves contrast. Ideal for "
                            "landscapes.</p>"));

    ++type;
    item = new ListWidgetBWPreviewItem(d->bwFilters, i18n("Yellow-Green Filter"), d->previewPixmapFactory, type);
    item->setWhatsThis(i18n("<b>Black & White with Yellow-Green Filter</b>:"
                            "<p>Simulate black and white film exposure using a yellow-green filter. "
                            "A yellow-green filter is highly effective for outdoor portraits because "
                            "red is rendered dark while green appears lighter. Great for correcting skin tones, "
                            "bringing out facial expressions in close-ups and emphasizing the feeling of liveliness. "
                            "This filter is highly effective for indoor portraits under tungsten lighting.</p>"));

    ++type;
    item = new ListWidgetBWPreviewItem(d->bwFilters, i18n("Blue Filter"), d->previewPixmapFactory, type);
    item->setWhatsThis(i18n("<b>Black & White with Blue Filter</b>:"
                            "<p>Simulate black and white film exposure using a blue filter. "
                            "This accentuates haze and fog. Used for dye transfer and contrast effects.</p>"));

    d->strengthInput = new RIntNumInput(vbox);
    d->strengthInput->input()->setLabel(i18n("Strength:"), Qt::AlignLeft | Qt::AlignVCenter);
    d->strengthInput->setRange(1, 5, 1);
    d->strengthInput->setSliderEnabled(true);
    d->strengthInput->setDefaultValue(1);
    d->strengthInput->setWhatsThis(i18n("Here, set the strength adjustment of the lens filter."));

    // -------------------------------------------------------------

    d->bwTone = new QListWidget(d->tab);
    d->bwTone->setIconSize(d->thumbnailImage.size());

    type = BWNoTone;

    item = new ListWidgetBWPreviewItem(d->bwTone, i18n("No Tone Filter"), d->previewPixmapFactory, type);
    item->setWhatsThis(i18n("<b>No Tone Filter</b>:"
                            "<p>Do not apply a tone filter to the image.</p>"));

    ++type;
    item = new ListWidgetBWPreviewItem(d->bwTone, i18n("Sepia Tone"), d->previewPixmapFactory, type);
    item->setWhatsThis(i18n("<b>Black & White with Sepia Tone</b>:"
                            "<p>Gives a warm highlight and mid-tone while adding a bit of coolness to "
                            "the shadows - very similar to the process of bleaching a print and "
                            "re-developing in a sepia toner.</p>"));

    ++type;
    item = new ListWidgetBWPreviewItem(d->bwTone, i18n("Brown Tone"), d->previewPixmapFactory, type);
    item->setWhatsThis(i18n("<b>Black & White with Brown Tone</b>:"
                            "<p>This filter is more neutral than the Sepia Tone "
                            "filter.</p>"));

    ++type;
    item = new ListWidgetBWPreviewItem(d->bwTone, i18n("Cold Tone"), d->previewPixmapFactory, type);
    item->setWhatsThis(i18n("<b>Black & White with Cold Tone</b>:"
                            "<p>Start subtly and replicate printing on a cold tone black and white "
                            "paper such as a bromide enlarging "
                            "paper.</p>"));

    ++type;
    item = new ListWidgetBWPreviewItem(d->bwTone, i18n("Selenium Tone"), d->previewPixmapFactory, type);
    item->setWhatsThis(i18n("<b>Black & White with Selenium Tone</b>:"
                            "<p>This effect replicates traditional selenium chemical toning done "
                            "in the darkroom.</p>"));

    ++type;
    item = new ListWidgetBWPreviewItem(d->bwTone, i18n("Platinum Tone"), d->previewPixmapFactory, type);
    item->setWhatsThis(i18n("<b>Black & White with Platinum Tone</b>:"
                            "<p>This effect replicates traditional platinum chemical toning done "
                            "in the darkroom.</p>"));

    ++type;
    item = new ListWidgetBWPreviewItem(d->bwTone, i18n("Green Tone"), d->previewPixmapFactory, type);
    item->setWhatsThis(i18n("<b>Black & White with greenish tint</b>:"
                            "<p>This effect is also known as Verdante.</p>"));

    // -------------------------------------------------------------

    QWidget* curveBox = new QWidget();
    d->curvesBox = new CurvesBox(256, 256, d->originalImage->bits(), d->originalImage->width(),
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
//    gridTab2->setRowMinimumHeight(3, d->gboxSettings->spacingHint());
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
    updatePreviews();
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
    delete [] d->destinationPreviewData;
}

void BWSepiaTool::updatePreviews()
{
    for (int i = 0 ; i < d->bwFilters->count(); ++i)
    {
        ListWidgetBWPreviewItem* item = dynamic_cast<ListWidgetBWPreviewItem*>(d->bwFilters->item(i));
        item->updatePreview();
    }

    for (int i = 0 ; i < d->bwFilm->count(); ++i)
    {
        ListWidgetBWPreviewItem* item = dynamic_cast<ListWidgetBWPreviewItem*>(d->bwFilm->item(i));
        item->updatePreview();
    }

    for (int i = 0 ; i < d->bwTone->count(); ++i)
    {
        ListWidgetBWPreviewItem* item = dynamic_cast<ListWidgetBWPreviewItem*>(d->bwTone->item(i));
        item->updatePreview();
    }
}

void BWSepiaTool::slotFilterSelected()
{
    int filter = d->bwFilters->currentRow();
    if (filter == BWNoFilter)
        d->strengthInput->setEnabled(false);
    else
        d->strengthInput->setEnabled(true);

    slotEffect();
}

QPixmap BWSepiaTool::getThumbnailForEffect(int type)
{
    DImg thumb = d->thumbnailImage.copy();
    int w      = thumb.width();
    int h      = thumb.height();
    bool sb    = thumb.sixteenBit();
    bool a     = thumb.hasAlpha();

    if (type < BWGeneric)
    {
        // In Filter view, we will render a preview of the B&W filter with the generic B&W film.
        blackAndWhiteConversion(thumb.bits(), w, h, sb, type);
        blackAndWhiteConversion(thumb.bits(), w, h, sb, BWGeneric);
    }
    else
    {
        // In Film and Tone view, we will render the preview without to use the B&W Filter
        blackAndWhiteConversion(thumb.bits(), w, h, sb, type);
    }

    if (d->curvesBox->curves())   // in case we're called before the creator is done
    {
        uchar *targetData = new uchar[w*h*(sb ? 8 : 4)];
        d->curvesBox->curves()->curvesLutSetup(AlphaChannel);
        d->curvesBox->curves()->curvesLutProcess(thumb.bits(), targetData, w, h);

        DImg preview(w, h, sb, a, targetData);
        BCGModifier cmod;
        cmod.setContrast((double)(d->cInput->value()/100.0) + 1.00);
        cmod.applyBCG(preview);

        thumb.putImageData(preview.bits());

        delete [] targetData;
    }
    return (thumb.convertToPixmap());
}

void BWSepiaTool::slotScaleChanged()
{
    d->curvesBox->setScale(d->gboxSettings->histogramBox()->scale());
}

void BWSepiaTool::slotSpotColorChanged(const DColor& color)
{
    d->curvesBox->setCurveGuide(color);
}

void BWSepiaTool::slotColorSelectedFromTarget( const DColor& color )
{
    d->gboxSettings->histogramBox()->histogram()->setHistogramGuideByColor(color);
}

void BWSepiaTool::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);

    d->tab->setCurrentIndex(group.readEntry(d->configSettingsTabEntry,           (int)BWFiltersTab));
    d->bwFilters->setCurrentRow(group.readEntry(d->configBWFilterEntry,          0));
    d->bwFilm->setCurrentRow(group.readEntry(d->configBWFilmEntry,               0));
    d->bwTone->setCurrentRow(group.readEntry(d->configBWToneEntry,               0));
    d->cInput->setValue(group.readEntry(d->configContrastAdjustmentEntry,        d->cInput->defaultValue()));
    d->strengthInput->setValue(group.readEntry(d->configStrengthAdjustmentEntry, d->strengthInput->defaultValue()));

    d->curvesBox->readCurveSettings(group);

    // we need to call the set methods here, otherwise the curve will not be updated correctly
    d->gboxSettings->histogramBox()->setChannel(group.readEntry(d->configHistogramChannelEntry,
                    (int)LuminosityChannel));
    d->gboxSettings->histogramBox()->setScale(group.readEntry(d->configHistogramScaleEntry,
                    (int)LogScaleHistogram));

    slotFilterSelected();
}

void BWSepiaTool::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);

    group.writeEntry(d->configSettingsTabEntry,        d->tab->currentIndex());
    group.writeEntry(d->configHistogramChannelEntry,   d->gboxSettings->histogramBox()->channel());
    group.writeEntry(d->configHistogramScaleEntry,     d->gboxSettings->histogramBox()->scale());
    group.writeEntry(d->configBWFilterEntry,           d->bwFilters->currentRow());
    group.writeEntry(d->configBWFilmEntry,             d->bwFilm->currentRow());
    group.writeEntry(d->configBWToneEntry,             d->bwTone->currentRow());
    group.writeEntry(d->configContrastAdjustmentEntry, d->cInput->value());
    group.writeEntry(d->configStrengthAdjustmentEntry, d->strengthInput->value());

    d->curvesBox->writeCurveSettings(group);

//    for (int j = 0 ; j < 17 ; ++j)
//    {
//        QPoint p = d->curvesBox->curves()->getCurvePoint(ValueChannel, j);
//
//        if (d->originalImage->sixteenBit() && p.x() != -1)
//        {
//            p.setX(p.x()/255);
//            p.setY(p.y()/255);
//        }
//
//        group.writeEntry(QString("CurveAdjustmentPoint%1").arg(j), p);
//    }

    d->previewWidget->writeSettings();
    group.sync();
}

void BWSepiaTool::slotResetSettings()
{
    blockWidgetSignals(true);

    d->bwFilters->setCurrentRow(0);
    d->bwFilm->setCurrentRow(0);
    d->bwTone->setCurrentRow(0);

    d->cInput->slotReset();
    d->strengthInput->slotReset();

    for (int channel = 0 ; channel < 5 ; ++channel)
       d->curvesBox->curves()->curvesChannelReset(channel);

    d->curvesBox->reset();

    blockWidgetSignals(false);

    d->gboxSettings->histogramBox()->histogram()->reset();
    d->previewPixmapFactory->invalidate();
    updatePreviews();
    d->bwFilters->update();
    d->bwTone->update();

    slotEffect();
}

void BWSepiaTool::slotEffect()
{
    kapp->setOverrideCursor( Qt::WaitCursor );

    d->gboxSettings->histogramBox()->histogram()->stopHistogramComputation();

    delete [] d->destinationPreviewData;

    ImageIface* iface        = d->previewWidget->imageIface();
    d->destinationPreviewData = iface->getPreviewImage();
    int w                    = iface->previewWidth();
    int h                    = iface->previewHeight();
    bool a                   = iface->previewHasAlpha();
    bool sb                  = iface->previewSixteenBit();

    // Apply black and white filter.

    blackAndWhiteConversion(d->destinationPreviewData, w, h, sb, d->bwFilters->currentRow());

    // Apply black and white film type.

    blackAndWhiteConversion(d->destinationPreviewData, w, h, sb, d->bwFilm->currentRow() + BWGeneric);

    // Apply color tone filter.

    blackAndWhiteConversion(d->destinationPreviewData, w, h, sb, d->bwTone->currentRow() + BWNoTone);

    // Calculate and apply the curve on image.

    uchar *targetData = new uchar[w*h*(sb ? 8 : 4)];
    d->curvesBox->curves()->curvesLutSetup(AlphaChannel);
    d->curvesBox->curves()->curvesLutProcess(d->destinationPreviewData, targetData, w, h);

    // Adjust contrast.

    DImg preview(w, h, sb, a, targetData);
    BCGModifier cmod;
    cmod.setContrast((double)(d->cInput->value()/100.0) + 1.00);
    cmod.applyBCG(preview);
    iface->putPreviewImage(preview.bits());

    d->previewWidget->updatePreview();

    // Update histogram.

    memcpy(d->destinationPreviewData, preview.bits(), preview.numBytes());
    d->gboxSettings->histogramBox()->histogram()->updateData(d->destinationPreviewData, w, h, sb, 0, 0, 0, false);
    delete [] targetData;

    kapp->restoreOverrideCursor();
}

void BWSepiaTool::finalRendering()
{
    kapp->setOverrideCursor( Qt::WaitCursor );
    ImageIface* iface = d->previewWidget->imageIface();
    uchar *data       = iface->getOriginalImage();
    int w             = iface->originalWidth();
    int h             = iface->originalHeight();
    bool a            = iface->originalHasAlpha();
    bool sb           = iface->originalSixteenBit();

    if (data)
    {
        // Apply black and white filter.

        blackAndWhiteConversion(data, w, h, sb, d->bwFilters->currentRow());

        // Apply black and white film type.

        blackAndWhiteConversion(data, w, h, sb, d->bwFilm->currentRow() + BWGeneric);

        // Apply color tone filter.

        blackAndWhiteConversion(data, w, h, sb, d->bwTone->currentRow() + BWNoTone);

        // Calculate and apply the curve on image.

        uchar *targetData = new uchar[w*h*(sb ? 8 : 4)];
        d->curvesBox->curves()->curvesLutSetup(AlphaChannel);
        d->curvesBox->curves()->curvesLutProcess(data, targetData, w, h);

        // Adjust contrast.

        DImg img(w, h, sb, a, targetData);
        BCGModifier cmod;
        cmod.setContrast((double)(d->cInput->value()/100.0) + 1.00);
        cmod.applyBCG(img);

        iface->putOriginalImage(i18n("Convert to Black && White"), img.bits());

        delete [] data;
        delete [] targetData;
    }

    kapp->restoreOverrideCursor();
}

void BWSepiaTool::blackAndWhiteConversion(uchar *data, int w, int h, bool sb, int type)
{
    // Value to multiply RGB 8 bits component of mask used by changeTonality() method.
    int mul = sb ? 255 : 1;
    DImgImageFilters filter;
    double strength = 1.0 + ((double)d->strengthInput->value() - 1.0) * (1.0 / 3.0);

    switch (type)
    {
       case BWNoFilter:
          d->redAttn   = 0.0;
          d->greenAttn = 0.0;
          d->blueAttn  = 0.0;
          break;

       case BWGreenFilter:
          d->redAttn   = -0.20 * strength;
          d->greenAttn = +0.11 * strength;
          d->blueAttn  = +0.09 * strength;
          break;

       case BWOrangeFilter:
          d->redAttn   = +0.48 * strength;
          d->greenAttn = -0.37 * strength;
          d->blueAttn  = -0.11 * strength;
          break;

       case BWRedFilter:
          d->redAttn   = +0.60 * strength;
          d->greenAttn = -0.49 * strength;
          d->blueAttn  = -0.11 * strength;
          break;

       case BWYellowFilter:
          d->redAttn   = +0.30 * strength;
          d->greenAttn = -0.31 * strength;
          d->blueAttn  = +0.01 * strength;
          break;

       case BWYellowGreenFilter:
          d->redAttn   = +0.25 * strength;
          d->greenAttn = +0.65 * strength;
          d->blueAttn  = +0.15 * strength;
          break;

       case BWBlueFilter:
          d->redAttn   = +0.15 * strength;
          d->greenAttn = +0.15 * strength;
          d->blueAttn  = +0.80 * strength;
          break;

       // --------------------------------------------------------------------------------

       case BWGeneric:
       case BWNoTone:
          d->redMult   = 0.24;
          d->greenMult = 0.68;
          d->blueMult  = 0.08;
          filter.channelMixerImage(data, w, h, sb, true, true,
                 d->redMult + d->redMult*d->redAttn, d->greenMult + d->greenMult*d->greenAttn, d->blueMult + d->blueMult*d->blueAttn,
                 0.0, 1.0, 0.0,
                 0.0, 0.0, 1.0);
          break;

       case BWAgfa200X:
          d->redMult   = 0.18;
          d->greenMult = 0.41;
          d->blueMult  = 0.41;
          filter.channelMixerImage(data, w, h, sb, true, true,
                 d->redMult + d->redMult*d->redAttn, d->greenMult + d->greenMult*d->greenAttn, d->blueMult + d->blueMult*d->blueAttn,
                 0.0, 1.0, 0.0,
                 0.0, 0.0, 1.0);
          break;

       case BWAgfapan25:
          d->redMult   = 0.25;
          d->greenMult = 0.39;
          d->blueMult  = 0.36;
          filter.channelMixerImage(data, w, h, sb, true, true,
                 d->redMult + d->redMult*d->redAttn, d->greenMult + d->greenMult*d->greenAttn, d->blueMult + d->blueMult*d->blueAttn,
                 0.0, 1.0, 0.0,
                 0.0, 0.0, 1.0);
          break;

       case BWAgfapan100:
          d->redMult   = 0.21;
          d->greenMult = 0.40;
          d->blueMult  = 0.39;
          filter.channelMixerImage(data, w, h, sb, true, true,
                 d->redMult + d->redMult*d->redAttn, d->greenMult + d->greenMult*d->greenAttn, d->blueMult + d->blueMult*d->blueAttn,
                 0.0, 1.0, 0.0,
                 0.0, 0.0, 1.0);
          break;

       case BWAgfapan400:
          d->redMult   = 0.20;
          d->greenMult = 0.41;
          d->blueMult  = 0.39;
          filter.channelMixerImage(data, w, h, sb, true, true,
                 d->redMult + d->redMult*d->redAttn, d->greenMult + d->greenMult*d->greenAttn, d->blueMult + d->blueMult*d->blueAttn,
                 0.0, 1.0, 0.0,
                 0.0, 0.0, 1.0);
          break;

       case BWIlfordDelta100:
          d->redMult   = 0.21;
          d->greenMult = 0.42;
          d->blueMult  = 0.37;
          filter.channelMixerImage(data, w, h, sb, true, true,
                 d->redMult + d->redMult*d->redAttn, d->greenMult + d->greenMult*d->greenAttn, d->blueMult + d->blueMult*d->blueAttn,
                 0.0, 1.0, 0.0,
                 0.0, 0.0, 1.0);
          break;

       case BWIlfordDelta400:
          d->redMult   = 0.22;
          d->greenMult = 0.42;
          d->blueMult  = 0.36;
          filter.channelMixerImage(data, w, h, sb, true, true,
                 d->redMult + d->redMult*d->redAttn, d->greenMult + d->greenMult*d->greenAttn, d->blueMult + d->blueMult*d->blueAttn,
                 0.0, 1.0, 0.0,
                 0.0, 0.0, 1.0);
          break;

       case BWIlfordDelta400Pro3200:
          d->redMult   = 0.31;
          d->greenMult = 0.36;
          d->blueMult  = 0.33;
          filter.channelMixerImage(data, w, h, sb, true, true,
                 d->redMult + d->redMult*d->redAttn, d->greenMult + d->greenMult*d->greenAttn, d->blueMult + d->blueMult*d->blueAttn,
                 0.0, 1.0, 0.0,
                 0.0, 0.0, 1.0);
          break;

       case BWIlfordFP4:
          d->redMult   = 0.28;
          d->greenMult = 0.41;
          d->blueMult  = 0.31;
          filter.channelMixerImage(data, w, h, sb, true, true,
                 d->redMult + d->redMult*d->redAttn, d->greenMult + d->greenMult*d->greenAttn, d->blueMult + d->blueMult*d->blueAttn,
                 0.0, 1.0, 0.0,
                 0.0, 0.0, 1.0);
          break;

       case BWIlfordHP5:
          d->redMult   = 0.23;
          d->greenMult = 0.37;
          d->blueMult  = 0.40;
          filter.channelMixerImage(data, w, h, sb, true, true,
                 d->redMult + d->redMult*d->redAttn, d->greenMult + d->greenMult*d->greenAttn, d->blueMult + d->blueMult*d->blueAttn,
                 0.0, 1.0, 0.0,
                 0.0, 0.0, 1.0);
          break;

       case BWIlfordPanF:
          d->redMult   = 0.33;
          d->greenMult = 0.36;
          d->blueMult  = 0.31;
          filter.channelMixerImage(data, w, h, sb, true, true,
                 d->redMult + d->redMult*d->redAttn, d->greenMult + d->greenMult*d->greenAttn, d->blueMult + d->blueMult*d->blueAttn,
                 0.0, 1.0, 0.0,
                 0.0, 0.0, 1.0);
          break;

       case BWIlfordXP2Super:
          d->redMult   = 0.21;
          d->greenMult = 0.42;
          d->blueMult  = 0.37;
          filter.channelMixerImage(data, w, h, sb, true, true,
                 d->redMult + d->redMult*d->redAttn, d->greenMult + d->greenMult*d->greenAttn, d->blueMult + d->blueMult*d->blueAttn,
                 0.0, 1.0, 0.0,
                 0.0, 0.0, 1.0);
          break;

       case BWKodakTmax100:
          d->redMult   = 0.24;
          d->greenMult = 0.37;
          d->blueMult  = 0.39;
          filter.channelMixerImage(data, w, h, sb, true, true,
                 d->redMult + d->redMult*d->redAttn, d->greenMult + d->greenMult*d->greenAttn, d->blueMult + d->blueMult*d->blueAttn,
                 0.0, 1.0, 0.0,
                 0.0, 0.0, 1.0);
          break;

       case BWKodakTmax400:
          d->redMult   = 0.27;
          d->greenMult = 0.36;
          d->blueMult  = 0.37;
          filter.channelMixerImage(data, w, h, sb, true, true,
                 d->redMult + d->redMult*d->redAttn, d->greenMult + d->greenMult*d->greenAttn, d->blueMult + d->blueMult*d->blueAttn,
                 0.0, 1.0, 0.0,
                 0.0, 0.0, 1.0);
          break;

       case BWKodakTriX:
          d->redMult   = 0.25;
          d->greenMult = 0.35;
          d->blueMult  = 0.40;
          filter.channelMixerImage(data, w, h, sb, true, true,
                 d->redMult + d->redMult*d->redAttn, d->greenMult + d->greenMult*d->greenAttn, d->blueMult + d->blueMult*d->blueAttn,
                 0.0, 1.0, 0.0,
                 0.0, 0.0, 1.0);
          break;

       // --------------------------------------------------------------------------------

       case BWSepiaTone:
          filter.changeTonality(data, w, h, sb, 162*mul, 132*mul, 101*mul);
          break;

       case BWBrownTone:
          filter.changeTonality(data, w, h, sb, 129*mul, 115*mul, 104*mul);
          break;

       case BWColdTone:
          filter.changeTonality(data, w, h, sb, 102*mul, 109*mul, 128*mul);
          break;

       case BWSeleniumTone:
          filter.changeTonality(data, w, h, sb, 122*mul, 115*mul, 122*mul);
          break;

       case BWPlatinumTone:
          filter.changeTonality(data, w, h, sb, 115*mul, 110*mul, 106*mul);
          break;

       case BWGreenTone:
          filter.changeTonality(data, w, h, sb, 125*mul, 125*mul, 105*mul);
          break;
    }
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

        d->bwFilters->setCurrentRow(stream.readLine().toInt());
        d->bwTone->setCurrentRow(stream.readLine().toInt());
        d->cInput->setValue(stream.readLine().toInt());

        for (int i = 0 ; i < 5 ; ++i)
            d->curvesBox->curves()->curvesChannelReset(i);

        d->curvesBox->curves()->setCurveType(d->curvesBox->channel(), ImageCurves::CURVE_SMOOTH);
        d->curvesBox->reset();

        QPoint disable(-1, -1);
        QPoint p;
        for (int j = 0 ; j < 17 ; ++j)
        {
            p.setX( stream.readLine().toInt() );
            p.setY( stream.readLine().toInt() );

            if (d->originalImage->sixteenBit() && p != disable)
            {
                p.setX(p.x()*255);
                p.setY(p.y()*255);
            }

            d->curvesBox->curves()->setCurvePoint(LuminosityChannel, j, p);
        }

        for (int i = 0 ; i < 5 ; ++i)
            d->curvesBox->curves()->curvesCalculateCurve(i);

        blockWidgetSignals(false);

        d->gboxSettings->histogramBox()->histogram()->reset();
        d->previewPixmapFactory->invalidate();
        updatePreviews();
        d->bwFilters->update();
        d->bwTone->update();

        slotEffect();
    }
    else
        KMessageBox::error(kapp->activeWindow(), i18n("Cannot load settings from the Black & White text file."));

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
        stream << d->bwFilters->currentItem() << "\n";
        stream << d->bwTone->currentItem() << "\n";
        stream << d->cInput->value() << "\n";

        for (int j = 0 ; j < 17 ; ++j)
        {
            QPoint p = d->curvesBox->curves()->getCurvePoint(LuminosityChannel, j);
            if (d->originalImage->sixteenBit())
            {
                p.setX(p.x()/255);
                p.setY(p.y()/255);
            }
            stream << p.x() << "\n";
            stream << p.y() << "\n";
        }
    }
    else
        KMessageBox::error(kapp->activeWindow(),
                           i18n("Cannot save settings to the Black & White text file."));

    file.close();
}

void BWSepiaTool::blockWidgetSignals(bool b)
{
    d->bwFilm->blockSignals(b);
    d->bwFilters->blockSignals(b);
    d->bwTone->blockSignals(b);
    d->cInput->blockSignals(b);
    d->strengthInput->blockSignals(b);
}

}  // namespace DigikamImagesPluginCore
