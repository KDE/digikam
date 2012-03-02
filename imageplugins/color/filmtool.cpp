/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-07-20
 * Description : film negative tool
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

#include "filmtool.moc"

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

// KDE includes

#include <kapplication.h>
#include <kconfig.h>
#include <kcursor.h>
#include <kfiledialog.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kicon.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>

// LibKDcraw includes

#include <libkdcraw/rnuminput.h>

// Local includes

#include "dgradientslider.h"
#include "dimg.h"
#include "editortoolsettings.h"
#include "histogrambox.h"
#include "histogramwidget.h"
#include "imagehistogram.h"
#include "imageiface.h"
#include "imagelevels.h"
#include "imageregionwidget.h"
#include "filmfilter.h"

using namespace KDcrawIface;

namespace DigikamColorImagePlugin
{

class FilmTool::FilmToolPriv
{

public:

    enum ColorPicker
    {
        NoPicker   = 0,
        OrangeMask = 1
    };

public:

    FilmToolPriv() :
        destinationPreviewData(0),
        histoSegments(0),
        resetButton(0),
        pickOrangeMask(0),
        strengthInput(0),
        gammaInput(0),
        cnType(0),
        levelsHistogramWidget(0),
        redInputLevels(0),
        greenInputLevels(0),
        blueInputLevels(0),
        previewWidget(0),
        levels(0),
        originalImage(0),
        gboxSettings(0)
    {}

    static const QString configGroupName;
    static const QString configGammaInputEntry;
    static const QString configStrengthEntry;
    static const QString configFilmProfileEntry;
    static const QString configFilmProfileName;
    static const QString configWhitePointEntry;
    static const QString configHistogramChannelEntry;
    static const QString configHistogramScaleEntry;

    uchar*               destinationPreviewData;

    int                  histoSegments;

    QPushButton*         resetButton;
    QToolButton*         pickOrangeMask;

    FilmContainer        filmContainer;

    RDoubleNumInput*     strengthInput;
    RDoubleNumInput*     gammaInput;
    QListWidget*         cnType;

    HistogramWidget*     levelsHistogramWidget;

    DGradientSlider*     redInputLevels;
    DGradientSlider*     greenInputLevels;
    DGradientSlider*     blueInputLevels;

    ImageRegionWidget*   previewWidget;

    ImageLevels*         levels;

    DImg*                originalImage;

