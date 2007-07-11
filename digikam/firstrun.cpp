/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-04-25
 * Description : a widget to use in first run dialog
 *
 * Copyright (C) 2006-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
 
#include <qvariant.h>
#include <qlabel.h>
#include <q3frame.h>
//Added by qt3to4:
#include <Q3VBoxLayout>
#include <Q3GridLayout>
#include <kurlrequester.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <q3whatsthis.h>

// KDE includes.

#include <kdialog.h>
#include <klocale.h>

// Local includes.

#include "firstrun.h"
#include "firstrun.moc"

namespace Digikam
{

FirstRunWidget::FirstRunWidget( QWidget* parent )
              : QWidget( parent )
{
    setName( "FirstRunWidget" );
    Q3VBoxLayout *vlayout = new Q3VBoxLayout( this, 0, 6 ); 

    m_textLabel2 = new QLabel( this );
    vlayout->addWidget( m_textLabel2 );

    Q3Frame *line1 = new Q3Frame( this );
    line1->setFrameShape( Q3Frame::HLine );
    line1->setFrameShadow( Q3Frame::Sunken );
    line1->setFrameShape( Q3Frame::HLine );
    vlayout->addWidget( line1 );

    Q3GridLayout *grid = new Q3GridLayout( 0, 1, 1, 0, 6 ); 

    m_pixLabel = new QLabel( this );
    m_pixLabel->setAlignment( int( Qt::AlignTop ) );
    grid->addMultiCellWidget( m_pixLabel, 0, 1, 0, 0 );

    m_path = new KUrlRequester( this );
    m_path->setShowLocalProtocol( true );

    grid->addWidget( m_path, 1, 1 );

    m_textLabel1 = new QLabel( this );
    m_textLabel1->setAlignment( int( Qt::WordBreak |Qt::AlignVCenter ) );
    grid->addWidget( m_textLabel1, 0, 1 );
    
    vlayout->addLayout( grid );
    vlayout->addItem( new QSpacerItem( 16, 16, QSizePolicy::Minimum, QSizePolicy::MinimumExpanding ) );
    
    languageChange();
    resize( QSize(479, 149).expandedTo(minimumSizeHint()) );
    clearWState( WState_Polished );
}

FirstRunWidget::~FirstRunWidget()
{
}

void FirstRunWidget::languageChange()
{
    m_textLabel2->setText( i18n( "<b>Albums Library Folder</b>" ) );
    m_pixLabel->setText( QString() );
    m_textLabel1->setText( i18n( "<p>digiKam will store the photo albums which you create in a "
                                 "common <b>Albums Library Folder</b>. "
                                 "Please select which folder you would like digiKam "
                                 "to use as the common Albums Library Folder below.</p>" 
                                 "<p><b>Do not use a mount path hosted by a remote computer.</b></p>") );
}

}  // namespace Digikam
