/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-04-07
 * Description : a tool to resize an image
 *
 * Copyright (C) 2005-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "resizetool.h"

// C++ includes

#include <cmath>

// Qt includes

#include <QBrush>
#include <QCheckBox>
#include <QCloseEvent>
#include <QRadioButton>
#include <QComboBox>
#include <QEvent>
#include <QFile>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QImage>
#include <QLabel>
#include <QPixmap>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>
#include <QVector>
#include <QIcon>
#include <QApplication>
#include <QStandardPaths>
#include <QMessageBox>

// KDE includes

#include <klocalizedstring.h>
#include <ksharedconfig.h>

// Local includes

#include "dlayoutbox.h"
#include "dnuminput.h"
#include "dimg.h"
#include "dimgbuiltinfilter.h"
#include "imageiface.h"
#include "imageguidewidget.h"
#include "editortoolsettings.h"
#include "editortooliface.h"
#include "dimgthreadedfilter.h"
#include "greycstorationfilter.h"
#include "greycstorationsettings.h"
#include "dactivelabel.h"
#include "dexpanderbox.h"
#include "dfiledialog.h"

namespace DigikamEditorResizeToolPlugin
{

class Q_DECL_HIDDEN ResizeTool::Private
{
public:

    enum WidthPreset
    {
        Original = 0,
        Tiny,
        Small,
        Medium,
        Big,
        Large,
        Huge,
        UHD4K,
        A3,
        A4,
        A6,
        Letter,
        Print1,
        Print2,
        Print3,
        Print4
    };

    enum Units
    {
        Pixels,
        Inches,
        Centimeters,
        Millimeters
    };

    explicit Private()
      : orgWidth(0),
        orgHeight(0),
        prevW(0),
        prevH(0),
        prevWP(0.0),
        prevHP(0.0),
        restorationTips(nullptr),
        presetCBox(nullptr),
        units(nullptr),
        preserveRatioBox(nullptr),
        useGreycstorationBox(nullptr),
        mainTab(nullptr),
        cimgLogoLabel(nullptr),
        previewWidget(nullptr),
        wInput(nullptr),
        hInput(nullptr),
        resolution(nullptr),
        wpInput(nullptr),
        hpInput(nullptr),
        gboxSettings(nullptr),
        settingsWidget(nullptr)
    {
    }

    QSize presetLengthValue(WidthPreset preset);

    static const QString    configGroupName;
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

    int                     orgWidth;
    int                     orgHeight;
    int                     prevW;
    int                     prevH;

    double                  prevWP;
    double                  prevHP;

    QLabel*                 restorationTips;

    QComboBox*              presetCBox;
    QComboBox*              units;

    QCheckBox*              preserveRatioBox;
    QCheckBox*              useGreycstorationBox;

    QTabWidget*             mainTab;

    DActiveLabel*           cimgLogoLabel;

    ImageGuideWidget*       previewWidget;

    DDoubleNumInput*        wInput;
    DDoubleNumInput*        hInput;
    DIntNumInput*           resolution;

    DDoubleNumInput*        wpInput;
    DDoubleNumInput*        hpInput;

