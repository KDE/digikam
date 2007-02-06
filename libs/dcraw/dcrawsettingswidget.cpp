/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date   : 2006-09-13
 * Description : dcraw settings widgets
 *
 * Copyright 2006 by Gilles Caulier
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

// Qt includes.

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qwhatsthis.h>
#include <qstring.h>

// KDE includes.

#include <kdialog.h>
#include <klocale.h>
#include <knuminput.h>

// Local includes.

#include "dcrawsettingswidget.h"
#include "dcrawsettingswidget.moc"

namespace Digikam
{

class DcrawSettingsWidgetPriv
{
public:

    DcrawSettingsWidgetPriv()
    {
        sixteenBitsImage         = 0;
        cameraWBCheckBox         = 0;
        fourColorCheckBox        = 0;
        brightnessLabel          = 0;
        brightnessSpinBox        = 0;
        autoColorBalanceCheckBox = 0;
        unclipColorLabel         = 0;
        secondarySensorCheckBox  = 0;
        RAWQualityComboBox       = 0;
        RAWQualityLabel          = 0;
        enableNoiseReduction     = 0;
        NRSigmaDomain            = 0;
        NRSigmaRange             = 0;
        NRSigmaRangelabel        = 0;
        NRSigmaDomainlabel       = 0;
        unclipColorComboBox      = 0;
        reconstructLabel         = 0;
        reconstructSpinBox       = 0;
        outputColorSpaceLabel    = 0;
        outputColorSpaceComboBox = 0;
    }

    QLabel          *brightnessLabel;
    QLabel          *RAWQualityLabel;
    QLabel          *NRSigmaRangelabel;
    QLabel          *NRSigmaDomainlabel;
    QLabel          *unclipColorLabel;
    QLabel          *reconstructLabel;
    QLabel          *outputColorSpaceLabel;

    QComboBox       *RAWQualityComboBox;
    QComboBox       *unclipColorComboBox;
    QComboBox       *outputColorSpaceComboBox;

    QCheckBox       *sixteenBitsImage;
    QCheckBox       *cameraWBCheckBox;
    QCheckBox       *fourColorCheckBox;
    QCheckBox       *autoColorBalanceCheckBox;
    QCheckBox       *secondarySensorCheckBox;
    QCheckBox       *enableNoiseReduction;

    KIntNumInput    *reconstructSpinBox;

