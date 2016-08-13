/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-02-05
 * Description : film color negative inverter tool
 *
 * Copyright (C) 2012 by Matthias Welwarsky <matthias at welwarsky dot de>
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

#include "filmtool.h"

// C++ includes

#include <cmath>

// Qt includes

#include <QButtonGroup>
#include <QColor>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QToolButton>
#include <QListWidget>
#include <QCheckBox>
#include <QIcon>

// KDE includes

#include <klocalizedstring.h>
#include <ksharedconfig.h>

// Local includes

#include "dnuminput.h"
#include "dgradientslider.h"
#include "dimg.h"
#include "editortoolsettings.h"
#include "histogrambox.h"
#include "histogramwidget.h"
#include "imagehistogram.h"
#include "imageiface.h"
#include "imagelevels.h"
#include "imageregionwidget.h"
#include "filmfilter_p.h"

namespace Digikam
{

class FilmTool::Private
{

public:

    enum ColorPicker
    {
        NoPicker   = 0,
        OrangeMask = 1
    };

public:

    Private() :
        histoSegments(0),
        resetButton(0),
        pickWhitePoint(0),
        autoButton(0),
        exposureInput(0),
        gammaInput(0),
        cnType(0),
        colorBalanceInput(0),
        levelsHistogramWidget(0),
        redInputLevels(0),
        greenInputLevels(0),
        blueInputLevels(0),
        previewWidget(0),
        levels(0),
        originalImage(0),
        gboxSettings(0)
    {
    }

    static const QString configGroupName;
    static const QString configGammaInputEntry;
    static const QString configExposureEntry;
    static const QString configFilmProfileEntry;
    static const QString configFilmProfileName;
    static const QString configWhitePointEntry;
    static const QString configHistogramChannelEntry;
    static const QString configHistogramScaleEntry;
    static const QString configApplyColorBalance;

    int                  histoSegments;

    QPushButton*         resetButton;
    QToolButton*         pickWhitePoint;
    QToolButton*         autoButton;

    FilmContainer        filmContainer;

    DDoubleNumInput*     exposureInput;
    DDoubleNumInput*     gammaInput;
    QListWidget*         cnType;
    QCheckBox*           colorBalanceInput;

    HistogramWidget*     levelsHistogramWidget;

    DGradientSlider*     redInputLevels;
    DGradientSlider*     greenInputLevels;
    DGradientSlider*     blueInputLevels;

    ImageRegionWidget*   previewWidget;

    ImageLevels*         levels;

    DImg*                originalImage;

