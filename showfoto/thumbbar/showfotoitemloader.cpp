/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-07-3
 * Description : Showfoto item loader
 *
 * Copyright (C) 2013 by Mohamed Anwer <mohammed dot ahmed dot anwer at gmail dot com>
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

#include "showfotoitemloader.h"

// C ANSI includes

extern "C"
{
#include <unistd.h>
}

// C++ includes

#include <typeinfo>
#include <cstdio>

// Qt includes

#include <QMutex>
#include <QDir>
#include <QWaitCondition>
#include <QVariant>
#include <QImage>
#include <QFile>
#include <QRegExp>
#include <QFileInfo>
#include <QPointer>
#include <QtConcurrentRun>
#include <QFuture>
#include <QFutureWatcher>
#include <QDebug>
#include <QObject>
#include <QList>

// KDE includes

#include <kde_file.h>
#include <kiconloader.h>
#include <kio/renamedialog.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kurl.h>
#include <kdebug.h>
#include <kprocess.h>
#include <kmacroexpander.h>
#include <libkdcraw/kdcraw.h>
#include <KAction>
#include "kmessagebox.h"

// Local includes

#include "kimageio.h"
#include "showfoto.h"
#include "thumbbar.h"
#include "setupmisc.h"
#include "showfotoiteminfo.h"
#include "showfotoimagemodel.h"


namespace ShowFoto
{


class ShowfotoItemLoader::Private
{

public:

    Private() :
        itemsNb(0),
        model(0)
    {
    }