    EditorToolSettings*     gboxSettings;
    GreycstorationSettings* settingsWidget;
};

QSize ResizeTool::Private::presetLengthValue(WidthPreset preset)
{
    QSize size;

    switch (preset)
    {
        case Private::Original:
            size = QSize(orgWidth, orgHeight);
            break;

        case Private::Tiny:
            size = QSize(320, 200);
            break;

        case Private::Small:
            size = QSize(640, 480);
            break;

        case Private::Medium:
            size = QSize(960, 640);
            break;

        case Private::Big:
            size = QSize(1024, 768);
            break;

        case Private::Large:
            size = QSize(1280, 720);
            break;

        case Private::Huge:
            size = QSize(1920, 1080);
            break;

        case Private::UHD4K:
            size = QSize(3840, 2160);
            break;

        case Private::A3:
            size = QSize(3508, 4961);
            break;

        case Private::A4:
            size = QSize(2480, 3508);
            break;

        case Private::A6:
            size = QSize(1240, 1748);
            break;

        case Private::Letter:
            size = QSize(2550, 3300);
            break;

        case Private::Print1:
            size = QSize(1200, 1800);
            break;

        case Private::Print2:
            size = QSize(1500, 2100);
            break;

        case Private::Print3:
            size = QSize(2400, 3000);
            break;

        default:
            size = QSize(3300, 4200);
            break;
    }

    return size;
}

const QString ResizeTool::Private::configGroupName(QLatin1String("resize Tool"));
const QString ResizeTool::Private::configFastApproxEntry(QLatin1String("FastApprox"));
const QString ResizeTool::Private::configInterpolationEntry(QLatin1String("Interpolation"));
const QString ResizeTool::Private::configAmplitudeEntry(QLatin1String("Amplitude"));
const QString ResizeTool::Private::configSharpnessEntry(QLatin1String("Sharpness"));
const QString ResizeTool::Private::configAnisotropyEntry(QLatin1String("Anisotropy"));
const QString ResizeTool::Private::configAlphaEntry(QLatin1String("Alpha"));
const QString ResizeTool::Private::configSigmaEntry(QLatin1String("Sigma"));
const QString ResizeTool::Private::configGaussPrecEntry(QLatin1String("GaussPrec"));
const QString ResizeTool::Private::configDlEntry(QLatin1String("Dl"));
const QString ResizeTool::Private::configDaEntry(QLatin1String("Da"));
const QString ResizeTool::Private::configIterationEntry(QLatin1String("Iteration"));
const QString ResizeTool::Private::configTileEntry(QLatin1String("Tile"));
const QString ResizeTool::Private::configBTileEntry(QLatin1String("BTile"));

// -------------------------------------------------------------

ResizeTool::ResizeTool(QObject* const parent)
    : EditorToolThreaded(parent),
      d(new Private)
{
    setObjectName(QLatin1String("resizeimage"));

    d->previewWidget = new ImageGuideWidget(nullptr, false, ImageGuideWidget::HVGuideMode, Qt::red, 1, false);
    setToolView(d->previewWidget);
    setPreviewModeMask(PreviewToolBar::UnSplitPreviewModes);

    // -------------------------------------------------------------

    d->gboxSettings = new EditorToolSettings(nullptr);
    d->gboxSettings->setButtons(EditorToolSettings::Default |
                                EditorToolSettings::Try     |
                                EditorToolSettings::Ok      |
                                EditorToolSettings::Load    |
                                EditorToolSettings::SaveAs  |
                                EditorToolSettings::Cancel);

    ImageIface iface;
    d->orgWidth  = iface.originalSize().width();
    d->orgHeight = iface.originalSize().height();
    d->prevW     = d->orgWidth;
    d->prevH     = d->orgHeight;
    d->prevWP    = 100.0;
    d->prevHP    = 100.0;

    // -------------------------------------------------------------

    d->mainTab               = new QTabWidget();
    QWidget* const firstPage = new QWidget(d->mainTab);
    QGridLayout* const grid  = new QGridLayout(firstPage);

    d->mainTab->addTab(firstPage, i18n("New Size"));

    QLabel* const label = new QLabel(i18n("Preset Resolutions:"), firstPage);
    d->presetCBox       = new QComboBox(firstPage);
    d->presetCBox->addItem(i18nc("Original (size)", "Original (%1 x %2 pixels)", d->presetLengthValue(Private::Original).width(),
                                                                                 d->presetLengthValue(Private::Original).height()),
                                                                                 Private::Original);
    d->presetCBox->addItem(i18nc("Tiny (size)",     "Tiny (%1 x %2 pixels)",     d->presetLengthValue(Private::Tiny).width(),
                                                                                 d->presetLengthValue(Private::Tiny).height()),
                                                                                 Private::Tiny);
    d->presetCBox->addItem(i18nc("Small (size)",    "Small (%1 x %2 pixels)",    d->presetLengthValue(Private::Small).width(),
                                                                                 d->presetLengthValue(Private::Small).height()),
                                                                                 Private::Small);
    d->presetCBox->addItem(i18nc("Medium (size)",   "Medium (%1 x %2 pixels)",   d->presetLengthValue(Private::Medium).width(),
                                                                                 d->presetLengthValue(Private::Medium).height()),
                                                                                 Private::Medium);
    d->presetCBox->addItem(i18nc("Big (size)",      "Big (%1 x %2 pixels)",      d->presetLengthValue(Private::Big).width(),
                                                                                 d->presetLengthValue(Private::Big).height()),
                                                                                 Private::Big);
    d->presetCBox->addItem(i18nc("Large (size)",    "Large (%1 x %2 pixels)",    d->presetLengthValue(Private::Large).width(),
                                                                                 d->presetLengthValue(Private::Large).height()),
                                                                                 Private::Large);
    d->presetCBox->addItem(i18nc("Huge (size)",     "Huge (%1 x %2 pixels)",     d->presetLengthValue(Private::Huge).width(),
                                                                                 d->presetLengthValue(Private::Huge).height()),
                                                                                 Private::Huge);
    d->presetCBox->addItem(i18nc("UHD4K (size)",    "UHD-4K (%1 x %2 pixels)",   d->presetLengthValue(Private::UHD4K).width(),
                                                                                 d->presetLengthValue(Private::UHD4K).height()),
                                                                                 Private::UHD4K);
    d->presetCBox->addItem(i18nc("A3 (size)",       "A3 (297 x 420 mm)"),        Private::A3);
    d->presetCBox->addItem(i18nc("A4 (size)",       "A4 (210 x 297 mm)"),        Private::A4);
    d->presetCBox->addItem(i18nc("A6 (size)",       "A6 (105 x 148 mm)"),        Private::A6);
    d->presetCBox->addItem(i18nc("Letter (size)",   "Letter (8.5 x 11 in)"),     Private::Letter);
    d->presetCBox->addItem(i18nc("Print (size)",    "4 x 6 in"),                 Private::Print1);
    d->presetCBox->addItem(i18nc("Print (size)",    "5 x 7 in"),                 Private::Print2);
    d->presetCBox->addItem(i18nc("Print (size)",    "8 x 10 in"),                Private::Print3);
    d->presetCBox->addItem(i18nc("Print (size)",    "11 x 14 in"),               Private::Print4);
    d->presetCBox->insertSeparator((int)Private::A3);
    d->presetCBox->insertSeparator((int)Private::Tiny);

    QLabel* const label1    = new QLabel(i18n("Units:"), firstPage);
    d->units                = new QComboBox(firstPage);
    d->units->addItem(i18n("Pixels (px)"),      Private::Pixels);
    d->units->addItem(i18n("Inches (in)"),      Private::Inches);
    d->units->addItem(i18n("Centimeters (cm)"), Private::Centimeters);
    d->units->addItem(i18n("Millimeters (mm)"), Private::Millimeters);

    d->preserveRatioBox = new QCheckBox(i18n("Maintain aspect ratio"), firstPage);
    d->preserveRatioBox->setWhatsThis( i18n("Enable this option to maintain aspect "
                                            "ratio with new image sizes."));

    QLabel* const label2 = new QLabel(i18n("Width:"), firstPage);
    d->wInput            = new DDoubleNumInput(firstPage);
    d->wInput->setRange(1.0, (double)qMax(d->orgWidth * 10, 15000), 1.0);
    d->wInput->setSuffix(QLatin1Char(' ') + i18n("px"));
    d->wInput->setObjectName(QLatin1String("wInput"));
    d->wInput->setDefaultValue((double)d->orgWidth);
    d->wInput->setDecimals(0);
    d->wInput->setWhatsThis( i18n("Set here the new image width in pixels."));

    QLabel* const label3 = new QLabel(i18n("Height:"), firstPage);
    d->hInput            = new DDoubleNumInput(firstPage);
    d->hInput->setRange(1.0, (double)qMax(d->orgHeight * 10, 15000), 1.0);
    d->hInput->setSuffix(QLatin1Char(' ') + i18n("px"));
    d->hInput->setObjectName(QLatin1String("hInput"));
    d->hInput->setDefaultValue((double)d->orgHeight);
    d->hInput->setDecimals(0);
    d->hInput->setWhatsThis( i18n("New image height in pixels (px)."));

    QLabel* const label4 = new QLabel(i18n("Width (%):"), firstPage);
    d->wpInput           = new DDoubleNumInput(firstPage);
    d->wpInput->setObjectName(QLatin1String("wpInput"));
    d->wpInput->setRange(1.0, 999.0, 1.0);
    d->wpInput->setDefaultValue(100.0);
    d->wInput->setDecimals(0);
    d->wpInput->setWhatsThis( i18n("New image width in percent (%)."));

    QLabel* const label5 = new QLabel(i18n("Height (%):"), firstPage);
    d->hpInput           = new DDoubleNumInput(firstPage);
    d->hpInput->setObjectName(QLatin1String("hpInput"));
    d->hpInput->setRange(1.0, 999.0, 1.0);
    d->hpInput->setDefaultValue(100.0);
    d->wInput->setDecimals(0);
    d->hpInput->setWhatsThis( i18n("New image height in percent (%)."));

    QLabel* const label6 = new QLabel(i18n("Resolution:"), firstPage);
    d->resolution        = new DIntNumInput(firstPage);
    d->resolution->setRange(1, 500, 1);
    d->resolution->setDefaultValue(300);
    d->resolution->setSuffix(i18n(" Pixel/Inch"));
    d->resolution->setObjectName(QLatin1String("resolution"));
    d->resolution->setWhatsThis( i18n("New image resolution in pixels/inch."));

    d->cimgLogoLabel        = new DActiveLabel(QUrl(QLatin1String("http://cimg.sourceforge.net")),
                                               QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                                                      QLatin1String("digikam/data/logo-cimg.png")), firstPage);
    d->cimgLogoLabel->setToolTip(i18n("Visit CImg library website"));