    EditorToolSettings*  gboxSettings;
};

const QString FilmTool::Private::configGroupName(QLatin1String("film Tool"));
const QString FilmTool::Private::configGammaInputEntry(QLatin1String("GammaInput"));
const QString FilmTool::Private::configExposureEntry(QLatin1String("Exposure"));
const QString FilmTool::Private::configFilmProfileEntry(QLatin1String("FilmProfile"));
const QString FilmTool::Private::configFilmProfileName(QLatin1String("FilmProfileName"));
const QString FilmTool::Private::configWhitePointEntry(QLatin1String("WhitePoint_%1"));
const QString FilmTool::Private::configHistogramChannelEntry(QLatin1String("Histogram Channel"));
const QString FilmTool::Private::configHistogramScaleEntry(QLatin1String("Histogram Scale"));
const QString FilmTool::Private::configApplyColorBalance(QLatin1String("Apply Color Balance"));

// --------------------------------------------------------

FilmTool::FilmTool(QObject* const parent)
    : EditorToolThreaded(parent),
      d(new Private)
{
    setObjectName(QLatin1String("film"));
    setToolName(i18n("Color Negative Film"));
    setToolIcon(QIcon::fromTheme(QLatin1String("colorneg")));
    setInitPreview(true);

    ImageIface iface;
    d->originalImage = iface.original();

    d->histoSegments = d->originalImage->sixteenBit() ? 65535 : 255;
    d->levels        = new ImageLevels(d->originalImage->sixteenBit());

    // -------------------------------------------------------------

    d->previewWidget = new ImageRegionWidget;
    setToolView(d->previewWidget);
    setPreviewModeMask(PreviewToolBar::AllPreviewModes);

    // -------------------------------------------------------------

    d->gboxSettings = new EditorToolSettings;
    d->gboxSettings->setButtons(EditorToolSettings::Default|
                                EditorToolSettings::Ok|
                                EditorToolSettings::Cancel);

    d->gboxSettings->setTools(EditorToolSettings::Histogram);
    d->gboxSettings->setHistogramType(LRGBC);

    // we don't need to use the Gradient in this tool
    d->gboxSettings->histogramBox()->setGradientVisible(false);
    d->gboxSettings->histogramBox()->setChannel(ColorChannels);

    // -------------------------------------------------------------

    d->levelsHistogramWidget = new HistogramWidget(256, 140, d->gboxSettings->plainPage(), false);
    d->levelsHistogramWidget->updateData(*d->originalImage);

    d->levelsHistogramWidget->setWhatsThis(i18n("This is the histogram drawing of the selected channel "
                                           "from the original image."));
    d->levelsHistogramWidget->setChannelType(ColorChannels);
    QHBoxLayout* const inputLevelsLayout = new QHBoxLayout;
    inputLevelsLayout->addWidget(d->levelsHistogramWidget);

    // -------------------------------------------------------------

    d->redInputLevels = new DGradientSlider();
    d->redInputLevels->setColors(QColor("Red"), QColor("White"));
    d->redInputLevels->setToolTip( i18n( "Input range of red color channel." ) );
    d->redInputLevels->installEventFilter(this);

    d->greenInputLevels = new DGradientSlider();
    d->greenInputLevels->setColors(QColor("Green"), QColor("White"));
    d->greenInputLevels->setToolTip( i18n( "Input range of green color channel." ) );
    d->greenInputLevels->installEventFilter(this);

    d->blueInputLevels = new DGradientSlider();
    d->blueInputLevels->setColors(QColor("Blue"), QColor("White"));
    d->blueInputLevels->setToolTip( i18n( "Input range of blue color channel." ) );
    d->blueInputLevels->installEventFilter(this);

    d->gboxSettings->histogramBox()->setHistogramMargin(d->redInputLevels->gradientOffset());

    inputLevelsLayout->setContentsMargins(d->redInputLevels->gradientOffset(), 0,
                                          d->redInputLevels->gradientOffset(), 0);

    // -------------------------------------------------------------

    d->cnType = new QListWidget();
    QList<FilmContainer::ListItem*> profiles = d->filmContainer.profileItemList(d->cnType);
    QList<FilmContainer::ListItem*>::ConstIterator it;

    for (it = profiles.constBegin(); it != profiles.constEnd(); it++)
        d->cnType->addItem(*it);

    // -------------------------------------------------------------

    d->colorBalanceInput = new QCheckBox(i18n("Color Balance"));
    d->colorBalanceInput->setCheckState(Qt::Checked);
    d->colorBalanceInput->setToolTip(i18n("Check to apply the built-in color balance of the film profile. "
                                          "Un-check if you want to apply color balance yourself."));

    // -------------------------------------------------------------

    d->pickWhitePoint = new QToolButton();
    d->pickWhitePoint->setIcon(QIcon::fromTheme(QLatin1String("color-picker-white")));
    d->pickWhitePoint->setCheckable(true);
    d->pickWhitePoint->setToolTip( i18n( "White point color picker" ) );
    d->pickWhitePoint->setWhatsThis(i18n("With this button, you can pick the color of the orange mask "
            "of the scanned color negative. It represents white point of the negative, "
            "or the darkest black tone of the positive image "
            "after inversion. It is also the reference point for applying the film profile."));

    d->resetButton = new QPushButton(i18n("&Reset"));
    d->resetButton->setIcon(QIcon::fromTheme(QLatin1String("document-revert")));
    d->resetButton->setToolTip( i18n( "Reset white point." ) );
    d->resetButton->setWhatsThis(i18n("If you press this button, the white point is "
                                      "reset to pure white."));

    d->autoButton = new QToolButton();
    d->autoButton->setIcon(QIcon::fromTheme(QLatin1String("system-run")));
    d->autoButton->setToolTip( i18n( "Adjust white point automatically." ) );
    d->autoButton->setWhatsThis(i18n("If you press this button, the white point is calculated "
            "from the image data automatically. This function requires to have some residual "
            "orange mask around the exposed area of the negative."));

    QLabel* const space = new QLabel();
    space->setFixedWidth(d->gboxSettings->spacingHint());

    QHBoxLayout* const l3 = new QHBoxLayout();
    l3->addWidget(d->pickWhitePoint);
    l3->addWidget(d->autoButton);
    l3->addWidget(space);
    l3->addWidget(d->resetButton);
    l3->addStretch(10);

    // -------------------------------------------------------------

    d->exposureInput = new DDoubleNumInput();
    d->exposureInput->setDecimals(2);
    d->exposureInput->setRange(0.0, 40.0, 0.01);
    d->exposureInput->setDefaultValue(1.0);
    d->exposureInput->setToolTip( i18n( "Exposure correction." ) );
    d->exposureInput->setWhatsThis(i18n("Move the slider to higher values until maximum brightness is achieved "
                                        "without clipping any color channel. Use the output histogram to evaluate each channel."));

    d->gammaInput = new DDoubleNumInput();
    d->gammaInput->setDecimals(2);
    d->gammaInput->setRange(0.1, 3.0, 0.01);
    d->gammaInput->setDefaultValue(1.8);
    d->gammaInput->setToolTip(i18n( "Gamma input value." ));
    d->gammaInput->setWhatsThis(i18n("Linear raw scans of film negatives require application of a gamma curve. "
                                     "Standard values are 1.8 or 2.2."));

    // -------------------------------------------------------------

    QGridLayout* const grid = new QGridLayout();
    grid->addLayout(inputLevelsLayout,    0, 0, 1, 4);
    grid->addWidget(d->redInputLevels,    1, 0, 1, 4);
    grid->addWidget(d->greenInputLevels,  2, 0, 1, 4);
    grid->addWidget(d->blueInputLevels,   3, 0, 1, 4);
    grid->addWidget(d->cnType,            4, 0, 1, 4);
    grid->addWidget(d->exposureInput,     5, 0, 1, 4);
    grid->addWidget(d->gammaInput,        6, 0, 1, 4);
    grid->addLayout(l3,                   7, 0, 1, 2);
    grid->addWidget(d->colorBalanceInput, 7, 2, 1, 2, Qt::AlignRight);

    // TODO: fill in rest of settings elements

    //grid->setRowStretch(7, 10);
    //grid->setColumnStretch(2, 10);
    //grid->setColumnStretch(4, 10);
    grid->setContentsMargins(QMargins());
    grid->setSpacing(d->gboxSettings->spacingHint());
    d->gboxSettings->plainPage()->setLayout(grid);

    // -------------------------------------------------------------

    d->filmContainer.setSixteenBit(d->originalImage->sixteenBit());
    d->filmContainer.setWhitePoint(DColor(QColor(QLatin1String("white")), d->originalImage->sixteenBit()));

    // -------------------------------------------------------------

    setToolSettings(d->gboxSettings);

    // Button Slots -------------------------------------------------

    connect(d->autoButton, SIGNAL(clicked()),
            this, SLOT(slotAutoWhitePoint()));

    connect(d->pickWhitePoint, SIGNAL(toggled(bool)),
            this, SLOT(slotPickerColorButtonActived(bool)));

    // Slots --------------------------------------------------------

    connect(d->previewWidget, SIGNAL(signalCapturedPointFromOriginal(Digikam::DColor,QPoint)),
            this, SLOT(slotColorSelectedFromTarget(Digikam::DColor,QPoint)));

    connect(d->exposureInput, SIGNAL(valueChanged(double)),
            this, SLOT(slotExposureChanged(double)));

    connect(d->gammaInput, SIGNAL(valueChanged(double)),
            this, SLOT(slotGammaInputChanged(double)));

    connect(d->resetButton, SIGNAL(clicked()),
            this, SLOT(slotResetWhitePoint()));

    connect(d->cnType, SIGNAL(itemActivated(QListWidgetItem*)),
            this, SLOT(slotFilmItemActivated(QListWidgetItem*)));

    connect(d->colorBalanceInput, SIGNAL(stateChanged(int)),
            this, SLOT(slotColorBalanceStateChanged(int)));
}

FilmTool::~FilmTool()
{
    delete d->levels;
    delete d;
}

void FilmTool::slotResetSettings()
{
    bool sb = d->originalImage->sixteenBit();
    int max = sb ? 65535 : 255;

    FilmContainer::CNFilmProfile cnType = FilmContainer::CNNeutral;

    QString profileName                   = QLatin1String("Neutral");
    QList<QListWidgetItem*> matchingItems = d->cnType->findItems(profileName, Qt::MatchExactly);
    d->cnType->setCurrentItem(matchingItems.first());

    double gamma      = 1.8;
    d->gammaInput->setValue(gamma);
    gammaInputChanged(gamma);

    double exposure = 1.0;
    d->exposureInput->setValue(exposure);

    d->filmContainer  = FilmContainer(cnType, gamma, d->originalImage->sixteenBit());
    d->filmContainer.setExposure(exposure);

    int red   = max;
    int green = max;
    int blue  = max;

    red       = sb ? red   : red   / 256;
    green     = sb ? green : green / 256;
    blue      = sb ? blue  : blue  / 256;

    DColor whitePoint = DColor(red, green, blue, max, sb);
    d->filmContainer.setWhitePoint(whitePoint);
    setLevelsFromFilm();

    d->levelsHistogramWidget->reset();
    d->gboxSettings->histogramBox()->histogram()->reset();
    d->gboxSettings->histogramBox()->setChannel(ColorChannels);
    d->gboxSettings->histogramBox()->setScale(LogScaleHistogram);

    slotAdjustSliders();
    slotChannelChanged();
    slotScaleChanged();
}

void FilmTool::slotChannelChanged()
{
    d->levelsHistogramWidget->setChannelType(d->gboxSettings->histogramBox()->channel());
}

void FilmTool::slotScaleChanged()
{
   d->levelsHistogramWidget->setScaleType(d->gboxSettings->histogramBox()->scale());
}

void FilmTool::slotAdjustSliders()
{
    // adjust all Levels sliders
    d->redInputLevels->setLeftValue(
            (double)d->levels->getLevelLowInputValue(RedChannel) / d->histoSegments);
    d->redInputLevels->setRightValue(
            (double)d->levels->getLevelHighInputValue(RedChannel) / d->histoSegments);

    d->greenInputLevels->setLeftValue(
            (double)d->levels->getLevelLowInputValue(GreenChannel) / d->histoSegments);
    d->greenInputLevels->setRightValue(
            (double)d->levels->getLevelHighInputValue(GreenChannel) / d->histoSegments);

    d->blueInputLevels->setLeftValue(
            (double)d->levels->getLevelLowInputValue(BlueChannel) / d->histoSegments);
    d->blueInputLevels->setRightValue(
            (double)d->levels->getLevelHighInputValue(BlueChannel) / d->histoSegments);
}

void FilmTool::setLevelsFromFilm()
{
    LevelsContainer l = d->filmContainer.toLevels();

    for (int i = RedChannel; i <= BlueChannel; i++)
    {
        d->levels->setLevelLowInputValue(i, l.lInput[i]);
        d->levels->setLevelHighInputValue(i, l.hInput[i]);
        d->levels->setLevelLowOutputValue(i, l.lOutput[i]);
        d->levels->setLevelHighOutputValue(i, l.hOutput[i]);
        d->levels->setLevelGammaValue(i, l.gamma[i]);
    }

    slotAdjustSliders();
}

void FilmTool::slotExposureChanged(double val)
{
    d->filmContainer.setExposure(val);
    setLevelsFromFilm();
    slotTimer();
}

void FilmTool::gammaInputChanged(double val)
{
    d->filmContainer.setGamma(val);
    setLevelsFromFilm();
}

void FilmTool::slotGammaInputChanged(double val)
{
    gammaInputChanged(val);
    slotTimer();
}

void FilmTool::slotFilmItemActivated(QListWidgetItem* item)
{
    double gamma    = d->filmContainer.gamma();
    double strength = d->filmContainer.exposure();
    DColor wp       = d->filmContainer.whitePoint();

    FilmContainer::CNFilmProfile type = (FilmContainer::CNFilmProfile)(item->type()-QListWidgetItem::UserType);
    d->filmContainer                  = FilmContainer(type, gamma, d->originalImage->sixteenBit());
    d->filmContainer.setExposure(strength);
    d->filmContainer.setApplyBalance(d->colorBalanceInput->checkState() == Qt::Checked);
    d->filmContainer.setWhitePoint(wp);
    setLevelsFromFilm();
    slotTimer();
}

void FilmTool::slotColorSelectedFromTarget(const Digikam::DColor& color, const QPoint& p)
{
    DColor wp00 = color;
    DColor wp01 = d->originalImage->getPixelColor(p.x(), p.y()+1);
    DColor wp10 = d->originalImage->getPixelColor(p.x()+1, p.y());
    DColor wp11 = d->originalImage->getPixelColor(p.x()+1, p.y()+1);

    wp00.blendAdd(wp01);
    wp00.blendAdd(wp10);
    wp00.blendAdd(wp11);
    wp00.multiply(0.25);

    d->filmContainer.setWhitePoint(wp00);
    d->previewWidget->setCapturePointMode(false);
    d->pickWhitePoint->setChecked(false);

    setLevelsFromFilm();
    slotTimer();
}

void FilmTool::slotPickerColorButtonActived(bool checked)
{
    if (checked)
        d->previewWidget->setCapturePointMode(true);
}

void FilmTool::slotAutoWhitePoint()
{
    ImageHistogram* const hist = d->levelsHistogramWidget->currentHistogram();
    bool sixteenBit            = d->originalImage->sixteenBit();
    int high_input[4];

    for (int channel = RedChannel; channel <= BlueChannel; channel++)
    {
        double new_count = 0.0;
        double percentage;
        double next_percentage;
        double count     = hist->getCount(channel, 0, sixteenBit ? 65535 : 255);

        for (int i = (sixteenBit ? 65535 : 255) ; i > 0 ; --i)
        {
            new_count       += hist->getValue(channel, i);
            percentage      = new_count / count;
            next_percentage = (new_count + hist->getValue(channel, i - 1)) / count;

            if (fabs(percentage - 0.006) < fabs(next_percentage - 0.006))
            {
                high_input[channel] = i - 1;
                break;
            }
        }

    }

    DColor wp = DColor(high_input[RedChannel], high_input[GreenChannel],
            high_input[BlueChannel], 0, sixteenBit);
    d->filmContainer.setWhitePoint(wp);

    setLevelsFromFilm();
    slotPreview();
}

void FilmTool::slotResetWhitePoint()
{
    d->filmContainer.setSixteenBit(d->originalImage->sixteenBit());
    d->filmContainer.setWhitePoint(DColor(QColor(QLatin1String("white")), d->originalImage->sixteenBit()));

    setLevelsFromFilm();
    slotPreview();
}

void FilmTool::slotColorBalanceStateChanged(int state)
{
    bool apply = (state == Qt::Checked);
    d->filmContainer.setApplyBalance(apply);

    slotPreview();
}

void FilmTool::readSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);

    bool sb = d->originalImage->sixteenBit();
    int max = sb ? 65535 : 255;

    FilmContainer::CNFilmProfile cnType   = (FilmContainer::CNFilmProfile)
                                            group.readEntry(d->configFilmProfileEntry, (int)FilmContainer::CNNeutral);

    QString profileName                   = group.readEntry(d->configFilmProfileName, "Neutral");
    QList<QListWidgetItem*> matchingItems = d->cnType->findItems(profileName, Qt::MatchExactly);
    d->cnType->setCurrentItem(matchingItems.first());

    double gamma      = group.readEntry(d->configGammaInputEntry, 1.8);
    d->gammaInput->setValue(gamma);
    gammaInputChanged(gamma);

    double exposure = group.readEntry(d->configExposureEntry, 1.0);
    d->exposureInput->setValue(exposure);

    d->filmContainer  = FilmContainer(cnType, gamma, d->originalImage->sixteenBit());
    d->filmContainer.setExposure(exposure);

    int red   = group.readEntry(d->configWhitePointEntry.arg(1), max);
    int green = group.readEntry(d->configWhitePointEntry.arg(2), max);
    int blue  = group.readEntry(d->configWhitePointEntry.arg(3), max);

    red       = sb ? red   : red   / 256;
    green     = sb ? green : green / 256;
    blue      = sb ? blue  : blue  / 256;

    DColor whitePoint = DColor(red, green, blue, max, sb);
    d->filmContainer.setWhitePoint(whitePoint);
    setLevelsFromFilm();

    bool apply = group.readEntry(d->configApplyColorBalance, true);
    d->filmContainer.setApplyBalance(apply);
    d->colorBalanceInput->setCheckState(apply? Qt::Checked : Qt::Unchecked);

    d->levelsHistogramWidget->reset();
    d->gboxSettings->histogramBox()->histogram()->reset();

    ChannelType ch = (ChannelType)group.readEntry(d->configHistogramChannelEntry, (int)ColorChannels);

    // restore the previous channel
    d->gboxSettings->histogramBox()->setChannel(ch);

    d->gboxSettings->histogramBox()->setScale((HistogramScale)group.readEntry(d->configHistogramScaleEntry,
            (int)LogScaleHistogram));

    slotAdjustSliders();
    slotChannelChanged();
    slotScaleChanged();
}

