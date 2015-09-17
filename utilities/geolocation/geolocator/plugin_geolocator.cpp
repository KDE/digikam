/** ===========================================================
 * @file
 *
 * This file is a part of kipi-plugins project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2006-05-16
 * @brief  A plugin to synchronize pictures with a GPS device.
 *
 * @author Copyright (C) 2006-2015 by Gilles Caulier
 *         <a href="mailto:caulier dot gilles at gmail dot com">caulier dot gilles at gmail dot com</a>
 * @author Copyright (C) 2010-2014 by Michael G. Hansen
 *         <a href="mailto:mike at mghansen dot de">mike at mghansen dot de</a>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "plugin_geolocator.h"

// Qt includes

#include <QPointer>
#include <QApplication>
#include <QAction>

// KDE includes

#include <kactioncollection.h>
#include <kactionmenu.h>
#include <kconfig.h>
#include <klocalizedstring.h>
#include <kmessagebox.h>
#include <kpluginfactory.h>

// Libkipi includes

#include <KIPI/ImageCollection>
#include <KIPI/Interface>

// Local includes

#include "kphostsettings.h"
#include "digikam_debug.h"
#include "gpssyncdialog.h"

namespace Digikam
{

K_PLUGIN_FACTORY( GeolocatorFactory, registerPlugin<Plugin_Geolocator>(); )

Plugin_Geolocator::Plugin_Geolocator(QObject* const parent, const QVariantList&)
    : Plugin(parent, "Geolocator")
{
    m_action_geolocation = 0;
    m_interface          = 0;

    qCDebug(DIGIKAM_GENERAL_LOG) << "Plugin_Geolocator plugin loaded" ;

    setUiBaseName("kipiplugin_geolocatorui.rc");
    setupXML();
}

Plugin_Geolocator::~Plugin_Geolocator()
{
}

void Plugin_Geolocator::setup(QWidget* const widget)
{
    Plugin::setup( widget );
    setupActions();

    m_interface = interface();

    if (!m_interface)
    {
        qCCritical(DIGIKAM_GENERAL_LOG) << "Kipi interface is null!" ;
        return;
    }

    ImageCollection selection = m_interface->currentSelection();
    m_action_geolocation->setEnabled( selection.isValid() && !selection.images().isEmpty() );

    connect(m_interface, SIGNAL(selectionChanged(bool)),
            m_action_geolocation, SLOT(setEnabled(bool)));
}

void Plugin_Geolocator::setupActions()
{
    setDefaultCategory(ImagesPlugin);

    m_action_geolocation = new QAction(this);
    m_action_geolocation->setText(i18n("Geo-location"));
    m_action_geolocation->setIcon(QIcon::fromTheme(QStringLiteral("applications-internet")));
    m_action_geolocation->setEnabled(false);

    connect(m_action_geolocation, SIGNAL(triggered(bool)),
            this, SLOT(slotGeolocator()));

    addAction(QStringLiteral("geolocator"), m_action_geolocation);
}

bool Plugin_Geolocator::checkSidecarSettings()
{
    KPHostSettings hset;
    
    if ( (hset.metadataSettings().metadataWritingMode != KExiv2Iface::KExiv2::WRITETOIMAGEONLY) &&
         (!hset.metadataSettings().useXMPSidecar4Reading) )
    {
        const int result = KMessageBox::warningContinueCancel(
                QApplication::activeWindow(),
                i18n(
                        "You have enabled writing to sidecar files for metadata storage in the host application,"
                        " but not for reading."
                        " This means that any metadata stored in the sidecar files will be overwritten here.\n"
                        "Please enable reading of sidecar files in the host application or continue at your own risk."
                    ),
                i18n("Warning: Sidecar settings"),
                KStandardGuiItem::cont(),
                KStandardGuiItem::cancel(),
                QString(),
                KMessageBox::Dangerous
            );
        
        if (result!=KMessageBox::Continue)
        {
            return false;
        }
    }
    
    return true;
}

void Plugin_Geolocator::slotGeolocator()
{
    ImageCollection images = m_interface->currentSelection();

    if ( !images.isValid() || images.images().isEmpty() )
    {
        return;
    }

    if (!checkSidecarSettings())
    {
        return;
    }

    GPSSyncDialog* const dialog = new GPSSyncDialog(QApplication::activeWindow());

    dialog->setImages( images.images() );
    dialog->show();
}

} // namespace Digikam

#include "plugin_geolocator.moc"
