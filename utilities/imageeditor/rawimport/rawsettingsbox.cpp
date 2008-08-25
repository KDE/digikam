/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-08-11
 * Description : Raw import settings box
 *
 * Copyright (C) 2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes.

#include <QString>
#include <QLayout>
#include <QButtonGroup>
#include <QComboBox>
#include <QLabel>
#include <QToolButton>
#include <QToolBox>
#include <QPushButton>

// KDE includes.

#include <kapplication.h>
#include <ktabwidget.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kconfig.h>
#include <kstandarddirs.h>
#include <kfiledialog.h>
#include <kvbox.h>

// LibKDcraw includes.

#include <libkdcraw/dcrawsettingswidget.h>
#include <libkdcraw/rnuminput.h>

// Local includes.

#include "ddebug.h"
#include "imagedialog.h"
#include "imagehistogram.h"
#include "imagecurves.h"
#include "iccpreviewwidget.h"
#include "histogramwidget.h"
#include "curveswidget.h"
#include "colorgradientwidget.h"
#include "rawsettingsbox.h"
#include "rawsettingsbox.moc"

using namespace KDcrawIface;

namespace Digikam
{

class RawSettingsBoxPriv
{
public:

    enum ColorChannel
    {
        LuminosityChannel=0,
        RedChannel,
        GreenChannel,
        BlueChannel,
        ColorChannels
    };

    enum AllColorsColorType
    {
        AllColorsRed=0,
        AllColorsGreen,
        AllColorsBlue
    };

public:

