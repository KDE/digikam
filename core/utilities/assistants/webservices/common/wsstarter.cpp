/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2018-09-02
 * Description : Start Web Service methods.
 *
 * Copyright (C) 2018 by Maik Qualmann <metzpinguin at gmail dot com>
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

#include "wsstarter.h"

// Qt includes

#include <QPointer>

// Local includes

#include "digikam_debug.h"
#include "digikam_config.h"
#include "flickrwindow.h"
#include "dbwindow.h"
#include "odwindow.h"
#include "pwindow.h"
#include "boxwindow.h"
#include "fbwindow.h"
#include "gswindow.h"
#include "imageshackwindow.h"
#include "imgurwindow.h"
#include "piwigowindow.h"
#include "rajcewindow.h"
#include "smugwindow.h"
#include "yfwindow.h"
#include "mediawikiwindow.h"

#ifdef HAVE_VKONTAKTE
#   include "vkwindow.h"
#endif

#ifdef HAVE_KIO
#   include "ftexportwindow.h"
#   include "ftimportwindow.h"
#endif

namespace Digikam
{

class Q_DECL_HIDDEN WSStarter::Private
{
public:

    explicit Private()
    {
    }

    QPointer<BOXWindow>        boxWindow;
    QPointer<DBWindow>         dbWindow;
    QPointer<FbWindow>         fbWindow;

#ifdef HAVE_KIO
    QPointer<FTExportWindow>   ftExportWindow;
    QPointer<FTImportWindow>   ftImportWindow;
#endif

    QPointer<FlickrWindow>     flickrWindow;
    QPointer<GSWindow>         gdWindow;
    QPointer<GSWindow>         gpWindow;
    QPointer<GSWindow>         gpImportWindow;
    QPointer<ImageShackWindow> imageShackWindow;
    QPointer<ImgurWindow>      imgurWindow;
    QPointer<MediaWikiWindow>  mediaWikiWindow;
    QPointer<ODWindow>         odWindow;
    QPointer<PWindow>          pWindow;
    QPointer<PiwigoWindow>     piwigoWindow;
    QPointer<RajceWindow>      rajceWindow;
    QPointer<SmugWindow>       smugWindow;
    QPointer<SmugWindow>       smugImportWindow;

#ifdef HAVE_VKONTAKTE
    QPointer<VKWindow>         vkWindow;
#endif

    QPointer<YFWindow>         yfWindow;
};

class Q_DECL_HIDDEN WSStarterCreator
{
public:

