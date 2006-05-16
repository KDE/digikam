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
        labelQuality            = 0;
        labelNRSigmaDomain      = 0;
        labelNRSigmaRange       = 0;
        labelJPEGcompression    = 0;
        labelPNGcompression     = 0;

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
        iccColorsCorrection     = 0;

        JPEGcompression         = 0;
        PNGcompression          = 0;
        TIFFcompression         = 0;
    }

    QLabel          *labelQuality;
    QLabel          *labelNRSigmaDomain;
    QLabel          *labelNRSigmaRange;
    QLabel          *labelJPEGcompression;
    QLabel          *labelPNGcompression;

    QComboBox       *iccColorsCorrection;
    
    QCheckBox       *SuperCCDsecondarySensor;    
    QCheckBox       *unclipColors;
    QCheckBox       *cameraColorBalance;
    QCheckBox       *automaticColorBalance;
    QCheckBox       *enableRAWQuality;
    QCheckBox       *enableNoiseReduction;
    QCheckBox       *RGBInterpolate4Colors;
    QCheckBox       *TIFFcompression;

    KIntNumInput    *RAWquality;
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

    QGridLayout* grid1 = new QGridLayout( RAWfileOptionsGroup->layout(), 9, 1, KDialog::spacingHint());
    
    d->RGBInterpolate4Colors = new QCheckBox(i18n("Interpolate RGB as four colors"), RAWfileOptionsGroup);
    QWhatsThis::add( d->RGBInterpolate4Colors, i18n("<p>Interpolate RGB as four colors. This blurs the image "
                                                    "a little, but it eliminates false 2x2 mesh patterns.<p>"));
    grid1->addMultiCellWidget(d->RGBInterpolate4Colors, 0, 0, 0, 1);

    d->automaticColorBalance = new QCheckBox(i18n("Automatic color balance"), RAWfileOptionsGroup);
    QWhatsThis::add( d->automaticColorBalance, i18n("<p>Automatic color balance. The default is to use a "
                                                    "fixed color balance based on a white card photographed "
                                                    "in sunlight.<p>"));
    grid1->addMultiCellWidget(d->automaticColorBalance, 1, 1, 0, 1);
    
    d->cameraColorBalance = new QCheckBox(i18n("Camera color balance"), RAWfileOptionsGroup);
    QWhatsThis::add( d->cameraColorBalance, i18n("<p>Use the color balance specified by the camera. "
                                                 "If this can't be found, reverts to the default.<p>"));
    grid1->addMultiCellWidget(d->cameraColorBalance, 2, 2, 0, 1);

    d->unclipColors = new QCheckBox(i18n("Unclip colors"), RAWfileOptionsGroup);
    QWhatsThis::add( d->unclipColors, i18n("<p>Enabled this option to leave the image color "
                     "completely unclipped else all colors will be cliped to prevent pink highlights.<p>"));
    grid1->addMultiCellWidget(d->unclipColors, 3, 3, 0, 1);

    d->SuperCCDsecondarySensor = new QCheckBox(i18n("Using Super CCD secondary sensors (Fuji cameras only)"), RAWfileOptionsGroup);
    QWhatsThis::add( d->SuperCCDsecondarySensor, i18n("<p>For Fuji Super CCD SR cameras, use the "
                     "secondary sensors, in effect underexposing the image by four stops to reveal "
                     "detail in the highlights. For all other camera types this option is ignored.<p>"));
    grid1->addMultiCellWidget(d->SuperCCDsecondarySensor, 4, 4, 0, 1);
    
    d->enableRAWQuality = new QCheckBox(i18n("Enable RAW decoding quality"), RAWfileOptionsGroup);
    QWhatsThis::add( d->enableRAWQuality, i18n("<p>Toggle quality decoding option for RAW images using a setting value.<p>"
                                               "Enable this option if you use dcraw version >= 0.8.0 in your system.<p>"));
    grid1->addMultiCellWidget(d->enableRAWQuality, 5, 5, 0, 1);
    
    d->RAWquality = new KIntNumInput(0, RAWfileOptionsGroup);
    d->RAWquality->setRange(0, 3, 1);
    d->labelQuality = new QLabel( i18n("Quality:"), RAWfileOptionsGroup);
    QWhatsThis::add( d->RAWquality, i18n("<p>The decoding quality value for RAW images:<p>"
                                         "<b>0</b>: medium quality (default - for slow computer)<p>"
                                         "<b>1</b>: good quality<p>"
                                         "<b>2</b>: high quality<p>"
                                         "<b>3</b>: very high quality (for speed computer)<p>"));
    grid1->addMultiCellWidget(d->labelQuality, 6, 6, 0, 0);
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
    
    QHBoxLayout *hlay = new QHBoxLayout( KDialog::spacingHint());
    QLabel *labelICCCorrection = new QLabel(i18n("ICC profile correction during decoding:"), RAWfileOptionsGroup);
    d->iccColorsCorrection = new QComboBox( false, RAWfileOptionsGroup );
    d->iccColorsCorrection->insertItem( i18n("Disabled (recommended)") );
    d->iccColorsCorrection->insertItem( i18n("Using embeded profile") );
    d->iccColorsCorrection->insertItem( i18n("Using digiKam ICC settings") );
    QWhatsThis::add( d->iccColorsCorrection, i18n("<p>This option toogle the right way to use ICC color profiles during "
                     "RAW files decoding.<p>"
                     "If you want to process all the ICC color correction outside RAW file decoding, use <b>Disabled</b>. "
                     "This option is <u>hightly recommended</u> to use the fine settings provided by digiKam color management workflow.<p>"
                     "If you want to use the embeded ICC profile includes into RAW files (if exists), use "
                     "<b>Using embeded profile</b>. Warning: with this option the ICC color correction processed outside "
                     "the RAW file decoding will be disable! This option require dcraw version >= 0.8.0.<p>"
                     "If you want to use the ICC profiles setttings from ICC color management page, use "
                     "<b>Using digiKam ICC Settings</b>. You need to enable and set the right ICC color "
                     "management for that. This option require dcraw version >= 0.8.0.<p>"));
    hlay->addWidget(labelICCCorrection);
    hlay->addWidget(d->iccColorsCorrection);
    grid1->addMultiCellLayout(hlay, 10, 10, 0, 1);
                                               
    grid1->setColStretch(1, 10);
    layout->addWidget(RAWfileOptionsGroup);

    // --------------------------------------------------------
    
    connect(d->enableRAWQuality, SIGNAL(toggled(bool)),
            d->RAWquality, SLOT(setEnabled(bool)));

    connect(d->enableRAWQuality, SIGNAL(toggled(bool)),
            d->labelQuality, SLOT(setEnabled(bool)));

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
    config->writeEntry("RAWQuality", d->RAWquality->value());
    config->writeEntry("EnableNoiseReduction", d->enableNoiseReduction->isChecked());
    config->writeEntry("NRSigmaDomain", d->NRSigmaDomain->value());
    config->writeEntry("NRSigmaRange", d->NRSigmaRange->value());
    config->writeEntry("SuperCCDsecondarySensor", d->SuperCCDsecondarySensor->isChecked());
    config->writeEntry("UnclipColors", d->unclipColors->isChecked());
    config->writeEntry("AutomaticColorBalance", d->automaticColorBalance->isChecked());
    config->writeEntry("CameraColorBalance", d->cameraColorBalance->isChecked());
    config->writeEntry("RGBInterpolate4Colors", d->RGBInterpolate4Colors->isChecked());
    config->writeEntry("RAWICCCorrectionMode", d->iccColorsCorrection->currentItem());
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
    d->RAWquality->setValue( config->readNumEntry("RAWQuality", 0) );
    d->enableNoiseReduction->setChecked(config->readBoolEntry("EnableNoiseReduction", false));
    d->NRSigmaDomain->setValue( config->readDoubleNumEntry("NRSigmaDomain", 2.0) );
    d->NRSigmaRange->setValue( config->readDoubleNumEntry("NRSigmaRange", 4.0) );
    d->SuperCCDsecondarySensor->setChecked(config->readBoolEntry("SuperCCDsecondarySensor", false));
    d->unclipColors->setChecked(config->readBoolEntry("UnclipColors", false));
    d->cameraColorBalance->setChecked(config->readBoolEntry("AutomaticColorBalance", true));
    d->automaticColorBalance->setChecked(config->readBoolEntry("CameraColorBalance", true));
    d->RGBInterpolate4Colors->setChecked(config->readBoolEntry("RGBInterpolate4Colors", false));
    d->iccColorsCorrection->setCurrentItem(config->readNumEntry("RAWICCCorrectionMode", RawDecodingSettings::NOICC));

    d->JPEGcompression->setValue( config->readNumEntry("JPEGCompression", 75) );
    d->PNGcompression->setValue( config->readNumEntry("PNGCompression", 9) );
    d->TIFFcompression->setChecked(config->readBoolEntry("TIFFCompression", false));
    
    d->RAWquality->setEnabled(d->enableRAWQuality->isChecked());
    d->labelQuality->setEnabled(d->enableRAWQuality->isChecked());
    d->NRSigmaDomain->setEnabled(d->enableNoiseReduction->isChecked());
    d->labelNRSigmaDomain->setEnabled(d->enableNoiseReduction->isChecked());
    d->NRSigmaRange->setEnabled(d->enableNoiseReduction->isChecked());
    d->labelNRSigmaRange->setEnabled(d->enableNoiseReduction->isChecked());
}

}  // namespace Digikam

#include "setupiofiles.moc"
