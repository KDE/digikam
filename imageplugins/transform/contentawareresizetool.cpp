/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-02-01
 * Description : Content aware resizing tool.
 *
 * Copyright (C) 2009-2010 by Julien Pontabry <julien dot pontabry at ulp dot u-strasbg dot fr>
 * Copyright (C) 2009-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009-2010 by Julien Narboux <julien at narboux dot fr>
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

#include "contentawareresizetool.moc"

// C++ includes

#include <cstdio>

// Qt includes

#include <QButtonGroup>
#include <QCheckBox>
#include <QGridLayout>
#include <QLabel>
#include <QToolButton>

// KDE includes

#include <kapplication.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kseparator.h>
#include <kstandarddirs.h>
#include <kvbox.h>

// LibKDraw includes

#include <libkdcraw/rcombobox.h>
#include <libkdcraw/rnuminput.h>
#include <libkdcraw/rexpanderbox.h>
#include <libkdcraw/version.h>

// Local includes

#include "editortoolsettings.h"
#include "imageiface.h"
#include "imageguidewidget.h"
#include "contentawarefilter.h"

using namespace KDcrawIface;

namespace DigikamTransformImagePlugin
{

class ContentAwareResizeTool::Private
{
public:

    enum MaskTool
    {
        redMask=0,
        greenMask,
        eraseMask
    };

public:

    Private() :
        orgWidth(0),
        orgHeight(0),
        prevW(0),
        prevH(0),
        prevWP(0.0),
        prevHP(0.0),
        preserveRatioBox(0),
        weightMaskBox(0),
        preserveSkinTones(0),
        previewWidget(0),
        gboxSettings(0),
        wInput(0),
        hInput(0),
        stepInput(0),
        maskPenSize(0),
        sideSwitchInput(0),
        wpInput(0),
        hpInput(0),
        mixedRescaleInput(0),
        rigidityInput(0),
        funcInput(0),
        resizeOrderInput(0),
        expanderBox(0),
        redMaskTool(0),
        greenMaskTool(0),
        eraseMaskTool(0),
        maskGroup(0)
    {}

    static const QString configGroupName;
    static const QString configStepEntry;
    static const QString configSideSwitchEntry;
    static const QString configRigidityEntry;
    static const QString configFunctionEntry;
    static const QString configOrderEntry;
    static const QString configMixedRescaleValueEntry;
    static const QString configBrushSizeEntry;
    static const QString configPreserveTonesEntry;

    int                  orgWidth;
    int                  orgHeight;
    int                  prevW;
    int                  prevH;

    double               prevWP;
    double               prevHP;

    QCheckBox*           preserveRatioBox;
    QCheckBox*           weightMaskBox;
    QCheckBox*           preserveSkinTones;

    ImageGuideWidget*    previewWidget;

    EditorToolSettings*  gboxSettings;

    RIntNumInput*        wInput;
    RIntNumInput*        hInput;
    RIntNumInput*        stepInput;
    RIntNumInput*        maskPenSize;
    RIntNumInput*        sideSwitchInput;

    RDoubleNumInput*     wpInput;
    RDoubleNumInput*     hpInput;
    RDoubleNumInput*     mixedRescaleInput;
    RDoubleNumInput*     rigidityInput;

    RComboBox*           funcInput;
    RComboBox*           resizeOrderInput;

    RExpanderBox*        expanderBox;

    QToolButton*         redMaskTool;
    QToolButton*         greenMaskTool;
    QToolButton*         eraseMaskTool;

