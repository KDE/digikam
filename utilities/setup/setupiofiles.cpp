/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2006-01-23
 * Description : setup image editor Input Output files.
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

// QT includes.

#include <qlayout.h>
#include <qlabel.h>
#include <qcolor.h>
#include <qhbox.h>
#include <qvgroupbox.h>
#include <qlabel.h>
#include <qwhatsthis.h>
#include <qcheckbox.h>
#include <qcombobox.h>

// KDE includes.

#include <klocale.h>
#include <kdialog.h>
#include <kcolorbutton.h>
#include <knuminput.h>
#include <kconfig.h>
#include <kapplication.h>

// Local includes.

#include "rawdecodingsettings.h"
#include "setupiofiles.h"

namespace Digikam
{

class SetupIOFilesPriv
{
public:


    SetupIOFilesPriv()
    {
        labelNRSigmaDomain      = 0;
        labelNRSigmaRange       = 0;
        labelJPEGcompression    = 0;
        labelPNGcompression     = 0;

        sixteenBitsImage        = 0;
        enableRAWQuality        = 0;
        RAWquality              = 0;
        cameraColorBalance      = 0;
        automaticColorBalance   = 0;
        SuperCCDsecondarySensor = 0;
        unclipColors            = 0;
        RGBInterpolate4Colors   = 0;
        enableNoiseReduction    = 0;
        NRSigmaDomain           = 0;
        NRSigmaRange            = 0;

        JPEGcompression         = 0;
        PNGcompression          = 0;
        TIFFcompression         = 0;
    }

    QLabel          *labelNRSigmaDomain;
    QLabel          *labelNRSigmaRange;
    QLabel          *labelJPEGcompression;
    QLabel          *labelPNGcompression;

    QComboBox       *RAWquality;

    QCheckBox       *sixteenBitsImage;
    QCheckBox       *SuperCCDsecondarySensor;
    QCheckBox       *unclipColors;
    QCheckBox       *cameraColorBalance;
    QCheckBox       *automaticColorBalance;
    QCheckBox       *enableRAWQuality;
    QCheckBox       *enableNoiseReduction;
    QCheckBox       *RGBInterpolate4Colors;
    QCheckBox       *TIFFcompression;

    KIntNumInput    *JPEGcompression;
    KIntNumInput    *PNGcompression;

