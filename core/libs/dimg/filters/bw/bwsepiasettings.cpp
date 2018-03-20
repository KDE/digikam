/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-02-23
 * Description : black and white settings view.
 *
 * Copyright (C) 2010-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "bwsepiasettings.h"

// Qt includes

#include <QGridLayout>
#include <QLabel>
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QCheckBox>
#include <QStandardPaths>
#include <QApplication>
#include <QStyle>
#include <QUrl>
#include <QMessageBox>
#include <QIcon>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dlayoutbox.h"
#include "dexpanderbox.h"
#include "dfiledialog.h"
#include "dnuminput.h"
#include "digikam_debug.h"
#include "previewlist.h"
#include "curvesbox.h"
#include "curveswidget.h"
#include "imagecurves.h"

namespace Digikam
{

class BWSepiaSettings::Private
{

public:

    enum SettingsTab
    {
        FilmTab = 0,
        BWFiltersTab,
        ToneTab,
        LuminosityTab
    };

public:

    Private() :
        bwFilters(0),
        bwFilm(0),
        bwTone(0),
        tab(0),
        cInput(0),
        strengthInput(0),
        curvesBox(0)
    {
    }

    static const QString       configSettingsTabEntry;
    static const QString       configBWFilterEntry;
    static const QString       configBWFilmEntry;
    static const QString       configBWToneEntry;
    static const QString       configContrastAdjustmentEntry;
    static const QString       configStrengthAdjustmentEntry;
    static const QString       configCurveEntry;

    PreviewList*               bwFilters;
    PreviewList*               bwFilm;
    PreviewList*               bwTone;

    DExpanderBoxExclusive*     tab;

    DIntNumInput*              cInput;
    DIntNumInput*              strengthInput;

    CurvesBox*                 curvesBox;

    DImg                       thumbImage;

public:

    PreviewListItem* addItem(PreviewList* const list, const QString& name,
                             BWSepiaContainer::BlackWhiteConversionType type);
};

const QString BWSepiaSettings::Private::configSettingsTabEntry(QLatin1String("Settings Tab"));
const QString BWSepiaSettings::Private::configBWFilterEntry(QLatin1String("BW Filter"));
const QString BWSepiaSettings::Private::configBWFilmEntry(QLatin1String("BW Film"));
const QString BWSepiaSettings::Private::configBWToneEntry(QLatin1String("BW Tone"));
const QString BWSepiaSettings::Private::configContrastAdjustmentEntry(QLatin1String("ContrastValueAdjustment"));
const QString BWSepiaSettings::Private::configStrengthAdjustmentEntry(QLatin1String("StrengthAdjustment"));
const QString BWSepiaSettings::Private::configCurveEntry(QLatin1String("BWSepiaCurve"));

PreviewListItem* BWSepiaSettings::Private::addItem(PreviewList* const list,
                                                   const QString& name,
                                                   BWSepiaContainer::BlackWhiteConversionType type)
{
    BWSepiaFilter* const filter = new BWSepiaFilter(&thumbImage, 0, BWSepiaContainer(type));
    PreviewListItem* const item = list->addItem(filter, name, type);
    return item;
}

// --------------------------------------------------------

BWSepiaSettings::BWSepiaSettings(QWidget* const parent, DImg* const img)
    : QWidget(parent),
      d(new Private)
{
    if (!img->isNull())
    {
        d->thumbImage = img->smoothScale(128, 128, Qt::KeepAspectRatio);
    }
    else
    {
        d->thumbImage = DImg(QIcon::fromTheme(QLatin1String("view-preview")).pixmap(128).toImage());
    }

    const int spacing = QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);

    QGridLayout* const grid = new QGridLayout(parent);

    d->tab = new DExpanderBoxExclusive(this);

    PreviewListItem* item;
    d->bwFilm = new PreviewList;

