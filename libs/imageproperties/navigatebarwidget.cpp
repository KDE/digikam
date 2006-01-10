/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-07-07
 * Description : 
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

// Qt includes.
 
#include <qlayout.h>
#include <qpushbutton.h>
#include <qtooltip.h>

// KDE includes.

#include <ksqueezedtextlabel.h>
#include <kiconloader.h>
#include <kdialogbase.h>
#include <klocale.h>
#include <kapplication.h>
 
// Local includes

#include "imagepropertiessidebardb.h"
#include "navigatebarwidget.h"

namespace Digikam
{

NavigateBarWidget::NavigateBarWidget(QWidget *parent, bool show)
                 : QWidget(parent, 0, Qt::WDestructiveClose)
{
    QHBoxLayout *lay = new QHBoxLayout(this);
    KIconLoader *iconLoader = KApplication::kApplication()->iconLoader();
    
    m_firstButton = new QPushButton( this );
    m_firstButton->setPixmap( iconLoader->loadIcon( "start", (KIcon::Group)KIcon::Toolbar ) );
    QToolTip::add( m_firstButton, i18n( "Go to the first item" ) );
    
    m_prevButton = new QPushButton( this );
    m_prevButton->setPixmap( iconLoader->loadIcon( "back", (KIcon::Group)KIcon::Toolbar ) );
    QToolTip::add( m_prevButton, i18n( "Go to the previous item" ) );
 
    m_nextButton = new QPushButton( this );
    m_nextButton->setPixmap( iconLoader->loadIcon( "forward", (KIcon::Group)KIcon::Toolbar ) );
    QToolTip::add( m_nextButton, i18n( "Go to the next item" ) );

    m_lastButton = new QPushButton( this );
    m_lastButton->setPixmap( iconLoader->loadIcon( "finish", (KIcon::Group)KIcon::Toolbar ) );
    QToolTip::add( m_lastButton, i18n( "Go to the last item" ) );        
    
    m_filename = new KSqueezedTextLabel( this );
    
    lay->addWidget(m_firstButton);
    lay->addWidget(m_prevButton);
    lay->addWidget(m_nextButton);
    lay->addWidget(m_lastButton);
    lay->addSpacing( KDialog::spacingHint() );
    lay->addWidget(m_filename);

    if (!show) hide();
    
    connect(m_firstButton, SIGNAL(clicked()),
            this, SIGNAL(signalFirstItem()));
    
    connect(m_prevButton, SIGNAL(clicked()),
            this, SIGNAL(signalPrevItem()));
    
    connect(m_nextButton, SIGNAL(clicked()),
            this, SIGNAL(signalNextItem()));
    
    connect(m_lastButton, SIGNAL(clicked()),
            this, SIGNAL(signalLastItem()));
}      

NavigateBarWidget::~NavigateBarWidget()
{
}

void NavigateBarWidget::setFileName(QString filename)
{
    m_filename->setText( filename );
}

void NavigateBarWidget::setButtonsState(int itemType)
{
    if (itemType == ItemFirst)
    {
       m_firstButton->setEnabled(false);
       m_prevButton->setEnabled(false);
       m_nextButton->setEnabled(true);
       m_lastButton->setEnabled(true);
    }
    else if (itemType == ItemLast)
    {
       m_firstButton->setEnabled(true);
       m_prevButton->setEnabled(true);
       m_nextButton->setEnabled(false);
       m_lastButton->setEnabled(false);
    }
    else 
    {
       m_firstButton->setEnabled(true);
       m_prevButton->setEnabled(true);
       m_nextButton->setEnabled(true);
       m_lastButton->setEnabled(true);
    }
}

}  // namespace Digikam

#include "navigatebarwidget.moc"
