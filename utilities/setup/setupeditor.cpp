/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-08-03
 * Description : setup Image Editor tab.
 *
 * Copyright 2004-2006 by Gilles Caulier
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
#include <qcolor.h>
#include <qhbox.h>
#include <qvgroupbox.h>
#include <qlabel.h>
#include <qwhatsthis.h>
#include <qcheckbox.h>

// KDE includes.

#include <klocale.h>
#include <kdialog.h>
#include <kcolorbutton.h>
#include <knuminput.h>
#include <kconfig.h>
#include <kapplication.h>
#include <klistview.h>
#include <ktrader.h>

// Local includes.

#include "setupeditor.h"

namespace Digikam
{

SetupEditor::SetupEditor(QWidget* parent )
           : QWidget(parent)
{
   QVBoxLayout *layout = new QVBoxLayout( parent );

   // --------------------------------------------------------

   QVGroupBox *RAWfileOptionsGroup = new QVGroupBox(i18n("RAW Image Decoding Options"), parent);

   m_RGBInterpolate4Colors = new QCheckBox(i18n("Interpolate RGB as four colors"), RAWfileOptionsGroup);
   QWhatsThis::add( m_RGBInterpolate4Colors, i18n("<p>Interpolate RGB as four colors. This blurs the image a little, "
                                                  "but it eliminates false 2x2 mesh patterns.<p>"));
   
   m_automaticColorBalance = new QCheckBox(i18n("Automatic color balance"), RAWfileOptionsGroup);
   QWhatsThis::add( m_automaticColorBalance, i18n("<p>Automatic color balance. The default is to use a fixed color "
                                                  "balance based on a white card photographed in sunlight.<p>"));
   
   m_cameraColorBalance = new QCheckBox(i18n("Camera color balance"), RAWfileOptionsGroup);
   QWhatsThis::add( m_cameraColorBalance, i18n("<p>Use the color balance specified by the camera. If this can't "
                                               "be found, reverts to the default.<p>"));
   
   m_enableRAWQuality = new QCheckBox(i18n("Enable RAW decoding quality"), RAWfileOptionsGroup);
   QWhatsThis::add( m_enableRAWQuality, i18n("<p>Toggle quality decoding option for RAW images.<p>"));

   m_RAWquality = new KIntNumInput(0, RAWfileOptionsGroup);
   m_RAWquality->setRange(0, 3, 1, true );
   m_RAWquality->setLabel( i18n("&RAW file decoding quality:"), AlignLeft|AlignVCenter );

   QWhatsThis::add( m_RAWquality, i18n("<p>The decoding quality value for RAW images:<p>"
                                       "<b>0</b>: medium quality (default - for slow computer)<p>"
                                       "<b>1</b>: good quality<p>"
                                       "<b>2</b>: high quality<p>"
                                       "<b>3</b>: very high quality (for speed computer)</b>"));

   layout->addWidget(RAWfileOptionsGroup);

   connect(m_enableRAWQuality, SIGNAL(toggled(bool)), 
           m_RAWquality, SLOT(setEnabled(bool)));

   // --------------------------------------------------------

   QVGroupBox *savingOptionsGroup = new QVGroupBox(i18n("Saving Images Options"),
                                                   parent);

   m_JPEGcompression = new KIntNumInput(75, savingOptionsGroup);
   m_JPEGcompression->setRange(1, 100, 1, true );
   m_JPEGcompression->setLabel( i18n("&JPEG quality:"), AlignLeft|AlignVCenter );

   QWhatsThis::add( m_JPEGcompression, i18n("<p>The quality value for JPEG images:<p>"
                                            "<b>1</b>: low quality (high compression and small file size)<p>"
                                            "<b>50</b>: medium quality<p>"
					    "<b>75</b>: good quality (default)<p>"
                                            "<b>100</b>: high quality (no compression and large file size)<p>"
                                            "<b>Note: JPEG is not a lossless image compression format.</b>"));
   
   m_PNGcompression = new KIntNumInput(1, savingOptionsGroup);
   m_PNGcompression->setRange(1, 9, 1, true );
   m_PNGcompression->setLabel( i18n("&PNG compression:"), AlignLeft|AlignVCenter );

   QWhatsThis::add( m_PNGcompression, i18n("<p>The compression value for PNG images:<p>"
                                           "<b>1</b>: low compression (large file size but "
                                           "short compression duration - default)<p>"
                                           "<b>5</b>: medium compression<p>"
                                           "<b>9</b>: high compression (small file size but "
                                           "long compression duration)<p>"
                                           "<b>Note: PNG is always a lossless image compression format.</b>"));

   m_TIFFcompression = new QCheckBox(i18n("Compress TIFF files"),
                                     savingOptionsGroup);
   
   QWhatsThis::add( m_TIFFcompression, i18n("<p>Toggle compression for TIFF images.<p>"
                                            "If you enable this option, you can reduce "
                                            "the final file size of the TIFF image.</p>"
                                            "<p>A lossless compression format (Adobe Deflate) "
                                            "is used to save the file.<p>"));

   layout->addWidget(savingOptionsGroup);

   // --------------------------------------------------------

   QVGroupBox *interfaceOptionsGroup = new QVGroupBox(i18n("Interface Options"),
                                                      parent);

   QHBox* colorBox = new QHBox(interfaceOptionsGroup);

   QLabel *backgroundColorlabel = new QLabel( i18n("&Background color:"),
                                             colorBox );

   m_backgroundColor = new KColorButton(colorBox);
   backgroundColorlabel->setBuddy(m_backgroundColor);
   QWhatsThis::add( m_backgroundColor, i18n("<p>Select here the background color to use "
                                            "for image editor area.") );
   backgroundColorlabel->setBuddy( m_backgroundColor );

   m_hideToolBar = new QCheckBox(i18n("H&ide toolbar in fullscreen mode"),
                                 interfaceOptionsGroup);

   layout->addWidget(interfaceOptionsGroup);

   // --------------------------------------------------------

   layout->addStretch();

   readSettings();
}

SetupEditor::~SetupEditor()
{
}

void SetupEditor::applySettings()
{
    KConfig* config = kapp->config();

    config->setGroup("ImageViewer Settings");
    config->writeEntry("BackgroundColor", m_backgroundColor->color());
    config->writeEntry("RAWQuality", m_RAWquality->value());
    config->writeEntry("EnableRAWQuality", m_enableRAWQuality->isChecked());
    config->writeEntry("AutomaticColorBalance", m_automaticColorBalance->isChecked());
    config->writeEntry("CameraColorBalance", m_cameraColorBalance->isChecked());
    config->writeEntry("RGBInterpolate4Colors", m_RGBInterpolate4Colors->isChecked());
    config->writeEntry("JPEGCompression", m_JPEGcompression->value());
    config->writeEntry("PNGCompression", m_PNGcompression->value());
    config->writeEntry("TIFFCompression", m_TIFFcompression->isChecked());
    config->writeEntry("FullScreen Hide ToolBar", m_hideToolBar->isChecked());
    config->sync();
}

void SetupEditor::readSettings()
{
    KConfig* config = kapp->config();
    QColor *Black = new QColor(Qt::black);

    config->setGroup("ImageViewer Settings");
    m_backgroundColor->setColor( config->readColorEntry("BackgroundColor", Black ) );
    m_RAWquality->setValue( config->readNumEntry("RAWQuality", 0) );
    m_enableRAWQuality->setChecked(config->readBoolEntry("EnableRAWQuality", false));
    m_cameraColorBalance->setChecked(config->readBoolEntry("AutomaticColorBalance", true));
    m_automaticColorBalance->setChecked(config->readBoolEntry("CameraColorBalance", true));
    m_RGBInterpolate4Colors->setChecked(config->readBoolEntry("RGBInterpolate4Colors", false));
    m_JPEGcompression->setValue( config->readNumEntry("JPEGCompression", 75) );
    m_PNGcompression->setValue( config->readNumEntry("PNGCompression", 9) );
    m_TIFFcompression->setChecked(config->readBoolEntry("TIFFCompression", false));
    m_hideToolBar->setChecked(config->readBoolEntry("FullScreen Hide ToolBar", false));
    
    m_RAWquality->setEnabled(m_enableRAWQuality->isChecked());
    delete Black;
}

}  // namespace Digikam

#include "setupeditor.moc"
