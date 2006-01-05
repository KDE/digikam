/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2006-01-02
 * Description : setup Image Editor plugins tab.
 * 
 * Copyright 2006 by Gilles Caulier
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

#include "setupimgplugins.h"

namespace Digikam
{

SetupImgPlugins::SetupImgPlugins(QWidget* parent )
               : QWidget(parent)
{
   QVBoxLayout *layout = new QVBoxLayout( parent );

   QVGroupBox *imagePluginsListGroup = new QVGroupBox(i18n("Image Plugins List"),
                                                      parent);

   m_pluginsNumber = new QLabel(imagePluginsListGroup);

   m_pluginList = new KListView( imagePluginsListGroup, "pluginList" );
   m_pluginList->addColumn( i18n( "Name" ) );
   m_pluginList->addColumn( "Library Name", 0 );   // Hidden column with the internal plugin library name.
   m_pluginList->addColumn( i18n( "Description" ) );
   m_pluginList->setResizeMode( QListView::LastColumn );
   m_pluginList->setAllColumnsShowFocus( true );
   QWhatsThis::add( m_pluginList, i18n("<p>You can set here the list of plugins "
                                       "which must be enabled/disabled for the future "
                                       "digiKam image editor instances."
                                       "<p>Note: the core image plugin cannot be disabled."));

   layout->addWidget( imagePluginsListGroup );

   // --------------------------------------------------------

   readSettings();
   initImagePluginsList();
   updateImagePluginsList(m_availableImagePluginList, m_enableImagePluginList);
}

SetupImgPlugins::~SetupImgPlugins()
{
}

void SetupImgPlugins::initImagePluginsList()
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

void SetupImgPlugins::updateImagePluginsList(QStringList lista, QStringList listl)
{
    QStringList::Iterator it = lista.begin();
    m_pluginsNumber->setText(i18n("Plugins found: %1")
                             .arg(lista.count()/3));

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

QStringList SetupImgPlugins::getImagePluginsListEnable()
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

void SetupImgPlugins::applySettings()
{
    KConfig* config = kapp->config();

    config->setGroup("ImageViewer Settings");
    config->writeEntry("ImagePlugins List", getImagePluginsListEnable());
    config->sync();
}

void SetupImgPlugins::readSettings()
{
    KConfig* config = kapp->config();

    config->setGroup("ImageViewer Settings");
    m_enableImagePluginList = config->readListEntry("ImagePlugins List");
}

}  // namespace Digikam

#include "setupimgplugins.moc"