    item = d->addItem(d->bwFilm, i18nc("generic black and white film", "Generic"), BWSepiaContainer::BWGeneric);
    item->setWhatsThis(i18n("<b>Generic</b>:"
                            "<p>Simulate a generic black and white film.</p>"));

    item = d->addItem(d->bwFilm, i18n("Agfa 200X"), BWSepiaContainer::BWAgfa200X);
    item->setWhatsThis(i18n("<b>Agfa 200X</b>:"
                            "<p>Simulate the Agfa 200X black and white film at 200 ISO.</p>"));

    item = d->addItem(d->bwFilm, i18n("Agfa Pan 25"), BWSepiaContainer::BWAgfapan25);
    item->setWhatsThis(i18n("<b>Agfa Pan 25</b>:"
                            "<p>Simulate the Agfa Pan black and white film at 25 ISO.</p>"));

    item = d->addItem(d->bwFilm, i18n("Agfa Pan 100"), BWSepiaContainer::BWAgfapan100);
    item->setWhatsThis(i18n("<b>Agfa Pan 100</b>:"
                            "<p>Simulate the Agfa Pan black and white film at 100 ISO.</p>"));

    item = d->addItem(d->bwFilm, i18n("Agfa Pan 400"), BWSepiaContainer::BWAgfapan400);
    item->setWhatsThis(i18n("<b>Agfa Pan 400</b>:"
                            "<p>Simulate the Agfa Pan black and white film at 400 ISO.</p>"));

    item = d->addItem(d->bwFilm, i18n("Ilford Delta 100"), BWSepiaContainer::BWIlfordDelta100);
    item->setWhatsThis(i18n("<b>Ilford Delta 100</b>:"
                            "<p>Simulate the Ilford Delta black and white film at 100 ISO.</p>"));

    item = d->addItem(d->bwFilm, i18n("Ilford Delta 400"), BWSepiaContainer::BWIlfordDelta400);
    item->setWhatsThis(i18n("<b>Ilford Delta 400</b>:"
                            "<p>Simulate the Ilford Delta black and white film at 400 ISO.</p>"));

    item = d->addItem(d->bwFilm, i18n("Ilford Delta 400 Pro 3200"), BWSepiaContainer::BWIlfordDelta400Pro3200);
    item->setWhatsThis(i18n("<b>Ilford Delta 400 Pro 3200</b>:"
                            "<p>Simulate the Ilford Delta 400 Pro black and white film at 3200 ISO.</p>"));

    item = d->addItem(d->bwFilm, i18n("Ilford FP4 Plus"), BWSepiaContainer::BWIlfordFP4);
    item->setWhatsThis(i18n("<b>Ilford FP4 Plus</b>:"
                            "<p>Simulate the Ilford FP4 Plus black and white film at 125 ISO.</p>"));

    item = d->addItem(d->bwFilm, i18n("Ilford HP5 Plus"), BWSepiaContainer::BWIlfordHP5);
    item->setWhatsThis(i18n("<b>Ilford HP5 Plus</b>:"
                            "<p>Simulate the Ilford HP5 Plus black and white film at 400 ISO.</p>"));

    item = d->addItem(d->bwFilm, i18n("Ilford PanF Plus"), BWSepiaContainer::BWIlfordPanF);
    item->setWhatsThis(i18n("<b>Ilford PanF Plus</b>:"
                            "<p>Simulate the Ilford PanF Plus black and white film at 50 ISO.</p>"));

    item = d->addItem(d->bwFilm, i18n("Ilford XP2 Super"), BWSepiaContainer::BWIlfordXP2Super);
    item->setWhatsThis(i18n("<b>Ilford XP2 Super</b>:"
                            "<p>Simulate the Ilford XP2 Super black and white film at 400 ISO.</p>"));

    item = d->addItem(d->bwFilm, i18n("Kodak Tmax 100"), BWSepiaContainer::BWKodakTmax100);
    item->setWhatsThis(i18n("<b>Kodak Tmax 100</b>:"
                            "<p>Simulate the Kodak Tmax black and white film at 100 ISO.</p>"));