    d->useGreycstorationBox = new QCheckBox(i18n("Restore photograph (slow)"), firstPage);
    d->useGreycstorationBox->setWhatsThis( i18n("Enable this option to scale-up an image to a huge size. "
                                           "<b>Warning</b>: This process can take some time."));

    d->restorationTips      = new QLabel(i18n("<b>Note:</b> use Restoration Mode to scale-up an image to a huge size. "
                                              "This process can take some time."), firstPage);
    d->restorationTips->setWordWrap(true);

    const int spacing = d->gboxSettings->spacingHint();

    grid->addWidget(label,                                       0, 0, 1, 3);
    grid->addWidget(d->presetCBox,                               1, 0, 1, 3);
    grid->addWidget(d->preserveRatioBox,                         2, 0, 1, 3);
    grid->addWidget(label1,                                      3, 0, 1, 1);
    grid->addWidget(d->units,                                    3, 1, 1, 1);
    grid->addWidget(label2,                                      4, 0, 1, 1);
    grid->addWidget(d->wInput,                                   4, 1, 1, 2);
    grid->addWidget(label3,                                      5, 0, 1, 1);
    grid->addWidget(d->hInput,                                   5, 1, 1, 2);
    grid->addWidget(label4,                                      6, 0, 1, 1);
    grid->addWidget(d->wpInput,                                  6, 1, 1, 2);
    grid->addWidget(label5,                                      7, 0, 1, 1);
    grid->addWidget(d->hpInput,                                  7, 1, 1, 2);
    grid->addWidget(label6,                                      8, 0, 1, 1);
    grid->addWidget(d->resolution,                               8, 1, 1, 2);
    grid->addWidget(new DLineWidget(Qt::Horizontal, firstPage),  9, 0, 1, 3);
    grid->addWidget(d->cimgLogoLabel,                           10, 0, 3, 1);
    grid->addWidget(d->useGreycstorationBox,                    10, 1, 1, 2);
    grid->addWidget(d->restorationTips,                         11, 1, 1, 2);
    grid->setRowStretch(12, 10);
    grid->setContentsMargins(spacing, spacing, spacing, spacing);
    grid->setSpacing(spacing);

