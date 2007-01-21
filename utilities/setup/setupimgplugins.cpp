/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date   : 2006-01-02
 * Description : setup Image Editor plugins tab.
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
#include "setupimgplugins.moc"

namespace Digikam
{

class SetupImgPluginsPriv
{
public:

    SetupImgPluginsPriv()
    {
        pluginsNumber = 0;
        pluginList    = 0;
    }

    QStringList  availableImagePluginList;
    QStringList  enableImagePluginList;
    
    QLabel      *pluginsNumber;
    
    KListView   *pluginList;    
};

SetupImgPlugins::SetupImgPlugins(QWidget* parent )
               : QWidget(parent)
{
    d = new SetupImgPluginsPriv;

    QVBoxLayout *mainLayout = new QVBoxLayout(parent);
    QVBoxLayout *layout     = new QVBoxLayout(this, 0, KDialog::spacingHint());

    QHBox *hBox                  = new QHBox(this);
    d->pluginsNumber             = new QLabel(hBox);
    QLabel *space                = new QLabel(hBox);
    QCheckBox *toggleAllCheckbox = new QCheckBox(i18n("Toggle All"), hBox);
    hBox->setStretchFactor(space, 10);
    
    d->pluginList = new KListView(this, "pluginList");
    d->pluginList->addColumn(i18n("Name"));
    d->pluginList->addColumn("Library Name", 0);   // No i18n here. Hidden column with the 
                                                   // internal plugin library name.
    d->pluginList->addColumn(i18n("Description"));
    d->pluginList->setResizeMode( QListView::LastColumn );
    d->pluginList->setAllColumnsShowFocus( true );
    QWhatsThis::add(d->pluginList, i18n("<p>You can set here the list of plugins "
                                        "which must be enabled/disabled for the future "
                                        "digiKam image editor sessions."
                                        "<p>Note: the core image plugin cannot be disabled."));
    
    layout->addWidget(hBox);
    layout->addWidget(d->pluginList);

    mainLayout->addWidget(this);
    
    // --------------------------------------------------------
    
    readSettings();
    initImagePluginsList();
    updateImagePluginsList(d->availableImagePluginList, d->enableImagePluginList);

    // --------------------------------------------------------

    connect(toggleAllCheckbox, SIGNAL(toggled(bool)),
            this, SLOT(toggleAll(bool)));
}

SetupImgPlugins::~SetupImgPlugins()
{
    delete d;
}

void SetupImgPlugins::initImagePluginsList()
{
    KTrader::OfferList offers = KTrader::self()->query("Digikam/ImagePlugin");
    KTrader::OfferList::ConstIterator iter;

    for(iter = offers.begin(); iter != offers.end(); ++iter)
    {
        KService::Ptr service = *iter;
        d->availableImagePluginList.append(service->name());      // Plugin name translated.
        d->availableImagePluginList.append(service->library());   // Plugin system library name.
        d->availableImagePluginList.append(service->comment());   // Plugin comments translated.
    }
}

void SetupImgPlugins::updateImagePluginsList(QStringList lista, QStringList listl)
{
    QStringList::Iterator it = lista.begin();
    d->pluginsNumber->setText(i18n("Plugins found: %1").arg(lista.count()/3));

    while( it != lista.end() )
    {
        QString pluginName = *it;
        it++;
        QString libraryName = *it;
        it++;
        QString pluginComments = *it;
        QCheckListItem *item = new QCheckListItem (d->pluginList, pluginName, QCheckListItem::CheckBox);

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
    QCheckListItem *item = (QCheckListItem*)d->pluginList->firstChild();

    while( item )
    {
        if (item->isOn())
        imagePluginList.append(item->text(1));        // Get the plugin library name.
        item = (QCheckListItem*)item->nextSibling();
    }

    return imagePluginList;
}

void SetupImgPlugins::toggleAll(bool on)
{
    QCheckListItem *item = (QCheckListItem*)d->pluginList->firstChild();

    while( item )
    {
        if (item->isEnabled())   // Do not touch the core plugin.
            item->setOn( on );

        item = (QCheckListItem*)item->nextSibling();
    }
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
    d->enableImagePluginList = config->readListEntry("ImagePlugins List");
}

}  // namespace Digikam

