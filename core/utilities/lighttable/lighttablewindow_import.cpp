/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-11-22
 * Description : digiKam light table - Import tools
 *
 * Copyright (C) 2007-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "lighttablewindow.h"
#include "lighttablewindow_p.h"

namespace Digikam
{

void LightTableWindow::slotImportFromScanner()
{
#ifdef HAVE_KSANE
    m_ksaneAction->activate(DigikamApp::instance()->scannerTargetPlace(), configGroupName());

    connect(m_ksaneAction, SIGNAL(signalImportedImage(QUrl)),
            this, SLOT(slotImportedImagefromScanner(QUrl)));
#endif
}

void LightTableWindow::slotImportedImagefromScanner(const QUrl& url)
{
    ImageInfo info = ScanController::instance()->scannedInfo(url.toLocalFile());
    loadImageInfos(ImageInfoList() << info, info, true);
}

void LightTableWindow::slotImportTool()
{
    QAction* const action = dynamic_cast<QAction*>(sender());
    int tool              = actionToWebService(action);

    if (tool != WSStarter::ExportUnknown)
    {
        WSStarter::importFromWebService(tool, new DBInfoIface(this, QList<QUrl>(),
                                                              ApplicationSettings::ImportExport), this);
    }
}

} // namespace Digikam