    // -------------------------------------------------------------

    d->settingsWidget = new GreycstorationSettings(d->mainTab);

    // -------------------------------------------------------------

    QGridLayout* grid2 = new QGridLayout();
    grid2->addWidget(d->mainTab, 0, 1, 1, 1);
    grid2->setContentsMargins(spacing, spacing, spacing, spacing);
    grid2->setSpacing(spacing);
    grid2->setRowStretch(1, 10);
    d->gboxSettings->plainPage()->setLayout(grid2);

    setToolSettings(d->gboxSettings);

    // -------------------------------------------------------------

    connect(d->presetCBox, SIGNAL(activated(int)),
            this, SLOT(slotPresetsChanged()));

    connect(d->units, SIGNAL(activated(int)),
            this, SLOT(slotUnitsChanged()));

    connect(d->wInput, SIGNAL(valueChanged(double)),
            this, SLOT(slotValuesChanged()));

    connect(d->hInput, SIGNAL(valueChanged(double)),
            this, SLOT(slotValuesChanged()));

    connect(d->wpInput, SIGNAL(valueChanged(double)),
            this, SLOT(slotValuesChanged()));

    connect(d->hpInput, SIGNAL(valueChanged(double)),
            this, SLOT(slotValuesChanged()));

    connect(d->resolution, SIGNAL(valueChanged(int)),
            this, SLOT(slotValuesChanged()));

