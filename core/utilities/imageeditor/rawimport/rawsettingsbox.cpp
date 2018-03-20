/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-08-11
 * Description : Raw import settings box
 *
 * Copyright (C) 2008-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "rawsettingsbox.h"

// Qt includes

#include <QButtonGroup>
#include <QLabel>
#include <QLayout>
#include <QPushButton>
#include <QString>
#include <QToolBox>
#include <QToolButton>
#include <QIcon>
#include <QTabWidget>

// KDE includes

#include <ksharedconfig.h>
#include <klocalizedstring.h>

// Local includes

#include "dlayoutbox.h"
#include "dexpanderbox.h"
#include "dnuminput.h"
#include "colorgradientwidget.h"
#include "curveswidget.h"
#include "histogrambox.h"
#include "histogramwidget.h"
#include "iccpreviewwidget.h"
#include "imagecurves.h"
#include "imagedialog.h"
#include "imagehistogram.h"
#include "digikam_globals.h"
#include "digikam_debug.h"
#include "drawdecoderwidget.h"

namespace Digikam
{

class RawSettingsBox::Private
{
public:

    enum AllColorsColorType
    {
        AllColorsRed = 0,
        AllColorsGreen,
        AllColorsBlue
    };

public:

    Private() :
        optionGroupName(QLatin1String("RAW Import Settings")),
        optionHistogramChannelEntry(QLatin1String("Histogram Channel")),
        optionHistogramScaleEntry(QLatin1String("Histogram Scale")),
        optionBrightnessEntry(QLatin1String("Brightness")),
        optionContrastEntry(QLatin1String("Contrast")),
        optionGammaEntry(QLatin1String("Gamma")),
        optionSaturationEntry(QLatin1String("Saturation")),
        optionMainExposureEntry(QLatin1String("MainExposure")),
        optionCurvePrefix(QLatin1String("RawCurve")),
        optionSettingsPageEntry(QLatin1String("Settings Page")),
        optionDecodingSettingsTabEntry(QLatin1String("Decoding Settings Tab"))
    {
        infoBox                = 0;
        advExposureBox         = 0;
        gammaLabel             = 0;
        gammaInput             = 0;
        saturationLabel        = 0;
        saturationInput        = 0;
        fineExposureLabel      = 0;
        mainExposureInput      = 0;
        contrastInput          = 0;
        contrastLabel          = 0;
        curveBox               = 0;
        curveWidget            = 0;
        resetCurveBtn          = 0;
        decodingSettingsBox    = 0;
        postProcessSettingsBox = 0;
        tabView                = 0;
        abortBtn               = 0;
        updateBtn              = 0;
        rawdecodingBox         = 0;
        brightnessLabel        = 0;
        brightnessInput        = 0;
    }

    const QString        optionGroupName;
    const QString        optionHistogramChannelEntry;
    const QString        optionHistogramScaleEntry;
    const QString        optionBrightnessEntry;
    const QString        optionContrastEntry;
    const QString        optionGammaEntry;
    const QString        optionSaturationEntry;
    const QString        optionMainExposureEntry;
    const QString        optionCurvePrefix;
    const QString        optionSettingsPageEntry;
    const QString        optionDecodingSettingsTabEntry;

    QWidget*             advExposureBox;
    QWidget*             curveBox;
    QWidget*             rawdecodingBox;

    QLabel*              brightnessLabel;
    QLabel*              contrastLabel;
    QLabel*              gammaLabel;
    QLabel*              saturationLabel;
    QLabel*              fineExposureLabel;

    QPushButton*         abortBtn;
    QPushButton*         updateBtn;
    QPushButton*         resetCurveBtn;

    QTabWidget*          tabView;

    CurvesWidget*        curveWidget;

    ImageDialogPreview*  infoBox;

    DExpanderBox*        postProcessSettingsBox;

    DIntNumInput*        contrastInput;
    DIntNumInput*        brightnessInput;

    DDoubleNumInput*     gammaInput;
    DDoubleNumInput*     saturationInput;
    DDoubleNumInput*     mainExposureInput;