    EditorToolSettings*  gboxSettings;
};
const QString FilmTool::FilmToolPriv::configGroupName("film Tool");
const QString FilmTool::FilmToolPriv::configGammaInputEntry("GammaInput");
const QString FilmTool::FilmToolPriv::configStrengthEntry("Strength");
const QString FilmTool::FilmToolPriv::configFilmProfileEntry("FilmProfile");
const QString FilmTool::FilmToolPriv::configFilmProfileName("FilmProfileName");
const QString FilmTool::FilmToolPriv::configWhitePointEntry("WhitePoint_%1");
const QString FilmTool::FilmToolPriv::configHistogramChannelEntry("Histogram Channel");
const QString FilmTool::FilmToolPriv::configHistogramScaleEntry("Histogram Scale");

// --------------------------------------------------------

FilmTool::FilmTool(QObject* parent)
    : EditorToolThreaded(parent),
      d(new FilmToolPriv)
{
    setObjectName("film");
    setToolName(i18n("Film"));
    //setToolIcon(SmallIcon("film"));

    ImageIface iface(0, 0);
    d->originalImage = iface.getOriginalImg();

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
    d->gboxSettings->setHistogramType(Digikam::LRGBC);

    // we don't need to use the Gradient in this tool
    d->gboxSettings->histogramBox()->setGradientVisible(false);

    d->gboxSettings->histogramBox()->setChannel(ColorChannels);

    // -------------------------------------------------------------

    d->levelsHistogramWidget = new HistogramWidget(256, 140, d->originalImage->bits(),
            d->originalImage->width(),
            d->originalImage->height(),
            d->originalImage->sixteenBit(),
            d->gboxSettings->plainPage(), false);
    d->levelsHistogramWidget->setWhatsThis(i18n("This is the histogram drawing of the selected channel "
                                           "from the original image."));
    d->levelsHistogramWidget->setChannelType(ColorChannels);
    QHBoxLayout* inputLevelsLayout = new QHBoxLayout;
    inputLevelsLayout->addWidget(d->levelsHistogramWidget);

    // -------------------------------------------------------------

    d->redInputLevels = new DGradientSlider();
    d->redInputLevels->setColors(QColor("Red"), QColor("White"));
    d->redInputLevels->setWhatsThis( i18n("Select the input intensity of the histogram here."));
    d->redInputLevels->setToolTip( i18n( "Input intensity." ) );
    d->redInputLevels->installEventFilter(this);

    d->greenInputLevels = new DGradientSlider();
    d->greenInputLevels->setColors(QColor("Green"), QColor("White"));
    d->greenInputLevels->setWhatsThis( i18n("Select the input intensity of the histogram here."));
    d->greenInputLevels->setToolTip( i18n( "Input intensity." ) );
    d->greenInputLevels->installEventFilter(this);

    d->blueInputLevels = new DGradientSlider();
    d->blueInputLevels->setColors(QColor("Blue"), QColor("White"));
    d->blueInputLevels->setWhatsThis( i18n("Select the input intensity of the histogram here."));
    d->blueInputLevels->setToolTip( i18n( "Input intensity." ) );
    d->blueInputLevels->installEventFilter(this);

    d->gboxSettings->histogramBox()->
            setHistogramMargin(d->redInputLevels->gradientOffset());

    inputLevelsLayout->setContentsMargins(d->redInputLevels->gradientOffset(), 0,
                                          d->redInputLevels->gradientOffset(), 0);

    // -------------------------------------------------------------

    d->cnType = new QListWidget();
    QList<FilmContainer::ListItem*> profiles = d->filmContainer.profileItemList(d->cnType);
    QList<FilmContainer::ListItem*>::ConstIterator it;
    for (it = profiles.constBegin(); it != profiles.constEnd(); it++)
        d->cnType->addItem(*it);

    // -------------------------------------------------------------

    d->pickOrangeMask = new QToolButton();
    d->pickOrangeMask->setIcon(KIcon("color-picker-black"));
    d->pickOrangeMask->setCheckable(true);
    d->pickOrangeMask->setToolTip( i18n( "Orange mask color picker" ) );
    d->pickOrangeMask->setWhatsThis(i18n("With this button, you can pick the color of the orange mask "
            "of the scanned color negative. It represents the darkest black tone of the positive image "
            "after inversion. It is also the reference point for applying the film profile."));

    d->resetButton = new QPushButton(i18n("&Reset"));
    d->resetButton->setIcon(KIconLoader::global()->loadIcon("document-revert", KIconLoader::Toolbar));
    d->resetButton->setToolTip( i18n( "Reset current channel levels' values." ) );
    d->resetButton->setWhatsThis(i18n("If you press this button, all levels' values "
                                      "from the currently selected channel "
                                      "will be reset to the default values."));

    QLabel* space = new QLabel();
    space->setFixedWidth(d->gboxSettings->spacingHint());

    QHBoxLayout* l3 = new QHBoxLayout();
    l3->addWidget(d->pickOrangeMask);
    l3->addWidget(space);
    l3->addWidget(d->resetButton);
    l3->addStretch(10);

    // -------------------------------------------------------------

    d->strengthInput = new RDoubleNumInput();
    d->strengthInput->setDecimals(2);
    d->strengthInput->setRange(0.0, 40.0, 0.01);
    d->strengthInput->setDefaultValue(1.0);
    d->strengthInput->setToolTip( i18n( "Strength value." ) );
    d->strengthInput->setWhatsThis( i18n("Select the conversion strength value here."));

    d->gammaInput = new RDoubleNumInput();
    d->gammaInput->setDecimals(2);
    d->gammaInput->setRange(0.1, 3.0, 0.01);
    d->gammaInput->setDefaultValue(1.0);
    d->gammaInput->setToolTip( i18n( "Gamma input value." ) );
    d->gammaInput->setWhatsThis( i18n("Select the gamma input value here."));

    // -------------------------------------------------------------

    QGridLayout* grid = new QGridLayout();
    grid->addLayout(inputLevelsLayout,    0, 0, 1, 7);
    grid->addWidget(d->redInputLevels,    1, 0, 1, 7);
    grid->addWidget(d->greenInputLevels,  2, 0, 1, 7);
    grid->addWidget(d->blueInputLevels,   3, 0, 1, 7);
    grid->addWidget(d->cnType,            4, 0, 1, 7);
    grid->addWidget(d->strengthInput,     5, 0, 1, 7);
    grid->addWidget(d->gammaInput,        6, 0, 1, 7);
    grid->addLayout(l3,                   7, 0, 1, 7);

    // TODO: fill in rest of settings elements

    //grid->setRowStretch(7, 10);
    //grid->setColumnStretch(2, 10);
    //grid->setColumnStretch(4, 10);
    grid->setMargin(0);
    grid->setSpacing(d->gboxSettings->spacingHint());
    d->gboxSettings->plainPage()->setLayout(grid);

    // -------------------------------------------------------------

    d->filmContainer.setSixteenBit(d->originalImage->sixteenBit());
    d->filmContainer.setWhitePoint(DColor(QColor("white"), d->originalImage->sixteenBit()));

    // -------------------------------------------------------------

    setToolSettings(d->gboxSettings);
    init();

    // Button Slots -------------------------------------------------

    connect(d->pickOrangeMask, SIGNAL(toggled(bool)),
            this, SLOT(slotPickerColorButtonActived(bool)));

    // Slots --------------------------------------------------------

    connect(d->previewWidget, SIGNAL(signalResized()),
            this, SLOT(slotEffect()));
    connect(d->previewWidget, SIGNAL(signalCapturedPointFromOriginal(Digikam::DColor,QPoint)),
            this, SLOT(slotColorSelectedFromTarget(Digikam::DColor,QPoint)));
    connect(d->strengthInput, SIGNAL(valueChanged(double)),
            this, SLOT(slotSaturationChanged(double)));
    connect(d->gammaInput, SIGNAL(valueChanged(double)),
            this, SLOT(slotGammaInputChanged(double)));
    connect(d->resetButton, SIGNAL(clicked()),
            this, SLOT(slotResetWhitePoint()));

    connect(d->cnType, SIGNAL(itemActivated(QListWidgetItem*)),
            this, SLOT(slotFilmItemActivated(QListWidgetItem*)));

    slotTimer();
}

FilmTool::~FilmTool()
{
    if (d->destinationPreviewData)
        delete [] d->destinationPreviewData;

    delete d->levels;
    delete d;
}

void FilmTool::slotResetSettings()
{
    bool sb = d->originalImage->sixteenBit();
    int max = sb ? 65535 : 255;

    FilmContainer::CNFilmProfile cnType = FilmContainer::CNNeutral;

    QString profileName = QString("Neutral");
    QList<QListWidgetItem*> matchingItems = d->cnType->findItems(profileName, Qt::MatchExactly);
    d->cnType->setCurrentItem(matchingItems.first());

    double gamma = 1.0;
    d->gammaInput->setValue(gamma);
    gammaInputChanged(gamma);

    double saturation = 1.0;
    d->strengthInput->setValue(saturation);

    d->filmContainer = FilmContainer(cnType, gamma, d->originalImage->sixteenBit());
    d->filmContainer.setStrength(saturation);

    int red = max;
    int green = max;
    int blue = max;

    red   = sb ? red : red / 256;
    green = sb ? green : green / 256;
    blue  = sb ? blue : blue / 256;

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
    d->levelsHistogramWidget->setChannelType(
            d->gboxSettings->histogramBox()->channel());
}

void FilmTool::slotScaleChanged()
{
   d->levelsHistogramWidget->setScaleType(
           d->gboxSettings->histogramBox()->scale());
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

    d->gammaInput->setValue(d->filmContainer.gamma());
    d->strengthInput->setValue(d->filmContainer.strength());
}

void FilmTool::setLevelsFromFilm()
{
    LevelsContainer l = d->filmContainer.toLevels();
    for (int i = RedChannel; i <= BlueChannel; i++) {
        d->levels->setLevelLowInputValue(i, l.lInput[i]);
        d->levels->setLevelHighInputValue(i, l.hInput[i]);
        d->levels->setLevelLowOutputValue(i, l.lOutput[i]);
        d->levels->setLevelHighOutputValue(i, l.hOutput[i]);
        d->levels->setLevelGammaValue(i, l.gamma[i]);
    }
    slotAdjustSliders();
}

void FilmTool::slotSaturationChanged(double val)
{
    d->filmContainer.setStrength(val);
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
    double gamma = d->filmContainer.gamma();
    double strength = d->filmContainer.strength();
    DColor wp = d->filmContainer.whitePoint();

    FilmContainer::CNFilmProfile type = (FilmContainer::CNFilmProfile)(item->type()-QListWidgetItem::UserType);
    d->filmContainer = FilmContainer(type, gamma, d->originalImage->sixteenBit());
    d->filmContainer.setStrength(strength);
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

    wp00.setRed(QMIN(wp00.red(), wp01.red()));
    wp00.setGreen(QMIN(wp00.green(), wp01.green()));
    wp00.setBlue(QMIN(wp00.blue(), wp01.blue()));

    wp00.setRed(QMIN(wp00.red(), wp10.red()));
    wp00.setGreen(QMIN(wp00.green(), wp10.green()));
    wp00.setBlue(QMIN(wp00.blue(), wp10.blue()));

    wp00.setRed(QMIN(wp00.red(), wp11.red()));
    wp00.setGreen(QMIN(wp00.green(), wp11.green()));
    wp00.setBlue(QMIN(wp00.blue(), wp11.blue()));

    d->filmContainer.setWhitePoint(wp00);
    d->previewWidget->setCapturePointMode(false);
    d->pickOrangeMask->setChecked(false);

    setLevelsFromFilm();
    slotTimer();
}

void FilmTool::slotPickerColorButtonActived(bool checked)
{
    if (checked)
        d->previewWidget->setCapturePointMode(true);
}

void FilmTool::slotResetWhitePoint()
{
    d->filmContainer.setSixteenBit(d->originalImage->sixteenBit());
    d->filmContainer.setWhitePoint(DColor(QColor("white"), d->originalImage->sixteenBit()));

    setLevelsFromFilm();
    slotEffect();
}

void FilmTool::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);

    bool sb = d->originalImage->sixteenBit();
    int max = sb ? 65535 : 255;

    FilmContainer::CNFilmProfile cnType = (FilmContainer::CNFilmProfile)
            group.readEntry(d->configFilmProfileEntry, (int)FilmContainer::CNNeutral);

    QString profileName = group.readEntry(d->configFilmProfileName, QString("Neutral"));
    QList<QListWidgetItem*> matchingItems = d->cnType->findItems(profileName, Qt::MatchExactly);
    d->cnType->setCurrentItem(matchingItems.first());

    double gamma = group.readEntry(d->configGammaInputEntry, 1.0);
    d->gammaInput->setValue(gamma);
    gammaInputChanged(gamma);

    double saturation = group.readEntry(d->configStrengthEntry, 1.0);
    d->strengthInput->setValue(saturation);

    d->filmContainer = FilmContainer(cnType, gamma, d->originalImage->sixteenBit());
    d->filmContainer.setStrength(saturation);

    int red = group.readEntry(d->configWhitePointEntry.arg(1), max);
    int green = group.readEntry(d->configWhitePointEntry.arg(2), max);
    int blue = group.readEntry(d->configWhitePointEntry.arg(3), max);

    red   = sb ? red : red / 256;
    green = sb ? green : green / 256;
    blue  = sb ? blue : blue / 256;

    DColor whitePoint = DColor(red, green, blue, max, sb);
    d->filmContainer.setWhitePoint(whitePoint);
    setLevelsFromFilm();

    d->levelsHistogramWidget->reset();
    d->gboxSettings->histogramBox()->histogram()->reset();

    ChannelType ch = (ChannelType)group.readEntry(d->configHistogramChannelEntry,
            (int)ColorChannels);

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
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);
    bool sb = d->originalImage->sixteenBit();

    group.writeEntry(d->configHistogramChannelEntry, (int)d->gboxSettings->histogramBox()->channel());
    group.writeEntry(d->configHistogramScaleEntry,   (int)d->gboxSettings->histogramBox()->scale());

    double gamma = d->gammaInput->value();
    group.writeEntry(d->configGammaInputEntry, gamma);

    double saturation = d->strengthInput->value();
    group.writeEntry(d->configStrengthEntry, saturation);

    int cnType = (int)d->filmContainer.cnType();
    group.writeEntry(d->configFilmProfileEntry, cnType);

   group.writeEntry(d->configFilmProfileName, d->cnType->currentItem()->text());

    int red = d->filmContainer.whitePoint().red();
    int green = d->filmContainer.whitePoint().green();
    int blue = d->filmContainer.whitePoint().blue();

    group.writeEntry(d->configWhitePointEntry.arg(1), sb ? red : red * 256);
    group.writeEntry(d->configWhitePointEntry.arg(2), sb ? green : green * 256);
    group.writeEntry(d->configWhitePointEntry.arg(3), sb ? blue : blue * 256);

    config->sync();

}