    connect(d->useGreycstorationBox, SIGNAL(toggled(bool)),
            this, SLOT(slotRestorationToggled(bool)) );

    // -------------------------------------------------------------

    GreycstorationContainer defaults;
    defaults.setResizeDefaultSettings();
    d->settingsWidget->setDefaultSettings(defaults);

    QTimer::singleShot(0, this, SLOT(slotResetSettings()));
}

ResizeTool::~ResizeTool()
{
    delete d;
}

void ResizeTool::readSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);

    GreycstorationContainer prm;
    GreycstorationContainer defaults;
    defaults.setResizeDefaultSettings();

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
}

void ResizeTool::writeSettings()
{
    GreycstorationContainer prm = d->settingsWidget->settings();
    KConfigGroup group          = KSharedConfig::openConfig()->group(d->configGroupName);

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
    group.writeEntry("RestorePhotograph",         d->useGreycstorationBox->isChecked());
    group.sync();
}

void ResizeTool::slotResetSettings()
{
    GreycstorationContainer prm;
    prm.setResizeDefaultSettings();

    d->settingsWidget->setSettings(prm);
    d->useGreycstorationBox->setChecked(false);
    slotRestorationToggled(d->useGreycstorationBox->isChecked());

    d->units->setCurrentIndex(0);
    d->presetCBox->setCurrentIndex(0);
    d->preserveRatioBox->setChecked(true);

    slotUnitsChanged();

    blockWidgetSignals(true);

    d->wInput->slotReset();
    d->hInput->slotReset();
    d->wpInput->slotReset();
    d->hpInput->slotReset();
    d->resolution->slotReset();

    blockWidgetSignals(false);
}

