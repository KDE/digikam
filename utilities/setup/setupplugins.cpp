//////////////////////////////////////////////////////////////////////////////
//
//    SETUPPLUGINS.CPP
//
//    Copyright (C) 2004 Gilles CAULIER <caulier dot gilles at free.fr>
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

// QT includes.

#include <qlayout.h>
#include <qstring.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qwhatsthis.h>

// KDE includes.

#include <klocale.h>
#include <kdialog.h>
#include <klistview.h>

// Local includes.

#include "setupplugins.h"


SetupPlugins::SetupPlugins(QWidget* parent )
            : QWidget(parent)
{
   QVBoxLayout *layout = new QVBoxLayout( parent, 10);
   layout->setSpacing( KDialog::spacingHint() );

   m_pluginList = new KListView( parent, "pluginList" );
   m_pluginList->setAllColumnsShowFocus( true );
   m_pluginList->setResizeMode( KListView::LastColumn );
   m_pluginList->setFullWidth( true );
   m_pluginList->addColumn( i18n( "Name" ) );
   m_pluginList->addColumn( i18n( "Description" ) );
   QWhatsThis::add( m_pluginList, i18n("<p>You can set here the list of plugins "
                                       "who must be loaded/unloaded from/to the current "
                                       "Digikam instance. At the next instance this list "
                                       "will be used during the initialisation."));
   layout->addWidget( m_pluginList );
}

SetupPlugins::~SetupPlugins()
{
}

void SetupPlugins::initPlugins(QStringList lista, QStringList listl)
{
    QStringList::Iterator it = lista.begin();
    
    while(  it != lista.end() )
        {
        QCheckListItem *item = new QCheckListItem (m_pluginList, *it, QCheckListItem::CheckBox);
        item->setText(0, *it);
        
        if (listl.contains(*it)) item->setOn(true);
        
        it++;
        item->setText(1, *it);
        it++;
        }
}

QStringList SetupPlugins::getPluginList()
{
    QStringList list;
    QCheckListItem * item = (QCheckListItem*)m_pluginList->firstChild();
        
    while( item )
            {
            if (item->isOn())
               list.append(item->text(0));
               
            item = (QCheckListItem*)item->nextSibling();
            }
            
    return list;
}


#include "setupplugins.moc"
