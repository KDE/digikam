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
#include <klistview.h>
#include <ktrader.h>

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
   
   QGroupBox *imagePluginsListGroup = new QGroupBox(1,
                                                    Qt::Horizontal, 
                                                    i18n("Image plugins list"),
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
                                       "Digikam image editor instances."
                                       "<p>Nota: the core image plugin cannot be disabled."));
   
   layout->addWidget( imagePluginsListGroup );
   
   // --------------------------------------------------------

   readSettings();
   initImagePluginsList();
   updateImagePluginsList(m_availableImagePluginList, m_enableImagePluginList);
}

SetupEditor::~SetupEditor()
{
}

void SetupEditor::initImagePluginsList()
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

void SetupEditor::updateImagePluginsList(QStringList lista, QStringList listl)
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
        
        if (libraryName == "digikamimageplugin_core")  // Always enable the Digikam core plugin.
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

QStringList SetupEditor::getImagePluginsListEnable()
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

void SetupEditor::applySettings()
{
    KConfig* config = kapp->config();

    config->setGroup("ImageViewer Settings");
    config->writeEntry("BackgroundColor", m_backgroundColor->color());
    config->writeEntry("JPEGCompression", m_JPEGcompression->value());
    config->writeEntry("ImagePlugins List", getImagePluginsListEnable());            
    config->sync();
}

void SetupEditor::readSettings()
{
    KConfig* config = kapp->config();
    QColor *Black = new QColor(Qt::black);

    config->setGroup("ImageViewer Settings");  
    m_backgroundColor->setColor( config->readColorEntry("BackgroundColor", Black ) );
    m_JPEGcompression->setValue( config->readNumEntry("JPEGCompression", 75) );
    m_enableImagePluginList = config->readListEntry("ImagePlugins List");
    
    delete Black;
}


#include "setupeditor.moc"
