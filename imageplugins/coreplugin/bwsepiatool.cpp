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

using namespace KDcrawIface;

namespace DigikamImagesPluginCore
{

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
    foreach (QPixmap* pm, m_previewPixmapMap.values()) // krazy:exclude=foreach
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

class BWPreviewItem : public QListWidgetItem
{

public:

    BWPreviewItem(QListWidget* parent, const QString& text,
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
    const QString                configCurveEntry;

    uchar*                       destinationPreviewData;

    QListWidget*                 bwFilters;
    QListWidget*                 bwFilm;
    QListWidget*                 bwTone;

    KTabWidget*                  tab;

    KDcrawIface::RIntNumInput*   cInput;
    KDcrawIface::RIntNumInput*   strengthInput;

    ImageGuideWidget*            previewWidget;

    CurvesBox*                   curvesBox;

    EditorToolSettings*          gboxSettings;

    DImg*                        originalImage;
    DImg                         thumbnailImage;

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

    d->tab    = new KTabWidget(d->gboxSettings->plainPage());

    d->bwFilm = new QListWidget(d->tab);
    d->bwFilm->setIconSize(d->thumbnailImage.size());
    d->previewPixmapFactory = new PreviewPixmapFactory(this, d->thumbnailImage.size());

    int type            = BWSepiaContainer::BWGeneric;
    BWPreviewItem* item = 0;

    item = new BWPreviewItem(d->bwFilm, i18nc("generic black and white film", "Generic"), d->previewPixmapFactory, type);
    item->setWhatsThis(i18n("<b>Generic</b>:"
                            "<p>Simulate a generic black and white film.</p>"));

    ++type;
    item = new BWPreviewItem(d->bwFilm, i18n("Agfa 200X"), d->previewPixmapFactory, type);
    item->setWhatsThis(i18n("<b>Agfa 200X</b>:"
                            "<p>Simulate the Agfa 200X black and white film at 200 ISO.</p>"));

    ++type;
    item = new BWPreviewItem(d->bwFilm, i18n("Agfa Pan 25"), d->previewPixmapFactory, type);
    item->setWhatsThis(i18n("<b>Agfa Pan 25</b>:"
                            "<p>Simulate the Agfa Pan black and white film at 25 ISO.</p>"));

    ++type;
    item = new BWPreviewItem(d->bwFilm, i18n("Agfa Pan 100"), d->previewPixmapFactory, type);
    item->setWhatsThis(i18n("<b>Agfa Pan 100</b>:"
                            "<p>Simulate the Agfa Pan black and white film at 100 ISO.</p>"));

    ++type;
    item = new BWPreviewItem(d->bwFilm, i18n("Agfa Pan 400"), d->previewPixmapFactory, type);
    item->setWhatsThis(i18n("<b>Agfa Pan 400</b>:"
                            "<p>Simulate the Agfa Pan black and white film at 400 ISO.</p>"));

    ++type;
    item = new BWPreviewItem(d->bwFilm, i18n("Ilford Delta 100"), d->previewPixmapFactory, type);
    item->setWhatsThis(i18n("<b>Ilford Delta 100</b>:"
                            "<p>Simulate the Ilford Delta black and white film at 100 ISO.</p>"));

    ++type;
    item = new BWPreviewItem(d->bwFilm, i18n("Ilford Delta 400"), d->previewPixmapFactory, type);
    item->setWhatsThis(i18n("<b>Ilford Delta 400</b>:"
                            "<p>Simulate the Ilford Delta black and white film at 400 ISO.</p>"));

    ++type;
    item = new BWPreviewItem(d->bwFilm, i18n("Ilford Delta 400 Pro 3200"), d->previewPixmapFactory, type);
    item->setWhatsThis(i18n("<b>Ilford Delta 400 Pro 3200</b>:"
                            "<p>Simulate the Ilford Delta 400 Pro black and white film at 3200 ISO.</p>"));

    ++type;
    item = new BWPreviewItem(d->bwFilm, i18n("Ilford FP4 Plus"), d->previewPixmapFactory, type);
    item->setWhatsThis(i18n("<b>Ilford FP4 Plus</b>:"
                            "<p>Simulate the Ilford FP4 Plus black and white film at 125 ISO.</p>"));

    ++type;
    item = new BWPreviewItem(d->bwFilm, i18n("Ilford HP5 Plus"), d->previewPixmapFactory, type);
    item->setWhatsThis(i18n("<b>Ilford HP5 Plus</b>:"
                            "<p>Simulate the Ilford HP5 Plus black and white film at 400 ISO.</p>"));

    ++type;
    item = new BWPreviewItem(d->bwFilm, i18n("Ilford PanF Plus"), d->previewPixmapFactory, type);
    item->setWhatsThis(i18n("<b>Ilford PanF Plus</b>:"
                            "<p>Simulate the Ilford PanF Plus black and white film at 50 ISO.</p>"));

    ++type;
    item = new BWPreviewItem(d->bwFilm, i18n("Ilford XP2 Super"), d->previewPixmapFactory, type);
    item->setWhatsThis(i18n("<b>Ilford XP2 Super</b>:"
                            "<p>Simulate the Ilford XP2 Super black and white film at 400 ISO.</p>"));

    ++type;
    item = new BWPreviewItem(d->bwFilm, i18n("Kodak Tmax 100"), d->previewPixmapFactory, type);
    item->setWhatsThis(i18n("<b>Kodak Tmax 100</b>:"
                            "<p>Simulate the Kodak Tmax black and white film at 100 ISO.</p>"));

    ++type;
    item = new BWPreviewItem(d->bwFilm, i18n("Kodak Tmax 400"), d->previewPixmapFactory, type);
    item->setWhatsThis(i18n("<b>Kodak Tmax 400</b>:"
                            "<p>Simulate the Kodak Tmax black and white film at 400 ISO.</p>"));

    ++type;
    item = new BWPreviewItem(d->bwFilm, i18n("Kodak TriX"), d->previewPixmapFactory, type);
    item->setWhatsThis(i18n("<b>Kodak TriX</b>:"
                            "<p>Simulate the Kodak TriX black and white film at 400 ISO.</p>"));

    // -------------------------------------------------------------

    type = BWSepiaContainer::BWIlfordSFX200;

    item = new BWPreviewItem(d->bwFilm, i18n("Ilford SPX 200"), d->previewPixmapFactory, type);
    item->setWhatsThis(i18n("<b>Ilford SPX 200</b>:"
                            "<p>Simulate the Ilford SPX infrared film at 200 ISO.</p>"));

    ++type;
    item = new BWPreviewItem(d->bwFilm, i18n("Ilford SPX 400"), d->previewPixmapFactory, type);
    item->setWhatsThis(i18n("<b>Ilford SPX 400</b>:"
                            "<p>Simulate the Ilford SPX infrared film at 400 ISO.</p>"));

    ++type;
    item = new BWPreviewItem(d->bwFilm, i18n("Ilford SPX 800"), d->previewPixmapFactory, type);
    item->setWhatsThis(i18n("<b>Ilford SPX 800</b>:"
                            "<p>Simulate the Ilford SPX infrared film at 800 ISO.</p>"));

    // -------------------------------------------------------------

    KVBox* vbox = new KVBox(d->tab);
    vbox->setSpacing(d->gboxSettings->spacingHint());

    d->bwFilters = new QListWidget(vbox);
    d->bwFilters->setIconSize(d->thumbnailImage.size());

    type = BWSepiaContainer::BWNoFilter;

    item = new BWPreviewItem(d->bwFilters, i18n("No Lens Filter"), d->previewPixmapFactory, type);
    item->setWhatsThis(i18n("<b>No Lens Filter</b>:"
                            "<p>Do not apply a lens filter when rendering the image.</p>"));

    ++type;
    item = new BWPreviewItem(d->bwFilters, i18n("Green Filter"), d->previewPixmapFactory, type);
    item->setWhatsThis(i18n("<b>Black & White with Green Filter</b>:"
                            "<p>Simulate black and white film exposure using a green filter. "
                            "This is useful for all scenic shoots, especially "
                            "portraits photographed against the sky.</p>"));

    ++type;
    item = new BWPreviewItem(d->bwFilters, i18n("Orange Filter"), d->previewPixmapFactory, type);
    item->setWhatsThis(i18n("<b>Black & White with Orange Filter</b>:"
                            "<p>Simulate black and white film exposure using an orange filter. "
                            "This will enhance landscapes, marine scenes and aerial "
                            "photography.</p>"));

    ++type;
    item = new BWPreviewItem(d->bwFilters, i18n("Red Filter"), d->previewPixmapFactory, type);
    item->setWhatsThis(i18n("<b>Black & White with Red Filter</b>:"
                            "<p>Simulate black and white film exposure using a red filter. "
                            "This creates dramatic sky effects, and simulates moonlight scenes "
                            "in the daytime.</p>"));

    ++type;
    item = new BWPreviewItem(d->bwFilters, i18n("Yellow Filter"), d->previewPixmapFactory, type);
    item->setWhatsThis(i18n("<b>Black & White with Yellow Filter</b>:"
                            "<p>Simulate black and white film exposure using a yellow filter. "
                            "This has the most natural tonal correction, and improves contrast. Ideal for "
                            "landscapes.</p>"));

    ++type;
    item = new BWPreviewItem(d->bwFilters, i18n("Yellow-Green Filter"), d->previewPixmapFactory, type);
    item->setWhatsThis(i18n("<b>Black & White with Yellow-Green Filter</b>:"
                            "<p>Simulate black and white film exposure using a yellow-green filter. "
                            "A yellow-green filter is highly effective for outdoor portraits because "
                            "red is rendered dark while green appears lighter. Great for correcting skin tones, "
                            "bringing out facial expressions in close-ups and emphasizing the feeling of liveliness. "
                            "This filter is highly effective for indoor portraits under tungsten lighting.</p>"));

    ++type;
    item = new BWPreviewItem(d->bwFilters, i18n("Blue Filter"), d->previewPixmapFactory, type);
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

    type = BWSepiaContainer::BWNoTone;

    item = new BWPreviewItem(d->bwTone, i18n("No Tone Filter"), d->previewPixmapFactory, type);
    item->setWhatsThis(i18n("<b>No Tone Filter</b>:"
                            "<p>Do not apply a tone filter to the image.</p>"));

    ++type;
    item = new BWPreviewItem(d->bwTone, i18n("Sepia Tone"), d->previewPixmapFactory, type);
    item->setWhatsThis(i18n("<b>Black & White with Sepia Tone</b>:"
                            "<p>Gives a warm highlight and mid-tone while adding a bit of coolness to "
                            "the shadows - very similar to the process of bleaching a print and "
                            "re-developing in a sepia toner.</p>"));

    ++type;
    item = new BWPreviewItem(d->bwTone, i18n("Brown Tone"), d->previewPixmapFactory, type);
    item->setWhatsThis(i18n("<b>Black & White with Brown Tone</b>:"
                            "<p>This filter is more neutral than the Sepia Tone "
                            "filter.</p>"));

    ++type;
    item = new BWPreviewItem(d->bwTone, i18n("Cold Tone"), d->previewPixmapFactory, type);
    item->setWhatsThis(i18n("<b>Black & White with Cold Tone</b>:"
                            "<p>Start subtly and replicate printing on a cold tone black and white "
                            "paper such as a bromide enlarging "
                            "paper.</p>"));

    ++type;
    item = new BWPreviewItem(d->bwTone, i18n("Selenium Tone"), d->previewPixmapFactory, type);
    item->setWhatsThis(i18n("<b>Black & White with Selenium Tone</b>:"
                            "<p>This effect replicates traditional selenium chemical toning done "
                            "in the darkroom.</p>"));

    ++type;
    item = new BWPreviewItem(d->bwTone, i18n("Platinum Tone"), d->previewPixmapFactory, type);
    item->setWhatsThis(i18n("<b>Black & White with Platinum Tone</b>:"
                            "<p>This effect replicates traditional platinum chemical toning done "
                            "in the darkroom.</p>"));

    ++type;
    item = new BWPreviewItem(d->bwTone, i18n("Green Tone"), d->previewPixmapFactory, type);
    item->setWhatsThis(i18n("<b>Black & White with greenish tint</b>:"
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
        BWPreviewItem* item = dynamic_cast<BWPreviewItem*>(d->bwFilters->item(i));
        item->updatePreview();
    }

    for (int i = 0 ; i < d->bwFilm->count(); ++i)
    {
        BWPreviewItem* item = dynamic_cast<BWPreviewItem*>(d->bwFilm->item(i));
        item->updatePreview();
    }

    for (int i = 0 ; i < d->bwTone->count(); ++i)
    {
        BWPreviewItem* item = dynamic_cast<BWPreviewItem*>(d->bwTone->item(i));
        item->updatePreview();
    }
}

void BWSepiaTool::slotFilterSelected()
{
    int filter = d->bwFilters->currentRow();
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
    d->bwFilters->setCurrentRow(group.readEntry(d->configBWFilterEntry,          0));
    d->bwFilm->setCurrentRow(group.readEntry(d->configBWFilmEntry,               0));
    d->bwTone->setCurrentRow(group.readEntry(d->configBWToneEntry,               0));
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
    group.writeEntry(d->configBWFilterEntry,           d->bwFilters->currentRow());
    group.writeEntry(d->configBWFilmEntry,             d->bwFilm->currentRow());
    group.writeEntry(d->configBWToneEntry,             d->bwTone->currentRow());
    group.writeEntry(d->configContrastAdjustmentEntry, d->cInput->value());
    group.writeEntry(d->configStrengthAdjustmentEntry, d->strengthInput->value());

    d->curvesBox->writeCurveSettings(group, d->configCurveEntry);

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

QPixmap BWSepiaTool::getThumbnailForEffect(int type)
{
    BWSepiaContainer prm;
    DImg thumb      = d->thumbnailImage.copy();
    prm.preview     = true;
    prm.previewType = type;
    BWSepiaFilter bw(&thumb, 0L, prm);
    bw.startFilterDirectly();
    return (bw.getTargetImage().convertToPixmap());
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
    prm.filmType        = d->bwFilm->currentRow();
    prm.filterType      = d->bwFilters->currentRow();
    prm.toneType        = d->bwTone->currentRow();
    prm.curves          = d->curvesBox->curves();
    prm.bcgPrm.contrast = ((double)(d->cInput->value()/100.0) + 1.00);
    prm.strength        = 1.0 + ((double)d->strengthInput->value() - 1.0) * (1.0 / 3.0); 
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
    ImageIface* iface = d->previewWidget->imageIface();
    uchar *data       = iface->getOriginalImage();
    int w             = iface->originalWidth();
    int h             = iface->originalHeight();
    bool sb           = iface->originalSixteenBit();

    if (data)
    {
        BWSepiaContainer prm;
        prm.filmType        = d->bwFilm->currentRow();
        prm.filterType      = d->bwFilters->currentRow();
        prm.toneType        = d->bwTone->currentRow();
        prm.curves          = d->curvesBox->curves();
        prm.bcgPrm.contrast = ((double)(d->cInput->value()/100.0) + 1.00);
        prm.strength        = 1.0 + ((double)d->strengthInput->value() - 1.0) * (1.0 / 3.0); 
        BWSepiaFilter bw(d->destinationPreviewData, w, h, sb, prm);
        
        iface->putPreviewImage(bw.getTargetImage().bits());      

        iface->putOriginalImage(i18n("Convert to Black && White"), bw.getTargetImage().bits());
    }

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

        d->bwFilters->setCurrentRow(stream.readLine().toInt());
        d->bwTone->setCurrentRow(stream.readLine().toInt());
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
        d->previewPixmapFactory->invalidate();
        updatePreviews();
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
        stream << d->bwFilters->currentItem() << "\n";
        stream << d->bwTone->currentItem() << "\n";
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