    KDoubleNumInput *brightnessSpinBox;
    KDoubleNumInput *NRSigmaDomain;
    KDoubleNumInput *NRSigmaRange;
};

DcrawSettingsWidget::DcrawSettingsWidget(QWidget *parent, const QString& dcrawVersion)
                   : QGroupBox(0, Qt::Vertical, 
                               i18n("RAW Decoding Settings (dcraw %1)").arg(dcrawVersion), 
                               parent)
{
    d = new DcrawSettingsWidgetPriv;
    QGridLayout* settingsBoxLayout = new QGridLayout(layout(), 12, 1, KDialog::spacingHint());

    // ---------------------------------------------------------------

    d->sixteenBitsImage = new QCheckBox(i18n("16 bits color depth"), this);
    QWhatsThis::add( d->sixteenBitsImage, i18n("<p>If enabled, all RAW files will be decoded to 16-bit "
                                               "color depth using a linear gamma curve. To prevent black "
                                               "picture rendering in the editor, it is recommended to use "
                                               "Color Management in this mode.<p>"
                                               "If disabled, all RAW files will be decoded to 8-bit "
                                               "color depth with a BT.709 gamma curve and a 99th-percentile "
                                               "white point. This mode is faster than 16-bit decoding."));
    settingsBoxLayout->addMultiCellWidget(d->sixteenBitsImage, 0, 0, 0, 1);

    d->fourColorCheckBox = new QCheckBox(i18n("Interpolate RGB as four colors"), this);
    QWhatsThis::add(d->fourColorCheckBox, i18n("<p><b>Interpolate RGB as four colors</b><p>"
                                          "The default is to assume that all green "
                                          "pixels are the same. If even-row green "
                                          "pixels are more sensitive to ultraviolet light "
                                          "than odd-row this difference causes a mesh "
                                          "pattern in the output; using this option solves "
                                          "this problem with minimal loss of detail.<p>"
                                          "To resume, this option blurs the image "
                                          "a little, but it eliminates false 2x2 mesh patterns "
                                          "with VNG quality method or mazes with AHD quality method."));
    settingsBoxLayout->addMultiCellWidget(d->fourColorCheckBox, 1, 1, 0, 1);    

    // ---------------------------------------------------------------

    d->cameraWBCheckBox = new QCheckBox(i18n("Use camera white balance"), this);
    QWhatsThis::add(d->cameraWBCheckBox, i18n("<p><b>Use camera white balance</b><p>"
                                         "Use the camera's custom white-balance settings. "
                                         "The default is to use fixed daylight values, "
                                         "calculated from sample images. "
                                         "If this can't be found, reverts to the default."));
    settingsBoxLayout->addMultiCellWidget(d->cameraWBCheckBox, 2, 2, 0, 1);    

    // ---------------------------------------------------------------

    d->autoColorBalanceCheckBox = new QCheckBox(i18n("Automatic color balance"), this);
    QWhatsThis::add( d->autoColorBalanceCheckBox, i18n("<p><b>Automatic color balance</b><p>"
                                                  "The default is to use a fixed color balance "
                                                  "based on a white card photographed "
                                                  "in sunlight.<p>"));
    settingsBoxLayout->addMultiCellWidget(d->autoColorBalanceCheckBox, 3, 3, 0, 1);    

    // ---------------------------------------------------------------

    d->secondarySensorCheckBox = new QCheckBox(i18n("Use Super-CCD secondary sensors"), this);
    QWhatsThis::add( d->secondarySensorCheckBox, i18n("<p><b>Use Super CCD secondary sensors</b><p>"
                                                 "For Fuji Super CCD SR cameras, use the "
                                                 "secondary sensors, in effect underexposing "
                                                 "the image by four stops to reveal "
                                                 "detail in the highlights. For all other camera "
                                                 "types this option is ignored.<p>"));
    settingsBoxLayout->addMultiCellWidget(d->secondarySensorCheckBox, 4, 4, 0, 1);   

    // ---------------------------------------------------------------

    d->unclipColorLabel    = new QLabel(i18n("Highlights:"), this);
    d->unclipColorComboBox = new QComboBox( false, this );
    d->unclipColorComboBox->insertItem( i18n("Solid white"), 0 );
    d->unclipColorComboBox->insertItem( i18n("Unclip"),      1 );
    d->unclipColorComboBox->insertItem( i18n("Reconstruct"), 2 );
    QWhatsThis::add( d->unclipColorComboBox, i18n("<p><b>Highlights</b><p>"
                                             "Select here the highlight clipping method:<p>"
                                             "<b>Solid white</b>: Clip all highlights to solid white<p>"
                                             "<b>Unclip</b>: Leave highlights unclipped in various "
                                             "shades of pink<p>"
                                             "<b>Reconstruct</b>: Reconstruct highlights using a "
                                             "level value."));
    settingsBoxLayout->addMultiCellWidget(d->unclipColorLabel, 5, 5, 0, 0);    
    settingsBoxLayout->addMultiCellWidget(d->unclipColorComboBox, 5, 5, 1, 1);    

    d->reconstructLabel   = new QLabel(i18n("Level:"), this);
    d->reconstructSpinBox = new KIntNumInput(this);
    d->reconstructSpinBox->setRange(0, 7, 1, true);
    QWhatsThis::add(d->reconstructSpinBox, i18n("<p><b>Level</b><p>"
                                               "Specify the reconstruct highlights level of output image. "
                                               "Low values favor whites and high values favor colors."));
    settingsBoxLayout->addMultiCellWidget(d->reconstructLabel, 6, 6, 0, 0);    
    settingsBoxLayout->addMultiCellWidget(d->reconstructSpinBox, 6, 6, 1, 1);    

    // ---------------------------------------------------------------

    d->brightnessLabel   = new QLabel(i18n("Brightness:"), this);
    d->brightnessSpinBox = new KDoubleNumInput(this);
    d->brightnessSpinBox->setPrecision(2);
    d->brightnessSpinBox->setRange(0.0, 10.0, 0.01, true);
    QWhatsThis::add(d->brightnessSpinBox, i18n("<p><b>Brightness</b><p>"
                                               "Specify the brightness level of ouput image."
                                               "The default value is 1.0.<p>"));
    settingsBoxLayout->addMultiCellWidget(d->brightnessLabel, 7, 7, 0, 0);    
    settingsBoxLayout->addMultiCellWidget(d->brightnessSpinBox, 7, 7, 1, 1);    

    // ---------------------------------------------------------------

    d->RAWQualityLabel    = new QLabel(i18n("Quality:"), this);
    d->RAWQualityComboBox = new QComboBox( false, this );
    d->RAWQualityComboBox->insertItem( i18n("Bilinear interpolation"), 0 );
    d->RAWQualityComboBox->insertItem( i18n("VNG interpolation"),      1 );
    d->RAWQualityComboBox->insertItem( i18n("AHD interpolation"),      2 );
    QWhatsThis::add( d->RAWQualityComboBox, i18n("<p><b>Quality</b><p>"
                "Select here the demosaicing RAW images decoding "
                "interpolation method. A demosaicing algorithm is a digital image process used to "
                "interpolate a complete image from the partial raw data received from the color-filtered "
                "image sensor internal to many digital cameras in form of a matrix of colored pixels. "
                "Also known as CFA interpolation or color reconstruction, another common spelling "
                "is demosaicking. There are 3 methods to demosaic RAW images:<p>"
                "<b>Bilinear</b>: use high-speed but low-quality bilinear "
                "interpolation (default - for a slow computer). In this method, "
                "the red value of a non-red pixel is computed as the average of "
                "the adjacent red pixels, and similar for blue and green.<p>"
                "<b>VNG</b>: use Variable Number of Gradients interpolation. "
                "This method computes gradients near the pixel of interest and uses "
                "the lower gradients (representing smoother and more similar parts "
                "of the image) to make an estimate.<p>"
                "<b>AHD</b>: use Adaptive Homogeneity-Directed interpolation. "
                "This method selects the direction of interpolation so as to "
                "maximize a homogeneity metric, thus typically minimizing color artifacts.<p>"));
    settingsBoxLayout->addMultiCellWidget(d->RAWQualityLabel, 8, 8, 0, 0); 
    settingsBoxLayout->addMultiCellWidget(d->RAWQualityComboBox, 8, 8, 1, 1);

    // ---------------------------------------------------------------

    d->enableNoiseReduction = new QCheckBox(i18n("Enable noise reduction"), this);
    QWhatsThis::add( d->enableNoiseReduction, i18n("<p><b>Enable Noise Reduction</b><p>"
                     "Toggle bilateral filter to smooth noise while "
                     "preserving edges. This option can be use to reduce low noise. The pictures edges "
                     "are preserved because it is applied in CIELab color space instead of RGB.<p>"));
    settingsBoxLayout->addMultiCellWidget(d->enableNoiseReduction, 9, 9, 0, 1);

    d->NRSigmaDomain = new KDoubleNumInput(this);
    d->NRSigmaDomain->setValue(2.0);
    d->NRSigmaDomain->setPrecision(1);
    d->NRSigmaDomain->setRange(0.1, 5.0, 0.1);
    d->NRSigmaDomainlabel = new QLabel(i18n("Domain:"), this);
    QWhatsThis::add( d->NRSigmaDomain, i18n("<p><b>Domain</b><p>"
                                       "Set here the noise reduction Sigma Domain in units of pixels. "
                                       "The default value is 2.0.<p>"));
    settingsBoxLayout->addMultiCellWidget(d->NRSigmaDomainlabel, 10, 10, 0, 0);
    settingsBoxLayout->addMultiCellWidget(d->NRSigmaDomain, 10, 10, 1, 1);

    d->NRSigmaRange = new KDoubleNumInput(this);
    d->NRSigmaRange->setValue(4.0);
    d->NRSigmaRange->setPrecision(1);
    d->NRSigmaRange->setRange(0.1, 5.0, 0.1);
    d->NRSigmaRangelabel = new QLabel(i18n("Range:"), this);
    QWhatsThis::add( d->NRSigmaRange, i18n("<p><b>Range</b><p>"
                                           "Set here the noise reduction Sigma Range in units of "
                                           "CIELab colorspace. The default value is 4.0.<p>"));
    settingsBoxLayout->addMultiCellWidget(d->NRSigmaRangelabel, 11, 11, 0, 0);
    settingsBoxLayout->addMultiCellWidget(d->NRSigmaRange, 11, 11, 1, 1);

    // ---------------------------------------------------------------

/*
    d->outputColorSpaceLabel    = new QLabel(i18n("Color space:"), this);
    d->outputColorSpaceComboBox = new QComboBox( false, this );
    d->outputColorSpaceComboBox->insertItem( i18n("Raw (linear)"), 0 );
    d->outputColorSpaceComboBox->insertItem( i18n("sRGB"),         1 );
    d->outputColorSpaceComboBox->insertItem( i18n("Adobe RGB"),    2 );
    d->outputColorSpaceComboBox->insertItem( i18n("Wide Gamut"),   3 );
    d->outputColorSpaceComboBox->insertItem( i18n("Pro-Photo"),    4 );
    QWhatsThis::add( d->outputColorSpaceComboBox, i18n("<p><b>Color space</b><p>"
                "Select here the output color space used to decode RAW data.<p>"
                "<b>Raw (linear)</b>: in this mode, none output color space is used "
                "during RAW decoding.<p>"
                "<b>sRGB</b>: this color space is an RGB color space, created "
                "cooperatively by Hewlett-Packard and Microsoft, that is the "
                "best choice for images destined for the Web and portrait photography.<p>"
                "<b>Adobe RGB</b>: this color space is an RGB color space, developed by "
                "Adobe, that used for photography applications such as advertising "
                "and fine art.<p>"
                "<b>Wide Gamut</b>: this color space is is an expanded version of the "
                "Adobe RGB color space.<p>"
                "<b>Pro-Photo</b>: this color space is an RGB color space, developed by "
                "Kodak, that offers an especially large gamut designed for use with "
                "photographic output in mind."));

    settingsBoxLayout->addMultiCellWidget(d->outputColorSpaceLabel, 12, 12, 0, 0); 
    settingsBoxLayout->addMultiCellWidget(d->outputColorSpaceComboBox, 12, 12, 1, 1);
*/

    // ---------------------------------------------------------------

    connect(d->unclipColorComboBox, SIGNAL(activated(int)),
            this, SLOT(slotUnclipColorActivated(int)));

    connect(d->enableNoiseReduction, SIGNAL(toggled(bool)),
            this, SLOT(slotNoiseReductionToggled(bool)));
}

DcrawSettingsWidget::~DcrawSettingsWidget()
{
    delete d;
}

void DcrawSettingsWidget::setDefaultSettings()
{
    setSixteenBits(false);
    setCameraWB(true);
    setAutoColorBalance(true);
    setFourColor(false);
    setUnclipColor(0);
    setSecondarySensor(false);
    setNoiseReduction(false);
    setBrightness(1.0);
    setSigmaDomain(2.0);
    setSigmaRange(4.0);
    setQuality(RawDecodingSettings::BILINEAR); 
//    setOutputColorSpace(RawDecodingSettings::SRGB); 
}

void DcrawSettingsWidget::slotUnclipColorActivated(int v)
{
    if (v == 2)     // Reconstruct Highlight method
    {
        d->reconstructLabel->setEnabled(true);
        d->reconstructSpinBox->setEnabled(true);
    }
    else
    {
        d->reconstructLabel->setEnabled(false);
        d->reconstructSpinBox->setEnabled(false);
    }
}

void DcrawSettingsWidget::slotNoiseReductionToggled(bool b)
{
    d->NRSigmaDomain->setEnabled(b);
    d->NRSigmaRange->setEnabled(b);
    d->NRSigmaRangelabel->setEnabled(b);
    d->NRSigmaDomainlabel->setEnabled(b);
}

bool DcrawSettingsWidget::sixteenBits()
{
    return d->sixteenBitsImage->isChecked();
}

bool DcrawSettingsWidget::useCameraWB()
{
    return d->cameraWBCheckBox->isChecked();
}

bool DcrawSettingsWidget::useAutoColorBalance()
{
    return d->autoColorBalanceCheckBox->isChecked();
}

bool DcrawSettingsWidget::useFourColor()
{
    return d->fourColorCheckBox->isChecked();
}

int DcrawSettingsWidget::unclipColor()
{
    switch(d->unclipColorComboBox->currentItem())
    {
        case 0:
            return 0;
            break;
        case 1:
            return 1;
            break;
        default:         // Reconstruct Highlight method
            return d->reconstructSpinBox->value()+2;
            break;
    }
}

bool DcrawSettingsWidget::useSecondarySensor()
{
    return d->secondarySensorCheckBox->isChecked();
}

double DcrawSettingsWidget::brightness()
{
    return d->brightnessSpinBox->value();
}

void DcrawSettingsWidget::setCameraWB(bool b)
{
    d->cameraWBCheckBox->setChecked(b);
}

RawDecodingSettings::DecodingQuality DcrawSettingsWidget::quality()
{
    switch(d->RAWQualityComboBox->currentItem())
    {
        case 1:
            return RawDecodingSettings::VNG;
            break;
        case 2:
            return RawDecodingSettings::AHD;
            break;
        default:    
            return RawDecodingSettings::BILINEAR;
            break;
    }
}

/*
RawDecodingSettings::OutputColorSpace DcrawSettingsWidget::outputColorSpace()
{
    return (RawDecodingSettings::OutputColorSpace)(d->outputColorSpaceComboBox->currentItem());
}
*/

bool DcrawSettingsWidget::useNoiseReduction()
{
    return d->enableNoiseReduction->isChecked();
}

double DcrawSettingsWidget::sigmaDomain()
{
    return d->NRSigmaDomain->value();
}

double DcrawSettingsWidget::sigmaRange()
{
    return d->NRSigmaRange->value();
}

void DcrawSettingsWidget::setSixteenBits(bool b)
{
    d->sixteenBitsImage->setChecked(b);
}

void DcrawSettingsWidget::setAutoColorBalance(bool b)
{
    d->autoColorBalanceCheckBox->setChecked(b);
}

void DcrawSettingsWidget::setFourColor(bool b)
{
    d->fourColorCheckBox->setChecked(b);
}

void DcrawSettingsWidget::setUnclipColor(int v)
{
    switch(v)
    {
        case 0:
            d->unclipColorComboBox->setCurrentItem(0);
            break;
        case 1:
            d->unclipColorComboBox->setCurrentItem(1);
            break;
        default:         // Reconstruct Highlight method
            d->unclipColorComboBox->setCurrentItem(2);
            d->reconstructSpinBox->setValue(v-2);
            break;
    }

    slotUnclipColorActivated(d->unclipColorComboBox->currentItem());
}

void DcrawSettingsWidget::setSecondarySensor(bool b)
{
    d->secondarySensorCheckBox->setChecked(b);
}

void DcrawSettingsWidget::setBrightness(double b)
{
    d->brightnessSpinBox->setValue(b);
}

void DcrawSettingsWidget::setSigmaDomain(double b)
{
    d->NRSigmaDomain->setValue(b);
}

void DcrawSettingsWidget::setSigmaRange(double b)
{
    d->NRSigmaRange->setValue(b);
}

void DcrawSettingsWidget::setQuality(RawDecodingSettings::DecodingQuality q)
{
    switch(q)
    {
        case RawDecodingSettings::VNG:
            d->RAWQualityComboBox->setCurrentItem(1);
            break;
        case RawDecodingSettings::AHD:
            d->RAWQualityComboBox->setCurrentItem(2);
            break;
        default:    
            d->RAWQualityComboBox->setCurrentItem(0);
            break;
    }
}

/*
void DcrawSettingsWidget::setOutputColorSpace(RawDecodingSettings::OutputColorSpace c)
{
    d->outputColorSpaceComboBox->setCurrentItem((int)c);
}
*/

void DcrawSettingsWidget::setNoiseReduction(bool b)
{
    d->enableNoiseReduction->setChecked(b);
    slotNoiseReductionToggled(b);
}

} // NameSpace Digikam
