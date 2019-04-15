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

#include <QRadioButton>
#include <QBrush>
#include <QCheckBox>
#include <QCloseEvent>
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
#include <QIcon>
#include <QApplication>
#include <QStandardPaths>
#include <QMessageBox>
#include <QString>
#include <QVector>

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
        Tiny = 0,
        Small,
        Medium,
        Big,
        Large,
        Huge,
        Profile,
        Sep1,
        A3,
        A4,
        A6,
        Letter,
        Sep2,
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
        Millimeters,
    };

    explicit Private()
      : orgWidth(0),
        orgHeight(0),
        prevW(0),
        prevH(0),
        prevUnit(0),
        prevWP(0.0),
        prevHP(0.0),
        restorationTips(0),
        labelPreset(0),
        presetCBox(0),
        units(0),
        useCustom(0),
        preserveRatioBox(0),
        useGreycstorationBox(0),
        mainTab(0),
        cimgLogoLabel(0),
        previewWidget(0),
        wInput(0),
        hInput(0),
        resolution(0),
        wpInput(0),
        hpInput(0),
        gboxSettings(0),
        settingsWidget(0)
    {
    }

    QVector<int> presetLengthValue(WidthPreset preset);
    int whr(QVector<int> dimension, int index);

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
    int                     prevUnit;

    double                  prevWP;
    double                  prevHP;

    QLabel*                 restorationTips;
    QLabel*                 labelPreset;

    QComboBox*              presetCBox;
    QComboBox*              units;

    QCheckBox*              useCustom;

    QCheckBox*              preserveRatioBox;
    QCheckBox*              useGreycstorationBox;

    QTabWidget*             mainTab;

    DActiveLabel*           cimgLogoLabel;

    ImageGuideWidget*       previewWidget;

    DDoubleNumInput*           wInput;
    DDoubleNumInput*           hInput;
    DIntNumInput*           resolution;

    DDoubleNumInput*        wpInput;
    DDoubleNumInput*        hpInput;

    EditorToolSettings*     gboxSettings;
    GreycstorationSettings* settingsWidget;
};

QVector<int> ResizeTool::Private::presetLengthValue(WidthPreset preset)
{
    QVector<int> dimension(3);

    switch (preset)
    {
        case Private::Tiny:
            dimension[0]  = 320;
            dimension[1] = 200;
            dimension[2] = 72;
            break;

        case Private::Small:
            dimension[0]  = 640;
            dimension[1] = 480;
            dimension[2] = 72;
            break;

        case Private::Medium:
            dimension[0]  = 960;
            dimension[1] = 640;
            dimension[2] = 144;
            break;

        case Private::Big:
            dimension[0]  = 1024;
            dimension[1] = 768;
            dimension[2] = 72;
            break;

        case Private::Large:
            dimension[0]  = 1280;
            dimension[1] = 720;
            dimension[2] = 144;
            break;

        case Private::Huge:
            dimension[0]  = 1920;
            dimension[1] = 1080;
            dimension[2] = 72;
            break;

        case Private::Profile:
            dimension[0]  = 1080;
            dimension[1] = 1080;
            dimension[2] = 144;
            break;

        case Private::A3:
            dimension[0]  = 3508;
            dimension[1] = 4961;
            dimension[2] = 300;
            break;

        case Private::A4:
            dimension[0]  = 2480;
            dimension[1] = 3508;
            dimension[2] = 300;
            break;

        case Private::A6:
            dimension[0]  = 1240;
            dimension[1] = 1748;
            dimension[2] = 300;
            break;

        case Private::Letter:
            dimension[0]  = 2550;
            dimension[1] = 3300;
            dimension[2] = 300;
            break;

        case Private::Print1:
            dimension[0]  = 1200;
            dimension[1] = 1800;
            dimension[2] = 300;
            break;

        case Private::Print2:
            dimension[0]  = 1500;
            dimension[1] = 2100;
            dimension[2] = 300;
            break;

        case Private::Print3:
            dimension[0]  = 2400;
            dimension[1] = 3000;
            dimension[2] = 300;
            break;

        default:
            dimension[0]  = 3300;
            dimension[1] = 4200;
            dimension[2] = 300;
            break;
    }

    return dimension;
}