    item = d->addItem(d->bwFilm, i18n("Kodak Tmax 400"), BWSepiaContainer::BWKodakTmax400);
    item->setWhatsThis(i18n("<b>Kodak Tmax 400</b>:"
                            "<p>Simulate the Kodak Tmax black and white film at 400 ISO.</p>"));

    item = d->addItem(d->bwFilm, i18n("Kodak TriX"), BWSepiaContainer::BWKodakTriX);
    item->setWhatsThis(i18n("<b>Kodak TriX</b>:"
                            "<p>Simulate the Kodak TriX black and white film at 400 ISO.</p>"));

    // -------------------------------------------------------------

    item = d->addItem(d->bwFilm, i18n("Ilford SPX 200 (Infrared)"), BWSepiaContainer::BWIlfordSFX200);
    item->setWhatsThis(i18n("<b>Ilford SPX 200</b>:"
                            "<p>Simulate the Ilford SPX infrared film at 200 ISO.</p>"));

    item = d->addItem(d->bwFilm, i18n("Ilford SPX 400 (Infrared)"), BWSepiaContainer::BWIlfordSFX400);
    item->setWhatsThis(i18n("<b>Ilford SPX 400</b>:"
                            "<p>Simulate the Ilford SPX infrared film at 400 ISO.</p>"));

    item = d->addItem(d->bwFilm, i18n("Ilford SPX 800 (Infrared)"), BWSepiaContainer::BWIlfordSFX800);
    item->setWhatsThis(i18n("<b>Ilford SPX 800</b>:"
                            "<p>Simulate the Ilford SPX infrared film at 800 ISO.</p>"));

    item = d->addItem(d->bwFilm, i18n("Kodak HIE (Infrared)"), BWSepiaContainer::BWKodakHIE);
    item->setWhatsThis(i18n("<b>Kodak HIE</b>:"
                            "<p>Simulate the Kodak HIE infrared film.</p>"));

    // -------------------------------------------------------------

    QWidget* vbox     = new QWidget();
    QVBoxLayout* vlay = new QVBoxLayout(vbox);
    d->bwFilters      = new PreviewList(vbox);

    item = d->addItem(d->bwFilters, i18n("No Lens Filter"), BWSepiaContainer::BWNoFilter);
    item->setWhatsThis(i18n("<b>No Lens Filter</b>:"
                            "<p>Do not apply a lens filter when rendering the image.</p>"));

    item = d->addItem(d->bwFilters, i18n("Green Filter"), BWSepiaContainer::BWGreenFilter);
    item->setWhatsThis(i18n("<b>Black & White with Green Filter</b>:"
                            "<p>Simulate black and white film exposure using a green filter. "
                            "This is useful for all scenic shoots, especially "
                            "portraits photographed against the sky.</p>"));

    item = d->addItem(d->bwFilters, i18n("Orange Filter"), BWSepiaContainer::BWOrangeFilter);
    item->setWhatsThis(i18n("<b>Black & White with Orange Filter</b>:"
                            "<p>Simulate black and white film exposure using an orange filter. "
                            "This will enhance landscapes, marine scenes and aerial "
                            "photography.</p>"));

    item = d->addItem(d->bwFilters, i18n("Red Filter"), BWSepiaContainer::BWRedFilter);
    item->setWhatsThis(i18n("<b>Black & White with Red Filter</b>:"
                            "<p>Simulate black and white film exposure using a red filter. "
                            "This creates dramatic sky effects, and simulates moonlight scenes "
                            "in the daytime.</p>"));

    item = d->addItem(d->bwFilters, i18n("Yellow Filter"), BWSepiaContainer::BWYellowFilter);
    item->setWhatsThis(i18n("<b>Black & White with Yellow Filter</b>:"
                            "<p>Simulate black and white film exposure using a yellow filter. "
                            "This has the most natural tonal correction, and improves contrast. Ideal for "
                            "landscapes.</p>"));