void ResizeTool::slotPresetsChanged()
{
    Private::WidthPreset preset = (Private::WidthPreset)d->presetCBox->currentData().toInt();
    QSize size                  = d->presetLengthValue(preset);

    switch (preset)
    {
        case Private::A3:
        case Private::A4:
        case Private::A6:
            d->units->setCurrentIndex(d->units->findData(Private::Millimeters));

            if (d->orgWidth > d->orgHeight)
            {
                size.transpose();
            }

            break;

        case Private::Letter:
        case Private::Print1:
        case Private::Print2:
        case Private::Print3:
        case Private::Print4:
            d->units->setCurrentIndex(d->units->findData(Private::Inches));

            if (d->orgWidth > d->orgHeight)
            {
                size.transpose();
            }

            break;

        default:
            d->units->setCurrentIndex(d->units->findData(Private::Pixels));
            break;
    }

    slotUnitsChanged();
    d->resolution->slotReset();

    if (!d->preserveRatioBox->isChecked())
    {
        d->wInput->setValue(pixelsToUnits(size.width()));
        d->hInput->setValue(pixelsToUnits(size.height()));
    }
    else
    {
        d->wInput->setValue(pixelsToUnits(size.width()));

        if (d->prevH > size.height())
        {
            d->hInput->setValue(pixelsToUnits(size.height()));
        }
    }
}

void ResizeTool::slotUnitsChanged()
{
    blockWidgetSignals(true);

    int decimals = 0;
    QString suffix;

    switch ((Private::Units)d->units->currentData().toInt())
    {
        case Private::Pixels:
            decimals = 0;
            suffix   = i18nc("Pixels", "px");

            break;

        case Private::Inches:
            decimals = 1;
            suffix   = i18nc("Inches", "in");

            break;

        case Private::Millimeters:
            decimals = 1;
            suffix   = i18nc("Millimeters", "mm");

            break;

        case Private::Centimeters:
            decimals = 2;
            suffix   = i18nc("Centimeters", "cm");

            break;

        default:
            break;
    }

    d->wInput->setRange(1.0, pixelsToUnits(qMax(d->orgWidth * 10, 15000)), 1.0);
    d->wInput->setDefaultValue(pixelsToUnits(d->orgWidth));
    d->wInput->setSuffix(QLatin1Char(' ') + suffix);
    d->wInput->setDecimals(decimals);
    d->wInput->setValue(pixelsToUnits(d->prevW));

    d->hInput->setRange(1.0, pixelsToUnits(qMax(d->orgHeight * 10, 15000)), 1.0);
    d->hInput->setDefaultValue(pixelsToUnits(d->orgHeight));
    d->hInput->setSuffix(QLatin1Char(' ') + suffix);
    d->hInput->setDecimals(decimals);
    d->hInput->setValue(pixelsToUnits(d->prevH));

    blockWidgetSignals(false);
}

int ResizeTool::unitsToPixels(double val)
{
    Private::Units units = (Private::Units)d->units->currentData().toInt();
    int res              = d->resolution->value();
    int pixel            = qRound(val);

    switch (units)
    {
        case Private::Inches:
            pixel = pixel * res;
            break;

        case Private::Millimeters:
            pixel = qRound(pixel * res / 25.4);
            break;

        case Private::Centimeters:
            pixel = qRound(pixel * res / 2.54);
            break;

        default:
            break;
    }

    return pixel;
}

double ResizeTool::pixelsToUnits(int pix)
{
    Private::Units units = (Private::Units)d->units->currentData().toInt();
    int    res           = d->resolution->value();
    double val           = (double)pix;

    switch (units)
    {
        case Private::Inches:
            val = val / (double)res;
            break;

        case Private::Millimeters:
            val = (val * 25.4) / (double)res;
            break;

        case Private::Centimeters:
            val = (val * 2.54) / (double)res;
            break;

        default:
            break;
    }

    return val;
}