    QButtonGroup*        maskGroup;
};

const QString ContentAwareResizeTool::Private::configGroupName("liquidrescale Tool");
const QString ContentAwareResizeTool::Private::configStepEntry("Step");
const QString ContentAwareResizeTool::Private::configSideSwitchEntry("SideSwitch");
const QString ContentAwareResizeTool::Private::configRigidityEntry("Rigidity");
const QString ContentAwareResizeTool::Private::configFunctionEntry("Function");
const QString ContentAwareResizeTool::Private::configOrderEntry("Order");
const QString ContentAwareResizeTool::Private::configMixedRescaleValueEntry("MixedRescaleValue");
const QString ContentAwareResizeTool::Private::configBrushSizeEntry("BrushSize");
const QString ContentAwareResizeTool::Private::configPreserveTonesEntry("PreserveTones");

// --------------------------------------------------------

ContentAwareResizeTool::ContentAwareResizeTool(QObject* const parent)
    : EditorToolThreaded(parent), d(new Private)
{
    setObjectName("liquidrescale");
    setToolName(i18n("Liquid Rescale"));
    setToolIcon(SmallIcon("transform-scale"));

    d->previewWidget = new ImageGuideWidget(0, false, ImageGuideWidget::HVGuideMode);
    d->previewWidget->installEventFilter(this);
    setToolView(d->previewWidget);
    setPreviewModeMask(PreviewToolBar::PreviewToggleOnMouseOver);

    // -------------------------------------------------------------

    d->gboxSettings = new EditorToolSettings;
    d->gboxSettings->setButtons(EditorToolSettings::Default|
                                EditorToolSettings::Ok|
                                EditorToolSettings::Try|
                                EditorToolSettings::Cancel);

    QGridLayout* grid = new QGridLayout(d->gboxSettings->plainPage());

    // Initialize data
    ImageIface iface;
    d->orgWidth  = iface.originalSize().width();
    d->orgHeight = iface.originalSize().height();
    d->prevW     = d->orgWidth;
    d->prevH     = d->orgHeight;
    d->prevWP    = 100.0;
    d->prevHP    = 100.0;

    // -------------------------------------------------------------

    QWidget* sizeSettingsContainer  = new QWidget;
    QGridLayout* sizeSettingsLayout = new QGridLayout;

    d->preserveRatioBox = new QCheckBox(i18n("Maintain aspect ratio"), d->gboxSettings->plainPage());
    d->preserveRatioBox->setWhatsThis(i18n("Enable this option to maintain aspect ratio with new image sizes."));
    d->preserveRatioBox->setChecked(true);

    QLabel* labelWidth = new QLabel(i18n("Width (px):"), d->gboxSettings->plainPage());
    d->wInput          = new RIntNumInput(d->gboxSettings->plainPage());
    d->wInput->setRange(1, 2*d->orgWidth, 1);
    d->wInput->setDefaultValue(d->orgWidth);
    d->wInput->setSliderEnabled(true);
    d->wInput->setObjectName("wInput");
    d->wInput->setWhatsThis(i18n("Set here the new image width in pixels."));

    QLabel* labelHeight = new QLabel(i18n("Height (px):"), d->gboxSettings->plainPage());
    d->hInput           = new RIntNumInput(d->gboxSettings->plainPage());
    d->hInput->setRange(1, 2*d->orgHeight, 1);
    d->hInput->setDefaultValue(d->orgHeight);
    d->hInput->setSliderEnabled(true);
    d->hInput->setObjectName("hInput");
    d->hInput->setWhatsThis(i18n("Set here the new image height in pixels."));

    QLabel* labelWidthP = new QLabel(i18n("Width (%):"), d->gboxSettings->plainPage());
    d->wpInput          = new RDoubleNumInput(d->gboxSettings->plainPage());
    d->wpInput->input()->setRange(1.0, 200.0, 1.0, true);
    d->wpInput->setDefaultValue(100.0);
    d->wpInput->setObjectName("wpInput");
    d->wpInput->setWhatsThis(i18n("New image width, as a percentage (%)."));

    QLabel* labelHeightP = new QLabel(i18n("Height (%):"), d->gboxSettings->plainPage());
    d->hpInput           = new RDoubleNumInput(d->gboxSettings->plainPage());
    d->hpInput->input()->setRange(1.0, 200.0, 1.0, true);
    d->hpInput->setDefaultValue(100.0);
    d->hpInput->setObjectName("hpInput");
    d->hpInput->setWhatsThis(i18n("New image height, as a percentage (%)."));

    sizeSettingsLayout->addWidget(d->preserveRatioBox, 0, 0, 1, 3);
    sizeSettingsLayout->addWidget(labelWidth,          1, 0, 1, 1);
    sizeSettingsLayout->addWidget(d->wInput,           1, 1, 1, 4);
    sizeSettingsLayout->addWidget(labelHeight,         2, 0, 1, 1);
    sizeSettingsLayout->addWidget(d->hInput,           2, 1, 1, 4);
    sizeSettingsLayout->addWidget(labelWidthP,         3, 0, 1, 1);
    sizeSettingsLayout->addWidget(d->wpInput,          3, 1, 1, 4);
    sizeSettingsLayout->addWidget(labelHeightP,        4, 0, 1, 1);
    sizeSettingsLayout->addWidget(d->hpInput,          4, 1, 1, 4);

    sizeSettingsContainer->setLayout(sizeSettingsLayout);

    // -------------------------------------------------------------

    QWidget* mixedRescaleContainer  = new QWidget;
    QGridLayout* mixedRescaleLayout = new QGridLayout;

    d->mixedRescaleInput = new RDoubleNumInput(d->gboxSettings->plainPage());
    d->mixedRescaleInput->input()->setRange(0.0, 100.0, 1.0, true);
    d->mixedRescaleInput->setDefaultValue(100.0);
    d->mixedRescaleInput->setObjectName("mixedRescaleInput");
    d->mixedRescaleInput->setWhatsThis(i18n("Specify here your desired content-aware rescaling percentage."));
    d->mixedRescaleInput->setEnabled(true);

    mixedRescaleLayout->addWidget(d->mixedRescaleInput,  0, 0, 1, 1);

    mixedRescaleContainer->setLayout(mixedRescaleLayout);

    // -------------------------------------------------------------

    QWidget* maskSettingsContainer  = new QWidget;
    QGridLayout* maskSettingsLayout = new QGridLayout;

    d->weightMaskBox  = new QCheckBox(i18n("Add weight masks"), d->gboxSettings->plainPage());
    d->weightMaskBox->setWhatsThis(i18n("Enable this option to add suppression and preservation masks."));
    d->weightMaskBox->setChecked(false);

    d->maskGroup = new QButtonGroup(d->gboxSettings->plainPage());
    d->maskGroup->setExclusive(true);

    QLabel* labeRedMaskTool = new QLabel(i18n("Suppression weight mask:"), d->gboxSettings->plainPage());
    d->redMaskTool          = new QToolButton(d->gboxSettings->plainPage());
    d->redMaskTool->setIcon(QPixmap(KStandardDirs::locate("data", "digikam/data/indicator-red.png")));
    d->redMaskTool->setCheckable(true);
    d->redMaskTool->setChecked(true);
    d->redMaskTool->setToolTip(i18n("Draw a suppression mask"));
    d->redMaskTool->setWhatsThis(i18n("Click on this button to draw zones marking which areas of the "
                                      "image are less important.  These zones will be deleted when reducing "
                                      "the picture, or duplicated when enlarging the picture."));
    d->redMaskTool->setEnabled(false);
    d->maskGroup->addButton(d->redMaskTool, Private::redMask);

    QLabel* labeGreenMaskTool = new QLabel(i18n("Preservation weight mask:"), d->gboxSettings->plainPage());
    d->greenMaskTool          = new QToolButton(d->gboxSettings->plainPage());
    d->greenMaskTool->setIcon(QPixmap(KStandardDirs::locate("data", "digikam/data/indicator-green.png")));
    d->greenMaskTool->setCheckable(true);
    d->greenMaskTool->setToolTip(i18n("Draw a preservation mask"));
    d->greenMaskTool->setWhatsThis(i18n("Click on this button to draw zones marking which areas of the "
                                        "image you want to preserve."));
    d->greenMaskTool->setEnabled(false);
    d->maskGroup->addButton(d->greenMaskTool, Private::greenMask);

    QLabel* labeEraseMaskTool = new QLabel(i18n("Erase mask:"), d->gboxSettings->plainPage());
    d->eraseMaskTool          = new QToolButton(d->gboxSettings->plainPage());
    d->eraseMaskTool->setIcon(QPixmap(KStandardDirs::locate("data", "digikam/data/indicator-gray.png")));
    d->eraseMaskTool->setCheckable(true);
    d->eraseMaskTool->setToolTip(i18n("Erase mask"));
    d->eraseMaskTool->setWhatsThis(i18n("Click on this button to erase mask regions."));
    d->eraseMaskTool->setEnabled(false);
    d->maskGroup->addButton(d->eraseMaskTool, Private::eraseMask);

    QLabel* labelMaskPenSize = new QLabel(i18n("Brush size:"), d->gboxSettings->plainPage());
    d->maskPenSize           = new RIntNumInput(d->gboxSettings->plainPage());
    d->maskPenSize->setRange(3, 64, 1);
    d->maskPenSize->setDefaultValue(10);
    d->maskPenSize->setSliderEnabled(true);
    d->maskPenSize->setObjectName("maskPenSize");
    d->maskPenSize->setWhatsThis(i18n("Specify here the size of the brush used to paint masks."));

    maskSettingsLayout->addWidget(d->weightMaskBox,  1, 0, 1, -1);
    maskSettingsLayout->addWidget(labeRedMaskTool,   2, 0, 1, 3);
    maskSettingsLayout->addWidget(d->redMaskTool,    2, 2, 1, 1);
    maskSettingsLayout->addWidget(labeGreenMaskTool, 3, 0, 1, 3);
    maskSettingsLayout->addWidget(d->greenMaskTool,  3, 2, 1, 1);
    maskSettingsLayout->addWidget(labeEraseMaskTool, 4, 0, 1, 3);
    maskSettingsLayout->addWidget(d->eraseMaskTool,  4, 2, 1, 1);
    maskSettingsLayout->addWidget(labelMaskPenSize,  5, 0, 1, 1);
    maskSettingsLayout->addWidget(d->maskPenSize,    5, 1, 1, 4);

    maskSettingsContainer->setLayout(maskSettingsLayout);

    // -------------------------------------------------------------

    QWidget* energyFunctionsContainer  = new QWidget;
    QGridLayout* energyFunctionsLayout = new QGridLayout;

    d->funcInput = new RComboBox(d->gboxSettings->plainPage());
    d->funcInput->addItem(i18n("Norm of brightness gradient"));
    d->funcInput->addItem(i18n("Sum of absolute values of brightness gradients"));
    d->funcInput->addItem(i18n("Absolute value of brightness gradient"));
    d->funcInput->addItem(i18n("Norm of luma gradient"));
    d->funcInput->addItem(i18n("Sum of absolute values of luma gradients"));
    d->funcInput->addItem(i18n("Absolute value of luma gradient"));

    d->funcInput->setDefaultIndex(2);
    d->funcInput->setWhatsThis(i18n("This option allows you to choose a gradient function. This function is used "
                                    "to determine which pixels should be removed or kept."));

    d->preserveSkinTones = new QCheckBox(i18n("Preserve Skin Tones"), d->gboxSettings->plainPage());
    d->preserveSkinTones->setWhatsThis(i18n("Enable this option to preserve pixels whose color is "
                                            "close to a skin tone."));
    d->preserveSkinTones->setChecked(false);

    energyFunctionsLayout->addWidget(d->funcInput,         1, 0, 1,-1);
    energyFunctionsLayout->addWidget(d->preserveSkinTones, 2, 0, 1, 3);

    energyFunctionsContainer->setLayout(energyFunctionsLayout);

    // -------------------------------------------------------------

    QWidget* advancedSettingsContainer  = new QWidget;
    QGridLayout* advancedSettingsLayout = new QGridLayout;

    QLabel* labelRigidity = new QLabel(i18n("Overall rigidity of the seams:"), d->gboxSettings->plainPage());
    d->rigidityInput      = new RDoubleNumInput(d->gboxSettings->plainPage());
    d->rigidityInput->input()->setRange(0.0, 10.0, 1.0, true);
    d->rigidityInput->setDefaultValue(0.0);
    d->rigidityInput->setWhatsThis(i18n("Use this value to give a negative bias to the seams which "
                                        "are not straight. May be useful to prevent distortions in "
                                        "some situations, or to avoid artifacts from pixel skipping "
                                        "(it is better to use low values in such case). This setting "
                                        "applies to the whole selected layer if no rigidity mask is used. "
                                        "Note: the bias is proportional to the difference in the transversal "
                                        "coordinate between each two successive points, elevated to the power "
                                        "of 1.5, and summed up for the whole seam."));

    QLabel* labelSteps = new QLabel(i18n("Maximum number of transversal steps:"),d->gboxSettings->plainPage());
    d->stepInput       = new RIntNumInput(d->gboxSettings->plainPage());
    d->stepInput->setRange(1, 5, 1);
    d->stepInput->setDefaultValue(1);
    d->stepInput->setSliderEnabled(true);
    d->stepInput->setWhatsThis(i18n("This option lets you choose the maximum transversal step "
                                    "that the pixels in the seams can take. In the standard "
                                    "algorithm, corresponding to the default value step = 1, "
                                    "each pixel in a seam can be shifted by at most one pixel "
                                    "with respect to its neighbors. This implies that the seams "
                                    "can form an angle of at most 45 degrees with respect to their "
                                    "base line. Increasing the step value lets you overcome this "
                                    "limit, but may lead to the introduction of artifacts. In order "
                                    "to balance the situation, you can use the rigidity setting."));

    QLabel* labelSideSwitch = new QLabel(i18n("Side switch frequency:"),d->gboxSettings->plainPage());
    d->sideSwitchInput      = new RIntNumInput(d->gboxSettings->plainPage());
    d->sideSwitchInput->setRange(1, 20, 1);
    d->sideSwitchInput->setDefaultValue(4);
    d->sideSwitchInput->setSliderEnabled(true);
    d->sideSwitchInput->setWhatsThis(i18n("During the carving process, at each step "
                                          "the optimal seam to be carved is chosen based on "
                                          "the relevance value for each pixel. However, in the case where two "
                                          "seams are equivalent (which may happen, for instance, when large portions "
                                          "of the image have the same color), the algorithm always "
                                          "chooses the seams from one side.  In some cases, this can pose "
                                          "problems, e.g. an object centered in the original image might not be "
                                          "centered in the resulting image. In order to overcome "
                                          "this effect, this setting allows the favored side to be switched "
                                          "automatically during rescaling, at the cost of slightly "
                                          "worse performance."));

    QLabel* labelResizeOrder = new QLabel(i18n("Resize Order:"),d->gboxSettings->plainPage());
    d->resizeOrderInput      = new RComboBox(d->gboxSettings->plainPage());
    d->resizeOrderInput->addItem(i18n("Horizontally first"));
    d->resizeOrderInput->addItem(i18n("Vertically first"));
    d->resizeOrderInput->setDefaultIndex(0);
    d->resizeOrderInput->setWhatsThis(i18n("Here you can set whether to resize horizontally first or "
                                           "vertically first."));

    advancedSettingsLayout->addWidget(labelRigidity,       1, 0, 1, 4);
    advancedSettingsLayout->addWidget(d->rigidityInput,    2, 0, 1,-1);
    advancedSettingsLayout->addWidget(labelSteps,          3, 0, 1, 4);
    advancedSettingsLayout->addWidget(d->stepInput,        4, 0, 1,-1);
    advancedSettingsLayout->addWidget(labelSideSwitch,     5, 0, 1, 4);
    advancedSettingsLayout->addWidget(d->sideSwitchInput,  6, 0, 1,-1);
    advancedSettingsLayout->addWidget(labelResizeOrder,    7, 0, 1, 4);
    advancedSettingsLayout->addWidget(d->resizeOrderInput, 8, 0, 1,-1);

    advancedSettingsContainer->setLayout(advancedSettingsLayout);

    // -------------------------------------------------------------

    d->expanderBox = new RExpanderBox;
    d->expanderBox->setObjectName("ContentAwareResizeTool Expander");
    d->expanderBox->addItem(sizeSettingsContainer, SmallIcon("transform-scale"), i18n("Target size"),
                            QString("SizeSettingsContainer"), true);
    d->expanderBox->addItem(mixedRescaleContainer, SmallIcon("transform-scale"),
                            i18n("Content-aware rescale percentage"),
                            QString("MixedRescaleContainer"), true);
    d->expanderBox->addItem(maskSettingsContainer, SmallIcon("transform-scale"), i18n("Mask Settings"),
                            QString("MaskSettingsContainer"), true);
    d->expanderBox->addItem(energyFunctionsContainer, SmallIcon("transform-scale"), i18n("Energy function"),
                            QString("EnergyFunctionsContainer"), true);
    d->expanderBox->addItem(advancedSettingsContainer, SmallIcon("system-run"), i18n("Advanced Settings"),
                            QString("AdvancedSettingsContainer"), true);
    d->expanderBox->addStretch();

    grid->addWidget(d->expanderBox, 0, 0, 1, -1);
    grid->setRowStretch(0, 1);
    grid->setMargin(d->gboxSettings->spacingHint());
    grid->setSpacing(d->gboxSettings->spacingHint());

    setToolSettings(d->gboxSettings);

    // -------------------------------------------------------------

    connect(d->wInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotValuesChanged()));