    DRawDecoderWidget* decodingSettingsBox;
};

RawSettingsBox::RawSettingsBox(const QUrl& url, QWidget* const parent)
    : EditorToolSettings(parent),
      d(new Private)
{
    setButtons(Default | Ok | Cancel);
    setTools(Histogram);
    setHistogramType(LRGBC);

    QGridLayout* const gridSettings = new QGridLayout(plainPage());
    d->tabView                      = new QTabWidget(plainPage());

    // - RAW Decoding view --------------------------------------------------------------

    d->rawdecodingBox               = new QWidget(d->tabView);
    QGridLayout* const rawGrid      = new QGridLayout(d->rawdecodingBox);
    d->decodingSettingsBox          = new DRawDecoderWidget(d->rawdecodingBox,
                                                            DRawDecoderWidget::SIXTEENBITS |
                                                            DRawDecoderWidget::COLORSPACE);
    d->decodingSettingsBox->setObjectName(QLatin1String("RawSettingsBox Expander"));

    // Note: do not touch the url edit's fileDialog() here.
    // This creates the file dialog, which involved an event loop, which is evil.
    // Adjust file dialog in slotFileDialogAboutToOpen

    d->abortBtn  = new QPushButton(d->rawdecodingBox);
    d->abortBtn->setText(i18n("Abort"));
    d->abortBtn->setIcon(QIcon::fromTheme(QLatin1String("dialog-cancel")));
    d->abortBtn->setEnabled(false);
    d->abortBtn->setToolTip(i18n("Abort the current Raw image preview."));

    d->updateBtn = new QPushButton(d->rawdecodingBox);
    d->updateBtn->setText(i18n("Update"));
    d->updateBtn->setIcon(QIcon::fromTheme(QLatin1String("view-refresh")));
    d->updateBtn->setEnabled(false);
    d->updateBtn->setToolTip(i18n("Generate a Raw image preview using current settings."));

    const int spacing = spacingHint();

    rawGrid->addWidget(d->decodingSettingsBox, 0, 0, 1, 3);
    rawGrid->addWidget(d->abortBtn,            1, 0, 1, 1);
    rawGrid->addWidget(d->updateBtn,           1, 2, 1, 1);
    rawGrid->setColumnStretch(1, 10);
    rawGrid->setContentsMargins(spacing, spacing, spacing, spacing);
    rawGrid->setSpacing(spacing);

    // - Post-processing view --------------------------------------------------------------

    d->postProcessSettingsBox            = new DExpanderBox(d->tabView);
    d->postProcessSettingsBox->setObjectName(QLatin1String("PostProcessingSettingsBox Expander"));

    d->advExposureBox                    = new QWidget(d->postProcessSettingsBox);
    QGridLayout* const advExposureLayout = new QGridLayout(d->advExposureBox);

    d->brightnessLabel = new QLabel(i18n("Brightness:"), d->advExposureBox);
    d->brightnessInput = new DIntNumInput(d->advExposureBox);
    d->brightnessInput->setRange(-100, 100, 1);
    d->brightnessInput->setDefaultValue(0);
    d->brightnessInput->setWhatsThis(i18n("Set here the brightness adjustment of the image."));

    d->contrastLabel = new QLabel(i18n("Contrast:"), d->advExposureBox);
    d->contrastInput = new DIntNumInput(d->advExposureBox);
    d->contrastInput->setRange(-100, 100, 1);
    d->contrastInput->setDefaultValue(0);
    d->contrastInput->setWhatsThis(i18n("Set here the contrast adjustment of the image."));

    d->gammaLabel = new QLabel(i18n("Gamma:"), d->advExposureBox);
    d->gammaInput = new DDoubleNumInput(d->advExposureBox);
    d->gammaInput->setDecimals(2);
    d->gammaInput->setRange(0.1, 3.0, 0.01);
    d->gammaInput->setDefaultValue(1.0);
    d->gammaInput->setWhatsThis(i18n("Set here the gamma adjustment of the image"));

    d->saturationLabel = new QLabel(i18n("Saturation:"), d->advExposureBox);
    d->saturationInput = new DDoubleNumInput(d->advExposureBox);
    d->saturationInput->setDecimals(2);
    d->saturationInput->setRange(0.0, 2.0, 0.01);
    d->saturationInput->setDefaultValue(1.0);
    d->saturationInput->setWhatsThis(i18n("Set here the color saturation correction."));

    d->fineExposureLabel = new QLabel(i18n("Exposure (E.V):"), d->advExposureBox);
    d->mainExposureInput = new DDoubleNumInput(d->advExposureBox);
    d->mainExposureInput->setDecimals(2);
    d->mainExposureInput->setRange(-3.0, 3.0, 0.1);
    d->mainExposureInput->setDefaultValue(0.0);
    d->mainExposureInput->setWhatsThis(i18n("This value in E.V will be used to perform "
                                            "an exposure compensation of the image."));

    advExposureLayout->addWidget(d->brightnessLabel,   0, 0, 1, 1);
    advExposureLayout->addWidget(d->brightnessInput,   0, 1, 1, 2);
    advExposureLayout->addWidget(d->contrastLabel,     1, 0, 1, 1);
    advExposureLayout->addWidget(d->contrastInput,     1, 1, 1, 2);
    advExposureLayout->addWidget(d->gammaLabel,        2, 0, 1, 1);
    advExposureLayout->addWidget(d->gammaInput,        2, 1, 1, 2);
    advExposureLayout->addWidget(d->saturationLabel,   3, 0, 1, 1);
    advExposureLayout->addWidget(d->saturationInput,   3, 1, 1, 2);
    advExposureLayout->addWidget(d->fineExposureLabel, 4, 0, 1, 1);
    advExposureLayout->addWidget(d->mainExposureInput, 4, 1, 1, 2);
    advExposureLayout->setRowStretch(5, 10);
    advExposureLayout->setContentsMargins(spacing, spacing, spacing, spacing);
    advExposureLayout->setSpacing(0);

    // ---------------------------------------------------------------

    d->curveBox                          = new QWidget(d->postProcessSettingsBox);
    QGridLayout* const curveLayout       = new QGridLayout(d->curveBox);

    ColorGradientWidget* const vGradient = new ColorGradientWidget(Qt::Vertical, 10, d->curveBox);
    vGradient->setColors(QColor("white"), QColor("black"));

    QLabel* const spacev                 = new QLabel(d->curveBox);
    spacev->setFixedWidth(1);

    d->curveWidget     = new CurvesWidget(256, 192, d->curveBox);
    d->curveWidget->setWhatsThis(i18n("This is the curve adjustment of the image luminosity"));

    d->resetCurveBtn   = new QPushButton(i18n("Reset"), d->curveBox);
    d->resetCurveBtn->setIcon(QIcon::fromTheme(QLatin1String("document-revert")));
    d->resetCurveBtn->setToolTip(i18n("Reset curve to linear"));

    QLabel* const spaceh                 = new QLabel(d->curveBox);
    spaceh->setFixedHeight(1);

    ColorGradientWidget* const hGradient = new ColorGradientWidget(Qt::Horizontal, 10, d->curveBox);
    hGradient->setColors(QColor("black"), QColor("white"));

    curveLayout->addWidget(vGradient,        0, 0, 1, 1);
    curveLayout->addWidget(spacev,           0, 1, 1, 1);
    curveLayout->addWidget(d->curveWidget,   0, 2, 1, 2);
    curveLayout->addWidget(spaceh,           1, 2, 1, 2);
    curveLayout->addWidget(hGradient,        2, 2, 1, 2);
    curveLayout->addWidget(d->resetCurveBtn, 3, 3, 1, 1);
    curveLayout->setRowStretch(4, 10);
    curveLayout->setColumnStretch(2, 10);
    curveLayout->setContentsMargins(spacing, spacing, spacing, spacing);
    curveLayout->setSpacing(0);

    // ---------------------------------------------------------------

    d->postProcessSettingsBox->addItem(d->advExposureBox, i18n("Exposure"),         QLatin1String("exposure"), true);
    d->postProcessSettingsBox->addItem(d->curveBox,       i18n("Luminosity Curve"), QLatin1String("luminositycurve"), false);
    d->postProcessSettingsBox->setItemIcon(0, QIcon::fromTheme(QLatin1String("contrast")));
    d->postProcessSettingsBox->setItemIcon(1, QIcon::fromTheme(QLatin1String("adjustcurves")));
    d->postProcessSettingsBox->addStretch();

    // - Image info view --------------------------------------------------------------

    d->infoBox = new ImageDialogPreview(d->postProcessSettingsBox);
    d->infoBox->slotShowPreview(url);

    // ---------------------------------------------------------------

    d->decodingSettingsBox->setItemIcon(DRawDecoderWidget::DEMOSAICING,
                                        QIcon::fromTheme(QLatin1String("image-x-adobe-dng")));
    d->decodingSettingsBox->setItemIcon(DRawDecoderWidget::WHITEBALANCE,
                                        QIcon::fromTheme(QLatin1String("bordertool")));
    d->decodingSettingsBox->setItemIcon(DRawDecoderWidget::CORRECTIONS,
                                        QIcon::fromTheme(QLatin1String("zoom-draw")));
    d->decodingSettingsBox->setItemIcon(DRawDecoderWidget::COLORMANAGEMENT,
                                        QIcon::fromTheme(QLatin1String("preferences-desktop-display-color")));
    d->decodingSettingsBox->updateMinimumWidth();

    d->tabView->insertTab(0, d->rawdecodingBox,         i18n("Raw Decoding"));
    d->tabView->insertTab(1, d->postProcessSettingsBox, i18n("Post Processing"));
    d->tabView->insertTab(2, d->infoBox,                i18n("Info"));

    // ---------------------------------------------------------------

    button(Default)->setText(i18n("Reset"));
    button(Default)->setIcon(QIcon::fromTheme(QLatin1String("document-revert")));
    button(Default)->setToolTip(i18n("Reset all settings to default values."));

    button(Ok)->setText(i18n("Import"));
    button(Ok)->setIcon(QIcon::fromTheme(QLatin1String("dialog-ok-apply")));
    button(Ok)->setToolTip(i18n("Import image to editor using current settings."));

    button(Cancel)->setText(i18n("Use Default"));
    button(Cancel)->setIcon(QIcon::fromTheme(QLatin1String("go-home")));
    button(Cancel)->setToolTip(i18n("Use general Raw decoding settings to load this image in editor."));

    // ---------------------------------------------------------------

    gridSettings->addWidget(d->tabView, 0, 0, 1, 5);
    gridSettings->setColumnStretch(2, 10);
    gridSettings->setContentsMargins(QMargins());
    gridSettings->setSpacing(spacing);

    // ---------------------------------------------------------------

    connect(d->resetCurveBtn, SIGNAL(clicked()),
            this, SLOT(slotResetCurve()));

    connect(d->updateBtn, SIGNAL(clicked()),
            this, SIGNAL(signalUpdatePreview()));

    connect(d->abortBtn, SIGNAL(clicked()),
            this, SIGNAL(signalAbortPreview()));

    connect(d->decodingSettingsBox, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotDemosaicingChanged()));