    item = d->addItem(d->bwFilters, i18n("Yellow-Green Filter"), BWSepiaContainer::BWYellowGreenFilter);
    item->setWhatsThis(i18n("<b>Black & White with Yellow-Green Filter</b>:"
                            "<p>Simulate black and white film exposure using a yellow-green filter. "
                            "A yellow-green filter is highly effective for outdoor portraits because "
                            "red is rendered dark while green appears lighter. Great for correcting skin tones, "
                            "bringing out facial expressions in close-ups and emphasizing the feeling of liveliness. "
                            "This filter is highly effective for indoor portraits under tungsten lighting.</p>"));

    item = d->addItem(d->bwFilters, i18n("Blue Filter"), BWSepiaContainer::BWBlueFilter);
    item->setWhatsThis(i18n("<b>Black & White with Blue Filter</b>:"
                            "<p>Simulate black and white film exposure using a blue filter. "
                            "This accentuates haze and fog. Used for dye transfer and contrast effects.</p>"));

    DHBox* const hbox1   = new DHBox(vbox);
    QLabel* const label1 = new QLabel(i18n("Strength:"), hbox1);
    label1->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    d->strengthInput     = new DIntNumInput(hbox1);
    d->strengthInput->setRange(1, 5, 1);
    d->strengthInput->setDefaultValue(1);
    d->strengthInput->setWhatsThis(i18n("Here, set the strength adjustment of the lens filter."));

    vlay->addWidget(d->bwFilters);
    vlay->addWidget(hbox1);
    vlay->setContentsMargins(QMargins());
    vlay->setSpacing(spacing);

    // -------------------------------------------------------------

    d->bwTone = new PreviewList;

    item = d->addItem(d->bwTone, i18n("No Tone Filter"), BWSepiaContainer::BWNoTone);
    item->setWhatsThis(i18n("<b>No Tone Filter</b>:"
                            "<p>Do not apply a tone filter to the image.</p>"));

    item = d->addItem(d->bwTone, i18n("Sepia Filter"), BWSepiaContainer::BWSepiaTone);
    item->setWhatsThis(i18n("<b>Black & White with Sepia Tone</b>:"
                            "<p>Gives a warm highlight and mid-tone while adding a bit of coolness to "
                            "the shadows - very similar to the process of bleaching a print and "
                            "re-developing in a sepia toner.</p>"));

    item = d->addItem(d->bwTone, i18n("Brown Filter"), BWSepiaContainer::BWBrownTone);
    item->setWhatsThis(i18n("<b>Black & White with Brown Tone</b>:"
                            "<p>This filter is more neutral than the Sepia Tone "
                            "filter.</p>"));

    item = d->addItem(d->bwTone, i18n("Cold Filter"), BWSepiaContainer::BWColdTone);
    item->setWhatsThis(i18n("<b>Black & White with Cold Tone</b>:"
                            "<p>Start subtly and replicate printing on a cold tone black and white "
                            "paper such as a bromide enlarging "
                            "paper.</p>"));

    item = d->addItem(d->bwTone, i18n("Selenium Filter"), BWSepiaContainer::BWSeleniumTone);
    item->setWhatsThis(i18n("<b>Black & White with Selenium Tone</b>:"
                            "<p>This effect replicates traditional selenium chemical toning done "
                            "in the darkroom.</p>"));

    item = d->addItem(d->bwTone, i18n("Platinum Filter"), BWSepiaContainer::BWPlatinumTone);
    item->setWhatsThis(i18n("<b>Black & White with Platinum Tone</b>:"
                            "<p>This effect replicates traditional platinum chemical toning done "
                            "in the darkroom.</p>"));

    item = d->addItem(d->bwTone, i18n("Green Filter"), BWSepiaContainer::BWGreenTone);
    item->setWhatsThis(i18n("<b>Black & White with greenish tint</b>:"
                            "<p>This effect is also known as Verdante.</p>"));