    KDoubleNumInput *NRSigmaDomain;
    KDoubleNumInput *NRSigmaRange;
};

SetupIOFiles::SetupIOFiles(QWidget* parent )
            : QWidget(parent)
{
    d = new SetupIOFilesPriv;
    QVBoxLayout *layout = new QVBoxLayout( parent, 0, KDialog::spacingHint() );

    // --------------------------------------------------------

    QGroupBox *RAWfileOptionsGroup = new QGroupBox(0, Qt::Vertical, i18n("RAW Image Decoding Options"), parent);

    QGridLayout* grid1 = new QGridLayout( RAWfileOptionsGroup->layout(), 10, 1, KDialog::spacingHint());

    d->sixteenBitsImage = new QCheckBox(i18n("16 bits color depth"), RAWfileOptionsGroup);
    QWhatsThis::add( d->sixteenBitsImage, i18n("<p>If enable, all RAW files will be decoded to 16-bit "
                                               "color depth using a linear gamma curve. To prevent black "
                                               "picture rendering on editor, it's recommended to use "
                                               "Color Management in this mode.<p>"
                                               "If disable, all RAW files will be decoded to 8-bit "
                                               "color depth with a BT.709 gamma curve and a 99th-percentile "
                                               "white point. This mode is more faster than 16-bit decoding."));
    grid1->addMultiCellWidget(d->sixteenBitsImage, 0, 0, 0, 1);

    d->RGBInterpolate4Colors = new QCheckBox(i18n("Interpolate RGB as four colors"), RAWfileOptionsGroup);
    QWhatsThis::add( d->RGBInterpolate4Colors, i18n("<p>Interpolate RGB as four colors. This blurs the image "
                                                    "a little, but it eliminates false 2x2 mesh patterns.<p>"));
    grid1->addMultiCellWidget(d->RGBInterpolate4Colors, 1, 1, 0, 1);

    d->automaticColorBalance = new QCheckBox(i18n("Automatic color balance"), RAWfileOptionsGroup);
    QWhatsThis::add( d->automaticColorBalance, i18n("<p>Automatic color balance. The default is to use a "
                                                    "fixed color balance based on a white card photographed "
                                                    "in sunlight.<p>"));
    grid1->addMultiCellWidget(d->automaticColorBalance, 2, 2, 0, 1);

    d->cameraColorBalance = new QCheckBox(i18n("Camera color balance"), RAWfileOptionsGroup);
    QWhatsThis::add( d->cameraColorBalance, i18n("<p>Use the color balance specified by the camera. "
                                                 "If this can't be found, reverts to the default.<p>"));
    grid1->addMultiCellWidget(d->cameraColorBalance, 3, 3, 0, 1);

    d->unclipColors = new QCheckBox(i18n("Unclip colors"), RAWfileOptionsGroup);
    QWhatsThis::add( d->unclipColors, i18n("<p>Enabled this option to leave the image color "
                     "completely unclipped else all colors will be cliped to prevent pink highlights.<p>"));
    grid1->addMultiCellWidget(d->unclipColors, 4, 4, 0, 1);

    d->SuperCCDsecondarySensor = new QCheckBox(i18n("Using Super CCD secondary sensors (Fuji cameras only)"), RAWfileOptionsGroup);
    QWhatsThis::add( d->SuperCCDsecondarySensor, i18n("<p>For Fuji Super CCD SR cameras, use the "
                     "secondary sensors, in effect underexposing the image by four stops to reveal "
                     "detail in the highlights. For all other camera types this option is ignored.<p>"));
    grid1->addMultiCellWidget(d->SuperCCDsecondarySensor, 5, 5, 0, 1);

    d->enableRAWQuality = new QCheckBox(i18n("Enable RAW decoding quality"), RAWfileOptionsGroup);
    QWhatsThis::add( d->enableRAWQuality, i18n("<p>Toggle quality decoding option for RAW images using a setting value.<p>"
                                               "Enable this option if you use dcraw version >= 0.8.0 in your system.<p>"));
    grid1->addMultiCellWidget(d->enableRAWQuality, 6, 6, 0, 0);

    d->RAWquality = new QComboBox( false, RAWfileOptionsGroup );
    d->RAWquality->insertItem( i18n("Bilinear interpolation") );
    d->RAWquality->insertItem( i18n("VNG interpolation") );
    d->RAWquality->insertItem( i18n("AHD interpolation") );
    QWhatsThis::add( d->RAWquality, i18n("<p>Select here the demosaicing RAW images decoding interpolation "
                     "method. A demosaicing algorithm is a digital image process used to interpolate a "
                     "complete image from the partial raw data received from the color-filtered image "
                     "sensor internal to many digital cameras in form of a matrix of colored pixels. "
                     "Also known as CFA interpolation or color reconstruction, another common spelling "
                     "is demosaicing. There are 3 methods to demosaicing RAW images:<p>"
                     "<b>Bilinear</b>: use high-speed but low-quality bilinear "
                     "interpolation (default - for slow computer). In this method, "
                     "the red value of a non-red pixel is computed as the average of "
                     "the adjacent red pixels, and similar for blue and green.<p>"
                     "<b>VNG</b>: use Variable Number of Gradients interpolation. "
                     "This method computes gradients near the pixel of interest and uses "
                     "the lower gradients (representing smoother and more similar parts "
                     "of the image) to make an estimate.<p>"
                     "<b>AHD</b>: use Adaptive Homogeneity-Directed interpolation. "
                     "This method selects the direction of interpolation so as to "
                     "maximize a homogeneity metric, thus typically minimizing color "
                     "artifacts.<p>"));
    grid1->addMultiCellWidget(d->RAWquality, 6, 6, 1, 1);

    d->enableNoiseReduction = new QCheckBox(i18n("Enable noise reduction during decoding (warning: slow)"),
                                            RAWfileOptionsGroup);
    QWhatsThis::add( d->enableNoiseReduction, i18n("<p>Toggle bilateral filter to smooth noise while "
                     "preserving edges. This option can be use to reduce low noise. The pictures edges "
                     "are preserved because it is applied in CIELab color space instead of RGB.<p>"
                     "<b>Warnings</b>:<p>"
                     "This filter isn't adapted if your RAW picture have been taken using hight "
                     "sensitivity (over 1600 ISO). Its recommended to use the noise reduction filter "
                     "from image editor instead.<p>"
                     "This filter can take a while. Do not use if your computer is slow.<p>"
                     "This option require dcraw version >= 0.8.0.<p>"));
    grid1->addMultiCellWidget(d->enableNoiseReduction, 7, 7, 0, 1);

    d->NRSigmaDomain = new KDoubleNumInput(RAWfileOptionsGroup);
    d->NRSigmaDomain->setValue(2.0);
    d->NRSigmaDomain->setPrecision(1);
    d->NRSigmaDomain->setRange(0.1, 5.0, 0.1);
    d->labelNRSigmaDomain = new QLabel(i18n("Sigma domain:"), RAWfileOptionsGroup);
    QWhatsThis::add( d->NRSigmaDomain, i18n("<p>The noise reduction Sigma Domain in units of pixels. "
                                            "The default value is 2.0.<p>"));
    grid1->addMultiCellWidget(d->labelNRSigmaDomain, 8, 8, 0, 0);
    grid1->addMultiCellWidget(d->NRSigmaDomain, 8, 8, 1, 1);

    d->NRSigmaRange = new KDoubleNumInput(RAWfileOptionsGroup);
    d->NRSigmaRange->setValue(4.0);
    d->NRSigmaRange->setPrecision(1);
    d->NRSigmaRange->setRange(0.1, 5.0, 0.1);
    d->labelNRSigmaRange = new QLabel(i18n("Sigma range:"), RAWfileOptionsGroup);
    QWhatsThis::add( d->NRSigmaRange, i18n("<p>The noise reduction Sigma Range in units of "
                                           "CIELab colorspace. The default value is 4.0.<p>"));
    grid1->addMultiCellWidget(d->labelNRSigmaRange, 9, 9, 0, 0);
    grid1->addMultiCellWidget(d->NRSigmaRange, 9, 9, 1, 1);

    grid1->setColStretch(1, 10);
    layout->addWidget(RAWfileOptionsGroup);

    // --------------------------------------------------------

    connect(d->enableRAWQuality, SIGNAL(toggled(bool)),
            d->RAWquality, SLOT(setEnabled(bool)));

    connect(d->enableNoiseReduction, SIGNAL(toggled(bool)),
            d->NRSigmaDomain, SLOT(setEnabled(bool)));

    connect(d->enableNoiseReduction, SIGNAL(toggled(bool)),
            d->labelNRSigmaDomain, SLOT(setEnabled(bool)));

    connect(d->enableNoiseReduction, SIGNAL(toggled(bool)),
            d->NRSigmaRange, SLOT(setEnabled(bool)));

    connect(d->enableNoiseReduction, SIGNAL(toggled(bool)),
            d->labelNRSigmaRange, SLOT(setEnabled(bool)));

    // --------------------------------------------------------

    QGroupBox *savingOptionsGroup = new QGroupBox(0, Qt::Vertical, i18n("Saving Images Options"), parent);

    QGridLayout* grid2 = new QGridLayout( savingOptionsGroup->layout(), 2, 1, KDialog::spacingHint());

    d->JPEGcompression = new KIntNumInput(75, savingOptionsGroup);
    d->JPEGcompression->setRange(1, 100, 1, true );
    d->labelJPEGcompression = new QLabel(i18n("JPEG quality:"), savingOptionsGroup);

    QWhatsThis::add( d->JPEGcompression, i18n("<p>The quality value for JPEG images:<p>"
                                                "<b>1</b>: low quality (high compression and small "
                                                "file size)<p>"
                                                "<b>50</b>: medium quality<p>"
                                                "<b>75</b>: good quality (default)<p>"
                                                "<b>100</b>: high quality (no compression and "
                                                "large file size)<p>"
                                                "<b>Note: JPEG is not a lossless image "
                                                "compression format.</b>"));
    grid2->addMultiCellWidget(d->labelJPEGcompression, 0, 0, 0, 0);
    grid2->addMultiCellWidget(d->JPEGcompression, 0, 0, 1, 1);

    d->PNGcompression = new KIntNumInput(1, savingOptionsGroup);
    d->PNGcompression->setRange(1, 9, 1, true );
    d->labelPNGcompression = new QLabel(i18n("PNG compression:"), savingOptionsGroup);

    QWhatsThis::add( d->PNGcompression, i18n("<p>The compression value for PNG images:<p>"
                                             "<b>1</b>: low compression (large file size but "
                                             "short compression duration - default)<p>"
                                             "<b>5</b>: medium compression<p>"
                                             "<b>9</b>: high compression (small file size but "
                                             "long compression duration)<p>"
                                             "<b>Note: PNG is always a lossless image "
                                             "compression format.</b>"));
    grid2->addMultiCellWidget(d->labelPNGcompression, 1, 1, 0, 0);
    grid2->addMultiCellWidget(d->PNGcompression, 1, 1, 1, 1);

    d->TIFFcompression = new QCheckBox(i18n("Compress TIFF files"),
                                       savingOptionsGroup);

    QWhatsThis::add( d->TIFFcompression, i18n("<p>Toggle compression for TIFF images.<p>"
                                              "If you enable this option, you can reduce "
                                              "the final file size of the TIFF image.</p>"
                                              "<p>A lossless compression format (Deflate) "
                                              "is used to save the file.<p>"));
    grid2->addMultiCellWidget(d->TIFFcompression, 2, 2, 0, 1);
    grid2->setColStretch(1, 10);

    layout->addWidget(savingOptionsGroup);
    layout->addStretch();

    readSettings();
}

SetupIOFiles::~SetupIOFiles()
{
    delete d;
}

void SetupIOFiles::applySettings()
{
    KConfig* config = kapp->config();

    config->setGroup("ImageViewer Settings");
    config->writeEntry("EnableRAWQuality", d->enableRAWQuality->isChecked());

    // Translation combobox item to dcraw quality value. See dcraw man page for details.
    config->writeEntry("RAWQuality", (d->RAWquality->currentItem() > 0 ? 
                                      d->RAWquality->currentItem()+1 : d->RAWquality->currentItem()));

    config->writeEntry("SixteenBitsImage", d->sixteenBitsImage->isChecked());
    config->writeEntry("EnableNoiseReduction", d->enableNoiseReduction->isChecked());
    config->writeEntry("NRSigmaDomain", d->NRSigmaDomain->value());
    config->writeEntry("NRSigmaRange", d->NRSigmaRange->value());
    config->writeEntry("SuperCCDsecondarySensor", d->SuperCCDsecondarySensor->isChecked());
    config->writeEntry("UnclipColors", d->unclipColors->isChecked());
    config->writeEntry("AutomaticColorBalance", d->automaticColorBalance->isChecked());
    config->writeEntry("CameraColorBalance", d->cameraColorBalance->isChecked());
    config->writeEntry("RGBInterpolate4Colors", d->RGBInterpolate4Colors->isChecked());
    config->writeEntry("JPEGCompression", d->JPEGcompression->value());
    config->writeEntry("PNGCompression", d->PNGcompression->value());
    config->writeEntry("TIFFCompression", d->TIFFcompression->isChecked());
    config->sync();
}

void SetupIOFiles::readSettings()
{
    KConfig* config = kapp->config();
    config->setGroup("ImageViewer Settings");

    d->enableRAWQuality->setChecked(config->readBoolEntry("EnableRAWQuality", true));

    // Translation dcraw quality value to combobox item. See dcraw man page for details.
    int q = config->readNumEntry("RAWQuality", 0);
    d->RAWquality->setCurrentItem( (q > 1) ? q-1 : q );

    d->sixteenBitsImage->setChecked(config->readBoolEntry("SixteenBitsImage", false));
    d->enableNoiseReduction->setChecked(config->readBoolEntry("EnableNoiseReduction", false));
    d->NRSigmaDomain->setValue( config->readDoubleNumEntry("NRSigmaDomain", 2.0) );
    d->NRSigmaRange->setValue( config->readDoubleNumEntry("NRSigmaRange", 4.0) );
    d->SuperCCDsecondarySensor->setChecked(config->readBoolEntry("SuperCCDsecondarySensor", false));
    d->unclipColors->setChecked(config->readBoolEntry("UnclipColors", false));
    d->cameraColorBalance->setChecked(config->readBoolEntry("CameraColorBalance", true));
    d->automaticColorBalance->setChecked(config->readBoolEntry("AutomaticColorBalance", true));
    d->RGBInterpolate4Colors->setChecked(config->readBoolEntry("RGBInterpolate4Colors", false));

    d->JPEGcompression->setValue( config->readNumEntry("JPEGCompression", 75) );
    d->PNGcompression->setValue( config->readNumEntry("PNGCompression", 9) );
    d->TIFFcompression->setChecked(config->readBoolEntry("TIFFCompression", false));

    d->RAWquality->setEnabled(d->enableRAWQuality->isChecked());
    d->NRSigmaDomain->setEnabled(d->enableNoiseReduction->isChecked());
    d->labelNRSigmaDomain->setEnabled(d->enableNoiseReduction->isChecked());
    d->NRSigmaRange->setEnabled(d->enableNoiseReduction->isChecked());
    d->labelNRSigmaRange->setEnabled(d->enableNoiseReduction->isChecked());
}

}  // namespace Digikam

#include "setupiofiles.moc"
