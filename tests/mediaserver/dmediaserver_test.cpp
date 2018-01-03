/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-12-28
 * Description : stand alone test for DMediaServer
 *
 * Copyright (C) 2012-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes

#include <QString>
#include <QStringList>
#include <QApplication>
#include <QStandardPaths>
#include <QDir>
#include <QUrl>
#include <QMap>
#include <QDebug>
#include <QProgressDialog>

// Local includes

#include "dmediaservermngr.h"
#include "dfiledialog.h"

using namespace Digikam;

int main(int argc, char* argv[])
{
    QApplication   app(argc, argv);
    QList<QUrl>    list;
    MediaServerMap map;

    QDir().mkpath(QStandardPaths::writableLocation(QStandardPaths::DataLocation));
        
    if (argc <= 1)
    {
        QStringList files = DFileDialog::getOpenFileNames(0, QString::fromLatin1("Select Files to Share With Media Server"),
                                                          QStandardPaths::standardLocations(QStandardPaths::PicturesLocation).first(),
                                                          QLatin1String("Image Files (*.png *.jpg *.tif *.bmp *.gif)"));

        foreach(const QString& f, files)
        {
            list.append(QUrl::fromLocalFile(f));
        }
    }
    else
    {
        for (int i = 1 ; i < argc ; i++)
        {
            list.append(QUrl::fromLocalFile(QString::fromLocal8Bit(argv[i])));
        }
    }

    if (!list.isEmpty())
    {
        map.insert(QLatin1String("Test Collection"), list);
        DMediaServerMngr::instance()->setCollectionMap(map);
    }
    else
    {
        if (!DMediaServerMngr::instance()->load())
            return -1;
    }
 
    if (DMediaServerMngr::instance()->startMediaServer())
    {
        QProgressDialog* const pdlg = new QProgressDialog(0);
        pdlg->setLabelText(QLatin1String("Sharing files on the network"));
        pdlg->setMinimumDuration(0);
        pdlg->setCancelButtonText(QLatin1String("Close"));
        pdlg->setMaximum(0);
        pdlg->setMinimum(0);
        pdlg->setValue(0);
        pdlg->exec();
    }
    else
    {
        qDebug() << "Failed to start the Media Server...";
    }
    
    DMediaServerMngr::instance()->save();
    DMediaServerMngr::instance()->cleanUp();

    return 0;
}