int ResizeTool::Private::whr(QVector<int> dimension, int index)
{
    return dimension[index];
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

    d->previewWidget = new ImageGuideWidget(0, false, ImageGuideWidget::HVGuideMode, Qt::red, 1, false);
    setToolView(d->previewWidget);
    setPreviewModeMask(PreviewToolBar::UnSplitPreviewModes);

    // -------------------------------------------------------------

    d->gboxSettings = new EditorToolSettings(0);
    d->gboxSettings->setButtons(EditorToolSettings::Default|
                                EditorToolSettings::Try|
                                EditorToolSettings::Ok|
                                EditorToolSettings::Load|
                                EditorToolSettings::SaveAs|
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

    d->labelPreset          = new QLabel(i18n("Preset Resolutions:"), firstPage);
    d->presetCBox           = new QComboBox(firstPage);
    d->presetCBox->insertItem(Private::Tiny,    i18np("Tiny (1 pixel)",     "Tiny (%1 x %2 pixels) %3 ppi",     d->whr(d->presetLengthValue(Private::Tiny),0),     d->whr(d->presetLengthValue(Private::Tiny),1),     d->whr(d->presetLengthValue(Private::Tiny),2)));
    d->presetCBox->insertItem(Private::Small,   i18np("Small (1 pixel)",    "Small (%1 x %2 pixels) %3 ppi",    d->whr(d->presetLengthValue(Private::Small),0),    d->whr(d->presetLengthValue(Private::Small),1),    d->whr(d->presetLengthValue(Private::Small),2)));
    d->presetCBox->insertItem(Private::Medium,  i18np("Medium (1 pixel)",   "Medium (%1 x %2 pixels) %3 ppi",   d->whr(d->presetLengthValue(Private::Medium),0),   d->whr(d->presetLengthValue(Private::Medium),1),   d->whr(d->presetLengthValue(Private::Medium),2)));
    d->presetCBox->insertItem(Private::Big,     i18np("Big (1 pixel)",      "Big (%1*%2 pixels) %3 ppi",        d->whr(d->presetLengthValue(Private::Big),0),      d->whr(d->presetLengthValue(Private::Big),1),      d->whr(d->presetLengthValue(Private::Big),2)));
    d->presetCBox->insertItem(Private::Large,   i18np("Large (1 pixel)",    "Large (%1*%2 pixels) %3 ppi",      d->whr(d->presetLengthValue(Private::Large),0),    d->whr(d->presetLengthValue(Private::Large),1),    d->whr(d->presetLengthValue(Private::Large),2)));
    d->presetCBox->insertItem(Private::Huge,    i18np("Huge (1 pixel)",     "Huge (%1*%2 pixels) %3 ppi",       d->whr(d->presetLengthValue(Private::Huge),0),     d->whr(d->presetLengthValue(Private::Huge),1),     d->whr(d->presetLengthValue(Private::Huge),2)));
    d->presetCBox->insertItem(Private::Profile, i18np("Profile (1 pixel)",  "Profile (%1*%2 pixels) %3 ppi",    d->whr(d->presetLengthValue(Private::Profile),0),  d->whr(d->presetLengthValue(Private::Profile),1),  d->whr(d->presetLengthValue(Private::Profile),2)));
    d->presetCBox->insertItem(Private::A3,      i18np("A3 (1 pixel)",       "A3 (210*297 mm) %1 ppi",           d->whr(d->presetLengthValue(Private::A3),2)));
    d->presetCBox->insertItem(Private::A4,      i18np("A4 (1 pixel)",       "A4 (210*297 mm) %1 ppi",           d->whr(d->presetLengthValue(Private::A4),2)));
    d->presetCBox->insertItem(Private::A6,      i18np("A6 (1 pixel)",       "A6 (105*148 mm) %1 ppi",           d->whr(d->presetLengthValue(Private::A6),2)));
    d->presetCBox->insertItem(Private::Letter,  i18np("Letter (1 pixel)",   "Letter (8.5*11 in) %1 ppi",        d->whr(d->presetLengthValue(Private::Letter),2)));
    d->presetCBox->insertItem(Private::Print1,  i18np("Print1 (1 pixel)",   "4*6 in %1 ppi",                    d->whr(d->presetLengthValue(Private::Print1),2)));
    d->presetCBox->insertItem(Private::Print2,  i18np("Print2 (1 pixel)",   "5*7 in %1 ppi",                    d->whr(d->presetLengthValue(Private::Print2),2)));
    d->presetCBox->insertItem(Private::Print2,  i18np("Print3 (1 pixel)",   "8*10 in %1 ppi",                   d->whr(d->presetLengthValue(Private::Print3),2)));
    d->presetCBox->insertItem(Private::Print2,  i18np("Print3 (1 pixel)",   "11*14 in %1 ppi",                  d->whr(d->presetLengthValue(Private::Print4),2)));

    d->presetCBox->insertSeparator(7);
    d->presetCBox->insertSeparator(12);

    d->units                = new QComboBox(firstPage);
    QLabel* const label = new QLabel(i18n("Units:"), firstPage);
    d->prevUnit = 0;
    d->units->insertItem(Private::Pixels,       i18n("Pixels (px)"));
    d->units->insertItem(Private::Inches,       i18n("Inches (in)"));
    d->units->insertItem(Private::Centimeters,  i18n("Centimeters (cm)"));
    d->units->insertItem(Private::Millimeters,  i18n("Millimeters (mm)"));

    d->useCustom            = new QCheckBox(i18n("Use Custom Units"),  firstPage);

    d->preserveRatioBox = new QCheckBox(i18n("Maintain aspect ratio"), firstPage);
    d->preserveRatioBox->setWhatsThis( i18n("Enable this option to maintain aspect "
                                            "ratio with new image sizes."));

    QLabel* const label1 = new QLabel(i18n("Width:"), firstPage);
    d->wInput            = new DDoubleNumInput(firstPage);
    d->wInput->setRange(1, qMax(d->orgWidth * 10, 12000), 1);
    d->wInput->setDefaultValue(d->orgWidth);
    d->wInput->setSuffix(i18n(" px"));
    d->wInput->setObjectName(i18n("wInput"));
    d->wInput->setWhatsThis( i18n("Set here the new image width in pixels."));

    QLabel* const label2 = new QLabel(i18n("Height:"), firstPage);
    d->hInput            = new DDoubleNumInput(firstPage);
    d->hInput->setRange(1, qMax(d->orgHeight * 10, 12000), 1);
    d->hInput->setDefaultValue(d->orgHeight);
    d->hInput->setSuffix(i18n(" px"));
    d->hInput->setObjectName(i18n("hInput"));
    d->hInput->setWhatsThis( i18n("New image height in pixels (px)."));

    QLabel* const label3 = new QLabel(i18n("Width (%):"), firstPage);
    d->wpInput           = new DDoubleNumInput(firstPage);
    d->wpInput->setRange(1.0, 999.0, 1.0);
    d->wpInput->setDefaultValue(100.0);
    d->wpInput->setObjectName(i18n("wpInput"));
    d->wpInput->setWhatsThis( i18n("New image width in percent (%)."));

    QLabel* const label4 = new QLabel(i18n("Height (%):"), firstPage);
    d->hpInput           = new DDoubleNumInput(firstPage);
    d->hpInput->setRange(1.0, 999.0, 1.0);
    d->hpInput->setDefaultValue(100.0);
    d->hpInput->setObjectName(i18n("hpInput"));
    d->hpInput->setWhatsThis( i18n("New image height in percent (%)."));

    QLabel* const label5 = new QLabel(i18n("Resolution:"), firstPage);
    d->resolution        = new DIntNumInput(firstPage);
    d->resolution->setRange(1, 500, 1);
    d->resolution->setDefaultValue(96);
    d->resolution->setSuffix(i18n(" Pixel/Inch"));
    d->resolution->setObjectName(i18n("resolution"));
    d->resolution->setWhatsThis( i18n("New image resolution in pixels/inch."));

    d->cimgLogoLabel = new DActiveLabel(QUrl(QLatin1String("http://cimg.sourceforge.net")),
                                        QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("digikam/data/logo-cimg.png")),
                                        firstPage);
    d->cimgLogoLabel->setToolTip(i18n("Visit CImg library website"));

    d->useGreycstorationBox = new QCheckBox(i18n("Restore photograph (slow)"), firstPage);
    d->useGreycstorationBox->setWhatsThis( i18n("Enable this option to scale-up an image to a huge size. "
                                           "<b>Warning</b>: This process can take some time."));

    d->restorationTips = new QLabel(i18n("<b>Note:</b> use Restoration Mode to scale-up an image to a huge size. "
                                         "This process can take some time."), firstPage);
    d->restorationTips->setWordWrap(true);

    const int spacing = d->gboxSettings->spacingHint();

    d->units->setMaximumWidth(200);
    grid->addWidget(d->labelPreset,                             0, 0, 1, 3);
    grid->addWidget(d->presetCBox,                              1, 0, 1, 3);
    grid->addWidget(d->useCustom,                               2, 0, 1, 3);
    grid->addWidget(d->preserveRatioBox,                        3, 0, 1, 3);
    grid->addWidget(label,                                      4, 0, 1, 1);
    grid->addWidget(d->units,                                   4, 1, 1, 1);
    grid->addWidget(label1,                                     5, 0, 1, 1);
    grid->addWidget(d->wInput,                                  5, 1, 1, 1);
    grid->addWidget(label2,                                     6, 0, 1, 1);
    grid->addWidget(d->hInput,                                  6, 1, 1, 2);
    grid->addWidget(label3,                                     7, 0, 1, 1);
    grid->addWidget(d->wpInput,                                 7, 1, 1, 2);
    grid->addWidget(label4,                                     8, 0, 1, 1);
    grid->addWidget(d->hpInput,                                 8, 1, 1, 2);
    grid->addWidget(label5,                                     9, 0, 1, 1);
    grid->addWidget(d->resolution,                              9, 1, 1, 2);
    grid->addWidget(new DLineWidget(Qt::Horizontal, firstPage), 10, 0, 1, 3);
    grid->addWidget(d->cimgLogoLabel,                           11, 0, 3, 1);
    grid->addWidget(d->useGreycstorationBox,                    11, 1, 1, 2);
    grid->addWidget(d->restorationTips,                         12, 1, 1, 2);
    grid->setRowStretch(13, 10);
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
            this, SLOT(slotSettingsChanged()));

    connect(d->useCustom, SIGNAL(toggled(bool)),
            this, SLOT(slotSettingsChanged()));

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

    blockWidgetSignals(true);

    d->useCustom->setChecked(true);
    d->preserveRatioBox->setChecked(true);
    d->units->setCurrentIndex(0);
    d->units->setEnabled(d->useCustom->isChecked());
    d->wInput->setEnabled(d->useCustom->isChecked());
    d->hInput->setEnabled(d->useCustom->isChecked());
    d->wpInput->setEnabled(d->useCustom->isChecked());
    d->hpInput->setEnabled(d->useCustom->isChecked());
    d->preserveRatioBox->setEnabled(d->useCustom->isChecked());
    d->resolution->setEnabled(d->useCustom->isChecked());

    d->wInput->slotReset();
    d->hInput->slotReset();
    d->wpInput->slotReset();
    d->hpInput->slotReset();

    blockWidgetSignals(false);
}