void ResizeTool::slotValuesChanged()
{
    blockWidgetSignals(true);

    QString s(sender()->objectName());

    if (s == QLatin1String("wInput"))
    {
        double val  = unitsToPixels(d->wInput->value());
        double pval = val / (double)(d->orgWidth) * 100.0;

        d->wpInput->setValue(pval);

        if (d->preserveRatioBox->isChecked())
        {
            double h = pixelsToUnits((int)(pval * d->orgHeight / 100));
            d->hpInput->setValue(pval);
            d->hInput->setValue(h);
        }
    }
    else if (s == QLatin1String("hInput"))
    {
        double val  = unitsToPixels(d->hInput->value());
        double pval = val / (double)(d->orgHeight) * 100.0;

        d->hpInput->setValue(pval);

        if (d->preserveRatioBox->isChecked())
        {
            double w = pixelsToUnits((int)(pval * d->orgWidth / 100));
            d->wpInput->setValue(pval);
            d->wInput->setValue(w);
        }
    }
    else if (s == QLatin1String("wpInput"))
    {
        double val = d->wpInput->value();
        double w   = pixelsToUnits((int)(val * d->orgWidth / 100));
        d->wInput->setValue(w);

        if (d->preserveRatioBox->isChecked())
        {
            double h = pixelsToUnits((int)(val * d->orgHeight / 100));
            d->hpInput->setValue(val);
            d->hInput->setValue(h);
        }
    }
    else if (s == QLatin1String("hpInput"))
    {
        double val = d->hpInput->value();
        double h   = pixelsToUnits((int)(val * d->orgHeight / 100));
        d->hInput->setValue(h);

        if (d->preserveRatioBox->isChecked())
        {
            double w = pixelsToUnits((int)(val * d->orgWidth / 100));
            d->wpInput->setValue(val);
            d->wInput->setValue(w);
        }
    }
    else if (s == QLatin1String("resolution"))
    {
        double hval  = unitsToPixels(d->hInput->value());
        double pHval = hval / (double)(d->orgHeight) * 100.0;
        double wval  = unitsToPixels(d->wInput->value());
        double pWval = wval / (double)(d->orgWidth) * 100.0;

        d->wpInput->setValue(pWval);
        d->hpInput->setValue(pHval);
    }

    d->prevW  = unitsToPixels(d->wInput->value());
    d->prevH  = unitsToPixels(d->hInput->value());
    d->prevWP = d->wpInput->value();
    d->prevHP = d->hpInput->value();

    blockWidgetSignals(false);
}

void ResizeTool::preparePreview()
{
    int h = unitsToPixels(d->hInput->value());
    int w = unitsToPixels(d->wInput->value());

    if (d->prevW  != d->wInput->value()  || d->prevH  != d->hInput->value() ||
        d->prevWP != d->wpInput->value() || d->prevHP != d->hpInput->value())
    {
        slotValuesChanged();
    }

    ImageIface* const iface = d->previewWidget->imageIface();
    DImg* imTemp            = iface->original();

    if (d->useGreycstorationBox->isChecked())
    {
        setFilter(new GreycstorationFilter(imTemp,
                                           d->settingsWidget->settings(),
                                           GreycstorationFilter::Resize,
                                           w, h,
                                           QImage(),
                                           this));
    }
    else
    {
        // See bug #152192: CImg resize() sound like defective or unadapted
        // to resize image without good quality.
        DImgBuiltinFilter resize(DImgBuiltinFilter::Resize, QSize(w, h));
        setFilter(resize.createThreadedFilter(imTemp, this));
    }
}

void ResizeTool::prepareFinal()
{
    int h = unitsToPixels(d->hInput->value());
    int w = unitsToPixels(d->wInput->value());

    if (d->prevW  != d->wInput->value()  || d->prevH  != d->hInput->value() ||
        d->prevWP != d->wpInput->value() || d->prevHP != d->hpInput->value())
    {
        slotValuesChanged();
    }

    d->mainTab->setCurrentIndex(0);

    ImageIface iface;

    if (d->useGreycstorationBox->isChecked())
    {
        setFilter(new GreycstorationFilter(iface.original(),
                                           d->settingsWidget->settings(),
                                           GreycstorationFilter::Resize,
                                           w,
                                           h,
                                           QImage(),
                                           this));
    }
    else
    {
        // See bug #152192: CImg resize() sound like defective or unadapted
        // to resize image without good quality.
        DImgBuiltinFilter resize(DImgBuiltinFilter::Resize, QSize(w, h));
        setFilter(resize.createThreadedFilter(iface.original(), this));
    }
}

