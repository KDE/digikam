//////////////////////////////////////////////////////////////////////////////
//
//    SETUPPLUGINS.CPP
//
//    Copyright (C) 2004 Gilles Caulier <caulier dot gilles at free.fr>
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


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

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

// Includes files for plugins support.

#ifdef HAVE_KIPI
#include <libkipi/pluginloader.h>
#endif

// Local includes.

#include "setupplugins.h"


SetupPlugins::SetupPlugins(QWidget* parent )
            : QWidget(parent)
{
   QVBoxLayout *layout = new QVBoxLayout( parent, 10);
   layout->setSpacing( KDialog::spacingHint() );
   
   m_pluginsNumber = new QLabel(parent);
   layout->addWidget( m_pluginsNumber );
   QString pluginsListHelp = i18n("<p>Here you can see the list of plugins who can be "
                                  "loaded or unloaded from the current Digikam instance.");

#ifdef HAVE_KIPI
   KIPI::ConfigWidget* Kipiconfig = KIPI::PluginLoader::instance()->configWidget( parent );
   QWhatsThis::add( Kipiconfig, pluginsListHelp);
   layout->addWidget( Kipiconfig );
#else
   m_pluginList = new KListView( parent, "pluginList" );
   m_pluginList->addColumn( i18n( "Name" ) );
   m_pluginList->addColumn( i18n( "Description" ) );
   m_pluginList->setResizeMode( QListView::LastColumn );
   m_pluginList->setAllColumnsShowFocus( true );
   QWhatsThis::add( m_pluginList, pluginsListHelp);
   layout->addWidget( m_pluginList );
#endif   
}

SetupPlugins::~SetupPlugins()
{
}

void SetupPlugins::initPlugins(int kipiPluginsNumber)
{
    m_pluginsNumber->setText(i18n("KIPI plugins found: %1").arg(kipiPluginsNumber));   
}


// Only for DigikamPlugins !!!    
void SetupPlugins::initPlugins(QStringList lista, QStringList listl)
{
    QStringList::Iterator it = lista.begin();
    
    m_pluginsNumber->setText(i18n("Digikam plugins found: %1").arg(lista.count()/2));    
    
    while(  it != lista.end() )
        {
        QCheckListItem *item = new QCheckListItem (m_pluginList, *it, QCheckListItem::CheckBox);
        item->setText(0, *it);
        
        if (listl.contains(*it)) item->setOn(true);
        
        ++it;
        item->setText(1, *it);
        ++it;
        }
}

// Only for DigikamPlugins !!!
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