void ResizeTool::slotSettingsChanged()
{
    d->labelPreset->setEnabled(!d->useCustom->isChecked());
    d->presetCBox->setEnabled(!d->useCustom->isChecked());
    d->wInput->setEnabled(d->useCustom->isChecked());
    d->hInput->setEnabled(d->useCustom->isChecked());
    d->wpInput->setEnabled(d->useCustom->isChecked());
    d->hpInput->setEnabled(d->useCustom->isChecked());
    d->units->setEnabled(d->useCustom->isChecked());
    d->preserveRatioBox->setEnabled(d->useCustom->isChecked());
    d->resolution->setEnabled(d->useCustom->isChecked());

    if (d->useCustom->isChecked() == false)
    {
        Private::WidthPreset preset  = (Private::WidthPreset)d->presetCBox->currentIndex();
        QVector<int> dimension    = d->presetLengthValue(preset);
        d->resolution->setValue((int)dimension[2]);
        if ((d->orgWidth < d->orgHeight && (d->presetCBox->currentIndex() < 7)) ||
            (d->orgWidth > d->orgHeight && (d->presetCBox->currentIndex() > 10)))
        {
            d->hInput->setValue(pixelstoUnits((int)dimension[0], (int)dimension[2], d->units->currentIndex()));
            d->wInput->setValue(pixelstoUnits((int)dimension[1], (int)dimension[2], d->units->currentIndex()));
        }
        else
        {
            d->wInput->setValue(pixelstoUnits((int)dimension[0], (int)dimension[2], d->units->currentIndex()));
            d->hInput->setValue(pixelstoUnits((int)dimension[1], (int)dimension[2], d->units->currentIndex()));
        }
    }

    slotValuesChanged();
}