void ResizeTool::setPreviewImage()
{
    ImageIface* const iface = d->previewWidget->imageIface();
    int w                   = iface->previewSize().width();
    int h                   = iface->previewSize().height();
    DImg imTemp             = filter()->getTargetImage().smoothScale(w, h, Qt::KeepAspectRatio);
    DImg imDest(w, h, filter()->getTargetImage().sixteenBit(), filter()->getTargetImage().hasAlpha());

    QColor background = toolView()->backgroundRole();
    imDest.fill(DColor(background, filter()->getTargetImage().sixteenBit()));
    imDest.bitBltImage(&imTemp, (w-imTemp.width())/2, (h-imTemp.height())/2);

    iface->setPreview(imDest.smoothScale(iface->previewSize()));
    d->previewWidget->updatePreview();
}

void ResizeTool::renderingFinished()
{
    d->settingsWidget->setEnabled(d->useGreycstorationBox->isChecked());
}

void ResizeTool::setFinalImage()
{
    ImageIface iface;
    DImg targetImage = filter()->getTargetImage();
    iface.setOriginal(i18n("Resize"),
                           filter()->filterAction(),
                           targetImage);
}

void ResizeTool::blockWidgetSignals(bool b)
{
    d->preserveRatioBox->blockSignals(b);
    d->wInput->blockSignals(b);
    d->hInput->blockSignals(b);
    d->wpInput->blockSignals(b);
    d->hpInput->blockSignals(b);
}

void ResizeTool::slotRestorationToggled(bool b)
{
    d->settingsWidget->setEnabled(b);
    d->cimgLogoLabel->setEnabled(b);
    toolSettings()->enableButton(EditorToolSettings::Load, b);
    toolSettings()->enableButton(EditorToolSettings::SaveAs, b);
}

void ResizeTool::slotLoadSettings()
{
    QUrl loadBlowupFile = DFileDialog::getOpenFileUrl(qApp->activeWindow(), i18n("Photograph Resizing Settings File to Load"),
                                                      QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)),
                                                      QLatin1String("*"));

    if ( loadBlowupFile.isEmpty() )
    {
        return;
    }

    QFile file(loadBlowupFile.toLocalFile());

    if ( file.open(QIODevice::ReadOnly) )
    {
        if (!d->settingsWidget->loadSettings(file, QLatin1String("# Photograph Resizing Configuration File")))
        {
            QMessageBox::critical(qApp->activeWindow(), qApp->applicationName(),
                                  i18n("\"%1\" is not a Photograph Resizing settings text file.",
                                       loadBlowupFile.fileName()));
            file.close();
            return;
        }
    }
    else
    {
        QMessageBox::critical(qApp->activeWindow(), qApp->applicationName(),
                              i18n("Cannot load settings from the Photograph Resizing text file."));
    }

    file.close();
}

void ResizeTool::slotSaveAsSettings()
{
    QUrl saveBlowupFile = DFileDialog::getSaveFileUrl(qApp->activeWindow(), i18n("Photograph Resizing Settings File to Save"),
                                                      QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)),
                                                      QLatin1String("*"));

    if ( saveBlowupFile.isEmpty() )
    {
        return;
    }

    QFile file(saveBlowupFile.toLocalFile());

    if ( file.open(QIODevice::WriteOnly) )
    {
        d->settingsWidget->saveSettings(file, QLatin1String("# Photograph Resizing Configuration File"));
    }
    else
    {
        QMessageBox::critical(qApp->activeWindow(), qApp->applicationName(),
                              i18n("Cannot save settings to the Photograph Resizing text file."));
    }

    file.close();
}

} // namespace DigikamEditorResizeToolPlugin