    connect(d->hInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotValuesChanged()));

    connect(d->wpInput, SIGNAL(valueChanged(double)),
            this, SLOT(slotValuesChanged()));

    connect(d->hpInput, SIGNAL(valueChanged(double)),
            this, SLOT(slotValuesChanged()));

    connect(d->mixedRescaleInput, SIGNAL(valueChanged(double)),
            this, SLOT(slotMixedRescaleValueChanged()));

    connect(d->weightMaskBox, SIGNAL(stateChanged(int)),
            this, SLOT(slotWeightMaskBoxStateChanged(int)));

    connect(d->maskGroup, SIGNAL(buttonClicked(int)),
            this, SLOT(slotMaskColorChanged(int)));

    connect(d->maskPenSize, SIGNAL(valueChanged(int)),
            this, SLOT(slotMaskPenSizeChanged(int)));
}

ContentAwareResizeTool::~ContentAwareResizeTool()
{
    delete d;
}

void ContentAwareResizeTool::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);

    blockWidgetSignals(true);

    // NOTE: size settings are not restored here because they depends of image size.
    d->stepInput->setValue(group.readEntry(d->configStepEntry,                      d->stepInput->defaultValue()));
    d->stepInput->setValue(group.readEntry(d->configSideSwitchEntry,                d->sideSwitchInput->defaultValue()));
    d->rigidityInput->setValue(group.readEntry(d->configRigidityEntry,              d->rigidityInput->defaultValue()));
    d->funcInput->setCurrentIndex(group.readEntry(d->configFunctionEntry,           d->funcInput->defaultIndex()));
    d->resizeOrderInput->setCurrentIndex(group.readEntry(d->configOrderEntry,       d->resizeOrderInput->defaultIndex()));
    d->mixedRescaleInput->setValue(group.readEntry(d->configMixedRescaleValueEntry, d->mixedRescaleInput->defaultValue()));
    d->maskPenSize->setValue(group.readEntry(d->configBrushSizeEntry,               d->maskPenSize->defaultValue()));
    d->preserveSkinTones->setChecked(group.readEntry(d->configPreserveTonesEntry,   false));

