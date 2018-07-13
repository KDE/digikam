/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-11-22
 * Description : stand alone digiKam image editor - Import tools
 *
 * Copyright (C) 2004-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
    
void ShowFoto::slotImportFromScanner()
{
#ifdef HAVE_KSANE

    QString place    = QDir::homePath();
    QStringList pics = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);

    if (!pics.isEmpty())
        place = pics.first();

    QUrl trg = saveDestinationUrl();

    if (!trg.isEmpty())
    {
        QString path = trg.adjusted(QUrl::RemoveFilename).toLocalFile();

        if (!path.isEmpty())
            place = path;
    }

    m_ksaneAction->activate(place, configGroupName());

    connect(m_ksaneAction, SIGNAL(signalImportedImage(QUrl)),
            this, SLOT(slotImportedImagefromScanner(QUrl)));
#endif
}

void ShowFoto::slotImportedImagefromScanner(const QUrl& url)
{
    openUrls(QList<QUrl>() << url);
}

void ShowFoto::slotImportTool()
{
    QAction* const tool = dynamic_cast<QAction*>(sender());

    if (tool == m_importGphotoAction)
    {
        QPointer<GSWindow> w = new GSWindow(new DMetaInfoIface(this, QList<QUrl>() << d->thumbBar->currentUrl()),
                   this, QLatin1String("googlephotoimport"));
        w->exec();
        delete w;
    }
    else if (tool == m_importSmugmugAction)
    {
        QPointer<SmugWindow> w = new SmugWindow(new DMetaInfoIface(this, QList<QUrl>() << d->thumbBar->currentUrl()),
                     this, true);
        w->exec();
        delete w;
    }

#ifdef HAVE_KIO
    else if (tool == m_importFileTransferAction)
    {
        QPointer<FTImportWindow> w = new FTImportWindow(new DMetaInfoIface(this, QList<QUrl>() << d->thumbBar->currentUrl()),
                         this);
        w->exec();
        delete w;
    }
#endif
}

} // namespace ShowFoto
