//////////////////////////////////////////////////////////////////////////////
//
//    IMAGEBCGEDIT.CPP
//
//    Copyright (C) 2003-2004 Renchi Raju <renchi at pooh.tam.uiuc.edu>
//                            Gilles CAULIER <caulier dot gilles at free.fr>
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
//////////////////////////////////////////////////////////////////////////////

// Qt includes.

#include <qstring.h>
#include <qlabel.h>
#include <qvbox.h>
#include <qlayout.h>
#include <qgroupbox.h>
#include <qwhatsthis.h>

// KDE includes.

#include <klocale.h>
#include <knuminput.h>

// Local includes.

#include "imagebcgedit.h"


ImageBCGEdit::ImageBCGEdit( QWidget *parent )
            : KDialogBase( parent, "AdjustImage", true,
                           i18n("Adjust Image"), Help|Ok|Cancel, Ok, false)                  
{
    QWidget* box = new QWidget( this );
    setMainWidget(box);
    QVBoxLayout *dvlay = new QVBoxLayout( box, 10, spacingHint() );
    QGridLayout *grid = new QGridLayout( 2, 3 );
    dvlay->addLayout( grid );
    
    // --------------------------------------------------------
        
    m_label_gammaValue = new QLabel (i18n("Gamma:"), box);
    grid->addWidget( m_label_gammaValue, 0, 0 );
       
    m_gammaValue = new KIntNumInput(10, box);
    m_gammaValue->setRange(0, 200, 1, true );
    QWhatsThis::add( m_gammaValue, i18n("<p>Select here the Gamma value of image.") );
    m_label_gammaValue->setBuddy( m_gammaValue );
    grid->addWidget( m_gammaValue, 0, 1 );
    
    // --------------------------------------------------------
        
    m_label_brightnessValue = new QLabel (i18n("Brightness:"), box);
    grid->addWidget( m_label_brightnessValue, 1, 0 );
       
    m_brightnessValue = new KIntNumInput(0, box);
    m_brightnessValue->setRange(-100, 100, 1, true );
    QWhatsThis::add( m_brightnessValue, i18n("<p>Select here the brightness value of image.") );
    m_label_brightnessValue->setBuddy( m_brightnessValue );
    grid->addWidget( m_brightnessValue, 1, 1 );
    
    // --------------------------------------------------------
        
    m_label_contrastValue = new QLabel (i18n("Contrast:"), box);
    grid->addWidget( m_label_contrastValue, 2, 0 );
       
    m_contrastValue = new KIntNumInput(0, box);
    m_contrastValue->setRange(-100, 100, 1, true );
    QWhatsThis::add( m_contrastValue, i18n("<p>Select here the Gamma value of image.") );
    m_label_contrastValue->setBuddy( m_contrastValue );
    grid->addWidget( m_contrastValue, 2, 1 );
        
    // --------------------------------------------------------
    
    connect(m_gammaValue, SIGNAL(valueChanged (int)),
            this, SIGNAL(signalGammaValueChanged (int) ));
            
    connect(m_brightnessValue, SIGNAL(valueChanged (int)),
            this, SIGNAL(signalContrastValueChanged (int) ));
            
    connect(m_contrastValue, SIGNAL(valueChanged (int)),
            this, SIGNAL(signalContrastValueChanged (int) ));                        
}

ImageBCGEdit::~ImageBCGEdit()
{
    
}


#include "imagebcgedit.moc"