    // -------------------------------------------------------------

    QWidget* lumBox = new QWidget();

    // NOTE: add a method to be able to use curves widget without image data as simple curve editor.
    if (!img->isNull())
    {
        d->curvesBox = new CurvesBox(256, 192, *img, lumBox);
    }
    else
    {
        d->curvesBox = new CurvesBox(256, 192, DImg(1, 1, true, false, (uchar*)"\x00\x00\x00\x00\x00\x00\x00\x00"), lumBox);
    }

    d->curvesBox->enableCurveTypes(true);
    d->curvesBox->enableResetButton(true);
    d->curvesBox->setWhatsThis(i18n("This is the curve adjustment of the image luminosity"));

    // -------------------------------------------------------------

    DHBox* const hbox2   = new DHBox(lumBox);
    QLabel* const label2 = new QLabel(i18n("Contrast:"), hbox2);
    label2->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    d->cInput = new DIntNumInput(hbox2);
    d->cInput->setRange(-100, 100, 1);
    d->cInput->setDefaultValue(0);
    d->cInput->setWhatsThis(i18n("Set here the contrast adjustment of the image."));

    QGridLayout* gridTab2 = new QGridLayout(lumBox);
    gridTab2->addWidget(d->curvesBox, 0, 0, 1, 1);
    gridTab2->addWidget(hbox2,        1, 0, 1, 1);
    gridTab2->setRowStretch(2, 10);
    gridTab2->setContentsMargins(spacing, spacing, spacing, spacing);
    gridTab2->setSpacing(0);

    // -------------------------------------------------------------

    // Some new icons may be needed : a film roll, a lens filter and ?
    d->tab->addItem(d->bwFilm, QIcon::fromTheme(QLatin1String("filmgrain")),
                    i18n("Film"), QLatin1String("Film"), true);
    d->tab->addItem(vbox, QIcon::fromTheme(QLatin1String("lensautofix")),
                    i18n("Lens Filters"), QLatin1String("Lens Filters"), false);
    d->tab->addItem(d->bwTone, QIcon::fromTheme(QLatin1String("fill-color")),
                    i18n("Tone"), QLatin1String("Tone"), false);
    d->tab->addItem(lumBox, QIcon::fromTheme(QLatin1String("adjustcurves")),
                    i18n("Luminosity"), QLatin1String("Luminosity"), false);
    d->tab->addStretch();

    grid->addWidget(d->tab, 0, 0, 1, 10);
    grid->setRowStretch(0, 10);
    grid->setContentsMargins(spacing, spacing, spacing, spacing);
    grid->setSpacing(spacing);

    // -------------------------------------------------------------

    connect(d->bwFilters, SIGNAL(itemSelectionChanged()),
            this, SLOT(slotFilterSelected()));