void ResizeTool::slotUnitsChanged()
{
    Private::Units units  = (Private::Units)d->units->currentIndex();
    switch (units)
    {
        case Private::Pixels:
        {
            double wVal  = unitstoPixels(d->wInput->value(), d->resolution->value(), d->prevUnit);
            double hVal  = unitstoPixels((double)d->hInput->value(), d->resolution->value(), d->prevUnit);

            d->wInput->setSuffix(i18n(" px"));
            d->wInput->setRange(1, 12000, 1);
            d->wInput->setDefaultValue(1024);
            d->wInput->setValue((int)wVal);

            d->hInput->setSuffix(i18n(" px"));
            d->hInput->setRange(1, 12000, 1);
            d->hInput->setDefaultValue(1024);
            d->hInput->setValue((int)hVal);
        }
        break;

        case Private::Inches:
        {
            double wval  = unitstoPixels(d->wInput->value(), d->resolution->value(), d->prevUnit);
            double wVal = pixelstoUnits(wval, d->resolution->value(), 1);
            double hval  = unitstoPixels(d->hInput->value(), d->resolution->value(), d->prevUnit);
            double hVal = pixelstoUnits(hval, d->resolution->value(), 1);

            d->wInput->setSuffix(i18n(" in"));
            d->wInput->setRange(1.0, 50.0, 1.0);
            d->wInput->setDefaultValue(10.0);
            d->wInput->setDecimals(1);
            d->wInput->setValue(wVal);

            d->hInput->setSuffix(i18n(" in"));
            d->hInput->setRange(1.0, 50.0, 1.0);
            d->hInput->setDefaultValue(10.0);
            d->hInput->setDecimals(1);
            d->hInput->setValue(hVal);
        }
            break;

        case Private::Millimeters:
        {
            double wval  = unitstoPixels(d->wInput->value(), d->resolution->value(), d->prevUnit);
            double wVal = pixelstoUnits(wval, d->resolution->value(), 3);
            double hval  = unitstoPixels(d->hInput->value(), d->resolution->value(), d->prevUnit);
            double hVal = pixelstoUnits(hval, d->resolution->value(), 3);

            d->wInput->setSuffix(i18n(" mm"));
            d->wInput->setRange(1.0, 2000.0, 1.0);
            d->wInput->setDefaultValue(300.0);
            d->wInput->setDecimals(1);
            d->wInput->setValue(wVal);

            d->hInput->setSuffix(i18n(" mm"));
            d->hInput->setRange(1.0, 2000.0, 1.0);
            d->hInput->setDefaultValue(300.0);
            d->hInput->setDecimals(1);
            d->hInput->setValue(hVal);
        }
            break;

        case Private::Centimeters:
        {
            double wval  = unitstoPixels(d->wInput->value(), d->resolution->value(), d->prevUnit);
            double wVal = pixelstoUnits(wval, d->resolution->value(), 2);
            double hval  = unitstoPixels(d->hInput->value(), d->resolution->value(), d->prevUnit);
            double hVal = pixelstoUnits(hval, d->resolution->value(), 2);

            d->wInput->setSuffix(i18n(" cm"));
            d->wInput->setRange(1.0, 200.0, 1.0);
            d->wInput->setDefaultValue(30.0);
            d->wInput->setDecimals(1);
            d->wInput->setValue(wVal);

            d->hInput->setSuffix(i18n(" cm"));
            d->hInput->setRange(1.0, 200.0, 1.0);
            d->hInput->setDefaultValue(30.0);
            d->hInput->setDecimals(1);
            d->hInput->setValue(hVal);
        }
            break;

        default:
            break;
    }

    d->prevUnit = d->units->currentIndex();
    slotSettingsChanged();
}

