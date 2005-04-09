/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-04-02
 * Description : setup tab for showfoto image editor options.
 * 
 * Copyright 2005 by Gilles Caulier
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


SetupEditor::SetupEditor(QWidget* parent )
           : QWidget(parent)
{
   QVBoxLayout *layout = new QVBoxLayout( parent );

   // --------------------------------------------------------

   QVGroupBox *savingOptionsGroup = new QVGroupBox(i18n("Saving Images Options"), parent);

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

   QVGroupBox *interfaceOptionsGroup = new QVGroupBox(i18n("Interface Options"), parent);

   QHBox* colorBox = new QHBox(interfaceOptionsGroup);

   QLabel *backgroundColorlabel = new QLabel( i18n("&Background color:"), colorBox );

   m_backgroundColor = new KColorButton(colorBox);
   backgroundColorlabel->setBuddy(m_backgroundColor);
   QWhatsThis::add( m_backgroundColor, i18n("<p>Select here the background color to use "
                                            "for image editor area.") );
   backgroundColorlabel->setBuddy( m_backgroundColor );

   m_hideToolBar = new QCheckBox(i18n("H&ide toolbar in fullscreen mode"), interfaceOptionsGroup);
   m_hideThumbBar = new QCheckBox(i18n("H&ide thumbbar in fullscreen mode"), interfaceOptionsGroup);
   m_useTrashCheck = new QCheckBox(i18n("&Deleting items should move them to trash"), interfaceOptionsGroup);
   m_showSplashCheck = new QCheckBox(i18n("&Show splash screen at startup"), interfaceOptionsGroup);
   
   layout->addWidget(interfaceOptionsGroup);
      
   // --------------------------------------------------------

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
    config->writeEntry("JPEGCompression", m_JPEGcompression->value());
    config->writeEntry("PNGCompression", m_PNGcompression->value());
    config->writeEntry("TIFFCompression", m_TIFFcompression->isChecked());
    config->writeEntry("FullScreenHideToolBar", m_hideToolBar->isChecked());
    config->writeEntry("FullScreenHideThumbBar", m_hideThumbBar->isChecked());
    config->writeEntry("DeleteItem2Trash", m_useTrashCheck->isChecked());
    config->writeEntry("ShowSplash", m_showSplashCheck->isChecked());
    config->sync();
}

void SetupEditor::readSettings()
{
    KConfig* config = kapp->config();
    QColor *Black = new QColor(Qt::black);

    config->setGroup("ImageViewer Settings");
    m_backgroundColor->setColor( config->readColorEntry("BackgroundColor", Black ) );
    m_JPEGcompression->setValue( config->readNumEntry("JPEGCompression", 75) );
    m_PNGcompression->setValue( config->readNumEntry("PNGCompression", 9) );
    m_TIFFcompression->setChecked(config->readBoolEntry("TIFFCompression", false));
    m_hideToolBar->setChecked(config->readBoolEntry("FullScreenHideToolBar", false));
    m_hideThumbBar->setChecked(config->readBoolEntry("FullScreenHideThumblBar", true));
    m_useTrashCheck->setChecked(config->readBoolEntry("DeleteItem2Trash", false));
    m_showSplashCheck->setChecked(config->readBoolEntry("ShowSplash", true));

    delete Black;
}

#include "setupeditor.moc"