void FilmTool::writeSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);
    bool sb                   = d->originalImage->sixteenBit();

    group.writeEntry(d->configHistogramChannelEntry, (int)d->gboxSettings->histogramBox()->channel());
    group.writeEntry(d->configHistogramScaleEntry,   (int)d->gboxSettings->histogramBox()->scale());

    double gamma = d->gammaInput->value();
    group.writeEntry(d->configGammaInputEntry, gamma);

    double exposure = d->exposureInput->value();
    group.writeEntry(d->configExposureEntry, exposure);

    int cnType = (int)d->filmContainer.cnType();
    group.writeEntry(d->configFilmProfileEntry, cnType);

    group.writeEntry(d->configFilmProfileName, d->cnType->currentItem()->text());

    int red   = d->filmContainer.whitePoint().red();
    int green = d->filmContainer.whitePoint().green();
    int blue  = d->filmContainer.whitePoint().blue();

    group.writeEntry(d->configWhitePointEntry.arg(1), sb ? red   : red   * 256);
    group.writeEntry(d->configWhitePointEntry.arg(2), sb ? green : green * 256);
    group.writeEntry(d->configWhitePointEntry.arg(3), sb ? blue  : blue  * 256);

    bool apply = d->colorBalanceInput->checkState() == Qt::Checked;
    group.writeEntry(d->configApplyColorBalance, apply);
    config->sync();
}