    connect(d->curveWidget, SIGNAL(signalCurvesChanged()),
            this, SIGNAL(signalPostProcessingChanged()));

    connect(d->brightnessInput, SIGNAL(valueChanged(int)),
            this, SIGNAL(signalPostProcessingChanged()));

    connect(d->contrastInput, SIGNAL(valueChanged(int)),
            this, SIGNAL(signalPostProcessingChanged()));

    connect(d->gammaInput, SIGNAL(valueChanged(double)),
            this, SIGNAL(signalPostProcessingChanged()));

    connect(d->saturationInput, SIGNAL(valueChanged(double)),
            this, SIGNAL(signalPostProcessingChanged()));

    connect(d->mainExposureInput, SIGNAL(valueChanged(double)),
            this, SIGNAL(signalPostProcessingChanged()));

    connect(d->decodingSettingsBox->inputProfileUrlEdit(), SIGNAL(signalOpenFileDialog()),
            this, SLOT(slotFileDialogAboutToOpen()));

    connect(d->decodingSettingsBox->outputProfileUrlEdit(), SIGNAL(signalOpenFileDialog()),
            this, SLOT(slotFileDialogAboutToOpen()));
}

RawSettingsBox::~RawSettingsBox()
{
    delete d->curveWidget;
    delete d;
}