void FilmTool::prepareEffect()
{
    d->gboxSettings->histogramBox()->histogram()->stopHistogramComputation();

    DImg preview = d->previewWidget->getOriginalRegionImage(true);
    setFilter(new FilmFilter(&preview, this, d->filmContainer));
}

void FilmTool::prepareFinal()
{
    ImageIface iface(0, 0);
    setFilter(new FilmFilter(iface.getOriginalImg(), this, d->filmContainer));
}

void FilmTool::putPreviewData()
{
    DImg preview = filter()->getTargetImage();
    d->previewWidget->setPreviewImage(preview);

    // Update histogram.

    if (d->destinationPreviewData)
        delete [] d->destinationPreviewData;

    d->destinationPreviewData = preview.copyBits();
    d->gboxSettings->histogramBox()->histogram()->updateData(d->destinationPreviewData,
            preview.width(), preview.height(), preview.sixteenBit(),
            0, 0, 0, false);
}

void FilmTool::putFinalData()
{
    ImageIface iface(0, 0);
    iface.putOriginalImage(i18n("Film"),
            filter()->filterAction(), filter()->getTargetImage().bits());
}

bool FilmTool::eventFilter(QObject* obj, QEvent* ev)
{
    // pass the event on to the parent class
    return EditorToolThreaded::eventFilter(obj, ev);
}

}  // namespace DigikamColorImagePlugin
