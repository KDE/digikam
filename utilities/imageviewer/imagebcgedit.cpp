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
#include <qpushbutton.h>
#include <qlayout.h>
#include <qgroupbox.h>
#include <qtoolbutton.h>

// KDE includes.

#include <klocale.h>

// Local includes.

#include "imagebcgedit.h"


ImageBCGEdit::ImageBCGEdit( QWidget *parent )
            : KDialogBase( Plain, QString::null, User1, User1,
                           parent, 0, true, true, i18n("Close") )
{
    setCaption( i18n("Adjust Image" ));

    QVBoxLayout *topLayout = new QVBoxLayout( plainPage(),
                                              0, spacingHint() );

    QLabel *topLabel = new QLabel( plainPage() );
    topLabel->setText( i18n( "Edit Image Setting") );
    topLayout->addWidget( topLabel  );

    // --------------------------------------------------------

    QGroupBox *groupBox = new QGroupBox( plainPage() );
    groupBox->setColumnLayout( 0, Qt::Horizontal );
    QGridLayout* gridLayout = new QGridLayout( groupBox->layout() );
    gridLayout->setSpacing( spacingHint() );

    QToolButton* gammaDecBtn = new QToolButton( groupBox );
    gammaDecBtn->setText("-");
    QLabel* gammaLabel = new QLabel(i18n("Gamma"), groupBox);
    QToolButton* gammaIncBtn = new QToolButton( groupBox );
    gammaIncBtn->setText("+");

    gridLayout->addWidget(gammaDecBtn, 0, 0);
    gridLayout->addWidget(gammaLabel,  0, 1);
    gridLayout->addWidget(gammaIncBtn, 0, 2);

    QToolButton* brightnessDecBtn = new QToolButton( groupBox );
    brightnessDecBtn->setText("-");
    QLabel* brightnessLabel = new QLabel(i18n("Brightness"), groupBox);
    QToolButton* brightnessIncBtn = new QToolButton( groupBox );
    brightnessIncBtn->setText("+");

    gridLayout->addWidget(brightnessDecBtn, 1, 0);
    gridLayout->addWidget(brightnessLabel,  1, 1);
    gridLayout->addWidget(brightnessIncBtn, 1, 2);

    QToolButton* contrastDecBtn = new QToolButton( groupBox );
    contrastDecBtn->setText("-");
    QLabel* contrastLabel = new QLabel(i18n("Contrast"), groupBox);
    QToolButton* contrastIncBtn = new QToolButton( groupBox );
    contrastIncBtn->setText("+");

    gridLayout->addWidget(contrastDecBtn, 2, 0);
    gridLayout->addWidget(contrastLabel,  2, 1);
    gridLayout->addWidget(contrastIncBtn, 2, 2);

    topLayout->addWidget( groupBox );
    
    // --------------------------------------------------------

    connect(gammaIncBtn, SIGNAL(clicked()),
            this, SIGNAL(signalGammaIncrease()));
            
    connect(gammaDecBtn, SIGNAL(clicked()),
            this, SIGNAL(signalGammaDecrease()));

    connect(brightnessIncBtn, SIGNAL(clicked()),
            this, SIGNAL(signalBrightnessIncrease()));
            
    connect(brightnessDecBtn, SIGNAL(clicked()),
            this, SIGNAL(signalBrightnessDecrease()));

    connect(contrastIncBtn, SIGNAL(clicked()),
            this, SIGNAL(signalContrastIncrease()));
            
    connect(contrastDecBtn, SIGNAL(clicked()),
            this, SIGNAL(signalContrastDecrease()));

}

ImageBCGEdit::~ImageBCGEdit()
{
    
}

void ImageBCGEdit::slotUser1()
{
    slotClose();
}

#include "imagebcgedit.moc"
