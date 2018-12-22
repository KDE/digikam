/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-11-22
 * Description : stand alone digiKam image editor - Export tools
 *
 * Copyright (C) 2004-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "showfoto.h"
#include "showfoto_p.h"

namespace ShowFoto
{

void ShowFoto::slotHtmlGallery()
{
#ifdef HAVE_HTMLGALLERY
    QPointer<HTMLWizard> w = new HTMLWizard(this, new DMetaInfoIface(this, d->thumbBar->urls()));
    w->exec();
    delete w;
#endif
}

void ShowFoto::slotCalendar()
{
    QPointer<CalWizard> w = new CalWizard(d->thumbBar->urls(), this);
    w->exec();
    delete w;
}

void ShowFoto::slotPanorama()
{
#ifdef HAVE_PANORAMA
    PanoManager::instance()->checkBinaries();
    PanoManager::instance()->setItemsList(d->thumbBar->urls());
    PanoManager::instance()->run();
#endif
}

void ShowFoto::slotExpoBlending()
{
    ExpoBlendingManager::instance()->checkBinaries();
    ExpoBlendingManager::instance()->setItemsList(d->thumbBar->urls());
    ExpoBlendingManager::instance()->run();
}

void ShowFoto::slotVideoSlideshow()
{
#ifdef HAVE_MEDIAPLAYER
    QPointer<VidSlideWizard> w = new VidSlideWizard(this, new DMetaInfoIface(this, d->thumbBar->urls()));
    w->exec();
    delete w;
#endif
}

void ShowFoto::slotSendByMail()
{
    QPointer<MailWizard> w = new MailWizard(this, new DMetaInfoIface(this, d->thumbBar->urls()));
    w->exec();
    delete w;
}

void ShowFoto::slotMediaServer()
{
    QPointer<DMediaServerDlg> w = new DMediaServerDlg(this, new DMetaInfoIface(this, d->thumbBar->urls()));
    w->exec();
    delete w;
}

void ShowFoto::slotExportTool()
{
    QAction* const action = dynamic_cast<QAction*>(sender());
    int tool              = actionToWebService(action);

    if (tool != WSStarter::ExportUnknown)
    {
        WSStarter::exportToWebService(tool, new DMetaInfoIface(this, d->thumbBar->urls()), this);
    }
}

} // namespace ShowFoto