void RawSettingsBox::slotDemosaicingChanged()
{
    enableUpdateBtn(true);
}

void RawSettingsBox::enableUpdateBtn(bool b)
{
    d->updateBtn->setEnabled(b);
}

bool RawSettingsBox::updateBtnEnabled() const
{
    return d->updateBtn->isEnabled();
}

void RawSettingsBox::setBusy(bool b)
{
    d->decodingSettingsBox->setEnabled(!b);
    d->abortBtn->setEnabled(b);
}

void RawSettingsBox::setDemosaicedImage(DImg& img)
{
    d->curveWidget->stopHistogramComputation();
    d->curveWidget->updateData(img);
}

void RawSettingsBox::setPostProcessedImage(DImg& img)
{
    histogramBox()->histogram()->stopHistogramComputation();
    histogramBox()->histogram()->updateData(img);
}

void RawSettingsBox::resetSettings()
{
    d->decodingSettingsBox->resetToDefault();
    d->brightnessInput->slotReset();
    d->contrastInput->slotReset();
    d->gammaInput->slotReset();
    d->saturationInput->slotReset();
    d->mainExposureInput->slotReset();
    slotResetCurve();
}

void RawSettingsBox::slotResetCurve()
{
    d->curveWidget->reset();
    emit signalPostProcessingChanged();
}