#if KDCRAW_VERSION >= 0x020000
    d->expanderBox->readSettings(group);
#else
    d->expanderBox->readSettings();
#endif

    enableContentAwareSettings(d->mixedRescaleInput->value() > 0.0);

    blockWidgetSignals(false);
}

void ContentAwareResizeTool::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);

    // NOTE: size settings are not saved here because they depends of image size.
    group.writeEntry(d->configStepEntry,              d->stepInput->value());
    group.writeEntry(d->configSideSwitchEntry,        d->sideSwitchInput->value());
    group.writeEntry(d->configRigidityEntry,          d->rigidityInput->value());
    group.writeEntry(d->configFunctionEntry,          d->funcInput->currentIndex());
    group.writeEntry(d->configOrderEntry,             d->resizeOrderInput->currentIndex());
    group.writeEntry(d->configMixedRescaleValueEntry, d->mixedRescaleInput->value());
    group.writeEntry(d->configBrushSizeEntry,         d->maskPenSize->value());
    group.writeEntry(d->configPreserveTonesEntry,     d->preserveSkinTones->isChecked());

#if KDCRAW_VERSION >= 0x020000
    d->expanderBox->writeSettings(group);
#else
    d->expanderBox->writeSettings();
#endif

    group.sync();
}

void ContentAwareResizeTool::slotResetSettings()
{
    blockWidgetSignals(true);

    d->preserveRatioBox->setChecked(true);
    d->wInput->slotReset();
    d->hInput->slotReset();
    d->wpInput->slotReset();
    d->hpInput->slotReset();
    d->mixedRescaleInput->slotReset();

    blockWidgetSignals(false);
}