    WSStarter object;
};

Q_GLOBAL_STATIC(WSStarterCreator, creator)

// ------------------------------------------------------------------------------------------------

WSStarter* WSStarter::instance()
{
    return &creator->object;
}

void WSStarter::cleanUp()
{
    delete instance()->d->boxWindow;
    delete instance()->d->dbWindow;
    delete instance()->d->fbWindow;

#ifdef HAVE_KIO
    delete instance()->d->ftExportWindow;
    delete instance()->d->ftImportWindow;
#endif

    delete instance()->d->flickrWindow;
    delete instance()->d->gdWindow;
    delete instance()->d->gpWindow;
    delete instance()->d->gpImportWindow;
    delete instance()->d->imageShackWindow;
    delete instance()->d->imgurWindow;
    delete instance()->d->mediaWikiWindow;
    delete instance()->d->odWindow;
    delete instance()->d->pWindow;
    delete instance()->d->piwigoWindow;
    delete instance()->d->rajceWindow;
    delete instance()->d->smugWindow;
    delete instance()->d->smugImportWindow;

#ifdef HAVE_VKONTAKTE
    delete instance()->d->vkWindow;
#endif

    delete instance()->d->yfWindow;
}

void WSStarter::exportToWebService(int tool, DInfoInterface* const iface, QWidget* const parent)
{
    instance()->toWebService(tool, iface, parent);
}

void WSStarter::importFromWebService(int tool, DInfoInterface* const iface, QWidget* const parent)
{
    instance()->fromWebService(tool, iface, parent);
}

// ------------------------------------------------------------------------------------------------

WSStarter::WSStarter()
    : d(new Private)
{
}

WSStarter::~WSStarter()
{
    delete d;
}

void WSStarter::toWebService(int tool, DInfoInterface* const iface, QWidget* const parent)
{
    if (tool == Box)
    {
        if (checkWebService(static_cast<QWidget*>(d->boxWindow)))
        {
            return;
        }
        else
        {
            d->boxWindow = new BOXWindow(iface, parent);
            d->boxWindow->show();
        }
    }
    else if (tool == Dropbox)
    {
        if (checkWebService(static_cast<QWidget*>(d->dbWindow)))
        {
            return;
        }
        else
        {
            d->dbWindow = new DBWindow(iface, parent);
            d->dbWindow->show();
        }
    }
    else if (tool == Facebook)
    {
        if (checkWebService(static_cast<QWidget*>(d->fbWindow)))
        {
            return;
        }
        else
        {
            d->fbWindow = new FbWindow(iface, parent);
            d->fbWindow->show();
        }
    }

#ifdef HAVE_KIO
    else if (tool == FileTransfer)
    {
        if (checkWebService(static_cast<QWidget*>(d->ftExportWindow)))
        {
            return;
        }
        else
        {
            d->ftExportWindow = new FTExportWindow(iface, parent);
            d->ftExportWindow->show();
        }
    }
#endif

    else if (tool == Flickr)
    {
        if (checkWebService(static_cast<QWidget*>(d->flickrWindow)))
        {
            return;
        }
        else
        {
            d->flickrWindow = new FlickrWindow(iface, parent);
            d->flickrWindow->show();
        }
    }
    else if (tool == Gdrive)
    {
        if (checkWebService(static_cast<QWidget*>(d->gdWindow)))
        {
            return;
        }
        else
        {
            d->gdWindow = new GSWindow(iface, parent, QLatin1String("googledriveexport"));
            d->gdWindow->show();
        }
    }
    else if (tool == Gphoto)
    {
        if (checkWebService(static_cast<QWidget*>(d->gpWindow)))
        {
            return;
        }
        else
        {
            d->gpWindow = new GSWindow(iface, parent, QLatin1String("googlephotoexport"));
            d->gpWindow->show();
        }
    }
    else if (tool == Imageshack)
    {
        if (checkWebService(static_cast<QWidget*>(d->imageShackWindow)))
        {
            return;
        }
        else
        {
            d->imageShackWindow = new ImageShackWindow(iface, parent);
            d->imageShackWindow->show();
        }
    }
    else if (tool == Imgur)
    {
        if (checkWebService(static_cast<QWidget*>(d->imgurWindow)))
        {
            return;
        }
        else
        {
            d->imgurWindow = new ImgurWindow(iface, parent);
            d->imgurWindow->show();
        }
    }
    else if (tool == Mediawiki)
    {
        if (checkWebService(static_cast<QWidget*>(d->mediaWikiWindow)))
        {
            return;
        }
        else
        {
            d->mediaWikiWindow = new MediaWikiWindow(iface, parent);
            d->mediaWikiWindow->show();
        }
    }
    else if (tool == Onedrive)
    {
        if (checkWebService(static_cast<QWidget*>(d->odWindow)))
        {
            return;
        }
        else
        {
            d->odWindow = new ODWindow(iface, parent);
            d->odWindow->show();
        }
    }
    else if (tool == Pinterest)
    {
        if (checkWebService(static_cast<QWidget*>(d->pWindow)))
        {
            return;
        }
        else
        {
            d->pWindow = new PWindow(iface, parent);
            d->pWindow->show();
        }
    }
    else if (tool == Piwigo)
    {
        if (checkWebService(static_cast<QWidget*>(d->piwigoWindow)))
        {
            return;
        }
        else
        {
            d->piwigoWindow = new PiwigoWindow(iface, parent);
            d->piwigoWindow->show();
        }
    }
    else if (tool == Rajce)
    {
        if (checkWebService(static_cast<QWidget*>(d->rajceWindow)))
        {
            return;
        }
        else
        {
            d->rajceWindow = new RajceWindow(iface, parent);
            d->rajceWindow->show();
        }
    }
    else if (tool == Smugmug)
    {
        if (checkWebService(static_cast<QWidget*>(d->smugWindow)))
        {
            return;
        }
        else
        {
            d->smugWindow = new SmugWindow(iface, parent);
            d->smugWindow->show();
        }
    }

#ifdef HAVE_VKONTAKTE
    else if (tool == Vkontakte)
    {
        if (checkWebService(static_cast<QWidget*>(d->vkWindow)))
        {
            return;
        }
        else
        {
            d->vkWindow = new VKWindow(iface, parent);
            d->vkWindow->show();
        }
    }
#endif

    else if (tool == Yandexfotki)
    {
        if (checkWebService(static_cast<QWidget*>(d->yfWindow)))
        {
            return;
        }
        else
        {
            d->yfWindow = new YFWindow(iface, parent);
            d->yfWindow->show();
        }
    }
}

void WSStarter::fromWebService(int tool, DInfoInterface* const iface, QWidget* const parent)
{
    if (tool == ImportGphoto)
    {
        if (checkWebService(static_cast<QWidget*>(d->gpImportWindow)))
        {
            return;
        }
        else
        {
            d->gpImportWindow = new GSWindow(iface, parent, QLatin1String("googlephotoimport"));
            d->gpImportWindow->show();
        }
    }

#ifdef HAVE_KIO
    else if (tool == ImportFileTransfer)
    {
        if (checkWebService(static_cast<QWidget*>(d->ftImportWindow)))
        {
            return;
        }
        else
        {
            d->ftImportWindow = new FTImportWindow(iface, parent);
            d->ftImportWindow->show();
        }
    }
#endif

    else if (tool == ImportSmugmug)
    {
        if (checkWebService(static_cast<QWidget*>(d->smugImportWindow)))
        {
            return;
        }
        else
        {
            d->smugImportWindow = new SmugWindow(iface, parent, true);
            d->smugImportWindow->show();
        }
    }
}

bool WSStarter::checkWebService(QWidget* const widget)
{
    if (widget && (widget->isMinimized() || !widget->isHidden()))
    {
        widget->showNormal();       // krazy:exclude=qmethods
        widget->activateWindow();
        widget->raise();
        return true;
    }

    return false;
}

} // namespace Digikam