   int                              itemsNb;
   KUrl :: List                     urlList;
   QDir                             dir;
   ShowfotoImageModel*              model;
   ShowfotoItemInfoList             InfoList;

};

ShowfotoItemLoader::ShowfotoItemLoader()
    : d(new Private)
{
    d->model = new ShowfotoImageModel(this);
    connect(this,SIGNAL(signalInfoList(ShowfotoItemInfoList&)),
            d->model,SLOT(reAddShowfotoItemInfos(ShowfotoItemInfoList&)));
}

ShowfotoItemLoader::~ShowfotoItemLoader()
{
    delete d;
}



void  ShowfotoItemLoader::slotLoadCurrentItem(const KUrl::List& urlList)
{
    qDebug()<< "i got the list";
    for (KUrl::List::const_iterator it = urlList.constBegin();
        it != urlList.constEnd(); ++it)
    {
        KUrl url = *it;

        if (url.isLocalFile())
        {
            QFileInfo fi(url.toLocalFile());

            if (fi.isDir())
            {
                // Local Dir

                openFolder(url);
                emit signalToggleNav(1);
            }
            else
            {
                // Local file

                if (urlList.count() == 1)
                {
                    // Special case if just one item in urls is passed.
                    // We need to handle whole current dir content in thummbar.
                    // See B.K.O #316752 for details.
                    openFolder(url.directory());
                    emit signalSetSelceted(url);
                    emit signalSetCurrentItem();
                }
                else
                {
                    emit signalNewThumbItem(url);
                    emit signalLastDirectory(it);
                    emit signalToggleNav(1);

                }
            }
        }
        else
        {
            //Remote File
            emit signalNewThumbItem(url);
            emit signalLastDirectory(it);
            emit signalToggleNav(1);

        }
    }

    if ( urlList.isEmpty() )
    {
        emit signalNoCurrentItem();
        emit signalToggleAction(false);
        emit signalToggleNav(0);
    }
}




void ShowfotoItemLoader::openFolder(const KUrl& url)
{
    qDebug()<< "I got the url";
    if (!url.isValid() || !url.isLocalFile())
    {
        return;
    }

    // Parse KDE image IO mime types registration to get files filter pattern.

    QStringList mimeTypes = KImageIO::mimeTypes(KImageIO::Reading);
    QString filter;

    for (QStringList::ConstIterator it = mimeTypes.constBegin() ; it != mimeTypes.constEnd() ; ++it)
    {
        QString format = KImageIO::typeForMime(*it).at(0).toUpper();
        filter.append ("*.");
        filter.append (format);
        filter.append (" ");
    }

    // Because KImageIO return only *.JPEG and *.TIFF mime types.
    if ( filter.contains("*.TIFF") )
    {
        filter.append (" *.TIF");
    }

    if ( filter.contains("*.JPEG") )
    {
        filter.append (" *.JPG");
        filter.append (" *.JPE");
    }

    // Added RAW files extensions supported by dcraw program and
    // defines to digikam/libs/dcraw/rawfiles.h
    filter.append (" ");
    filter.append ( QString(KDcrawIface::KDcraw::rawFiles()) );
    filter.append (" ");

    QString patterns = filter.toLower();
    patterns.append (" ");
    patterns.append (filter.toUpper());

    kDebug() << "patterns=" << patterns;


    // Get all image files from directory.

    QDir dir(url.toLocalFile(), patterns);
    dir.setFilter ( QDir::Files );
    d->dir = dir;

    if (!dir.exists())
    {
        return;
    }

    // Determine sort ordering for the entries from configuration setting:

    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(EditorWindow::CONFIG_GROUP_NAME);

    QDir::SortFlags flag;
    bool            reverse   = group.readEntry("ReverseSort", false);

    switch (group.readEntry("SortOrder", (int)SetupMisc::SortByDate))
    {
        case SetupMisc::SortByName:
        {
            flag = QDir::Name;  // Ordering by file name.

            if (reverse)
            {
                flag = flag | QDir::Reversed;
            }

            break;
        }
        case SetupMisc::SortByFileSize:
        {
            flag = QDir::Size;  // Ordering by file size.

            // Disabled reverse in the settings leads e.g. to increasing file sizes
            // Note, that this is just the opposite to the sort order for QDir.
            if (!reverse)
            {
                flag = flag | QDir::Reversed;
            }

            break;
        }
        default:
        {
            flag = QDir::Time;  // Ordering by file date.

            // Disabled reverse in the settings leads e.g. to increasing dates
            // Note, that this is just the opposite to the sort order for QDir.
            if (!reverse)
            {
                flag = flag | QDir::Reversed;
            }

            break;
        }
    }

    dir.setSorting(flag);

    QFileInfoList fileinfolist = dir.entryInfoList();

    if (fileinfolist.isEmpty())
    {
        emit signalSorry();
        return;
    }

    QFileInfoList::const_iterator fi;
    ShowfotoItemInfo iteminfo;
    int i = 0;

    // And open all items in image editor.

    for (fi = fileinfolist.constBegin(); fi != fileinfolist.constEnd(); ++fi)
    {
        emit signalNewThumbItem(KUrl(fi->filePath()));
        iteminfo.id = 1 + i;
        iteminfo.name = (*fi).fileName();
        iteminfo.mime = (*fi).suffix();
        iteminfo.size = (*fi).size();
        iteminfo.folder = (*fi).path();
        iteminfo.url = (*fi).filePath();
        d->InfoList.append(iteminfo);
        i++;
    }

    qDebug()<< "signal emmited with the list";
    emit signalInfoList(d->InfoList);

}


void ShowfotoItemLoader::slotOpenFolder(const KUrl &url)
{
    openFolder(url);
}

void ShowfotoItemLoader::slotOpenFile(const KUrl::List &urls)
{
    ShowfotoItemInfo iteminfo;
    int i = 0;
    for (KUrl::List::const_iterator it = urls.constBegin();
         it != urls.constEnd(); ++it)
    {
        emit signalNewThumbItem((*it));
        emit signalLastDirectory(it);
        QFileInfo fi((*it).toLocalFile());
        iteminfo.id = 1 + i;
        iteminfo.name = fi.fileName();
        iteminfo.mime = fi.suffix();
        iteminfo.size = fi.size();
        iteminfo.url  = fi.filePath();
        iteminfo.folder = fi.path();
        d->InfoList.append(iteminfo);
        i++;
    }
    emit signalInfoList(d->InfoList);

}




}//namespace Digikam