void ContentAwareResizeTool::slotValuesChanged()
{
    blockWidgetSignals(true);

    QString s(sender()->objectName());

    if (s == "wInput")
    {
        double val  = d->wInput->value();
        double pval = val / (double)(d->orgWidth) * 100.0;

        d->wpInput->setValue(pval);

        if (d->preserveRatioBox->isChecked())
        {
            int h = (int)(pval * d->orgHeight / 100);

            d->hpInput->setValue(pval);
            d->hInput->setValue(h);
        }
    }
    else if (s == "hInput")
    {
        double val  = d->hInput->value();
        double pval = val / (double)(d->orgHeight) * 100.0;

        d->hpInput->setValue(pval);

        if (d->preserveRatioBox->isChecked())
        {
            int w = (int)(pval * d->orgWidth / 100);

            d->wpInput->setValue(pval);
            d->wInput->setValue(w);
        }
    }
    else if (s == "wpInput")
    {
        double val = d->wpInput->value();
        int w      = (int)(val * d->orgWidth / 100);

        d->wInput->setValue(w);

        if (d->preserveRatioBox->isChecked())
        {
            int h = (int)(val * d->orgHeight / 100);

            d->hpInput->setValue(val);
            d->hInput->setValue(h);
        }
    }
    else if (s == "hpInput")
    {
        double val = d->hpInput->value();
        int h      = (int)(val * d->orgHeight / 100);

        d->hInput->setValue(h);

        if (d->preserveRatioBox->isChecked())
        {
            int w = (int)(val * d->orgWidth / 100);

            d->wpInput->setValue(val);
            d->wInput->setValue(w);
        }
    }

    d->prevW  = d->wInput->value();
    d->prevH  = d->hInput->value();
    d->prevWP = d->wpInput->value();
    d->prevHP = d->hpInput->value();

    blockWidgetSignals(false);
}