double ResizeTool::unitstoPixels(double val, int res, int unitIndex)
{
    Private::Units units  = (Private::Units)unitIndex;
    switch (units)
    {
        case Private::Inches:
            val = val * (double)res;
            break;

        case Private::Millimeters:
            val = val * ((double)res / 25.4);
            break;

        case Private::Centimeters:
            val = val * ((double)res / 2.54);
            break;

        default:
            val = val;
            break;
    }

    return val;
}

double ResizeTool::pixelstoUnits(double val, int res, int unitIndex)
{
    Private::Units units  = (Private::Units)unitIndex;
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
            val = val;
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
        double val  = unitstoPixels(d->wInput->value(), d->resolution->value(), d->units->currentIndex());
        double pval = val / (double)(d->orgWidth) * 100.0;

        d->wpInput->setValue(pval);

        if (d->preserveRatioBox->isChecked())
        {
            double h = pixelstoUnits((int)(pval * d->orgHeight / 100), d->resolution->value(), d->units->currentIndex());
            d->hpInput->setValue(pval);
            d->hInput->setValue(h);
        }
    }
    else if (s == QLatin1String("hInput"))
    {
        double val  = unitstoPixels(d->hInput->value(), d->resolution->value(), d->units->currentIndex());
        double pval = val / (double)(d->orgHeight) * 100.0;

        d->hpInput->setValue(pval);

        if (d->preserveRatioBox->isChecked())
        {
            double w = pixelstoUnits((int)(pval * d->orgWidth / 100), d->resolution->value(), d->units->currentIndex());
            d->wpInput->setValue(pval);
            d->wInput->setValue(w);
        }
    }
    else if (s == QLatin1String("wpInput"))
    {
        double val = d->wpInput->value();
        double w      = pixelstoUnits((int)(val * d->orgWidth / 100), d->resolution->value(), d->units->currentIndex());
        d->wInput->setValue(w);

        if (d->preserveRatioBox->isChecked())
        {
            double h = pixelstoUnits((int)(val * d->orgHeight / 100), d->resolution->value(), d->units->currentIndex());
            d->hpInput->setValue(val);
            d->hInput->setValue(h);
        }
    }
    else if (s == QLatin1String("hpInput"))
    {
        double val = d->hpInput->value();
        double h = pixelstoUnits((int)(val * d->orgHeight / 100), d->resolution->value(), d->units->currentIndex());
        d->hInput->setValue(h);

        if (d->preserveRatioBox->isChecked())
        {
            double w = pixelstoUnits((int)(val * d->orgWidth / 100), d->resolution->value(), d->units->currentIndex());
            d->wpInput->setValue(val);
            d->wInput->setValue(w);
        }
    }
    else if (s == QLatin1String("resolution"))
    {
        double hval  = unitstoPixels(d->hInput->value(), d->resolution->value(), d->units->currentIndex());
        double pHval = hval / (double)(d->orgHeight) * 100.0;
        double wval  = unitstoPixels(d->wInput->value(), d->resolution->value(), d->units->currentIndex());
        double pWval = wval / (double)(d->orgWidth) * 100.0;

        d->wpInput->setValue(pWval);
        d->hpInput->setValue(pHval);
    }

    d->prevW  = d->wInput->value();
    d->prevH  = d->hInput->value();
    d->prevWP = d->wpInput->value();
    d->prevHP = d->hpInput->value();

    blockWidgetSignals(false);
}

void ResizeTool::preparePreview()
{
    int h     = unitstoPixels(d->hInput->value(), d->resolution->value(), d->units->currentIndex());
    int w     = unitstoPixels(d->wInput->value(), d->resolution->value(), d->units->currentIndex());
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
    int h     = unitstoPixels(d->hInput->value(), d->resolution->value(), d->units->currentIndex());
    int w     = unitstoPixels(d->wInput->value(), d->resolution->value(), d->units->currentIndex());
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
