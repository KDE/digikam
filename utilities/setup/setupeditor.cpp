/* ============================================================
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

#include "setupeditor.h"


SetupEditor::SetupEditor(QWidget* parent )
           : QWidget(parent)
{
   QVBoxLayout *layout = new QVBoxLayout( parent );

   // --------------------------------------------------------

   QVGroupBox *savingOptionsGroup = new QVGroupBox(i18n("Saving Images Options"),
                                                   parent);

   m_JPEGcompression = new KIntNumInput(75, savingOptionsGroup);
   m_JPEGcompression->setRange(1, 100, 1, true );
   m_JPEGcompression->setLabel( i18n("&JPEG quality:"), AlignLeft|AlignVCenter );

   QWhatsThis::add( m_JPEGcompression, i18n("<p>The quality value for JPEG images:<p>"
                                            "<b>1</b>: low quality (high compression and small file size)<p>"
                                            "<b>50</b>: medium quality<p>"
					    "<b>75</b>: good quality (default)<p>"
                                            "<b>100</b>: high quality (no compression and large file size)<p>"
                                            "<b>Note: JPEG is not a lossless image compression format.</b>"));
   
   m_PNGcompression = new KIntNumInput(1, savingOptionsGroup);
   m_PNGcompression->setRange(1, 9, 1, true );
   m_PNGcompression->setLabel( i18n("&PNG compression:"), AlignLeft|AlignVCenter );

   QWhatsThis::add( m_PNGcompression, i18n("<p>The compression value for PNG images:<p>"
                                           "<b>1</b>: low compression (large file size but "
                                           "short compression duration - default)<p>"
                                           "<b>5</b>: medium compression<p>"
                                           "<b>9</b>: high compression (small file size but "
                                           "long compression duration)<p>"
                                           "<b>Note: PNG is always a lossless image compression format.</b>"));

   m_TIFFcompression = new QCheckBox(i18n("Compress TIFF files"),
                                     savingOptionsGroup);
   
   QWhatsThis::add( m_TIFFcompression, i18n("<p>Toggle compression for TIFF images.<p>"
                                            "If you enable this option, you can reduce "
                                            "the final file size of the TIFF image.</p>"
                                            "<p>A lossless compression format (Adobe Deflate) "
                                            "is used to save the file.<p>"));

   layout->addWidget(savingOptionsGroup);

   // --------------------------------------------------------

   QVGroupBox *interfaceOptionsGroup = new QVGroupBox(i18n("Interface Options"),
                                                      parent);

   QHBox* colorBox = new QHBox(interfaceOptionsGroup);

   QLabel *backgroundColorlabel = new QLabel( i18n("&Background color:"),
                                             colorBox );

   m_backgroundColor = new KColorButton(colorBox);
   backgroundColorlabel->setBuddy(m_backgroundColor);
   QWhatsThis::add( m_backgroundColor, i18n("<p>Select here the background color to use "
                                            "for image editor area.") );
   backgroundColorlabel->setBuddy( m_backgroundColor );

   m_hideToolBar = new QCheckBox(i18n("H&ide toolbar in fullscreen mode"),
                                 interfaceOptionsGroup);

   layout->addWidget(interfaceOptionsGroup);

   // --------------------------------------------------------

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
    config->writeEntry("PNGCompression", m_PNGcompression->value());
    config->writeEntry("TIFFCompression", m_TIFFcompression->isChecked());
    config->writeEntry("ImagePlugins List", getImagePluginsListEnable());
    config->writeEntry("FullScreen Hide ToolBar", m_hideToolBar->isChecked());
    config->sync();
}

void SetupEditor::readSettings()
{
    KConfig* config = kapp->config();
    QColor *Black = new QColor(Qt::black);

    config->setGroup("ImageViewer Settings");
    m_backgroundColor->setColor( config->readColorEntry("BackgroundColor", Black ) );
    m_JPEGcompression->setValue( config->readNumEntry("JPEGCompression", 75) );
    m_PNGcompression->setValue( config->readNumEntry("PNGCompression", 9) );
    m_TIFFcompression->setChecked(config->readBoolEntry("TIFFCompression", false));
    m_enableImagePluginList = config->readListEntry("ImagePlugins List");
    m_hideToolBar->setChecked(config->readBoolEntry("FullScreen Hide ToolBar", false));

    delete Black;
}


#include "setupeditor.moc"
