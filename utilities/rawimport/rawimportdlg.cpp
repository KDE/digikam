/* ============================================================
 *
 * This file is a part of kipi-plugins project
 * http://www.kipi-plugins.org
 *
 * Date        : 2008-08-04
 * Description : Raw import dialog
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

#include <qtimer.h>
#include <qframe.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qfileinfo.h>
#include <qhbuttongroup.h> 
#include <qcombobox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qvbox.h>

// KDE includes.

#include <kcursor.h>
#include <klocale.h>
#include <kconfig.h>
#include <kapplication.h>
#include <kiconloader.h>
#include <kstandarddirs.h>
#include <kdebug.h>

// LibKDcraw includes.

#include <libkdcraw/dcrawsettingswidget.h>

// Local includes.

#include "imagehistogram.h"
#include "histogramwidget.h"
#include "colorgradientwidget.h"
#include "rawpreview.h"
#include "rawimportdlg.h"
#include "rawimportdlg.moc"

namespace Digikam
{

class RawImportDlgPriv
{
public:

    enum HistogramScale
    {
        Linear=0,
        Logarithmic
    };

    enum ColorChannel
    {
        LuminosityChannel=0,
        RedChannel,
        GreenChannel,
        BlueChannel
    };

public:

    RawImportDlgPriv()
    {
        previewWidget       = 0;
        decodingSettingsBox = 0;
        channelCB           = 0;
        effectType          = 0;
        scaleBG             = 0;
        hGradient           = 0;
        histogramWidget     = 0;
    }

    QComboBox                        *channelCB;
    QComboBox                        *effectType;

    QHButtonGroup                    *scaleBG;

    ColorGradientWidget              *hGradient;

    HistogramWidget                  *histogramWidget;

    ImageInfo                         info;

    RawPreview                       *previewWidget;

    KDcrawIface::DcrawSettingsWidget *decodingSettingsBox;
};

RawImportDlg::RawImportDlg(const ImageInfo& info, QWidget *parent)
            : KDialogBase(parent, 0, false, i18n("Raw Import"),
                          Help|Default|User1|User2|User3|Cancel, Cancel, true,
                          i18n("&Preview"), i18n("&Load"), i18n("&Abort"))
{
    d = new RawImportDlgPriv;
    d->info = info;

    setButtonTip(User1, i18n("<p>Generate a Preview from current settings. "
                             "Uses a simple bilinear interpolation for "
                             "quick results."));

    setButtonTip(User2, i18n("<p>Load image to editor using current settings."));

    setButtonTip(User3, i18n("<p>Abort the current Preview"));

    setButtonTip(Close, i18n("<p>Exit Raw Import Tool"));


    QWidget *page = new QWidget(this);
    setMainWidget(page);
    QGridLayout *mainLayout = new QGridLayout(page, 1, 1, 0, spacingHint());
    d->previewWidget        = new RawPreview(page);

    // ---------------------------------------------------------------

    QWidget *gboxSettings     = new QWidget(page);
    QGridLayout* gridSettings = new QGridLayout(gboxSettings, 4, 4, spacingHint());

    QLabel *label1 = new QLabel(i18n("Channel:"), gboxSettings);
    label1->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
    d->channelCB = new QComboBox( false, gboxSettings );
    d->channelCB->insertItem( i18n("Luminosity") );
    d->channelCB->insertItem( i18n("Red") );
    d->channelCB->insertItem( i18n("Green") );
    d->channelCB->insertItem( i18n("Blue") );
    QWhatsThis::add(d->channelCB, i18n("<p>Select the histogram channel to display here:<p>"
                                       "<b>Luminosity</b>: display the image's luminosity values.<p>"
                                       "<b>Red</b>: display the red image-channel values.<p>"
                                       "<b>Green</b>: display the green image-channel values.<p>"
                                       "<b>Blue</b>: display the blue image-channel values.<p>"));

    d->scaleBG = new QHButtonGroup(gboxSettings);
    d->scaleBG->setExclusive(true);
    d->scaleBG->setFrameShape(QFrame::NoFrame);
    d->scaleBG->setInsideMargin( 0 );
    QWhatsThis::add(d->scaleBG, i18n("<p>Select the histogram scale here.<p>"
                                     "If the image's maximal counts are small, you can use the linear scale.<p>"
                                     "Logarithmic scale can be used when the maximal counts are big; "
                                     "if it is used, all values (small and large) will be visible on the graph."));

    QPushButton *linHistoButton = new QPushButton( d->scaleBG );
    QToolTip::add( linHistoButton, i18n( "<p>Linear" ) );
    d->scaleBG->insert(linHistoButton, Digikam::HistogramWidget::LinScaleHistogram);
    KGlobal::dirs()->addResourceType("histogram-lin", KGlobal::dirs()->kde_default("data") + "digikam/data");
    QString directory = KGlobal::dirs()->findResourceDir("histogram-lin", "histogram-lin.png");
    linHistoButton->setPixmap( QPixmap( directory + "histogram-lin.png" ) );
    linHistoButton->setToggleButton(true);

    QPushButton *logHistoButton = new QPushButton( d->scaleBG );
    QToolTip::add( logHistoButton, i18n( "<p>Logarithmic" ) );
    d->scaleBG->insert(logHistoButton, Digikam::HistogramWidget::LogScaleHistogram);
    KGlobal::dirs()->addResourceType("histogram-log", KGlobal::dirs()->kde_default("data") + "digikam/data");
    directory = KGlobal::dirs()->findResourceDir("histogram-log", "histogram-log.png");
    logHistoButton->setPixmap( QPixmap( directory + "histogram-log.png" ) );
    logHistoButton->setToggleButton(true);

    QHBoxLayout* l1 = new QHBoxLayout();
    l1->addWidget(label1);
    l1->addWidget(d->channelCB);
    l1->addStretch(10);
    l1->addWidget(d->scaleBG);

    // ---------------------------------------------------------------

    QVBox *histoBox   = new QVBox(gboxSettings);
    d->histogramWidget = new Digikam::HistogramWidget(256, 140, histoBox, false, true, true);
    QWhatsThis::add(d->histogramWidget, i18n("<p>Here you can see the target preview image histogram drawing "
                                             "of the selected image channel. This one is re-computed at any "
                                             "settings changes."));
    QLabel *space = new QLabel(histoBox);
    space->setFixedHeight(1);    
    d->hGradient = new Digikam::ColorGradientWidget( Digikam::ColorGradientWidget::Horizontal, 10, histoBox );
    d->hGradient->setColors( QColor( "black" ), QColor( "white" ) );

    // ---------------------------------------------------------------

    d->decodingSettingsBox  = new KDcrawIface::DcrawSettingsWidget(gboxSettings, true, true, false);

    gridSettings->addMultiCellLayout(l1,                     0, 0, 0, 4);
    gridSettings->addMultiCellWidget(histoBox,               1, 2, 0, 4);
    gridSettings->addMultiCellWidget(d->decodingSettingsBox, 3, 3, 0, 4);
    gridSettings->setRowStretch(4, 10);

    // ---------------------------------------------------------------

    mainLayout->addMultiCellWidget(d->previewWidget, 0, 1, 0, 0);
    mainLayout->addMultiCellWidget(gboxSettings,     0, 0, 1, 1);
    mainLayout->setColStretch(0, 10);
    mainLayout->setRowStretch(1, 10);

    // ---------------------------------------------------------------

    connect(d->channelCB, SIGNAL(activated(int)),
            this, SLOT(slotChannelChanged(int)));

    connect(d->scaleBG, SIGNAL(released(int)),
            this, SLOT(slotScaleChanged(int)));

    connect(d->previewWidget, SIGNAL(signalPreviewed(const DImg&)),
            this, SLOT(slotPreviewed(const DImg&)));

    // ---------------------------------------------------------------

    busy(false);
    readSettings();
    d->previewWidget->setImageInfo(&d->info);
}

RawImportDlg::~RawImportDlg()
{
    delete d;
}

void RawImportDlg::closeEvent(QCloseEvent *e)
{
    if (!e) return;
    saveSettings();
    e->accept();
}

void RawImportDlg::slotClose()
{
    saveSettings();
    KDialogBase::slotClose();
}

void RawImportDlg::slotDefault()
{
    d->decodingSettingsBox->setDefaultSettings();
}

void RawImportDlg::readSettings()
{
    KConfig* config = kapp->config();
    config->setGroup("RAW Import Settings");

    d->decodingSettingsBox->setWhiteBalance((KDcrawIface::RawDecodingSettings::WhiteBalance)
                                            config->readNumEntry("White Balance",
                                            KDcrawIface::RawDecodingSettings::CAMERA));
    d->decodingSettingsBox->setCustomWhiteBalance(config->readNumEntry("Custom White Balance", 6500));
    d->decodingSettingsBox->setCustomWhiteBalanceGreen(config->readDoubleNumEntry("Custom White Balance Green", 1.0));
    d->decodingSettingsBox->setFourColor(config->readBoolEntry("Four Color RGB", false));
    d->decodingSettingsBox->setUnclipColor(config->readNumEntry("Unclip Color", 0));
    d->decodingSettingsBox->setDontStretchPixels(config->readBoolEntry("Dont Stretch Pixels", false));
    d->decodingSettingsBox->setNoiseReduction(config->readBoolEntry("Use Noise Reduction", false));
    d->decodingSettingsBox->setBrightness(config->readDoubleNumEntry("Brightness Multiplier", 1.0));
    d->decodingSettingsBox->setUseBlackPoint(config->readBoolEntry("Use Black Point", false));
    d->decodingSettingsBox->setBlackPoint(config->readNumEntry("Black Point", 0));
    d->decodingSettingsBox->setNRThreshold(config->readNumEntry("NR Threshold", 100));
    d->decodingSettingsBox->setUseCACorrection(config->readBoolEntry("EnableCACorrection", false));
    d->decodingSettingsBox->setcaRedMultiplier(config->readDoubleNumEntry("caRedMultiplier", 1.0));
    d->decodingSettingsBox->setcaBlueMultiplier(config->readDoubleNumEntry("caBlueMultiplier", 1.0));

    d->decodingSettingsBox->setQuality(
        (KDcrawIface::RawDecodingSettings::DecodingQuality)config->readNumEntry("Decoding Quality", 
            (int)(KDcrawIface::RawDecodingSettings::BILINEAR))); 

    d->decodingSettingsBox->setOutputColorSpace(
        (KDcrawIface::RawDecodingSettings::OutputColorSpace)config->readNumEntry("Output Color Space", 
            (int)(KDcrawIface::RawDecodingSettings::SRGB))); 

    resize(configDialogSize(*config, QString("Raw Import Dialog")));
}

void RawImportDlg::saveSettings()
{
    KConfig* config = kapp->config();
    config->setGroup("RAW Import Settings");
    config->writeEntry("White Balance", d->decodingSettingsBox->whiteBalance());
    config->writeEntry("Custom White Balance", d->decodingSettingsBox->customWhiteBalance());
    config->writeEntry("Custom White Balance Green", d->decodingSettingsBox->customWhiteBalanceGreen());
    config->writeEntry("Four Color RGB", d->decodingSettingsBox->useFourColor());
    config->writeEntry("Unclip Color", d->decodingSettingsBox->unclipColor());
    config->writeEntry("Dont Stretch Pixels", d->decodingSettingsBox->useDontStretchPixels());
    config->writeEntry("Use Noise Reduction", d->decodingSettingsBox->useNoiseReduction());
    config->writeEntry("Brightness Multiplier", d->decodingSettingsBox->brightness());
    config->writeEntry("Use Black Point", d->decodingSettingsBox->useBlackPoint());
    config->writeEntry("Black Point", d->decodingSettingsBox->blackPoint());
    config->writeEntry("NR Threshold", d->decodingSettingsBox->NRThreshold());
    config->writeEntry("EnableCACorrection", d->decodingSettingsBox->useCACorrection());
    config->writeEntry("caRedMultiplier", d->decodingSettingsBox->caRedMultiplier());
    config->writeEntry("caBlueMultiplier", d->decodingSettingsBox->caBlueMultiplier());
    config->writeEntry("Decoding Quality", (int)d->decodingSettingsBox->quality());
    config->writeEntry("Output Color Space", (int)d->decodingSettingsBox->outputColorSpace());

    saveDialogSize(*config, QString("Raw Import Dialog"));
    config->sync();
}

void RawImportDlg::slotHelp()
{
    KApplication::kApplication()->invokeHelp("rawimport", "digikam");
}

// 'Preview' dialog button.
void RawImportDlg::slotUser1()
{
    KDcrawIface::RawDecodingSettings rawDecodingSettings;
    rawDecodingSettings.whiteBalance            = d->decodingSettingsBox->whiteBalance();
    rawDecodingSettings.customWhiteBalance      = d->decodingSettingsBox->customWhiteBalance();
    rawDecodingSettings.customWhiteBalanceGreen = d->decodingSettingsBox->customWhiteBalanceGreen();
    rawDecodingSettings.RGBInterpolate4Colors   = d->decodingSettingsBox->useFourColor();
    rawDecodingSettings.unclipColors            = d->decodingSettingsBox->unclipColor();
    rawDecodingSettings.DontStretchPixels       = d->decodingSettingsBox->useDontStretchPixels();
    rawDecodingSettings.enableNoiseReduction    = d->decodingSettingsBox->useNoiseReduction();
    rawDecodingSettings.brightness              = d->decodingSettingsBox->brightness();
    rawDecodingSettings.enableBlackPoint        = d->decodingSettingsBox->useBlackPoint();
    rawDecodingSettings.blackPoint              = d->decodingSettingsBox->blackPoint();
    rawDecodingSettings.NRThreshold             = d->decodingSettingsBox->NRThreshold();
    rawDecodingSettings.enableCACorrection      = d->decodingSettingsBox->useCACorrection();
    rawDecodingSettings.caMultiplier[0]         = d->decodingSettingsBox->caRedMultiplier();
    rawDecodingSettings.caMultiplier[1]         = d->decodingSettingsBox->caBlueMultiplier();
    rawDecodingSettings.RAWQuality              = d->decodingSettingsBox->quality();
    rawDecodingSettings.outputColorSpace        = d->decodingSettingsBox->outputColorSpace();

    // We will load an half size image to speed up preview computing.
    rawDecodingSettings.halfSizeColorImage      = true;

    d->previewWidget->setDecodingSettings(rawDecodingSettings);
}

// 'Load' dialog button.
void RawImportDlg::slotUser2()
{
    KDcrawIface::RawDecodingSettings rawDecodingSettings;
    rawDecodingSettings.whiteBalance            = d->decodingSettingsBox->whiteBalance();
    rawDecodingSettings.customWhiteBalance      = d->decodingSettingsBox->customWhiteBalance();
    rawDecodingSettings.customWhiteBalanceGreen = d->decodingSettingsBox->customWhiteBalanceGreen();
    rawDecodingSettings.RGBInterpolate4Colors   = d->decodingSettingsBox->useFourColor();
    rawDecodingSettings.unclipColors            = d->decodingSettingsBox->unclipColor();
    rawDecodingSettings.DontStretchPixels       = d->decodingSettingsBox->useDontStretchPixels();
    rawDecodingSettings.enableNoiseReduction    = d->decodingSettingsBox->useNoiseReduction();
    rawDecodingSettings.brightness              = d->decodingSettingsBox->brightness();
    rawDecodingSettings.enableBlackPoint        = d->decodingSettingsBox->useBlackPoint();
    rawDecodingSettings.blackPoint              = d->decodingSettingsBox->blackPoint();
    rawDecodingSettings.NRThreshold             = d->decodingSettingsBox->NRThreshold();
    rawDecodingSettings.enableCACorrection      = d->decodingSettingsBox->useCACorrection();
    rawDecodingSettings.caMultiplier[0]         = d->decodingSettingsBox->caRedMultiplier();
    rawDecodingSettings.caMultiplier[1]         = d->decodingSettingsBox->caBlueMultiplier();
    rawDecodingSettings.RAWQuality              = d->decodingSettingsBox->quality();
    rawDecodingSettings.outputColorSpace        = d->decodingSettingsBox->outputColorSpace();

    // TODO : Load in image editor with these settings.
}

// 'Abort' dialog button.
void RawImportDlg::slotUser3()
{
}

void RawImportDlg::busy(bool val)
{
    d->decodingSettingsBox->setEnabled(!val);
    enableButton (User1, !val);
    enableButton (User2, !val);
    enableButton (User3, val);
    enableButton (Close, !val);
}

void RawImportDlg::identified(const QString&, const QString& identity, const QPixmap& preview)
{
//    d->previewWidget->setInfo(d->inputFileName + QString(" :\n") + identity, Qt::white, preview);
}

void RawImportDlg::previewing(const QString&)
{
/*
    d->previewWidget->setCursor( KCursor::waitCursor() );
*/
}

