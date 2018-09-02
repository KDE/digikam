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
#endif

    QPointer<FlickrWindow>     flickrWindow;
    QPointer<GSWindow>         gdWindow;
    QPointer<GSWindow>         gpWindow;
    QPointer<ImageShackWindow> imageShackWindow;
    QPointer<ImgurWindow>      imgurWindow;
    QPointer<MediaWikiWindow>  mediaWikiWindow;
    QPointer<ODWindow>         odWindow;
    QPointer<PWindow>          pWindow;
    QPointer<PiwigoWindow>     piwigoWindow;
    QPointer<RajceWindow>      rajceWindow;
    QPointer<SmugWindow>       smugWindow;

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
#endif

    delete instance()->d->flickrWindow;
    delete instance()->d->gdWindow;
    delete instance()->d->gpWindow;
    delete instance()->d->imageShackWindow;
    delete instance()->d->imgurWindow;
    delete instance()->d->mediaWikiWindow;
    delete instance()->d->odWindow;
    delete instance()->d->pWindow;
    delete instance()->d->piwigoWindow;
    delete instance()->d->rajceWindow;
    delete instance()->d->smugWindow;

#ifdef HAVE_VKONTAKTE
    delete instance()->d->vkWindow;
#endif

    delete instance()->d->yfWindow;
}

void WSStarter::exportToWebService(int tool, DInfoInterface* const iface, QWidget* const parent)
{
    instance()->toWebService(tool, iface, parent);
}

// ------------------------------------------------------------------------------------------------

WSStarter::WSStarter()
    : d(new Private)
{
    //qDebug() << "New WSStarter";
}

WSStarter::~WSStarter()
{
    delete d;
}