    connect(d->strengthInput, SIGNAL(valueChanged(int)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->bwFilm, SIGNAL(itemSelectionChanged()),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->bwTone, SIGNAL(itemSelectionChanged()),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->cInput, SIGNAL(valueChanged(int)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->curvesBox, SIGNAL(signalCurvesChanged()),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->curvesBox, SIGNAL(signalChannelReset(int)),
            this, SIGNAL(signalSettingsChanged()));
}

BWSepiaSettings::~BWSepiaSettings()
{
    delete d;
}

void BWSepiaSettings::startPreviewFilters()
{
    d->bwFilters->startFilters();
    d->bwFilm->startFilters();
    d->bwTone->startFilters();
}

void BWSepiaSettings::slotFilterSelected()
{
    int filter = d->bwFilters->currentId();

    if (filter == BWSepiaContainer::BWNoFilter)
    {
        d->strengthInput->setEnabled(false);
    }
    else
    {
        d->strengthInput->setEnabled(true);
    }

    emit signalSettingsChanged();
}

BWSepiaContainer BWSepiaSettings::settings() const
{
    BWSepiaContainer prm;

    prm.filmType               = d->bwFilm->currentId();
    prm.filterType             = d->bwFilters->currentId();
    prm.toneType               = d->bwTone->currentId();
    prm.bcgPrm.contrast        = ((double)(d->cInput->value() / 100.0) + 1.00);
    prm.strength               = 1.0 + ((double)d->strengthInput->value() - 1.0) * (1.0 / 3.0);
    prm.curvesPrm              = d->curvesBox->curves()->getContainer(LuminosityChannel);

    return prm;
}

void BWSepiaSettings::setSettings(const BWSepiaContainer& settings)
{
    blockSignals(true);

    d->bwFilm->setCurrentId(settings.filmType);
    d->bwFilters->setCurrentId(settings.filterType);
    d->bwTone->setCurrentId(settings.toneType);
    d->cInput->setValue((int)((settings.bcgPrm.contrast - 1.00) * 100.0));
    d->strengthInput->setValue((int)(1.0 + (settings.strength - 1.0) * 3.0));
    d->curvesBox->curves()->setContainer(settings.curvesPrm);
    d->curvesBox->update();

    slotFilterSelected();
    blockSignals(false);
}

void BWSepiaSettings::resetToDefault()
{
    blockSignals(true);

    d->bwFilters->setCurrentId(BWSepiaContainer::BWNoFilter);
    d->bwFilm->setCurrentId(BWSepiaContainer::BWGeneric);
    d->bwTone->setCurrentId(BWSepiaContainer::BWNoTone);

    d->cInput->slotReset();
    d->strengthInput->slotReset();
    d->curvesBox->curves()->curvesChannelReset(LuminosityChannel);
    d->curvesBox->reset();

    blockSignals(false);
    slotFilterSelected();
}

BWSepiaContainer BWSepiaSettings::defaultSettings() const
{
    BWSepiaContainer prm;

    prm.bcgPrm.contrast = ((double)(d->cInput->defaultValue() / 100.0) + 1.00);
    prm.strength        = 1.0 + ((double)d->strengthInput->defaultValue() - 1.0) * (1.0 / 3.0);

    return prm;
}

void BWSepiaSettings::readSettings(KConfigGroup& group)
{
    BWSepiaContainer prm;
    BWSepiaContainer defaultPrm = defaultSettings();

    d->tab->readSettings(group);

    prm.filmType        = group.readEntry(d->configBWFilmEntry,             defaultPrm.filmType);
    prm.filterType      = group.readEntry(d->configBWFilterEntry,           defaultPrm.filterType);
    prm.toneType        = group.readEntry(d->configBWToneEntry,             defaultPrm.toneType);
    prm.bcgPrm.contrast = group.readEntry(d->configContrastAdjustmentEntry, defaultPrm.bcgPrm.contrast);
    prm.strength        = group.readEntry(d->configStrengthAdjustmentEntry, defaultPrm.strength);

    d->curvesBox->readCurveSettings(group, d->configCurveEntry);
    prm.curvesPrm = d->curvesBox->curves()->getContainer(LuminosityChannel);

    setSettings(prm);
}

void BWSepiaSettings::writeSettings(KConfigGroup& group)
{
    BWSepiaContainer prm = settings();

    d->tab->writeSettings(group);

    group.writeEntry(d->configBWFilmEntry,             prm.filmType);
    group.writeEntry(d->configBWFilterEntry,           prm.filterType);
    group.writeEntry(d->configBWToneEntry,             prm.toneType);
    group.writeEntry(d->configContrastAdjustmentEntry, prm.bcgPrm.contrast);
    group.writeEntry(d->configStrengthAdjustmentEntry, prm.strength);

    d->curvesBox->writeCurveSettings(group, d->configCurveEntry);
}

void BWSepiaSettings::loadSettings()
{
    QUrl loadFile = DFileDialog::getOpenFileUrl(qApp->activeWindow(), i18n("Black & White Settings File to Load"),
                                                QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)),
                                                QLatin1String("*"));

