/* ============================================================
 * File  : setupeditor.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-08-03
 * Description : setup tab for ImageEditor.
 * 
 * Copyright 2004 by Gilles Caulier
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
#include <qhgroupbox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qwhatsthis.h>

// KDE includes.

#include <klocale.h>
#include <kdialog.h>
#include <kcolorbutton.h>
#include <knuminput.h>
#include <kconfig.h>
#include <kapplication.h>

// Local includes.

#include "setupeditor.h"


SetupEditor::SetupEditor(QWidget* parent )
           : QWidget(parent)
{
   QVBoxLayout *layout = new QVBoxLayout( parent, 10);
   layout->setSpacing( KDialog::spacingHint() );

   // --------------------------------------------------------

   QGroupBox *savingOptionsGroup = new QGroupBox(1,
                                                 Qt::Horizontal, 
                                                 i18n("Saving images options"),
                                                 parent);

   m_JPEGcompression = new KIntNumInput(75, savingOptionsGroup);
   m_JPEGcompression->setRange(1, 100, 1, true );
   m_JPEGcompression->setLabel( i18n("JPEG compression:") );

   QWhatsThis::add( m_JPEGcompression, i18n("<p>The compression value of the JPEG images:<p>"
                                            "<b>1</b>: very high compression<p>"
                                            "<b>25</b>: high compression<p>"
                                            "<b>50</b>: medium compression<p>"
                                            "<b>75</b>: low compression (default value)<p>"
                                            "<b>100</b>: no compression"));

   layout->addWidget(savingOptionsGroup);
   
   // --------------------------------------------------------
   
   QGroupBox *interfaceOptionsGroup = new QGroupBox(2,
                                               Qt::Horizontal, 
                                               i18n("Interface options"),
                                               parent);
   
   QLabel *backgroundColorlabel = new QLabel( i18n("Background color:"), interfaceOptionsGroup);
   m_backgroundColor = new KColorButton(interfaceOptionsGroup);
   QWhatsThis::add( m_backgroundColor, i18n("<p>Select here the background color to use "
                                            "for image editor area.") );
   backgroundColorlabel->setBuddy( m_backgroundColor );
   
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
    config->writeEntry("JPEGCompression", m_JPEGcompression->value());
    config->sync();
}

void SetupEditor::readSettings()
{
    KConfig* config = kapp->config();
    QColor *Black = new QColor(Qt::black);

    config->setGroup("ImageViewer Settings");  
    m_backgroundColor->setColor( config->readColorEntry("BackgroundColor", Black ) );
    m_JPEGcompression->setValue( config->readNumEntry("JPEGCompression", 75) );
    delete Black;
}


#include "setupeditor.moc"