    RawSettingsBoxPriv()
    {
        channelCB              = 0;
        colorsCB               = 0;
        scaleBG                = 0;
        hGradient              = 0;
        histogramWidget        = 0;
        infoBox                = 0;
        advExposureBox         = 0;
        gammaLabel             = 0;
        gammaInput             = 0;
        saturationLabel        = 0;
        saturationInput        = 0;
        fineExposureLabel      = 0;
        fineExposureInput      = 0;
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

    QWidget             *advExposureBox;
    QWidget             *curveBox;
    QWidget             *rawdecodingBox;

    QComboBox           *channelCB;
    QComboBox           *colorsCB;

    QLabel              *brightnessLabel;
    QLabel              *contrastLabel;
    QLabel              *gammaLabel;
    QLabel              *saturationLabel;
    QLabel              *fineExposureLabel;

    QButtonGroup        *scaleBG;

    QPushButton         *abortBtn;
    QPushButton         *updateBtn;

    QToolButton         *resetCurveBtn;

    QToolBox            *postProcessSettingsBox;

    KTabWidget          *tabView;

    ColorGradientWidget *hGradient;

    CurvesWidget        *curveWidget;

    HistogramWidget     *histogramWidget;

    ImageDialogPreview  *infoBox;

    RIntNumInput        *contrastInput;
    RIntNumInput        *brightnessInput;

    RDoubleNumInput     *gammaInput;
    RDoubleNumInput     *saturationInput;
    RDoubleNumInput     *fineExposureInput;

    DcrawSettingsWidget *decodingSettingsBox;
};

RawSettingsBox::RawSettingsBox(const KUrl& url, QWidget *parent)
              : EditorToolSettings(Default|Ok|Cancel, parent)
{
    d = new RawSettingsBoxPriv;

    // ---------------------------------------------------------------

    QGridLayout* gridSettings = new QGridLayout(plainPage());

    QLabel *label1 = new QLabel(i18n("Channel:"), plainPage());
    label1->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
    d->channelCB   = new QComboBox(plainPage());
    d->channelCB->addItem( i18n("Luminosity") );
    d->channelCB->addItem( i18n("Red") );
    d->channelCB->addItem( i18n("Green") );
    d->channelCB->addItem( i18n("Blue") );
    d->channelCB->addItem( i18n("Colors") );
    d->channelCB->setWhatsThis(i18n("<p>Select the histogram channel to display here:<p>"
                                    "<b>Luminosity</b>: display the image's luminosity values.<p>"
                                    "<b>Red</b>: display the red image-channel values.<p>"
                                    "<b>Green</b>: display the green image-channel values.<p>"
                                    "<b>Blue</b>: display the blue image-channel values.<p>"
                                    "<b>Colors</b>: Display all color channel values at the same time."));

    // ---------------------------------------------------------------

    QWidget *scaleBox = new QWidget(plainPage());
    QHBoxLayout *hlay = new QHBoxLayout(scaleBox);
    d->scaleBG        = new QButtonGroup(scaleBox);
    scaleBox->setWhatsThis(i18n("<p>Select the histogram scale here.<p>"
                                "If the image's maximal counts are small, you can use the linear scale.<p>"
                                "Logarithmic scale can be used when the maximal counts are big; "
                                "if it is used, all values (small and large) will be visible on the graph."));

    QToolButton *linHistoButton = new QToolButton( scaleBox );
    linHistoButton->setToolTip( i18n( "<p>Linear" ) );
    linHistoButton->setIcon(KIcon("view-object-histogram-linear"));
    linHistoButton->setCheckable(true);
    d->scaleBG->addButton(linHistoButton, Digikam::CurvesWidget::LinScaleHistogram);

    QToolButton *logHistoButton = new QToolButton( scaleBox );
    logHistoButton->setToolTip( i18n( "<p>Logarithmic" ) );
    logHistoButton->setIcon(KIcon("view-object-histogram-logarithmic"));
    logHistoButton->setCheckable(true);
    d->scaleBG->addButton(logHistoButton, Digikam::CurvesWidget::LogScaleHistogram);

    hlay->setMargin(0);
    hlay->setSpacing(0);
    hlay->addWidget(linHistoButton);
    hlay->addWidget(logHistoButton);

    d->scaleBG->setExclusive(true);
    logHistoButton->setChecked(true);

    // ---------------------------------------------------------------

    QLabel *label10 = new QLabel(i18n("Colors:"), plainPage());
    label10->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
    d->colorsCB     = new QComboBox(plainPage());
    d->colorsCB->addItem( i18n("Red") );
    d->colorsCB->addItem( i18n("Green") );
    d->colorsCB->addItem( i18n("Blue") );
    d->colorsCB->setEnabled( false );
    d->colorsCB->setWhatsThis(i18n("<p>Select the main color displayed with Colors Channel mode here:<p>"
                                   "<b>Red</b>: Draw the red image channel in the foreground.<p>"
                                   "<b>Green</b>: Draw the green image channel in the foreground.<p>"
                                   "<b>Blue</b>: Draw the blue image channel in the foreground.<p>"));

    // ---------------------------------------------------------------

    KVBox *histoBox    = new KVBox(plainPage());
    d->histogramWidget = new HistogramWidget(256, 140, histoBox, false, true, true);
    d->histogramWidget->setWhatsThis(i18n("<p>Here you can see the target preview image histogram drawing "
                                          "of the selected image channel. This one is re-computed at any "
                                          "settings changes."));
    QLabel *space = new QLabel(histoBox);
    space->setFixedHeight(1);
    d->hGradient  = new ColorGradientWidget( ColorGradientWidget::Horizontal, 10, histoBox );
    d->hGradient->setColors( QColor( "black" ), QColor( "white" ) );

    // ---------------------------------------------------------------

    d->tabView             = new KTabWidget(plainPage());
    d->rawdecodingBox      = new QWidget(d->tabView);
    QGridLayout* rawGrid   = new QGridLayout(d->rawdecodingBox);
    d->decodingSettingsBox = new DcrawSettingsWidget(d->rawdecodingBox, true, true, false);

    KFileDialog *inputDlg  = d->decodingSettingsBox->inputProfileUrlEdit()->fileDialog();
    inputDlg->setPreviewWidget(new ICCPreviewWidget(inputDlg));

    KFileDialog *outputDlg = d->decodingSettingsBox->outputProfileUrlEdit()->fileDialog();
    outputDlg->setPreviewWidget(new ICCPreviewWidget(outputDlg));

    d->abortBtn = new QPushButton(d->rawdecodingBox);
    d->abortBtn->setText(i18n("Abort"));
    d->abortBtn->setIcon(SmallIcon("dialog-cancel"));
    d->abortBtn->setEnabled(false);
    d->abortBtn->setToolTip(i18n("Abort the current Raw image preview."));

    d->updateBtn = new QPushButton(d->rawdecodingBox);
    d->updateBtn->setText(i18n("Update"));
    d->updateBtn->setIcon(SmallIcon("view-refresh"));
    d->updateBtn->setEnabled(false);
    d->updateBtn->setToolTip(i18n("Generate a Raw image preview using current settings."));

    rawGrid->addWidget(d->decodingSettingsBox, 0, 0, 1, 3);
    rawGrid->addWidget(d->abortBtn,            1, 0, 1, 1);
    rawGrid->addWidget(d->updateBtn,           1, 2, 1, 1);
    rawGrid->setColumnStretch(1, 10);
    rawGrid->setSpacing(spacingHint());
    rawGrid->setMargin(spacingHint());

    // ---------------------------------------------------------------

    d->postProcessSettingsBox = new QToolBox(d->tabView);
    d->infoBox                = new ImageDialogPreview(d->postProcessSettingsBox);
    d->infoBox->showPreview(url);

    // ---------------------------------------------------------------

    d->advExposureBox              = new QWidget(d->postProcessSettingsBox);
    QGridLayout* advExposureLayout = new QGridLayout(d->advExposureBox);

    d->brightnessLabel = new QLabel(i18n("Brightness:"), d->advExposureBox);
    d->brightnessInput = new RIntNumInput(d->advExposureBox);
    d->brightnessInput->setRange(-100, 100, 1);
    d->brightnessInput->setDefaultValue(0);
    d->brightnessInput->setSliderEnabled(true);
    d->brightnessInput->input()->setWhatsThis(i18n("<p>Set here the brightness adjustment of the image."));

    d->contrastLabel = new QLabel(i18n("Contrast:"), d->advExposureBox);
    d->contrastInput = new RIntNumInput(d->advExposureBox);
    d->contrastInput->setRange(-100, 100, 1);
    d->contrastInput->setDefaultValue(0);
    d->contrastInput->setSliderEnabled(true);
    d->contrastInput->input()->setWhatsThis(i18n("<p>Set here the contrast adjustment of the image."));

    d->gammaLabel = new QLabel(i18n("Gamma:"), d->advExposureBox);
    d->gammaInput = new RDoubleNumInput(d->advExposureBox);
    d->gammaInput->setDecimals(2);
    d->gammaInput->setRange(0.1, 3.0, 0.01);
    d->gammaInput->setDefaultValue(1.0);
    d->gammaInput->input()->setWhatsThis(i18n("Set here the gamma adjustement of the image"));

    d->saturationLabel = new QLabel(i18n("Saturation:"), d->advExposureBox);
    d->saturationInput = new RDoubleNumInput(d->advExposureBox);
    d->saturationInput->setDecimals(2);
    d->saturationInput->setRange(0.0, 2.0, 0.01);
    d->saturationInput->setDefaultValue(1.0);
    d->saturationInput->input()->setWhatsThis(i18n("<p>Set here the color saturation correction."));

    d->fineExposureLabel = new QLabel(i18n("Exposure (E.V):"), d->advExposureBox);
    d->fineExposureInput = new RDoubleNumInput(d->advExposureBox);
    d->fineExposureInput->setDecimals(2);
    d->fineExposureInput->setRange(-3.0, 3.0, 0.1);
    d->fineExposureInput->setDefaultValue(0.0);
    d->fineExposureInput->input()->setWhatsThis(i18n("<p>This value in E.V will be used to perform "
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
    advExposureLayout->addWidget(d->fineExposureInput, 4, 1, 1, 2);
    advExposureLayout->setRowStretch(5, 10);
    advExposureLayout->setSpacing(0);
    advExposureLayout->setMargin(spacingHint());

    // ---------------------------------------------------------------

    d->curveBox              = new QWidget(d->postProcessSettingsBox);
    QGridLayout* curveLayout = new QGridLayout(d->curveBox);

    ColorGradientWidget* vGradient = new ColorGradientWidget(ColorGradientWidget::Vertical, 10, d->curveBox);
    vGradient->setColors( QColor( "white" ), QColor( "black" ) );

    QLabel *spacev = new QLabel(d->curveBox);
    spacev->setFixedWidth(1);

    d->curveWidget = new CurvesWidget(256, 192, d->curveBox);
    d->curveWidget->setWhatsThis(i18n("<p>This is the curve adjustment of the image luminosity"));

    d->resetCurveBtn = new QToolButton(d->curveBox);
    d->resetCurveBtn->setFixedSize(11, 11);
    d->resetCurveBtn->setIcon(SmallIcon("document-revert", 8));
    d->resetCurveBtn->setFocusPolicy(Qt::NoFocus);
    d->resetCurveBtn->setAutoRaise(true);
    d->resetCurveBtn->setToolTip(i18n("Reset curve to linear"));

    QLabel *spaceh = new QLabel(d->curveBox);
    spaceh->setFixedHeight(1);

    ColorGradientWidget *hGradient = new ColorGradientWidget(ColorGradientWidget::Horizontal, 10, d->curveBox);
    hGradient->setColors( QColor( "black" ), QColor( "white" ) );

    curveLayout->addWidget(vGradient,        0, 0, 1, 1);
    curveLayout->addWidget(spacev,           0, 1, 1, 1);
    curveLayout->addWidget(d->curveWidget,   0, 2, 1, 1);
    curveLayout->addWidget(spaceh,           1, 2, 1, 1);
    curveLayout->addWidget(d->resetCurveBtn, 1, 0, 2, 2);
    curveLayout->addWidget(hGradient,        2, 2, 1, 1);
    curveLayout->setRowStretch(3, 10);
    curveLayout->setSpacing(0);
    curveLayout->setMargin(spacingHint());

    // ---------------------------------------------------------------

    d->postProcessSettingsBox->addItem(d->advExposureBox, i18n("Exposure"));
    d->postProcessSettingsBox->addItem(d->curveBox,       i18n("Luminosity Curve"));
    d->postProcessSettingsBox->setItemIcon(0, SmallIcon("contrast"));
    d->postProcessSettingsBox->setItemIcon(1, SmallIcon("adjustcurves"));

    d->decodingSettingsBox->setItemIcon(DcrawSettingsWidget::DEMOSAICING,     SmallIcon("kdcraw"));
    d->decodingSettingsBox->setItemIcon(DcrawSettingsWidget::WHITEBALANCE,    SmallIcon("whitebalance"));
    d->decodingSettingsBox->setItemIcon(DcrawSettingsWidget::CORRECTIONS,     SmallIcon("lensdistortion"));
    d->decodingSettingsBox->setItemIcon(DcrawSettingsWidget::COLORMANAGEMENT, SmallIcon("colormanagement"));
    d->decodingSettingsBox->updateMinimumWidth();

    d->tabView->insertTab(0, d->rawdecodingBox,         i18n("Raw Decoding"));
    d->tabView->insertTab(1, d->postProcessSettingsBox, i18n("Post Processing"));
    d->tabView->insertTab(2, d->infoBox,                i18n("Info"));

    // ---------------------------------------------------------------

    button(Default)->setText(i18n("Reset"));
    button(Default)->setIcon(KIcon(SmallIcon("document-revert")));
    button(Default)->setToolTip(i18n("<p>Reset all settings to default values."));

    button(Ok)->setText(i18n("Import"));
    button(Ok)->setIcon(KIcon(SmallIcon("dialog-ok")));
    button(Ok)->setToolTip(i18n("<p>Import image to editor using current settings."));

    button(Cancel)->setText(i18n("Use Default"));
    button(Cancel)->setIcon(KIcon(SmallIcon("go-home")));
    button(Cancel)->setToolTip(i18n("<p>Use general Raw decoding settings to load this image in editor."));

    // ---------------------------------------------------------------

    gridSettings->addWidget(label1,       0, 0, 1, 1);
    gridSettings->addWidget(d->channelCB, 0, 1, 1, 1);
    gridSettings->addWidget(scaleBox,     0, 4, 1, 1);
    gridSettings->addWidget(label10,      1, 0, 1, 1);
    gridSettings->addWidget(d->colorsCB,  1, 1, 1, 1);
    gridSettings->addWidget(histoBox,     2, 0, 2, 5);
    gridSettings->addWidget(d->tabView,   4, 0, 1, 5);
    gridSettings->setRowStretch(5, 10);
    gridSettings->setColumnStretch(2, 10);
    gridSettings->setSpacing(spacingHint());
    gridSettings->setMargin(0);

    // ---------------------------------------------------------------

    connect(d->channelCB, SIGNAL(activated(int)),
            this, SLOT(slotChannelChanged(int)));

    connect(d->scaleBG, SIGNAL(buttonReleased(int)),
            this, SLOT(slotScaleChanged(int)));

    connect(d->colorsCB, SIGNAL(activated(int)),
            this, SLOT(slotColorsChanged(int)));

    connect(d->resetCurveBtn, SIGNAL(clicked()),
            this, SLOT(slotResetCurve()));

    connect(d->updateBtn, SIGNAL(clicked()),
            this, SIGNAL(signalUpdatePreview()));

    connect(d->abortBtn, SIGNAL(clicked()),
            this, SIGNAL(signalAbortPreview()));

    connect(d->decodingSettingsBox, SIGNAL(signalSettingsChanged()),
            this, SIGNAL(signalDemosaicingChanged()));

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

    connect(d->fineExposureInput, SIGNAL(valueChanged(double)),
            this, SIGNAL(signalPostProcessingChanged()));
}

RawSettingsBox::~RawSettingsBox()
{
    delete d->curveWidget;
    delete d;
}

void RawSettingsBox::enableUpdateBtn(bool b)
{
    d->updateBtn->setEnabled(b);
}

void RawSettingsBox::setBusy(bool b)
{
    d->decodingSettingsBox->setEnabled(!b);
    d->abortBtn->setEnabled(b);
}

void RawSettingsBox::setDemosaicedImage(DImg& img)
{
    d->curveWidget->stopHistogramComputation();
    d->curveWidget->updateData(img.bits(), img.width(), img.height(), img.sixteenBit());
}

void RawSettingsBox::setPostProcessedImage(DImg& img)
{
    d->histogramWidget->stopHistogramComputation();
    d->histogramWidget->updateData(img.bits(), img.width(), img.height(), img.sixteenBit());
}

void RawSettingsBox::resetSettings()
{
    d->decodingSettingsBox->setDefaultSettings();
    d->brightnessInput->slotReset();
    d->contrastInput->slotReset();
    d->gammaInput->slotReset();
    d->saturationInput->slotReset();
    d->fineExposureInput->slotReset();
    slotResetCurve();
}

void RawSettingsBox::slotResetCurve()
{
    d->curveWidget->reset();
    emit signalPostProcessingChanged();
}

HistogramWidget* RawSettingsBox::histogram() const
{
   return d->histogramWidget;
}

CurvesWidget* RawSettingsBox::curve() const
{
    return d->curveWidget;
}

void RawSettingsBox::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("RAW Import Settings");

    d->channelCB->setCurrentIndex(group.readEntry("Histogram Channel", (int)RawSettingsBoxPriv::LuminosityChannel));
    d->scaleBG->button(group.readEntry("Histogram Scale",
                       (int)Digikam::CurvesWidget::LogScaleHistogram))->setChecked(true);
    d->colorsCB->setCurrentIndex(group.readEntry("Histogram Color", (int)RawSettingsBoxPriv::AllColorsRed));

    d->decodingSettingsBox->setSixteenBits(group.readEntry("SixteenBitsImage", false));
    d->decodingSettingsBox->setWhiteBalance((DRawDecoding::WhiteBalance)
                                            group.readEntry("White Balance",
                                            (int)DRawDecoding::CAMERA));
    d->decodingSettingsBox->setCustomWhiteBalance(group.readEntry("Custom White Balance", 6500));
    d->decodingSettingsBox->setCustomWhiteBalanceGreen(group.readEntry("Custom White Balance Green", 1.0));
    d->decodingSettingsBox->setFourColor(group.readEntry("Four Color RGB", false));
    d->decodingSettingsBox->setUnclipColor(group.readEntry("Unclip Color", 0));
    d->decodingSettingsBox->setDontStretchPixels(group.readEntry("Dont Stretch Pixels", false));
    d->decodingSettingsBox->setNoiseReduction(group.readEntry("Use Noise Reduction", false));
    d->decodingSettingsBox->setUseBlackPoint(group.readEntry("Use Black Point", false));
    d->decodingSettingsBox->setBlackPoint(group.readEntry("Black Point", 0));
    d->decodingSettingsBox->setUseWhitePoint(group.readEntry("Use White Point", false));
    d->decodingSettingsBox->setWhitePoint(group.readEntry("White Point", 0));
    d->decodingSettingsBox->setMedianFilterPasses(group.readEntry("Median Filter Passes", 0));
    d->decodingSettingsBox->setNRThreshold(group.readEntry("NR Threshold", 100));
    d->decodingSettingsBox->setUseCACorrection(group.readEntry("EnableCACorrection", false));
    d->decodingSettingsBox->setcaRedMultiplier(group.readEntry("caRedMultiplier", 1.0));
    d->decodingSettingsBox->setcaBlueMultiplier(group.readEntry("caBlueMultiplier", 1.0));

    d->decodingSettingsBox->setQuality(
        (DRawDecoding::DecodingQuality)group.readEntry("Decoding Quality",
            (int)(DRawDecoding::BILINEAR)));

    d->decodingSettingsBox->setInputColorSpace(
        (DRawDecoding::InputColorSpace)group.readEntry("Input Color Space",
            (int)(DRawDecoding::NOINPUTCS)));

    d->decodingSettingsBox->setOutputColorSpace(
        (DRawDecoding::OutputColorSpace)group.readEntry("Output Color Space",
            (int)(DRawDecoding::SRGB)));

    d->decodingSettingsBox->setInputColorProfile(group.readEntry("Input Color Profile", QString()));
    d->decodingSettingsBox->setOutputColorProfile(group.readEntry("Output Color Profile", QString()));

    d->brightnessInput->setValue(group.readEntry("Brightness", 0));
    d->contrastInput->setValue(group.readEntry("Constrast", 0));
    d->gammaInput->setValue(group.readEntry("Gamma", 1.0));
    d->saturationInput->setValue(group.readEntry("Saturation", 1.0));
    d->fineExposureInput->setValue(group.readEntry("FineExposure", 0.0));

    d->curveWidget->reset();

    for (int j = 0 ; j <= 17 ; j++)
    {
        QPoint disable(-1, -1);
        QPoint p = group.readEntry(QString("CurveAjustmentPoint%1").arg(j), disable);
        if (!d->decodingSettingsBox->sixteenBits() && p != disable)
        {
            // Restore point as 16 bits depth.
            p.setX(p.x()/255);
            p.setY(p.y()/255);
        }
        d->curveWidget->curves()->setCurvePoint(ImageHistogram::ValueChannel, j, p);
    }
    d->curveWidget->curves()->curvesCalculateCurve(ImageHistogram::ValueChannel);

    d->tabView->setCurrentIndex(group.readEntry("Settings Page", 0));
    d->decodingSettingsBox->setCurrentIndex(group.readEntry("Decoding Settings Tab", (int)DcrawSettingsWidget::DEMOSAICING));
    d->postProcessSettingsBox->setCurrentIndex(group.readEntry("Post Processing Settings Tab", 0));

    slotChannelChanged(d->channelCB->currentIndex());
    slotScaleChanged(d->scaleBG->checkedId());
    slotColorsChanged(d->colorsCB->currentIndex());
}

void RawSettingsBox::saveSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("RAW Import Settings");

    group.writeEntry("Histogram Channel",          d->channelCB->currentIndex());
    group.writeEntry("Histogram Scale",            d->scaleBG->checkedId());
    group.writeEntry("Histogram Color",            d->colorsCB->currentIndex());

    group.writeEntry("SixteenBitsImage",           d->decodingSettingsBox->sixteenBits());
    group.writeEntry("White Balance",              (int)d->decodingSettingsBox->whiteBalance());
    group.writeEntry("Custom White Balance",       d->decodingSettingsBox->customWhiteBalance());
    group.writeEntry("Custom White Balance Green", d->decodingSettingsBox->customWhiteBalanceGreen());
    group.writeEntry("Four Color RGB",             d->decodingSettingsBox->useFourColor());
    group.writeEntry("Unclip Color",               d->decodingSettingsBox->unclipColor());
    group.writeEntry("Dont Stretch Pixels",        d->decodingSettingsBox->useDontStretchPixels());
    group.writeEntry("Use Noise Reduction",        d->decodingSettingsBox->useNoiseReduction());
    group.writeEntry("Use Black Point",            d->decodingSettingsBox->useBlackPoint());
    group.writeEntry("Black Point",                d->decodingSettingsBox->blackPoint());
    group.writeEntry("Use White Point",            d->decodingSettingsBox->useWhitePoint());
    group.writeEntry("White Point",                d->decodingSettingsBox->whitePoint());
    group.writeEntry("MedianFilterPasses",         d->decodingSettingsBox->medianFilterPasses());
    group.writeEntry("NR Threshold",               d->decodingSettingsBox->NRThreshold());
    group.writeEntry("EnableCACorrection",         d->decodingSettingsBox->useCACorrection());
    group.writeEntry("caRedMultiplier",            d->decodingSettingsBox->caRedMultiplier());
    group.writeEntry("caBlueMultiplier",           d->decodingSettingsBox->caBlueMultiplier());
    group.writeEntry("Decoding Quality",           (int)d->decodingSettingsBox->quality());
    group.writeEntry("Input Color Space",          (int)d->decodingSettingsBox->inputColorSpace());
    group.writeEntry("Output Color Space",         (int)d->decodingSettingsBox->outputColorSpace());
    group.writeEntry("Input Color Profile",        d->decodingSettingsBox->inputColorProfile());
    group.writeEntry("Output Color Profile",       d->decodingSettingsBox->outputColorProfile());

    group.writeEntry("Brightness",                 d->brightnessInput->value());
    group.writeEntry("Constrast",                  d->contrastInput->value());
    group.writeEntry("Gamma",                      d->gammaInput->value());
    group.writeEntry("Saturation",                 d->saturationInput->value());
    group.writeEntry("FineExposure",               d->fineExposureInput->value());

    for (int j = 0 ; j <= 17 ; j++)
    {
        QPoint p = d->curveWidget->curves()->getCurvePoint(ImageHistogram::ValueChannel, j);
        if (!d->curveWidget->curves()->isSixteenBits())
        {
            // Store point as 16 bits depth.
            p.setX(p.x()*255);
            p.setY(p.y()*255);
        }
        group.writeEntry(QString("CurveAjustmentPoint%1").arg(j), p);
    }

    group.writeEntry("Settings Page", d->tabView->currentIndex());
    group.writeEntry("Decoding Settings Tab", d->decodingSettingsBox->currentIndex());
    group.writeEntry("Post Processing Settings Tab", d->postProcessSettingsBox->currentIndex());
    group.sync();
}

DRawDecoding RawSettingsBox::settings()
{
    DRawDecoding settings;
    settings.sixteenBitsImage        = d->decodingSettingsBox->sixteenBits();
    settings.whiteBalance            = d->decodingSettingsBox->whiteBalance();
    settings.customWhiteBalance      = d->decodingSettingsBox->customWhiteBalance();
    settings.customWhiteBalanceGreen = d->decodingSettingsBox->customWhiteBalanceGreen();
    settings.RGBInterpolate4Colors   = d->decodingSettingsBox->useFourColor();
    settings.unclipColors            = d->decodingSettingsBox->unclipColor();
    settings.DontStretchPixels       = d->decodingSettingsBox->useDontStretchPixels();
    settings.enableNoiseReduction    = d->decodingSettingsBox->useNoiseReduction();
    settings.enableBlackPoint        = d->decodingSettingsBox->useBlackPoint();
    settings.blackPoint              = d->decodingSettingsBox->blackPoint();
    settings.enableWhitePoint        = d->decodingSettingsBox->useWhitePoint();
    settings.whitePoint              = d->decodingSettingsBox->whitePoint();
    settings.medianFilterPasses      = d->decodingSettingsBox->medianFilterPasses();
    settings.NRThreshold             = d->decodingSettingsBox->NRThreshold();
    settings.enableCACorrection      = d->decodingSettingsBox->useCACorrection();
    settings.caMultiplier[0]         = d->decodingSettingsBox->caRedMultiplier();
    settings.caMultiplier[1]         = d->decodingSettingsBox->caBlueMultiplier();
    settings.RAWQuality              = d->decodingSettingsBox->quality();
    settings.inputColorSpace         = d->decodingSettingsBox->inputColorSpace();
    settings.outputColorSpace        = d->decodingSettingsBox->outputColorSpace();
    settings.inputProfile            = d->decodingSettingsBox->inputColorProfile();
    settings.outputProfile           = d->decodingSettingsBox->outputColorProfile();

    settings.lightness               = (double)d->brightnessInput->value()/250.0;
    settings.contrast                = (double)(d->contrastInput->value()/100.0) + 1.00;
    settings.gamma                   = d->gammaInput->value();
    settings.saturation              = d->saturationInput->value();
    settings.exposureComp            = d->fineExposureInput->value();

    if (d->curveWidget->curves()->isDirty())
        settings.curveAdjust         = d->curveWidget->curves()->getCurvePoints(ImageHistogram::ValueChannel);

    return settings;
}

void RawSettingsBox::slotChannelChanged(int channel)
{
    switch(channel)
    {
        case RawSettingsBoxPriv::LuminosityChannel:
            d->histogramWidget->m_channelType = HistogramWidget::ValueHistogram;
            d->hGradient->setColors( QColor( "black" ), QColor( "white" ) );
            d->colorsCB->setEnabled(false);
            break;

        case RawSettingsBoxPriv::RedChannel:
            d->histogramWidget->m_channelType = HistogramWidget::RedChannelHistogram;
            d->hGradient->setColors( QColor( "black" ), QColor( "red" ) );
            d->colorsCB->setEnabled(false);
            break;

        case RawSettingsBoxPriv::GreenChannel:
            d->histogramWidget->m_channelType = HistogramWidget::GreenChannelHistogram;
            d->hGradient->setColors( QColor( "black" ), QColor( "green" ) );
            d->colorsCB->setEnabled(false);
            break;

        case RawSettingsBoxPriv::BlueChannel:
            d->histogramWidget->m_channelType = HistogramWidget::BlueChannelHistogram;
            d->hGradient->setColors( QColor( "black" ), QColor( "blue" ) );
            d->colorsCB->setEnabled(false);
            break;

        case RawSettingsBoxPriv::ColorChannels:
            d->histogramWidget->m_channelType = HistogramWidget::ColorChannelsHistogram;
            d->hGradient->setColors( QColor( "black" ), QColor( "white" ) );
            d->colorsCB->setEnabled(true);
            break;
    }

    d->histogramWidget->repaint();
}

void RawSettingsBox::slotScaleChanged(int scale)
{
    d->histogramWidget->m_scaleType = scale;
    d->histogramWidget->repaint();
}

void RawSettingsBox::slotColorsChanged(int color)
{
    switch(color)
    {
        case RawSettingsBoxPriv::AllColorsGreen:
            d->histogramWidget->m_colorType = HistogramWidget::GreenColor;
            break;

        case RawSettingsBoxPriv::AllColorsBlue:
            d->histogramWidget->m_colorType = HistogramWidget::BlueColor;
            break;

        default:          // Red.
            d->histogramWidget->m_colorType = HistogramWidget::RedColor;
            break;
    }

    d->histogramWidget->repaint();
}

} // NameSpace Digikam