    if (loadFile.isEmpty())
    {
        return;
    }

    QFile file(loadFile.toLocalFile());

    if (file.open(QIODevice::ReadOnly))
    {
        QTextStream stream(&file);

        if (stream.readLine() != QLatin1String("# Black & White Configuration File"))
        {
            QMessageBox::critical(qApp->activeWindow(), qApp->applicationName(),
                                  i18n("\"%1\" is not a Black & White settings text file.", loadFile.fileName()));
            file.close();
            return;
        }

        blockSignals(true);

        d->bwFilm->setCurrentId(stream.readLine().toInt());
        d->bwFilters->setCurrentId(stream.readLine().toInt());
        d->bwTone->setCurrentId(stream.readLine().toInt());
        d->cInput->setValue(stream.readLine().toInt());

        for (int i = 0 ; i < 5 ; ++i)
        {
            d->curvesBox->curves()->curvesChannelReset(i);
        }

        d->curvesBox->curves()->setCurveType(d->curvesBox->channel(), ImageCurves::CURVE_SMOOTH);
        d->curvesBox->reset();

        // TODO cant we use the kconfig mechanisms provided by CurveWidget here?
        QPoint disable = ImageCurves::getDisabledValue();
        QPoint p;

        for (int j = 0 ; j < ImageCurves::NUM_POINTS ; ++j)
        {
            p.setX(stream.readLine().toInt());
            p.setY(stream.readLine().toInt());

            if (d->curvesBox->curves()->isSixteenBits() && p != disable)
            {
                p.setX(p.x()*ImageCurves::MULTIPLIER_16BIT);
                p.setY(p.y()*ImageCurves::MULTIPLIER_16BIT);
            }

            d->curvesBox->curves()->setCurvePoint(LuminosityChannel, j, p);
        }

        d->curvesBox->curves()->curvesCalculateAllCurves();

        blockSignals(false);
    }
    else
    {
        QMessageBox::critical(qApp->activeWindow(), qApp->applicationName(),
                              i18n("Cannot load settings from the Black & White text file."));
    }

    file.close();
}

void BWSepiaSettings::saveAsSettings()
{
    QUrl saveFile = DFileDialog::getSaveFileUrl(qApp->activeWindow(), i18n("Black & White Settings File to Save"),
                                                QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)),
                                                QLatin1String("*"));

    if (saveFile.isEmpty())
    {
        return;
    }

    QFile file(saveFile.toLocalFile());

    if (file.open(QIODevice::WriteOnly))
    {
        QTextStream stream(&file);
        stream << QLatin1String("# Black & White Configuration File\n");
        stream << d->bwFilm->currentId() << "\n";
        stream << d->bwFilters->currentId() << "\n";
        stream << d->bwTone->currentId() << "\n";
        stream << d->cInput->value() << "\n";

        // TODO cant we use the kconfig mechanisms provided by CurveWidget here?
        for (int j = 0 ; j < ImageCurves::NUM_POINTS ; ++j)
        {
            QPoint p = d->curvesBox->curves()->getCurvePoint(LuminosityChannel, j);

            if (d->curvesBox->curves()->isSixteenBits())
            {
                p.setX(p.x() / ImageCurves::MULTIPLIER_16BIT);
                p.setY(p.y() / ImageCurves::MULTIPLIER_16BIT);
            }

            stream << p.x() << "\n";
            stream << p.y() << "\n";
        }
    }
    else
    {
        QMessageBox::critical(qApp->activeWindow(), qApp->applicationName(),
                              i18n("Cannot save settings to the Black & White text file."));
    }

    file.close();
}

void BWSepiaSettings::setScaleType(HistogramScale scale)
{
    d->curvesBox->setScale(scale);
}

}  // namespace Digikam