void ContentAwareResizeTool::enableMaskSettings(bool b)
{
    bool c = b && d->weightMaskBox->isChecked();
    d->weightMaskBox->setEnabled(b);
    d->redMaskTool  ->setEnabled(c);
    d->greenMaskTool->setEnabled(c);
    d->eraseMaskTool->setEnabled(c);
    d->maskPenSize  ->setEnabled(c);
}

void ContentAwareResizeTool::enableContentAwareSettings(bool b)
{
    d->stepInput->setEnabled(b);
    d->rigidityInput->setEnabled(b);
    d->sideSwitchInput->setEnabled(b);
    d->funcInput->setEnabled(b);
    d->preserveSkinTones->setEnabled(b);
    d->resizeOrderInput->setEnabled(b);
    enableMaskSettings(b);
}

void ContentAwareResizeTool::slotMixedRescaleValueChanged()
{
    blockWidgetSignals(true);
    enableContentAwareSettings(d->mixedRescaleInput->value()>0.0);
    blockWidgetSignals(false);
}

void ContentAwareResizeTool::disableSettings()
{
    d->preserveRatioBox->setEnabled(false);
    d->wInput->setEnabled(false);
    d->hInput->setEnabled(false);
    d->wpInput->setEnabled(false);
    d->hpInput->setEnabled(false);
    d->mixedRescaleInput->setEnabled(false);
    enableContentAwareSettings(false);
}

