/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2002-16-10
 * Description : main digiKam interface implementation - Export tools
 *
 * Copyright (C) 2002-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
void DigikamApp::slotExpoBlending()
{
    ExpoBlendingManager::instance()->checkBinaries();
    ExpoBlendingManager::instance()->setItemsList(view()->selectedUrls(ApplicationSettings::Tools));
    ExpoBlendingManager::instance()->run();
}

void DigikamApp::slotPanorama()
{
#ifdef HAVE_PANORAMA
    PanoManager::instance()->checkBinaries();
    PanoManager::instance()->setItemsList(view()->selectedUrls(ApplicationSettings::Tools));
    PanoManager::instance()->run();
#endif
}

void DigikamApp::slotVideoSlideshow()
{
#ifdef HAVE_MEDIAPLAYER
    QPointer<VidSlideWizard> w = new VidSlideWizard(this, new DBInfoIface(this, QList<QUrl>(), ApplicationSettings::Tools));
    w->exec();
    delete w;
#endif
}

void DigikamApp::slotSendByMail()
{
    QPointer<MailWizard> w = new MailWizard(this, new DBInfoIface(this, QList<QUrl>(), ApplicationSettings::Tools));
    w->exec();
    delete w;
}

void DigikamApp::slotPrintCreator()
{
    QPointer<AdvPrintWizard> w = new AdvPrintWizard(this, new DBInfoIface(this, QList<QUrl>(), ApplicationSettings::Tools));
    w->exec();
    delete w;
}

void DigikamApp::slotHtmlGallery()
{
#ifdef HAVE_HTMLGALLERY
    QPointer<HTMLWizard> w = new HTMLWizard(this, new DBInfoIface(this, QList<QUrl>(), ApplicationSettings::Tools));
    w->exec();
    delete w;
#endif
}

void DigikamApp::slotCalendar()
{
    QPointer<CalWizard> w = new CalWizard(view()->selectedUrls(ApplicationSettings::Tools), this);
    w->exec();
    delete w;
}

void DigikamApp::slotPresentation()
{
    d->view->presentation();
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
    QAction* const tool = dynamic_cast<QAction*>(sender());

    if (tool == m_exportDropboxAction)
    {
        QPointer<DBWindow> w = new DBWindow(new DBInfoIface(this, QList<QUrl>(), ApplicationSettings::ImportExport), this);
        w->exec();
        delete w;
    }
    else if (tool == m_exportOnedriveAction)
    {
        QPointer<ODWindow> w = new ODWindow(new DBInfoIface(this, QList<QUrl>(), ApplicationSettings::ImportExport), this);
        w->exec();
        delete w;
    }
    else if (tool == m_exportTwitterAction)
    {
        QPointer<ODWindow> w = new TwWindow(new DBInfoIface(this, QList<QUrl>(), ApplicationSettings::ImportExport), this);
        w->exec();
        delete w;
    }
    else if (tool == m_exportFacebookAction)
    {
        QPointer<FbWindow> w = new FbWindow(new DBInfoIface(this, QList<QUrl>(), ApplicationSettings::ImportExport), this);
        w->exec();
        delete w;
    }
    else if (tool == m_exportFlickrAction)
    {
        QPointer<FlickrWindow> w = new FlickrWindow(new DBInfoIface(this, QList<QUrl>(), ApplicationSettings::ImportExport), this);
        w->exec();
        delete w;
    }
    else if (tool == m_exportGdriveAction)
    {
        QPointer<GSWindow> w = new GSWindow(new DBInfoIface(this, QList<QUrl>(), ApplicationSettings::ImportExport),
                                            this, QLatin1String("googledriveexport"));
        w->exec();
        delete w;
    }
    else if (tool == m_exportGphotoAction)
    {
        QPointer<GSWindow> w = new GSWindow(new DBInfoIface(this, QList<QUrl>(), ApplicationSettings::ImportExport),
                                            this, QLatin1String("googlephotoexport"));
        w->exec();
        delete w;
    }
    else if (tool == m_exportImageshackAction)
    {
        QPointer<ImageShackWindow> w = new ImageShackWindow(new DBInfoIface(this, QList<QUrl>(), ApplicationSettings::ImportExport), this);
        w->exec();
        delete w;
    }
    else if (tool == m_exportImgurAction)
    {
        QPointer<ImgurWindow> w = new ImgurWindow(new DBInfoIface(this, QList<QUrl>(), ApplicationSettings::ImportExport), this);
        w->exec();
        delete w;
    }
    else if (tool == m_exportPiwigoAction)
    {
        QPointer<PiwigoWindow> w = new PiwigoWindow(new DBInfoIface(this, QList<QUrl>(), ApplicationSettings::ImportExport), this);
        w->exec();
        delete w;
    }
    else if (tool == m_exportRajceAction)
    {
        QPointer<RajceWindow> w = new RajceWindow(new DBInfoIface(this, QList<QUrl>(), ApplicationSettings::ImportExport), this);
        w->exec();
        delete w;
    }
    else if (tool == m_exportSmugmugAction)
    {
        QPointer<SmugWindow> w = new SmugWindow(new DBInfoIface(this, QList<QUrl>(), ApplicationSettings::ImportExport), this);
        w->exec();
        delete w;
    }
    else if (tool == m_exportYandexfotkiAction)
    {
        QPointer<YFWindow> w = new YFWindow(new DBInfoIface(this, QList<QUrl>(), ApplicationSettings::ImportExport), this);
        w->exec();
        delete w;
    }

#ifdef HAVE_MEDIAWIKI
    else if (tool == m_exportMediawikiAction)
    {
        QPointer<MediaWikiWindow> w = new MediaWikiWindow(new DBInfoIface(this, QList<QUrl>(), ApplicationSettings::ImportExport), this);
        w->exec();
        delete w;
    }
#endif

#ifdef HAVE_VKONTAKTE
    else if (tool == m_exportVkontakteAction)
    {
        QPointer<VKWindow> w = new VKWindow(new DBInfoIface(this, QList<QUrl>(), ApplicationSettings::ImportExport), this);
        w->exec();
        delete w;
    }
#endif

#ifdef HAVE_KIO
    else if (tool == m_exportFileTransferAction)
    {
        QPointer<FTExportWindow> w = new FTExportWindow(new DBInfoIface(this, QList<QUrl>(), ApplicationSettings::ImportExport), this);
        w->exec();
        delete w;
    }
#endif
}

} // namespace Digikam
