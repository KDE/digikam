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

#include <qstring.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qhbuttongroup.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qvbox.h>
#include <qtoolbutton.h>
#include <qtoolbox.h>
#include <qpushbutton.h>

// KDE includes.

#include <kapplication.h>
#include <ktabwidget.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kconfig.h>
#include <kstandarddirs.h>
#include <kfiledialog.h>

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

    QHButtonGroup       *scaleBG;

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

RawSettingsBox::RawSettingsBox(const KURL& url, QWidget *parent)
              : EditorToolSettings(Default|Ok|Cancel, NoTool, parent)
{
    d = new RawSettingsBoxPriv;

    // ---------------------------------------------------------------

    QGridLayout* gridSettings = new QGridLayout(plainPage(), 5, 4);

    QLabel *label1 = new QLabel(i18n("Channel:"), plainPage());
    label1->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
    d->channelCB   = new QComboBox(false, plainPage());
    d->channelCB->insertItem( i18n("Luminosity") );
    d->channelCB->insertItem( i18n("Red") );
    d->channelCB->insertItem( i18n("Green") );
    d->channelCB->insertItem( i18n("Blue") );
    d->channelCB->insertItem( i18n("Colors") );
    QWhatsThis::add(d->channelCB, i18n("<p>Select the histogram channel to display here:<p>"
                                       "<b>Luminosity</b>: display the image's luminosity values.<p>"
                                       "<b>Red</b>: display the red image-channel values.<p>"
                                       "<b>Green</b>: display the green image-channel values.<p>"
                                       "<b>Blue</b>: display the blue image-channel values.<p>"
                                       "<b>Colors</b>: Display all color channel values at the same time."));

    d->scaleBG = new QHButtonGroup(plainPage());
    d->scaleBG->setExclusive(true);
    d->scaleBG->setFrameShape(QFrame::NoFrame);
    d->scaleBG->setInsideMargin( 0 );
    QWhatsThis::add(d->scaleBG, i18n("<p>Select the histogram scale here.<p>"
                                     "If the image's maximal counts are small, you can use the linear scale.<p>"
                                     "Logarithmic scale can be used when the maximal counts are big; "
                                     "if it is used, all values (small and large) will be visible on the graph."));

    QPushButton *linHistoButton = new QPushButton( d->scaleBG );
    QToolTip::add( linHistoButton, i18n( "<p>Linear" ) );
    d->scaleBG->insert(linHistoButton, HistogramWidget::LinScaleHistogram);
    KGlobal::dirs()->addResourceType("histogram-lin", KGlobal::dirs()->kde_default("data") + "digikam/data");
    QString directory = KGlobal::dirs()->findResourceDir("histogram-lin", "histogram-lin.png");
    linHistoButton->setPixmap( QPixmap( directory + "histogram-lin.png" ) );
    linHistoButton->setToggleButton(true);

    QPushButton *logHistoButton = new QPushButton( d->scaleBG );
    QToolTip::add( logHistoButton, i18n( "<p>Logarithmic" ) );
    d->scaleBG->insert(logHistoButton, HistogramWidget::LogScaleHistogram);
    KGlobal::dirs()->addResourceType("histogram-log", KGlobal::dirs()->kde_default("data") + "digikam/data");
    directory = KGlobal::dirs()->findResourceDir("histogram-log", "histogram-log.png");
    logHistoButton->setPixmap( QPixmap( directory + "histogram-log.png" ) );
    logHistoButton->setToggleButton(true);

    QLabel *label10 = new QLabel(i18n("Colors:"), plainPage());
    label10->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
    d->colorsCB = new QComboBox(false, plainPage());
    d->colorsCB->insertItem( i18n("Red") );
    d->colorsCB->insertItem( i18n("Green") );
    d->colorsCB->insertItem( i18n("Blue") );
    d->colorsCB->setEnabled( false );
    QWhatsThis::add( d->colorsCB, i18n("<p>Select the main color displayed with Colors Channel mode here:<p>"
                                       "<b>Red</b>: Draw the red image channel in the foreground.<p>"
                                       "<b>Green</b>: Draw the green image channel in the foreground.<p>"
                                       "<b>Blue</b>: Draw the blue image channel in the foreground.<p>"));

    // ---------------------------------------------------------------

    QVBox *histoBox    = new QVBox(plainPage());
    d->histogramWidget = new HistogramWidget(256, 140, histoBox, false, true, true);
    QWhatsThis::add(d->histogramWidget, i18n("<p>Here you can see the target preview image histogram drawing "
                                             "of the selected image channel. This one is re-computed at any "
                                             "settings changes."));
    QLabel *space = new QLabel(histoBox);
    space->setFixedHeight(1);
    d->hGradient  = new ColorGradientWidget( ColorGradientWidget::Horizontal, 10, histoBox );
    d->hGradient->setColors( QColor( "black" ), QColor( "white" ) );

    // ---------------------------------------------------------------

    d->tabView             = new KTabWidget(plainPage());
    d->rawdecodingBox      = new QWidget(d->tabView);
    QGridLayout* rawGrid   = new QGridLayout(d->rawdecodingBox, 1, 2);
    d->decodingSettingsBox = new DcrawSettingsWidget(d->rawdecodingBox, true, true, false);

    KFileDialog *inputDlg  = d->decodingSettingsBox->inputProfileUrlEdit()->fileDialog();
    inputDlg->setPreviewWidget(new ICCPreviewWidget(inputDlg));

    KFileDialog *outputDlg = d->decodingSettingsBox->outputProfileUrlEdit()->fileDialog();
    outputDlg->setPreviewWidget(new ICCPreviewWidget(outputDlg));

    d->abortBtn = new QPushButton(d->rawdecodingBox);
    d->abortBtn->setText(i18n("Abort"));
    d->abortBtn->setIconSet(SmallIconSet("stop"));
    d->abortBtn->setEnabled(false);
    QToolTip::add(d->abortBtn, i18n("Abort the current Raw image preview."));

    d->updateBtn = new QPushButton(d->rawdecodingBox);
    d->updateBtn->setText(i18n("Update"));
    d->updateBtn->setIconSet(SmallIconSet("reload_page"));
    d->updateBtn->setEnabled(false);
    QToolTip::add(d->updateBtn, i18n("Generate a Raw image preview using current settings."));

    rawGrid->addMultiCellWidget(d->decodingSettingsBox, 0, 0, 0, 2);
    rawGrid->addMultiCellWidget(d->abortBtn,            1, 1, 0, 0);
    rawGrid->addMultiCellWidget(d->updateBtn,           1, 1, 2, 2);
    rawGrid->setColStretch(1, 10);
    rawGrid->setSpacing(spacingHint());
    rawGrid->setMargin(spacingHint());

    // ---------------------------------------------------------------

    d->postProcessSettingsBox = new QToolBox(d->tabView);
    d->infoBox                = new ImageDialogPreview(d->postProcessSettingsBox);
    d->infoBox->showPreview(url);

    // ---------------------------------------------------------------

    d->advExposureBox              = new QWidget(d->postProcessSettingsBox);
    QGridLayout* advExposureLayout = new QGridLayout(d->advExposureBox, 5, 2);

    d->brightnessLabel = new QLabel(i18n("Brightness:"), d->advExposureBox);
    d->brightnessInput = new RIntNumInput(d->advExposureBox);
    d->brightnessInput->setRange(-100, 100, 1);
    d->brightnessInput->setDefaultValue(0);
    QWhatsThis::add(d->brightnessInput->input(), i18n("<p>Set here the brightness adjustment of the image."));

    d->contrastLabel = new QLabel(i18n("Contrast:"), d->advExposureBox);
    d->contrastInput = new RIntNumInput(d->advExposureBox);
    d->contrastInput->setRange(-100, 100, 1);
    d->contrastInput->setDefaultValue(0);
    QWhatsThis::add(d->contrastInput->input(), i18n("<p>Set here the contrast adjustment of the image."));

    d->gammaLabel = new QLabel(i18n("Gamma:"), d->advExposureBox);
    d->gammaInput = new RDoubleNumInput(d->advExposureBox);
    d->gammaInput->setPrecision(2);
    d->gammaInput->setRange(0.1, 3.0, 0.01);
    d->gammaInput->setDefaultValue(1.0);
    QWhatsThis::add(d->gammaInput->input(), i18n("Set here the gamma adjustement of the image"));

    d->saturationLabel = new QLabel(i18n("Saturation:"), d->advExposureBox);
    d->saturationInput = new RDoubleNumInput(d->advExposureBox);
    d->saturationInput->setPrecision(2);
    d->saturationInput->setRange(0.0, 2.0, 0.01);
    d->saturationInput->setDefaultValue(1.0);
    QWhatsThis::add(d->saturationInput->input(), i18n("<p>Set here the color saturation correction."));

    d->fineExposureLabel = new QLabel(i18n("Exposure (E.V):"), d->advExposureBox);
    d->fineExposureInput = new RDoubleNumInput(d->advExposureBox);
    d->fineExposureInput->setPrecision(2);
    d->fineExposureInput->setRange(-3.0, 3.0, 0.1);
    d->fineExposureInput->setDefaultValue(0.0);
    QWhatsThis::add(d->fineExposureInput->input(), i18n("<p>This value in E.V will be used to perform "
                                                        "an exposure compensation of the image."));

    advExposureLayout->addMultiCellWidget(d->brightnessLabel,   0, 0, 0, 0);
    advExposureLayout->addMultiCellWidget(d->brightnessInput,   0, 0, 1, 2);
    advExposureLayout->addMultiCellWidget(d->contrastLabel,     1, 1, 0, 0);
    advExposureLayout->addMultiCellWidget(d->contrastInput,     1, 1, 1, 2);
    advExposureLayout->addMultiCellWidget(d->gammaLabel,        2, 2, 0, 0);
    advExposureLayout->addMultiCellWidget(d->gammaInput,        2, 2, 1, 2);
    advExposureLayout->addMultiCellWidget(d->saturationLabel,   3, 3, 0, 0);
    advExposureLayout->addMultiCellWidget(d->saturationInput,   3, 3, 1, 2);
    advExposureLayout->addMultiCellWidget(d->fineExposureLabel, 4, 4, 0, 0);
    advExposureLayout->addMultiCellWidget(d->fineExposureInput, 4, 4, 1, 2);
    advExposureLayout->setRowStretch(5, 10);
    advExposureLayout->setSpacing(0);
    advExposureLayout->setMargin(spacingHint());

    // ---------------------------------------------------------------

    d->curveBox              = new QWidget(d->postProcessSettingsBox);
    QGridLayout* curveLayout = new QGridLayout(d->curveBox, 3, 2);

    ColorGradientWidget* vGradient = new ColorGradientWidget(ColorGradientWidget::Vertical, 10, d->curveBox);
    vGradient->setColors( QColor( "white" ), QColor( "black" ) );

    QLabel *spacev = new QLabel(d->curveBox);
    spacev->setFixedWidth(1);

    d->curveWidget = new CurvesWidget(256, 192, d->curveBox);
    QWhatsThis::add(d->curveWidget, i18n("<p>This is the curve adjustment of the image luminosity"));

    d->resetCurveBtn = new QToolButton(d->curveBox);
    d->resetCurveBtn->setFixedSize(11, 11);
    d->resetCurveBtn->setIconSet(SmallIconSet("reload_page", 8));
    d->resetCurveBtn->setFocusPolicy(QWidget::NoFocus);
    d->resetCurveBtn->setAutoRaise(true);
    QToolTip::add(d->resetCurveBtn, i18n("Reset curve to linear"));

    QLabel *spaceh = new QLabel(d->curveBox);
    spaceh->setFixedHeight(1);

    ColorGradientWidget *hGradient = new ColorGradientWidget(ColorGradientWidget::Horizontal, 10, d->curveBox);
    hGradient->setColors( QColor( "black" ), QColor( "white" ) );

    curveLayout->addMultiCellWidget(vGradient,        0, 0, 0, 0);
    curveLayout->addMultiCellWidget(spacev,           0, 0, 1, 1);
    curveLayout->addMultiCellWidget(d->curveWidget,   0, 0, 2, 2);
    curveLayout->addMultiCellWidget(spaceh,           1, 1, 2, 2);
    curveLayout->addMultiCellWidget(d->resetCurveBtn, 1, 2, 0, 1);
    curveLayout->addMultiCellWidget(hGradient,        2, 2, 2, 2);
    curveLayout->setRowStretch(3, 10);
    curveLayout->setSpacing(0);
    curveLayout->setMargin(spacingHint());

    // ---------------------------------------------------------------

    d->postProcessSettingsBox->addItem(d->advExposureBox, i18n("Exposure"));
    d->postProcessSettingsBox->addItem(d->curveBox,       i18n("Luminosity Curve"));
    d->postProcessSettingsBox->setItemIconSet(0, SmallIconSet("contrast"));
    d->postProcessSettingsBox->setItemIconSet(1, SmallIconSet("adjustcurves"));

    d->decodingSettingsBox->setItemIconSet(DcrawSettingsWidget::DEMOSAICING,     SmallIconSet("kdcraw"));
    d->decodingSettingsBox->setItemIconSet(DcrawSettingsWidget::WHITEBALANCE,    SmallIconSet("whitebalance"));
    d->decodingSettingsBox->setItemIconSet(DcrawSettingsWidget::CORRECTIONS,     SmallIconSet("lensdistortion"));
    d->decodingSettingsBox->setItemIconSet(DcrawSettingsWidget::COLORMANAGEMENT, SmallIconSet("colormanagement"));
    d->decodingSettingsBox->updateMinimumWidth();

    d->tabView->insertTab(d->rawdecodingBox,         i18n("Raw Decoding"),    0);
    d->tabView->insertTab(d->postProcessSettingsBox, i18n("Post Processing"), 1);
    d->tabView->insertTab(d->infoBox,                i18n("Info"),            2);

    // ---------------------------------------------------------------

    button(Default)->setText(i18n("Reset"));
    button(Default)->setIconSet(SmallIconSet("reload_page"));
    QToolTip::add(button(Default), i18n("<p>Reset all settings to default values."));

    button(Ok)->setText(i18n("Import"));
    button(Ok)->setIconSet(SmallIconSet("ok"));
    QToolTip::add(button(Ok), i18n("<p>Import image to editor using current settings."));

    button(Cancel)->setText(i18n("Use Default"));
    button(Cancel)->setIconSet(SmallIconSet("gohome"));
    QToolTip::add(button(Cancel), i18n("<p>Use general Raw decoding settings to load this image in editor."));

    // ---------------------------------------------------------------

    gridSettings->addMultiCellWidget(label1,       0, 0, 0, 0);
    gridSettings->addMultiCellWidget(d->channelCB, 0, 0, 1, 1);
    gridSettings->addMultiCellWidget(d->scaleBG,   0, 0, 4, 4);
    gridSettings->addMultiCellWidget(label10,      1, 1, 0, 0);
    gridSettings->addMultiCellWidget(d->colorsCB,  1, 1, 1, 1);
    gridSettings->addMultiCellWidget(histoBox,     2, 3, 0, 4);
    gridSettings->addMultiCellWidget(d->tabView,   4, 4, 0, 4);
    gridSettings->setRowStretch(5, 10);
    gridSettings->setColStretch(2, 10);
    gridSettings->setSpacing(spacingHint());
    gridSettings->setMargin(0);

    // ---------------------------------------------------------------

    connect(d->channelCB, SIGNAL(activated(int)),
            this, SLOT(slotChannelChanged(int)));

    connect(d->scaleBG, SIGNAL(released(int)),
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
    KConfig* config = kapp->config();
    config->setGroup("RAW Import Settings");

    d->channelCB->setCurrentItem(config->readNumEntry("Histogram Channel", RawSettingsBoxPriv::LuminosityChannel));
    d->scaleBG->setButton(config->readNumEntry("Histogram Scale", HistogramWidget::LogScaleHistogram));
    d->colorsCB->setCurrentItem(config->readNumEntry("Histogram Color", RawSettingsBoxPriv::AllColorsRed));

    d->decodingSettingsBox->setSixteenBits(config->readBoolEntry("SixteenBitsImage", false));
    d->decodingSettingsBox->setWhiteBalance((DRawDecoding::WhiteBalance)
                                            config->readNumEntry("White Balance",
                                            DRawDecoding::CAMERA));
    d->decodingSettingsBox->setCustomWhiteBalance(config->readNumEntry("Custom White Balance", 6500));
    d->decodingSettingsBox->setCustomWhiteBalanceGreen(config->readDoubleNumEntry("Custom White Balance Green", 1.0));
    d->decodingSettingsBox->setFourColor(config->readBoolEntry("Four Color RGB", false));
    d->decodingSettingsBox->setUnclipColor(config->readNumEntry("Unclip Color", 0));
    d->decodingSettingsBox->setDontStretchPixels(config->readBoolEntry("Dont Stretch Pixels", false));
    d->decodingSettingsBox->setNoiseReduction(config->readBoolEntry("Use Noise Reduction", false));
    d->decodingSettingsBox->setUseBlackPoint(config->readBoolEntry("Use Black Point", false));
    d->decodingSettingsBox->setBlackPoint(config->readNumEntry("Black Point", 0));
    d->decodingSettingsBox->setUseWhitePoint(config->readBoolEntry("Use White Point", false));
    d->decodingSettingsBox->setWhitePoint(config->readNumEntry("White Point", 0));
    d->decodingSettingsBox->setMedianFilterPasses(config->readNumEntry("Median Filter Passes", 0));
    d->decodingSettingsBox->setNRThreshold(config->readNumEntry("NR Threshold", 100));
    d->decodingSettingsBox->setUseCACorrection(config->readBoolEntry("EnableCACorrection", false));
    d->decodingSettingsBox->setcaRedMultiplier(config->readDoubleNumEntry("caRedMultiplier", 1.0));
    d->decodingSettingsBox->setcaBlueMultiplier(config->readDoubleNumEntry("caBlueMultiplier", 1.0));

    d->decodingSettingsBox->setQuality(
        (DRawDecoding::DecodingQuality)config->readNumEntry("Decoding Quality",
            (int)(DRawDecoding::BILINEAR)));

    d->decodingSettingsBox->setInputColorSpace(
        (DRawDecoding::InputColorSpace)config->readNumEntry("Input Color Space",
            (int)(DRawDecoding::NOINPUTCS)));

    d->decodingSettingsBox->setOutputColorSpace(
        (DRawDecoding::OutputColorSpace)config->readNumEntry("Output Color Space",
            (int)(DRawDecoding::SRGB)));

    d->decodingSettingsBox->setInputColorProfile(config->readPathEntry("Input Color Profile", QString()));
    d->decodingSettingsBox->setOutputColorProfile(config->readPathEntry("Output Color Profile", QString()));

    d->brightnessInput->setValue(config->readNumEntry("Brightness", 0));
    d->contrastInput->setValue(config->readNumEntry("Contrast", 0));
    d->gammaInput->setValue(config->readDoubleNumEntry("Gamma", 1.0));
    d->saturationInput->setValue(config->readDoubleNumEntry("Saturation", 1.0));
    d->fineExposureInput->setValue(config->readDoubleNumEntry("FineExposure", 0.0));

    d->curveWidget->reset();

    for (int j = 0 ; j <= 17 ; j++)
    {
        QPoint disable(-1, -1);
        QPoint p = config->readPointEntry(QString("CurveAjustmentPoint%1").arg(j), &disable);
        if (!d->decodingSettingsBox->sixteenBits() && p != disable)
        {
            // Restore point as 16 bits depth.
            p.setX(p.x()/255);
            p.setY(p.y()/255);
        }
        d->curveWidget->curves()->setCurvePoint(ImageHistogram::ValueChannel, j, p);
    }
    d->curveWidget->curves()->curvesCalculateCurve(ImageHistogram::ValueChannel);

    d->tabView->setCurrentPage(config->readNumEntry("Settings Page", 0));
    d->decodingSettingsBox->setCurrentIndex(config->readNumEntry("Decoding Settings Tab", DcrawSettingsWidget::DEMOSAICING));
    d->postProcessSettingsBox->setCurrentIndex(config->readNumEntry("Post Processing Settings Tab", 0));

    slotChannelChanged(d->channelCB->currentItem());
    slotScaleChanged(d->scaleBG->selectedId());
    slotColorsChanged(d->colorsCB->currentItem());
}

void RawSettingsBox::writeSettings()
{
    KConfig* config = kapp->config();
    config->setGroup("RAW Import Settings");

    config->writeEntry("Histogram Channel",          d->channelCB->currentItem());
    config->writeEntry("Histogram Scale",            d->scaleBG->selectedId());
    config->writeEntry("Histogram Color",            d->colorsCB->currentItem());

    config->writeEntry("SixteenBitsImage",           d->decodingSettingsBox->sixteenBits());
    config->writeEntry("White Balance",              d->decodingSettingsBox->whiteBalance());
    config->writeEntry("Custom White Balance",       d->decodingSettingsBox->customWhiteBalance());
    config->writeEntry("Custom White Balance Green", d->decodingSettingsBox->customWhiteBalanceGreen());
    config->writeEntry("Four Color RGB",             d->decodingSettingsBox->useFourColor());
    config->writeEntry("Unclip Color",               d->decodingSettingsBox->unclipColor());
    config->writeEntry("Dont Stretch Pixels",        d->decodingSettingsBox->useDontStretchPixels());
    config->writeEntry("Use Noise Reduction",        d->decodingSettingsBox->useNoiseReduction());
    config->writeEntry("Use Black Point",            d->decodingSettingsBox->useBlackPoint());
    config->writeEntry("Black Point",                d->decodingSettingsBox->blackPoint());
    config->writeEntry("Use White Point",            d->decodingSettingsBox->useWhitePoint());
    config->writeEntry("White Point",                d->decodingSettingsBox->whitePoint());
    config->writeEntry("MedianFilterPasses",         d->decodingSettingsBox->medianFilterPasses());
    config->writeEntry("NR Threshold",               d->decodingSettingsBox->NRThreshold());
    config->writeEntry("EnableCACorrection",         d->decodingSettingsBox->useCACorrection());
    config->writeEntry("caRedMultiplier",            d->decodingSettingsBox->caRedMultiplier());
    config->writeEntry("caBlueMultiplier",           d->decodingSettingsBox->caBlueMultiplier());
    config->writeEntry("Decoding Quality",           (int)d->decodingSettingsBox->quality());
    config->writeEntry("Input Color Space",          (int)d->decodingSettingsBox->inputColorSpace());
    config->writeEntry("Output Color Space",         (int)d->decodingSettingsBox->outputColorSpace());
    config->writeEntry("Input Color Profile",        d->decodingSettingsBox->inputColorProfile());
    config->writeEntry("Output Color Profile",       d->decodingSettingsBox->outputColorProfile());

    config->writeEntry("Brightness",                 d->brightnessInput->value());
    config->writeEntry("Contrast",                   d->contrastInput->value());
    config->writeEntry("Gamma",                      d->gammaInput->value());
    config->writeEntry("Saturation",                 d->saturationInput->value());
    config->writeEntry("FineExposure",               d->fineExposureInput->value());

    for (int j = 0 ; j <= 17 ; j++)
    {
        QPoint p = d->curveWidget->curves()->getCurvePoint(ImageHistogram::ValueChannel, j);
        if (!d->curveWidget->curves()->isSixteenBits())
        {
            // Store point as 16 bits depth.
            p.setX(p.x()*255);
            p.setY(p.y()*255);
        }
        config->writeEntry(QString("CurveAjustmentPoint%1").arg(j), p);
    }

    config->writeEntry("Settings Page", d->tabView->currentPage());
    config->writeEntry("Decoding Settings Tab", d->decodingSettingsBox->currentIndex());
    config->writeEntry("Post Processing Settings Tab", d->postProcessSettingsBox->currentIndex());
    config->sync();
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

    d->histogramWidget->repaint(false);
}

void RawSettingsBox::slotScaleChanged(int scale)
{
    d->histogramWidget->m_scaleType = scale;
    d->histogramWidget->repaint(false);
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

    d->histogramWidget->repaint(false);
}

} // NameSpace Digikam