void WSStarter::toWebService(int tool, DInfoInterface* const iface, QWidget* const parent)
{
    if (tool == Box)
    {
        if (d->boxWindow && (d->boxWindow->isMinimized() || !d->boxWindow->isHidden()))
        {
            d->boxWindow->showNormal();       // krazy:exclude=qmethods
            d->boxWindow->activateWindow();
            d->boxWindow->raise();
        }
        else
        {
            d->boxWindow = new BOXWindow(iface, parent);
            d->boxWindow->show();
        }
    }
    else if (tool == Dropbox)
    {
        if (d->dbWindow && (d->dbWindow->isMinimized() || !d->dbWindow->isHidden()))
        {
            d->dbWindow->showNormal();       // krazy:exclude=qmethods
            d->dbWindow->activateWindow();
            d->dbWindow->raise();
        }
        else
        {
            d->dbWindow = new DBWindow(iface, parent);
            d->dbWindow->show();
        }
    }
    else if (tool == Facebook)
    {
        if (d->fbWindow && (d->fbWindow->isMinimized() || !d->fbWindow->isHidden()))
        {
            d->fbWindow->showNormal();       // krazy:exclude=qmethods
            d->fbWindow->activateWindow();
            d->fbWindow->raise();
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
        if (d->ftExportWindow && (d->ftExportWindow->isMinimized() || !d->ftExportWindow->isHidden()))
        {
            d->ftExportWindow->showNormal();       // krazy:exclude=qmethods
            d->ftExportWindow->activateWindow();
            d->ftExportWindow->raise();
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
        if (d->flickrWindow && (d->flickrWindow->isMinimized() || !d->flickrWindow->isHidden()))
        {
            d->flickrWindow->showNormal();       // krazy:exclude=qmethods
            d->flickrWindow->activateWindow();
            d->flickrWindow->raise();
        }
        else
        {
            d->flickrWindow = new FlickrWindow(iface, parent);
            d->flickrWindow->show();
        }
    }
    else if (tool == Gdrive)
    {
        if (d->gdWindow && (d->gdWindow->isMinimized() || !d->gdWindow->isHidden()))
        {
            d->gdWindow->showNormal();       // krazy:exclude=qmethods
            d->gdWindow->activateWindow();
            d->gdWindow->raise();
        }
        else
        {
            d->gdWindow = new GSWindow(iface, parent, QLatin1String("googledriveexport"));
            d->gdWindow->show();
        }
    }
    else if (tool == Gphoto)
    {
        if (d->gpWindow && (d->gpWindow->isMinimized() || !d->gpWindow->isHidden()))
        {
            d->gpWindow->showNormal();       // krazy:exclude=qmethods
            d->gpWindow->activateWindow();
            d->gpWindow->raise();
        }
        else
        {
            d->gpWindow = new GSWindow(iface, parent, QLatin1String("googlephotoexport"));
            d->gpWindow->show();
        }
    }
    else if (tool == Imageshack)
    {
        if (d->imageShackWindow && (d->imageShackWindow->isMinimized() || !d->imageShackWindow->isHidden()))
        {
            d->imageShackWindow->showNormal();       // krazy:exclude=qmethods
            d->imageShackWindow->activateWindow();
            d->imageShackWindow->raise();
        }
        else
        {
            d->imageShackWindow = new ImageShackWindow(iface, parent);
            d->imageShackWindow->show();
        }
    }
    else if (tool == Imgur)
    {
        if (d->imgurWindow && (d->imgurWindow->isMinimized() || !d->imgurWindow->isHidden()))
        {
            d->imgurWindow->showNormal();       // krazy:exclude=qmethods
            d->imgurWindow->activateWindow();
            d->imgurWindow->raise();
        }
        else
        {
            d->imgurWindow = new ImgurWindow(iface, parent);
            d->imgurWindow->show();
        }
    }
    else if (tool == Mediawiki)
    {
        if (d->mediaWikiWindow && (d->mediaWikiWindow->isMinimized() || !d->mediaWikiWindow->isHidden()))
        {
            d->mediaWikiWindow->showNormal();       // krazy:exclude=qmethods
            d->mediaWikiWindow->activateWindow();
            d->mediaWikiWindow->raise();
        }
        else
        {
            d->mediaWikiWindow = new MediaWikiWindow(iface, parent);
            d->mediaWikiWindow->show();
        }
    }
    else if (tool == Onedrive)
    {
        if (d->odWindow && (d->odWindow->isMinimized() || !d->odWindow->isHidden()))
        {
            d->odWindow->showNormal();       // krazy:exclude=qmethods
            d->odWindow->activateWindow();
            d->odWindow->raise();
        }
        else
        {
            d->odWindow = new ODWindow(iface, parent);
            d->odWindow->show();
        }
    }
    else if (tool == Pinterest)
    {
        if (d->pWindow && (d->pWindow->isMinimized() || !d->pWindow->isHidden()))
        {
            d->pWindow->showNormal();       // krazy:exclude=qmethods
            d->pWindow->activateWindow();
            d->pWindow->raise();
        }
        else
        {
            d->pWindow = new PWindow(iface, parent);
            d->pWindow->show();
        }
    }
    else if (tool == Piwigo)
    {
        if (d->piwigoWindow && (d->piwigoWindow->isMinimized() || !d->piwigoWindow->isHidden()))
        {
            d->piwigoWindow->showNormal();       // krazy:exclude=qmethods
            d->piwigoWindow->activateWindow();
            d->piwigoWindow->raise();
        }
        else
        {
            d->piwigoWindow = new PiwigoWindow(iface, parent);
            d->piwigoWindow->show();
        }
    }
    else if (tool == Rajce)
    {
        if (d->rajceWindow && (d->rajceWindow->isMinimized() || !d->rajceWindow->isHidden()))
        {
            d->rajceWindow->showNormal();       // krazy:exclude=qmethods
            d->rajceWindow->activateWindow();
            d->rajceWindow->raise();
        }
        else
        {
            d->rajceWindow = new RajceWindow(iface, parent);
            d->rajceWindow->show();
        }
    }
    else if (tool == Smugmug)
    {
        if (d->smugWindow && (d->smugWindow->isMinimized() || !d->smugWindow->isHidden()))
        {
            d->smugWindow->showNormal();       // krazy:exclude=qmethods
            d->smugWindow->activateWindow();
            d->smugWindow->raise();
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
        if (d->vkWindow && (d->vkWindow->isMinimized() || !d->vkWindow->isHidden()))
        {
            d->vkWindow->showNormal();       // krazy:exclude=qmethods
            d->vkWindow->activateWindow();
            d->vkWindow->raise();
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
        if (d->yfWindow && (d->yfWindow->isMinimized() || !d->yfWindow->isHidden()))
        {
            d->yfWindow->showNormal();       // krazy:exclude=qmethods
            d->yfWindow->activateWindow();
            d->yfWindow->raise();
        }
        else
        {
            d->yfWindow = new YFWindow(iface, parent);
            d->yfWindow->show();
        }
    }
}

} // namespace Digikam