CurvesWidget* RawSettingsBox::curvesWidget() const
{
    return d->curveWidget;
}

void RawSettingsBox::readSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->optionGroupName);

    histogramBox()->setChannel((ChannelType)group.readEntry(d->optionHistogramChannelEntry,
                                                            (int) LuminosityChannel));
    histogramBox()->setScale((HistogramScale)group.readEntry(d->optionHistogramScaleEntry,
                                                             (int) LogScaleHistogram));
    curvesWidget()->setScaleType((HistogramScale)group.readEntry(d->optionHistogramScaleEntry,
                                                                 (int) LogScaleHistogram));

    d->decodingSettingsBox->readSettings(group);

    d->brightnessInput->setValue(group.readEntry(d->optionBrightnessEntry, 0));
    d->contrastInput->setValue(group.readEntry(d->optionContrastEntry, 0));
    d->gammaInput->setValue(group.readEntry(d->optionGammaEntry, 1.0));
    d->saturationInput->setValue(group.readEntry(d->optionSaturationEntry, 1.0));
    d->mainExposureInput->setValue(group.readEntry(d->optionMainExposureEntry, 0.0));

    d->curveWidget->restoreCurve(group, d->optionCurvePrefix);

    d->tabView->setCurrentIndex(group.readEntry(d->optionSettingsPageEntry, 0));

    d->postProcessSettingsBox->readSettings(group);
}

void RawSettingsBox::writeSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->optionGroupName);

    group.writeEntry(d->optionHistogramChannelEntry, (int)histogramBox()->channel());
    group.writeEntry(d->optionHistogramScaleEntry,   (int)histogramBox()->scale());

    d->decodingSettingsBox->writeSettings(group);

    group.writeEntry(d->optionBrightnessEntry,   d->brightnessInput->value());
    group.writeEntry(d->optionContrastEntry,     d->contrastInput->value());
    group.writeEntry(d->optionGammaEntry,        d->gammaInput->value());
    group.writeEntry(d->optionSaturationEntry,   d->saturationInput->value());
    group.writeEntry(d->optionMainExposureEntry, d->mainExposureInput->value());

    d->curveWidget->saveCurve(group, d->optionCurvePrefix);

    group.writeEntry(d->optionSettingsPageEntry, d->tabView->currentIndex());

    d->postProcessSettingsBox->writeSettings(group);

    group.sync();
}

DRawDecoding RawSettingsBox::settings() const
{
    DRawDecoding settings(d->decodingSettingsBox->settings());

    settings.bcg.brightness    = (double)d->brightnessInput->value() / 250.0;
    settings.bcg.contrast      = (double)(d->contrastInput->value()  / 100.0) + 1.00;
    settings.bcg.gamma         = d->gammaInput->value();
    settings.wb.saturation     = d->saturationInput->value();
    settings.wb.expositionMain = d->mainExposureInput->value();

    if (d->curveWidget->curves()->isDirty())
    {
        d->curveWidget->curves()->curvesCalculateAllCurves();
        settings.curvesAdjust = d->curveWidget->curves()->getContainer();
    }

    return settings;
}

void RawSettingsBox::slotFileDialogAboutToOpen()
{
    //TODO : port to Qt5
    //requester->fileDialog()->setPreviewWidget(new ICCPreviewWidget(requester));
}

} // namespace Digikam
