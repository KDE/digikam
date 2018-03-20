/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2014-06-16
 * Description : CLI test program for training database
 *
 * Copyright (C) 2014-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <QApplication>
#include <QDir>
#include <QImage>
#include <QThreadPool>
#include <QRunnable>
#include <QDebug>

// KDE includes

#include <ksharedconfig.h>

// Local includes

#include "recognitiondatabase.h"
#include "coredbaccess.h"
#include "dbengineparameters.h"

using namespace Digikam;



// --------------------------------------------------------------------------------------------------

const int firstMultiplier  = 20;
const int secondMultiplier = 20;

class Runnable : public QRunnable
{
public:

    Runnable(int number, RecognitionDatabase db)
        : number(number), db(db)
    {
    }

    virtual void run()
    {
        QImage image(256, 256, QImage::Format_ARGB32);
        image.fill(Qt::red);

        Identity identity;

        // Populate database.

        for (int i = number * secondMultiplier ; i < number * secondMultiplier + secondMultiplier; i++)
        {
            QString name      = QString::fromLatin1("face%1").arg(i);
            qDebug() << "Record Identity " << name << " to DB";
            QMap<QString, QString> attributes;
            attributes[QString::fromLatin1("name")] = name;
            identity                                = db.addIdentity(attributes);
            db.train(identity, image, QString::fromLatin1("test application"));
        }

        qDebug() << "Trained group" << number;

        // Check records in database.

        for (int i=number*secondMultiplier ; i < number*secondMultiplier+secondMultiplier; i++)
        {
            QString name = QString::fromLatin1("face%1").arg(i);
            identity     = db.findIdentity(QString::fromLatin1("name"), name);

            if (!identity.isNull())
            {
                qDebug() << "Identity " << name << " is present in DB";
            }
            else
            {
                qDebug() << "Identity " << name << " is absent in DB";
            }
        }
    }

    const int number;
    RecognitionDatabase db;
};

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    app.setApplicationName(QString::fromLatin1("digikam"));          // for DB init.
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    DbEngineParameters prm    = DbEngineParameters::parametersFromConfig(config);
    CoreDbAccess::setParameters(prm, CoreDbAccess::MainApplication);
    RecognitionDatabase db;

    QThreadPool pool;
    pool.setMaxThreadCount(101);

    for (int i = 0 ; i < firstMultiplier ; i++)
    {
        Runnable* r= new Runnable(i, db);
        pool.start(r);
    }

    pool.waitForDone();

    // Process recognition in database.

    QImage image(256, 256, QImage::Format_ARGB32);
    QList<Identity> list = db.recognizeFaces(QList<QImage>() << image);

    if (!list.empty())
    {
        foreach(Identity id, list)
        {
            qDebug() << "Identity " << id.attribute(QString::fromLatin1("name")) << " recognized";
        }
    }
    else
    {
        qDebug() << "No Identity recognized from DB";
    }

    return 0;
}
