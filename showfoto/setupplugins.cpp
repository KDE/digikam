/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-04-02
 * Description : setup tab for showfoto plugins options.
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

#include "setupplugins.h"


SetupPlugins::SetupPlugins(QWidget* parent )
            : QWidget(parent)
{
   QVBoxLayout *layout = new QVBoxLayout( parent, 0, KDialog::spacingHint() );

   // --------------------------------------------------------

   m_pluginsNumber = new QLabel(parent);

   m_pluginList = new KListView( parent, "pluginList" );
   m_pluginList->addColumn( i18n( "Name" ) );
   m_pluginList->addColumn( "Library Name", 0 );   // Hidden column with the internal plugin library name.
   m_pluginList->addColumn( i18n( "Description" ) );
   m_pluginList->setResizeMode( QListView::LastColumn );
   m_pluginList->setAllColumnsShowFocus( true );
   QWhatsThis::add( m_pluginList, i18n("<p>You can set here the list of plugins "
                                       "which must be enabled/disabled for the future "
                                       "digiKam image editor instances."
                                       "<p>Note: the core image plugin cannot be disabled."));

   layout->addWidget( m_pluginsNumber );
   layout->addWidget( m_pluginList );
   
   // --------------------------------------------------------

   readSettings();
   initImagePluginsList();
   updateImagePluginsList(m_availableImagePluginList, m_enableImagePluginList);
}

SetupPlugins::~SetupPlugins()
{
}

void SetupPlugins::initImagePluginsList()
{
    KTrader::OfferList offers = KTrader::self()->query("Digikam/ImagePlugin");
    KTrader::OfferList::ConstIterator iter;

    for(iter = offers.begin(); iter != offers.end(); ++iter)
        {
        KService::Ptr service = *iter;
        m_availableImagePluginList.append(service->name());      // Plugin name translated.
        m_availableImagePluginList.append(service->library());   // Plugin system library name.
        m_availableImagePluginList.append(service->comment());   // Plugin comments translated.
        }
}

void SetupPlugins::updateImagePluginsList(QStringList lista, QStringList listl)
{
    QStringList::Iterator it = lista.begin();
    m_pluginsNumber->setText(i18n("Plugins found: %1").arg(lista.count()/3));

    while( it != lista.end() )
        {
        QString pluginName = *it;
        it++;
        QString libraryName = *it;
        it++;
        QString pluginComments = *it;
        QCheckListItem *item = new QCheckListItem (m_pluginList, pluginName, QCheckListItem::CheckBox);

        if (listl.contains(libraryName))
           item->setOn(true);

        if (libraryName == "digikamimageplugin_core")  // Always enable the digiKam core plugin.
           {
           item->setOn(true);
           item->setEnabled(false);
           }

        item->setText(0, pluginName);        // Added plugin name.
        item->setText(1, libraryName);       // Added library plugin name.
        item->setText(2, pluginComments);    // Added plugin comments.
        it++;
        }
}

QStringList SetupPlugins::getImagePluginsListEnable()
{
    QStringList imagePluginList;
    QCheckListItem *item = (QCheckListItem*)m_pluginList->firstChild();

    while( item )
        {
        if (item->isOn())
        imagePluginList.append(item->text(1));        // Get the plugin library name.
        item = (QCheckListItem*)item->nextSibling();
        }

    return imagePluginList;
}

void SetupPlugins::applySettings()
{
    KConfig* config = kapp->config();

    config->setGroup("ImageViewer Settings");
    config->writeEntry("ImagePlugins List", getImagePluginsListEnable());
    config->sync();
}

void SetupPlugins::readSettings()
{
    KConfig* config = kapp->config();
    config->setGroup("ImageViewer Settings");
    m_enableImagePluginList = config->readListEntry("ImagePlugins List");
}

#include "setupplugins.moc"
