/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-01-02
 * Description : setup Kipi plugins tab.
 * 
 * Copyright 2004-2006 by Gilles Caulier
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

// KIPI Includes.

#include <libkipi/version.h>

// Local includes.

#include "setupplugins.h"

namespace Digikam
{

SetupPlugins::SetupPlugins(QWidget* parent )
            : QWidget(parent)
{
   QVBoxLayout *layout = new QVBoxLayout( parent );

   QHBoxLayout *hlay = new QHBoxLayout(layout);
   m_pluginsNumber = new QLabel(parent);

   QLabel *KipiVersion = new QLabel(i18n("Kipi version: %1").arg(kipi_version), parent);
   KipiVersion->setAlignment ( Qt::AlignRight | Qt::AlignVCenter );

   hlay->addWidget(m_pluginsNumber, 1);
   hlay->addStretch(1);
   hlay->addWidget(KipiVersion, 1);

   m_Kipiconfig = KIPI::PluginLoader::instance()->configWidget( parent );
   QString pluginsListHelp = i18n("<p>A list of available Kipi plugins "
                                  "appears below.");
   QWhatsThis::add( m_Kipiconfig, pluginsListHelp);
   layout->addWidget( m_Kipiconfig );
}

SetupPlugins::~SetupPlugins()
{
}

void SetupPlugins::initPlugins(int kipiPluginsNumber)
{
    m_pluginsNumber->setText(i18n("1 Kipi plugin found",
                                  "%n Kipi plugins found",
                                  kipiPluginsNumber));
}

void SetupPlugins::applyPlugins()
{
    m_Kipiconfig->apply();
}

}  // namespace Digikam

#include "setupplugins.moc"
