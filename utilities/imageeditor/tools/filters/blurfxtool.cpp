/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-02-09
 * Description : a tool to apply Blur FX to images
 *
 * Copyright 2005-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright 2006-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "blurfxtool.h"

// Qt includes

#include <QDateTime>
#include <QGridLayout>
#include <QImage>
#include <QLabel>
#include <QSlider>
#include <QIcon>

// KDE includes

#include <ksharedconfig.h>
#include <kconfiggroup.h>
#include <klocalizedstring.h>

// Local includes

#include "dnuminput.h"
#include "blurfxfilter.h"
#include "dcombobox.h"
#include "editortoolsettings.h"
#include "imageiface.h"
#include "imageregionwidget.h"

namespace Digikam
{

class BlurFXTool::Private
{
public:

    Private() :
        effectTypeLabel(0),
        distanceLabel(0),
        levelLabel(0),
        effectType(0),
        distanceInput(0),
        levelInput(0),
        previewWidget(0),
        gboxSettings(0)
    {}


    static const QString configGroupName;
    static const QString configEffectTypeEntry;
    static const QString configDistanceAdjustmentEntry;
    static const QString configLevelAdjustmentEntry;

    QLabel*              effectTypeLabel;
    QLabel*              distanceLabel;
    QLabel*              levelLabel;

    DComboBox*           effectType;

    DIntNumInput*        distanceInput;
    DIntNumInput*        levelInput;

    ImageRegionWidget*   previewWidget;

    EditorToolSettings*  gboxSettings;
};

const QString BlurFXTool::Private::configGroupName(QLatin1String("blurfx Tool"));
const QString BlurFXTool::Private::configEffectTypeEntry(QLatin1String("EffectType"));
const QString BlurFXTool::Private::configDistanceAdjustmentEntry(QLatin1String("DistanceAdjustment"));
const QString BlurFXTool::Private::configLevelAdjustmentEntry(QLatin1String("LevelAdjustment"));

// --------------------------------------------------------

BlurFXTool::BlurFXTool(QObject* const parent)
    : EditorToolThreaded(parent),
      d(new Private)
{
    setObjectName(QLatin1String("blurfx"));
    setToolName(i18n("Blur Effects"));
    setToolIcon(QIcon::fromTheme(QLatin1String("blurfx")));

    // -------------------------------------------------------------

    d->gboxSettings = new EditorToolSettings;
    d->gboxSettings->setButtons(EditorToolSettings::Default|
                                EditorToolSettings::Ok|
                                EditorToolSettings::Cancel|
                                EditorToolSettings::Try);

    d->previewWidget = new ImageRegionWidget;
    d->previewWidget->setWhatsThis(i18n("This is the preview of the blur effect "
                                        "applied to the photograph."));

    // -------------------------------------------------------------

    d->effectTypeLabel = new QLabel(i18n("Type:"));
    d->effectType      = new DComboBox;
    d->effectType->addItem(i18n("Zoom Blur"));
    d->effectType->addItem(i18n("Radial Blur"));
    d->effectType->addItem(i18n("Far Blur"));
    d->effectType->addItem(i18n("Motion Blur"));
    d->effectType->addItem(i18n("Softener Blur"));
    d->effectType->addItem(i18n("Shake Blur"));
    d->effectType->addItem(i18n("Focus Blur"));
    d->effectType->addItem(i18n("Smart Blur"));
    d->effectType->addItem(i18n("Frost Glass"));
    d->effectType->addItem(i18n("Mosaic"));
    d->effectType->setDefaultIndex(BlurFXFilter::ZoomBlur);
    d->effectType->setWhatsThis(i18n("<p>Select the blurring effect to apply to image.</p>"
                                     "<p><b>Zoom Blur</b>:  blurs the image along radial lines starting from "
                                     "a specified center point. This simulates the blur of a zooming camera.</p>"
                                     "<p><b>Radial Blur</b>: blurs the image by rotating the pixels around "
                                     "the specified center point. This simulates the blur of a rotating camera.</p>"
                                     "<p><b>Far Blur</b>: blurs the image by using far pixels. This simulates the blur "
                                     "of an unfocalized camera lens.</p>"
                                     "<p><b>Motion Blur</b>: blurs the image by moving the pixels horizontally. "
                                     "This simulates the blur of a linear moving camera.</p>"
                                     "<p><b>Softener Blur</b>: blurs the image softly in dark tones and hardly in light "
                                     "tones. This gives images a dreamy and glossy soft focus effect. It is ideal "
                                     "for creating romantic portraits, glamour photographs, or giving images a warm "
                                     "and subtle glow.</p>"
                                     "<p><b>Shake Blur</b>: blurs the image by shaking randomly the pixels. "
                                     "This simulates the blur of a random moving camera.</p>"
                                     "<p><b>Focus Blur</b>: blurs the image corners to reproduce the astigmatism distortion "
                                     "of a lens.</p>"
                                     "<p><b>Smart Blur</b>: finds the edges of color in your image and blurs them without "
                                     "muddying the rest of the image.</p>"
                                     "<p><b>Frost Glass</b>: blurs the image by randomly disperse light coming through "
                                     "a frosted glass.</p>"
                                     "<p><b>Mosaic</b>: divides the photograph into rectangular cells and then "
                                     "recreates it by filling those cells with average pixel value.</p>"));

    d->distanceLabel = new QLabel(i18n("Distance:"));
    d->distanceInput = new DIntNumInput;
    d->distanceInput->setRange(0, 100, 1);
    d->distanceInput->setDefaultValue(3);
    d->distanceInput->setWhatsThis( i18n("Set here the blur distance in pixels."));

    d->levelLabel = new QLabel(i18nc("level to use for the effect", "Level:"));
    d->levelInput = new DIntNumInput;
    d->levelInput->setRange(0, 360, 1);
    d->levelInput->setDefaultValue(128);
    d->levelInput->setWhatsThis( i18n("This value controls the level to use with the current effect."));

    connect(d->effectType, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotEffectTypeChanged(int)));

