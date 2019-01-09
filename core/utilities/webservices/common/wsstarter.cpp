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
#include <QMessageBox>

// Local includes

#include "digikam_debug.h"
#include "digikam_config.h"
#include "fbwindow.h"
#include "flickrwindow.h"
#include "gswindow.h"
#include "rajcewindow.h"
#include "smugwindow.h"
#include "yfwindow.h"

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

    QPointer<FbWindow>         fbWindow;

#ifdef HAVE_KIO
    QPointer<FTExportWindow>   ftExportWindow;
    QPointer<FTImportWindow>   ftImportWindow;
#endif

    QPointer<FlickrWindow>     flickrWindow;
    QPointer<GSWindow>         gdWindow;
    QPointer<GSWindow>         gpWindow;
    QPointer<GSWindow>         gpImportWindow;
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
    if (creator.exists())
    {
        delete instance()->d->fbWindow;

#ifdef HAVE_KIO
        delete instance()->d->ftExportWindow;
        delete instance()->d->ftImportWindow;
#endif

        delete instance()->d->flickrWindow;
        delete instance()->d->gdWindow;
        delete instance()->d->gpWindow;
        delete instance()->d->gpImportWindow;
        delete instance()->d->rajceWindow;
        delete instance()->d->smugWindow;
        delete instance()->d->smugImportWindow;

#ifdef HAVE_VKONTAKTE
        delete instance()->d->vkWindow;
#endif

        delete instance()->d->yfWindow;
    }
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
    if (tool == ExportFacebook)
    {
        if (checkWebService(static_cast<QWidget*>(d->fbWindow)))
        {
            return;
        }
        else
        {
            // This message is not translated, will be removed after the review process.
            QMessageBox::warning(parent, QLatin1String("Facebook"),
                                 QLatin1String("This Facebook export tool is under "
                                               "review process and only works for "
                                               "registered test users."));
            delete d->fbWindow;
            d->fbWindow = new FbWindow(iface, parent);
            d->fbWindow->show();
        }
    }

#ifdef HAVE_KIO
    else if (tool == ExportFileTransfer)
    {
        if (checkWebService(static_cast<QWidget*>(d->ftExportWindow)))
        {
            return;
        }
        else
        {
            delete d->ftExportWindow;
            d->ftExportWindow = new FTExportWindow(iface, parent);
            d->ftExportWindow->show();
        }
    }
#endif

    else if (tool == ExportFlickr)
    {
        if (checkWebService(static_cast<QWidget*>(d->flickrWindow)))
        {
            return;
        }
        else
        {
            delete d->flickrWindow;
            d->flickrWindow = new FlickrWindow(iface, parent);
            d->flickrWindow->show();
        }
    }
    else if (tool == ExportGdrive)
    {
        if (checkWebService(static_cast<QWidget*>(d->gdWindow)))
        {
            return;
        }
        else
        {
            delete d->gdWindow;
            d->gdWindow = new GSWindow(iface, parent, QLatin1String("googledriveexport"));
            d->gdWindow->show();
        }
    }
    else if (tool == ExportGphoto)
    {
        if (checkWebService(static_cast<QWidget*>(d->gpWindow)))
        {
            return;
        }
        else
        {
            delete d->gpWindow;
            d->gpWindow = new GSWindow(iface, parent, QLatin1String("googlephotoexport"));
            d->gpWindow->show();
        }
    }
    else if (tool == ExportRajce)
    {
        if (checkWebService(static_cast<QWidget*>(d->rajceWindow)))
        {
            return;
        }
        else
        {
            delete d->rajceWindow;
            d->rajceWindow = new RajceWindow(iface, parent);
            d->rajceWindow->show();
        }
    }
    else if (tool == ExportSmugmug)
    {
        if (checkWebService(static_cast<QWidget*>(d->smugWindow)))
        {
            return;
        }
        else
        {
            delete d->smugWindow;
            d->smugWindow = new SmugWindow(iface, parent);
            d->smugWindow->show();
        }
    }

#ifdef HAVE_VKONTAKTE
    else if (tool == ExportVkontakte)
    {
        if (checkWebService(static_cast<QWidget*>(d->vkWindow)))
        {
            return;
        }
        else
        {
            delete d->vkWindow;
            d->vkWindow = new VKWindow(iface, parent);
            d->vkWindow->show();
        }
    }
#endif

    else if (tool == ExportYandexfotki)
    {
        if (checkWebService(static_cast<QWidget*>(d->yfWindow)))
        {
            return;
        }
        else
        {
            delete d->yfWindow;
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
            delete d->gpImportWindow;
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
            delete d->ftImportWindow;
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
            delete d->smugImportWindow;
            d->smugImportWindow = new SmugWindow(iface, parent, true);
            d->smugImportWindow->show();
        }
    }
}

bool WSStarter::checkWebService(QWidget* const widget) const
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
