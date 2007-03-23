/* ============================================================
 * Authors: Caulier Gilles <caulier dot gilles at gmail dot com>
 * Date   : 2006-04-25
 * Description : a widget to use in first run dialog
 *
 * Copyright 2006-2007 by Gilles Caulier 
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
#include <qframe.h>
#include <kurlrequester.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

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
    QVBoxLayout *vlayout = new QVBoxLayout( this, 0, 6 ); 

    m_textLabel2 = new QLabel( this );
    vlayout->addWidget( m_textLabel2 );

    QFrame *line1 = new QFrame( this );
    line1->setFrameShape( QFrame::HLine );
    line1->setFrameShadow( QFrame::Sunken );
    line1->setFrameShape( QFrame::HLine );
    vlayout->addWidget( line1 );

    QGridLayout *grid = new QGridLayout( 0, 1, 1, 0, 6 ); 

    m_pixLabel = new QLabel( this );
    m_pixLabel->setAlignment( int( QLabel::AlignTop ) );
    grid->addMultiCellWidget( m_pixLabel, 0, 1, 0, 0 );

    m_path = new KURLRequester( this );
    m_path->setShowLocalProtocol( true );

    grid->addWidget( m_path, 1, 1 );

    m_textLabel1 = new QLabel( this );
    m_textLabel1->setAlignment( int( QLabel::WordBreak | QLabel::AlignVCenter ) );
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