    // -------------------------------------------------------------

    const int spacing = d->gboxSettings->spacingHint();

    QGridLayout* const mainLayout = new QGridLayout;
    mainLayout->addWidget(d->effectTypeLabel, 0, 0, 1, 2);
    mainLayout->addWidget(d->effectType,      1, 0, 1, 2);
    mainLayout->addWidget(d->distanceLabel,   2, 0, 1, 2);
    mainLayout->addWidget(d->distanceInput,   3, 0, 1, 2);
    mainLayout->addWidget(d->levelLabel,      4, 0, 1, 2);
    mainLayout->addWidget(d->levelInput,      5, 0, 1, 2);
    mainLayout->setRowStretch(6, 10);
    mainLayout->setContentsMargins(spacing, spacing, spacing, spacing);
    mainLayout->setSpacing(spacing);
    d->gboxSettings->plainPage()->setLayout(mainLayout);

    // -------------------------------------------------------------

    setPreviewModeMask(PreviewToolBar::AllPreviewModes);
    setToolSettings(d->gboxSettings);
    setToolView(d->previewWidget);

    slotEffectTypeChanged(d->effectType->defaultIndex());
}

BlurFXTool::~BlurFXTool()
{
    delete d;
}

void BlurFXTool::readSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);

    blockWidgetSignals(true);

    d->effectType->setCurrentIndex(group.readEntry(d->configEffectTypeEntry,     d->effectType->defaultIndex()));
    d->distanceInput->setValue(group.readEntry(d->configDistanceAdjustmentEntry, d->distanceInput->defaultValue()));
    d->levelInput->setValue(group.readEntry(d->configLevelAdjustmentEntry,       d->levelInput->defaultValue()));
    slotEffectTypeChanged(d->effectType->defaultIndex());

    blockWidgetSignals(false);
}

void BlurFXTool::writeSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);

    group.writeEntry(d->configEffectTypeEntry,         d->effectType->currentIndex());
    group.writeEntry(d->configDistanceAdjustmentEntry, d->distanceInput->value());
    group.writeEntry(d->configLevelAdjustmentEntry,    d->levelInput->value());
    group.sync();
}

void BlurFXTool::slotResetSettings()
{
    blockWidgetSignals(true);

    d->effectType->slotReset();
    d->distanceInput->slotReset();
    d->levelInput->slotReset();

    blockWidgetSignals(false);

    slotEffectTypeChanged(d->effectType->defaultIndex());
}

