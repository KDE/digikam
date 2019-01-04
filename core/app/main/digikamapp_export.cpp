/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2002-16-10
 * Description : main digiKam interface implementation - Export tools
 *
 * Copyright (C) 2002-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "digikamapp.h"
#include "digikamapp_p.h"

namespace Digikam
{

void DigikamApp::slotPanorama()
{
#ifdef HAVE_PANORAMA
    PanoManager::instance()->checkBinaries();
    PanoManager::instance()->setItemsList(view()->selectedUrls(ApplicationSettings::Tools));
    PanoManager::instance()->run();
#endif
}

void DigikamApp::slotMediaServer()
{
    DBInfoIface* const iface = new DBInfoIface(this, QList<QUrl>(), ApplicationSettings::Tools);
    // NOTE: We overwrite the default albums chooser object name for load save check items state between sessions.
    // The goal is not mix these settings with other export tools.
    iface->setObjectName(QLatin1String("SetupMediaServerIface"));

    QPointer<DMediaServerDlg> w = new DMediaServerDlg(this, iface);
    w->exec();
    delete w;
}

void DigikamApp::slotExportTool()
{
    QAction* const action = dynamic_cast<QAction*>(sender());
    int tool              = actionToWebService(action);

    if (tool != WSStarter::ExportUnknown)
    {
        WSStarter::exportToWebService(tool, new DBInfoIface(this, QList<QUrl>(),
                                                            ApplicationSettings::ImportExport), this);
    }
}

} // namespace Digikam