void ContentAwareResizeTool::contentAwareResizeCore(DImg* const image, int target_width, int target_height, const QImage& mask)
{
    ContentAwareContainer settings;
    settings.preserve_skin_tones = d->preserveSkinTones->isChecked();
    settings.width               = target_width;
    settings.height              = target_height;
    settings.step                = d->stepInput->value();
    settings.side_switch_freq    = d->sideSwitchInput->value();
    settings.rigidity            = d->rigidityInput->value();
    settings.mask                = mask;
    settings.func                = (ContentAwareContainer::EnergyFunction)d->funcInput->currentIndex();
    settings.resize_order        = d->resizeOrderInput->currentIndex() == 0 ? Qt::Horizontal : Qt::Vertical;
    setFilter(new ContentAwareFilter(image, this, settings));
}

void ContentAwareResizeTool::preparePreview()
{
    if (d->prevW  != d->wInput->value()  || d->prevH  != d->hInput->value() ||
        d->prevWP != d->wpInput->value() || d->prevHP != d->hpInput->value())
    {
        slotValuesChanged();
    }

    disableSettings();

    ImageIface* const iface = d->previewWidget->imageIface();
    int w                   = iface->previewSize().width();
    int h                   = iface->previewSize().height();
    DImg imTemp             = iface->original()->smoothScale(w, h, Qt::KeepAspectRatio);
    int new_w               = (int)(w*d->wpInput->value()/100.0);
    int new_h               = (int)(h*d->hpInput->value()/100.0);

    if (d->mixedRescaleInput->value()<100.0) // mixed rescale
    {
        double stdRescaleP = (100.0 - d->mixedRescaleInput->value()) / 100.0;
        int diff_w         = (int)(stdRescaleP * (w - new_w));
        int diff_h         = (int)(stdRescaleP * (h - new_h));

        imTemp.resize(imTemp.width() - diff_w, imTemp.height() - diff_h);
    }

    QImage mask;

    if (d->weightMaskBox->isChecked())
    {
        mask = d->previewWidget->getMask();
    }

    contentAwareResizeCore(&imTemp, new_w, new_h, mask);
}

void ContentAwareResizeTool::prepareFinal()
{
    if (d->prevW  != d->wInput->value()  || d->prevH  != d->hInput->value() ||
        d->prevWP != d->wpInput->value() || d->prevHP != d->hpInput->value())
    {
        slotValuesChanged();
    }

    disableSettings();

    ImageIface iface;
    QImage     mask;

    if (d->mixedRescaleInput->value() < 100.0) // mixed rescale
    {
        double stdRescaleP = (100.0 - d->mixedRescaleInput->value()) / 100.0;
        int diff_w         = (int)(stdRescaleP * (iface.originalSize().width()  - d->wInput->value()));
        int diff_h         = (int)(stdRescaleP * (iface.originalSize().height() - d->hInput->value()));
        DImg image         = iface.original()->smoothScale(iface.originalSize().width()  - diff_w,
                             iface.originalSize().height() - diff_h,
                             Qt::IgnoreAspectRatio);

        if (d->weightMaskBox->isChecked())
        {
            mask = d->previewWidget->getMask().scaled(iface.originalSize().width()  - diff_w,
                    iface.originalSize().height() - diff_h);
        }

        contentAwareResizeCore(&image, d->wInput->value(), d->hInput->value(), mask);
    }
    else
    {
        if (d->weightMaskBox->isChecked())
        {
            mask = d->previewWidget->getMask().scaled(iface.originalSize());
        }

        contentAwareResizeCore(iface.original(), d->wInput->value(), d->hInput->value(), mask);
    }
}