void FilmTool::preparePreview()
{
    d->gboxSettings->histogramBox()->histogram()->stopHistogramComputation();

    DImg preview = d->previewWidget->getOriginalRegionImage(true);
    setFilter(new FilmFilter(&preview, this, d->filmContainer));
}

void FilmTool::prepareFinal()
{
    ImageIface iface;
    setFilter(new FilmFilter(iface.original(), this, d->filmContainer));
}

void FilmTool::setPreviewImage()
{
    DImg preview = filter()->getTargetImage();
    d->previewWidget->setPreviewImage(preview);

    // Update histogram.

    d->gboxSettings->histogramBox()->histogram()->updateData(preview.copy(), DImg(), false);
}

void FilmTool::setFinalImage()
{
    ImageIface iface;
    iface.setOriginal(i18n("Film"), filter()->filterAction(), filter()->getTargetImage());
}

bool FilmTool::eventFilter(QObject* obj, QEvent* ev)
{
    // swallow mouse evens for level sliders to make them immutable
    if (obj == d->redInputLevels || obj == d->greenInputLevels || obj == d->blueInputLevels)
    {
        if (ev->type() == QEvent::MouseButtonPress   ||
            ev->type() == QEvent::MouseButtonRelease ||
            ev->type() == QEvent::MouseMove          ||
            ev->type() == QEvent::MouseButtonDblClick)
            return true;
    }

    // pass all other events to the parent class
    return EditorToolThreaded::eventFilter(obj, ev);
}

}  // namespace Digikam