void RawImportDlg::slotPreviewed(const DImg& img)
{
    d->histogramWidget->stopHistogramComputation();
    d->histogramWidget->updateData(img.bits(), img.width(), img.height(), img.sixteenBit(), 0, 0, 0, true);
}

void RawImportDlg::previewFailed(const QString&)
{
/*    d->previewWidget->unsetCursor();
    d->previewWidget->setInfo(i18n("Failed to generate preview"), Qt::red);*/
}

void RawImportDlg::processing(const QString&)
{
/*  d->previewWidget->setCursor( KCursor::waitCursor() );
*/
}

void RawImportDlg::processed(const QString&, const QString& tmpFile)
{
}

void RawImportDlg::processingFailed(const QString&)
{
/*    d->previewWidget->unsetCursor();
    d->previewWidget->setInfo(i18n("Failed to convert Raw image"), Qt::red);*/
}

void RawImportDlg::slotChannelChanged(int channel)
{
    switch(channel)
    {
        case RawImportDlgPriv::LuminosityChannel:
            d->histogramWidget->m_channelType = Digikam::HistogramWidget::ValueHistogram;
            d->hGradient->setColors( QColor( "black" ), QColor( "white" ) );
            break;

        case RawImportDlgPriv::RedChannel:
            d->histogramWidget->m_channelType = Digikam::HistogramWidget::RedChannelHistogram;
            d->hGradient->setColors( QColor( "black" ), QColor( "red" ) );
            break;

        case RawImportDlgPriv::GreenChannel:
            d->histogramWidget->m_channelType = Digikam::HistogramWidget::GreenChannelHistogram;
            d->hGradient->setColors( QColor( "black" ), QColor( "green" ) );
            break;

        case RawImportDlgPriv::BlueChannel:
            d->histogramWidget->m_channelType = Digikam::HistogramWidget::BlueChannelHistogram;
            d->hGradient->setColors( QColor( "black" ), QColor( "blue" ) );
            break;
    }

    d->histogramWidget->repaint(false);
}

void RawImportDlg::slotScaleChanged(int scale)
{
    d->histogramWidget->m_scaleType = scale;
    d->histogramWidget->repaint(false);
}

} // NameSpace Digikam