void ContentAwareResizeTool::setPreviewImage()
{
    ImageIface* const iface = d->previewWidget->imageIface();
    int w                   = iface->previewSize().width();
    int h                   = iface->previewSize().height();
    DImg imTemp             = filter()->getTargetImage().smoothScale(w, h, Qt::KeepAspectRatio);
    DImg imDest(w, h, filter()->getTargetImage().sixteenBit(),
                filter()->getTargetImage().hasAlpha());

    QColor background = toolView()->backgroundRole();
    imDest.fill(DColor(background, filter()->getTargetImage().sixteenBit()));
    imDest.bitBltImage(&imTemp, (w-imTemp.width())/2, (h-imTemp.height())/2);

    iface->setPreview(imDest.smoothScale(iface->previewSize()));
    d->previewWidget->updatePreview();
}

void ContentAwareResizeTool::renderingFinished()
{
    d->preserveRatioBox->setEnabled(true);
    d->wInput->setEnabled(true);
    d->hInput->setEnabled(true);
    d->wpInput->setEnabled(true);
    d->hpInput->setEnabled(true);
    d->mixedRescaleInput->setEnabled(true);
    enableContentAwareSettings(true);
}

void ContentAwareResizeTool::setFinalImage()
{
    ImageIface iface;
    DImg targetImage = filter()->getTargetImage();
    iface.setOriginal(i18n("Liquid Rescale"), filter()->filterAction(), targetImage);
}

void ContentAwareResizeTool::blockWidgetSignals(bool b)
{
    d->preserveRatioBox->blockSignals(b);
    d->wInput->blockSignals(b);
    d->hInput->blockSignals(b);
    d->wpInput->blockSignals(b);
    d->hpInput->blockSignals(b);
    d->mixedRescaleInput->blockSignals(b);
    d->weightMaskBox->blockSignals(b);
    d->redMaskTool->blockSignals(b);
    d->greenMaskTool->blockSignals(b);
    d->eraseMaskTool->blockSignals(b);
}

void ContentAwareResizeTool::slotMaskColorChanged(int type)
{
    d->previewWidget->setEraseMode(type == Private::eraseMask);

    if (type == Private::redMask)
    {
        d->previewWidget->setPaintColor(QColor(255, 0, 0));
    }
    else if (type == Private::greenMask)
    {
        d->previewWidget->setPaintColor(QColor(0, 255, 0));
    }
    else // erase
    {
        d->previewWidget->setPaintColor(QColor(255, 255, 0));
    }
}

void ContentAwareResizeTool::slotWeightMaskBoxStateChanged(int state)
{
    if (state == Qt::Unchecked)
    {
        d->redMaskTool->setEnabled(false);
        d->greenMaskTool->setEnabled(false);
        d->eraseMaskTool->setEnabled(false);
        d->maskPenSize->setEnabled(false);
        d->previewWidget->setMaskEnabled(false);
    }
    else    // Checked
    {
        d->redMaskTool->setEnabled(true);
        d->greenMaskTool->setEnabled(true);
        d->eraseMaskTool->setEnabled(true);
        d->maskPenSize->setEnabled(true);
        d->previewWidget->setMaskEnabled(true);

        if (d->redMaskTool->isChecked())
        {
            d->previewWidget->setPaintColor(QColor(255, 0, 0, 255));
        }
        else
        {
            d->previewWidget->setPaintColor(QColor(0, 255, 0, 255));
        }
    }
}

void ContentAwareResizeTool::slotMaskPenSizeChanged(int size)
{
    d->previewWidget->setMaskPenSize(size);
}

bool ContentAwareResizeTool::eventFilter(QObject* obj, QEvent* ev)
{
    if (d->weightMaskBox->isChecked())
    {
        if (obj == d->previewWidget)
        {
            if (ev->type() == QEvent::Wheel)
            {
                QWheelEvent* wheel = static_cast<QWheelEvent *>(ev);

                if (wheel->delta() >= 0)
                    d->maskPenSize->setValue(d->maskPenSize->value() + (wheel->delta()/8/15)*(wheel->delta()/8/15));
                else
                    d->maskPenSize->setValue(d->maskPenSize->value() - (wheel->delta()/8/15)*(wheel->delta()/8/15));

                d->previewWidget->setMaskCursor();
            }
        }
    }

    return false;
}

} // namespace DigikamTransformImagePlugin
