/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-06-16
 * Description : Face Recognition CLI tool
 *               NOTE: This tool is able to use ORL database which are
 *               freely available set of images to test face recognition.
 *               It contain 10 photos of 20 different peoples from slightly
 *               different angles. See here for details:
 *               http://www.cl.cam.ac.uk/research/dtg/attarchive/facedatabase.html
 *
 * Copyright (C) 2010 by Aditya Bhatt <adityabhatt1991 at gmail dot com>
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
#include <QTime>
#include <QDebug>

// Local includes

#include "recognitiondatabase.h"
#include "coredbaccess.h"
#include "dbengineparameters.h"

using namespace Digikam;


// --------------------------------------------------------------------------------------------------

QStringList toPaths(char** argv, int startIndex, int argc)
{
    QStringList files;

    for (int i = startIndex ; i < argc ; i++)
    {
        files << QString::fromLocal8Bit(argv[i]);
    }

    return files;
}

QList<QImage> toImages(const QStringList& paths)
{
    QList<QImage> images;

    foreach (const QString& path, paths)
    {
        images << QImage(path);
    }

    return images;
}

// --------------------------------------------------------------------------------------------------

int main(int argc, char** argv)
{
    if (argc < 2 || (QString::fromLatin1(argv[1]) == QString::fromLatin1("train") && argc < 3))
    {
        qDebug() << "Bad Arguments!!!\nUsage: " << argv[0] << " identify <image1> <image2> ... | train name <image1> <image2> ... "
                                                              "| ORL <path to orl_faces>";
        return 0;
    }

    QCoreApplication app(argc, argv);
    app.setApplicationName(QString::fromLatin1("digikam"));          // for DB init.
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    DbEngineParameters prm    = DbEngineParameters::parametersFromConfig(config);
    CoreDbAccess::setParameters(prm, CoreDbAccess::MainApplication);
    RecognitionDatabase db;

    if (QString::fromLatin1(argv[1]) == QString::fromLatin1("identify"))
    {
        QStringList paths    = toPaths(argv, 2, argc);
        QList<QImage> images = toImages(paths);

        QTime time;
        time.start();
        QList<Identity> identities = db.recognizeFaces(images);
        int elapsed                = time.elapsed();

        qDebug() << "Recognition took " << elapsed << " for " << images.size() << ", " << ((float)elapsed/images.size()) << " per image";

        for (int i = 0 ; i < paths.size() ; i++)
        {
            qDebug() << "Identified " << identities[i].attribute(QString::fromLatin1("name")) << " in " << paths[i];
        }
    }
    else if (QString::fromLatin1(argv[1]) == QString::fromLatin1("train"))
    {
        QString name = QString::fromLocal8Bit(argv[2]);
        qDebug() << "Training " << name;

        QStringList paths    = toPaths(argv, 3, argc);
        QList<QImage> images = toImages(paths);
        Identity identity    = db.findIdentity(QString::fromLatin1("name"), name);

        if (identity.isNull())
        {
            qDebug() << "Adding new identity to database for name " << name;
            QMap<QString, QString> attributes;
            attributes[QString::fromLatin1("name")] = name;
            identity                                = db.addIdentity(attributes);
        }

        QTime time;
        time.start();

        db.train(identity, images, QString::fromLatin1("test application"));

        int elapsed = time.elapsed();
        qDebug() << "Training took " << elapsed << " for " << images.size() << ", " << ((float)elapsed/images.size()) << " per image";
    }
    else if (QString::fromLatin1(argv[1]) == QString::fromLatin1("orl"))
    {
        QString orlPath = QString::fromLocal8Bit(argv[2]);

        if (orlPath.isEmpty())
        {
            orlPath = QString::fromLatin1("orl_faces"); // relative to current dir
        }

        QDir orlDir(orlPath);

        if (!orlDir.exists())
        {
            qDebug() << "Cannot find orl_faces directory";
            return 0;
        }

        const int OrlIdentities       = 40;
        const int OrlSamples          = 10;
        const QString trainingContext = QString::fromLatin1("test application");

        QMap<int, Identity> idMap;
        QList<Identity> trainingToBeCleared;

        for (int i = 1 ; i <= OrlIdentities ; i++)
        {
            QMap<QString, QString> attributes;
            attributes[QString::fromLatin1("name")] = QString::number(i);
            Identity identity                       = db.findIdentity(attributes);

            if (identity.isNull())
            {
                Identity identity = db.addIdentity(attributes);
                idMap[i]          = identity;
                qDebug() << "Created identity " << identity.id() << " for ORL directory " << i;
            }
            else
            {
                qDebug() << "Already have identity for ORL directory " << i << ", clearing training data";
                idMap[i] = identity;
                trainingToBeCleared << identity;
            }
        }

        db.clearTraining(trainingToBeCleared, trainingContext);
        QMap<int, QStringList> trainingImages, recognitionImages;

        for (int i=1 ; i <= OrlIdentities ; i++)
        {
            for (int j = 1 ; j <= OrlSamples ; j++)
            {
                QString path = orlDir.path() + QString::fromLatin1("/s%1/%2.pgm").arg(i).arg(j);

                if (j <= OrlSamples/2)
                {
                    trainingImages[i] << path;
                }
                else
                {
                    recognitionImages[i] << path;
                }
            }
        }

        if (!QFileInfo(trainingImages.value(1).first()).exists())
        {
            qDebug() << "Could not find files of ORL database";
            return 0;
        }

        QTime time;
        time.start();

        int correct = 0, notRecognized = 0, falsePositive = 0, totalTrained = 0, totalRecognized = 0, elapsed = 0;

        for (QMap<int, QStringList>::const_iterator it = trainingImages.constBegin() ; it != trainingImages.constEnd() ; ++it)
        {
            Identity identity = db.findIdentity(QString::fromLatin1("name"), QString::number(it.key()));

            if (identity.isNull())
            {
                qDebug() << "Identity management failed for ORL person " << it.key();
            }

            QList<QImage> images = toImages(it.value());
            qDebug() << "Training ORL directory " << it.key();
            db.train(identity, images, trainingContext);
            totalTrained        += images.size();
        }

        elapsed = time.restart();

        if (totalTrained)
        {
            qDebug() << "Training 5/10 or ORL took " << elapsed << " ms, " << ((float)elapsed/totalTrained) << " ms per image";
        }

        for (QMap<int, QStringList>::const_iterator it = recognitionImages.constBegin() ; it != recognitionImages.constEnd() ; ++it)
        {
            Identity identity       = idMap.value(it.key());
            QList<QImage> images    = toImages(it.value());
            QList<Identity> results = db.recognizeFaces(images);

            qDebug() << "Result for " << it.value().first() << " is identity " << results.first().id();

            foreach (const Identity& foundId, results)
            {
                if (foundId.isNull())
                {
                    notRecognized++;
                }
                else if (foundId == identity)
                {
                    correct++;
                }
                else
                {
                    falsePositive++;
                }
            }

            totalRecognized += images.size();
        }

        elapsed = time.elapsed();

        if (totalRecognized)
        {
            qDebug() << "Recognition of 5/10 or ORL took " << elapsed << " ms, " << ((float)elapsed/totalRecognized) << " ms per image";
            qDebug() << correct       << " of 200 (" << (float(correct)       / totalRecognized*100) << "%) were correctly recognized";
            qDebug() << falsePositive << " of 200 (" << (float(falsePositive) / totalRecognized*100) << "%) were falsely assigned to an identity";
            qDebug() << notRecognized << " of 200 (" << (float(notRecognized) / totalRecognized*100) << "%) were not recognized";
        }
        else
        {
            qDebug() << "No face recognized";
        }
    }

    return 0;
}