void BlurFXTool::slotEffectTypeChanged(int type)
{
    d->distanceInput->setEnabled(true);
    d->distanceLabel->setEnabled(true);

    blockWidgetSignals(true);

    d->distanceInput->setRange(0, 200, 1);
    d->distanceInput->setValue(100);
    d->levelInput->setRange(0, 360, 1);
    d->levelInput->setValue(45);

    d->levelInput->setEnabled(false);
    d->levelLabel->setEnabled(false);

    switch (type)
    {
        case BlurFXFilter::ZoomBlur:
            break;

        case BlurFXFilter::RadialBlur:
        case BlurFXFilter::FrostGlass:
            d->distanceInput->setRange(0, 10, 1);
            d->distanceInput->setValue(3);
            break;

        case BlurFXFilter::FarBlur:
            d->distanceInput->setRange(0, 20, 1);
            d->distanceInput->setValue(10);
            break;

        case BlurFXFilter::MotionBlur:
        case BlurFXFilter::FocusBlur:
            d->distanceInput->setRange(0, 100, 1);
            d->distanceInput->setValue(20);
            d->levelInput->setEnabled(true);
            d->levelLabel->setEnabled(true);
            break;

        case BlurFXFilter::SoftenerBlur:
            d->distanceInput->setEnabled(false);
            d->distanceLabel->setEnabled(false);
            break;

        case BlurFXFilter::ShakeBlur:
            d->distanceInput->setRange(0, 100, 1);
            d->distanceInput->setValue(20);
            break;

        case BlurFXFilter::SmartBlur:
            d->distanceInput->setRange(0, 20, 1);
            d->distanceInput->setValue(3);
            d->levelInput->setEnabled(true);
            d->levelLabel->setEnabled(true);
            d->levelInput->setRange(0, 255, 1);
            d->levelInput->setValue(128);
            break;

        case BlurFXFilter::Mosaic:
            d->distanceInput->setRange(0, 50, 1);
            d->distanceInput->setValue(3);
            break;
    }

    blockWidgetSignals(false);
}

void BlurFXTool::preparePreview()
{
    d->gboxSettings->setEnabled(false);

    DImg image;

    switch (d->effectType->currentIndex())
    {
        case BlurFXFilter::ZoomBlur:
        case BlurFXFilter::RadialBlur:
        case BlurFXFilter::FocusBlur:
        {
            ImageIface iface;
            image = *iface.original();
            break;
        }

        case BlurFXFilter::FarBlur:
        case BlurFXFilter::MotionBlur:
        case BlurFXFilter::SoftenerBlur:
        case BlurFXFilter::ShakeBlur:
        case BlurFXFilter::SmartBlur:
        case BlurFXFilter::FrostGlass:
        case BlurFXFilter::Mosaic:
            image = d->previewWidget->getOriginalRegionImage();
            break;
    }

    int type  = d->effectType->currentIndex();
    int dist  = d->distanceInput->value();
    int level = d->levelInput->value();

    setFilter(new BlurFXFilter(&image, this, type, dist, level));
}

void BlurFXTool::prepareFinal()
{
    d->gboxSettings->setEnabled(false);

    int type  = d->effectType->currentIndex();
    int dist  = d->distanceInput->value();
    int level = d->levelInput->value();

    ImageIface iface;
    setFilter(new BlurFXFilter(iface.original(), this, type, dist, level));
}

void BlurFXTool::setPreviewImage()
{
    switch (d->effectType->currentIndex())
    {
        case BlurFXFilter::ZoomBlur:
        case BlurFXFilter::RadialBlur:
        case BlurFXFilter::FocusBlur:
        {
            QRect pRect  = d->previewWidget->getOriginalImageRegionToRender();
            DImg destImg = filter()->getTargetImage().copy(pRect);
            d->previewWidget->setPreviewImage(destImg);
            break;
        }
        case BlurFXFilter::FarBlur:
        case BlurFXFilter::MotionBlur:
        case BlurFXFilter::SoftenerBlur:
        case BlurFXFilter::ShakeBlur:
        case BlurFXFilter::SmartBlur:
        case BlurFXFilter::FrostGlass:
        case BlurFXFilter::Mosaic:
            d->previewWidget->setPreviewImage(filter()->getTargetImage());
            break;
    }
}

void BlurFXTool::setFinalImage()
{
    ImageIface iface;
    iface.setOriginal(i18n("Blur Effects"), filter()->filterAction(), filter()->getTargetImage());
}

void BlurFXTool::renderingFinished()
{
    d->gboxSettings->setEnabled(true);
}

void BlurFXTool::blockWidgetSignals(bool b)
{
    d->effectType->blockSignals(b);
    d->distanceInput->blockSignals(b);
    d->levelInput->blockSignals(b);
}

}  // namespace Digikam
