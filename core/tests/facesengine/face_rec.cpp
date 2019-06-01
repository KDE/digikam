/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2019-05-15
 * Description : CLI tool to test and verify Face Recognition
 *               NOTE: This tool integrates the whole Face Management
 *               work flow, especially designed to verify and benchmark
 *               Face Recognition algorithm. It is adapted from recognize.cpp
 *               developped by Aditya Bhatt. It uses ORL database, see 
 *               here for details:
 *               http://www.cl.cam.ac.uk/research/dtg/attarchive/facedatabase.html
 *
 * Copyright (C) 2019 by Thanh Trung Dinh <dinhthanhtrung1996 at gmail dot com>
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
#include <QCommandLineParser>

// Local includes

#include "recognitiondatabase.h"
#include "coredbaccess.h"
#include "dbengineparameters.h"

using namespace Digikam;


// --------------------------------------------------------------------------------------------------

QStringList toPaths(char** argv, int startIndex, int argc)
{
    QStringList files;

    for (int i = startIndex ; i < argc ; ++i)
    {
        files << QString::fromLatin1(argv[i]);
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

void prepareForTrain(QString orlDir,
                     QMap<unsigned, QStringList>& testset, QMap<unsigned, QStringList>& trainingset,
                     unsigned nbOfSamples, unsigned nbOfObjects,
                     float ratio)
{
    for (unsigned i = 1 ; i <= nbOfObjects ; ++i)
    {
        for (unsigned j = 1 ; j <= nbOfSamples ; ++j)
        {
            QString path = orlDir + QString::fromLatin1("s%1/%2.pgm").arg(i).arg(j);

            if (j <= nbOfSamples * ratio)
            {
                trainingset[i] << path;
            }
            else
            {
                testset[i] << path;
            }
        }
    }    
}

// --------------------------------------------------------------------------------------------------

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);
    app.setApplicationName(QString::fromLatin1("digikam"));          // for DB init.

    QCommandLineParser parser;
    parser.addOption(QCommandLineOption(QLatin1String("db"), QLatin1String("Faces database"), QLatin1String("path to db folder")));
    parser.addOption(QCommandLineOption(QLatin1String("rs"), QLatin1String("Split ratio (test set / whole set)"), QLatin1String("decimal")));
    parser.addOption(QCommandLineOption(QLatin1String("ts"), QLatin1String("Test set folder"), QLatin1String("path relative to db folder")));
    parser.addOption(QCommandLineOption(QLatin1String("ds"), QLatin1String("Training set (dev set) folder"), QLatin1String("path relative to db folder")));
    parser.addOption(QCommandLineOption(QLatin1String("ni"), QLatin1String("Number of total objects"), QLatin1String("nbIdentities")));
    parser.addOption(QCommandLineOption(QLatin1String("ns"), QLatin1String("Number of samples per object"), QLatin1String("nbSamples")));
    parser.addHelpOption();
    parser.process(app);

    bool monotholicDB = false;

    if(parser.optionNames().empty())
    {
        parser.showHelp();
        return 0;
    }
    else if(parser.isSet(QLatin1String("rs")))
    {
        monotholicDB = true;
    }
    else if(parser.isSet(QLatin1String("ts")))
    {
        if(!parser.isSet(QLatin1String("ds")))
        {
            qWarning() << "Dev set missing!!!";
            return 0;
        }
    }
    else if(parser.isSet(QLatin1String("ds")))
    {
        qWarning() << "Test set missing!!!";
        return 0;
    }

    unsigned nbOfSamples = parser.value(QLatin1String("ns")).toUInt();
    unsigned nbOfIdentities = parser.value(QLatin1String("ni")).toUInt();

    qDebug() << parser.value(QLatin1String("ns")) << ", " << nbOfIdentities;

    // Init config for digiKam
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    DbEngineParameters prm    = DbEngineParameters::parametersFromConfig(config);
    CoreDbAccess::setParameters(prm, CoreDbAccess::MainApplication);
    RecognitionDatabase db;

    // Create new IDs, clear existed IDs
    const QString trainingContext = QLatin1String("test_facerec");
    QMap<unsigned, Identity> idMap;
    QList<Identity> existedIDs;
    for (unsigned i = 1 ; i <= nbOfIdentities; ++i)
    {
        QMap<QString, QString> attributes;
        attributes[QString::fromLatin1("name")] = QString::number(i);
        Identity identity                       = db.findIdentity(attributes);

        if (identity.isNull())
        {
            Identity identity = db.addIdentity(attributes);
            idMap[i]          = identity;
            qDebug() << "Created identity " << identity.id();
        }
        else
        {
            qDebug() << "Already have identity " << i << ", clearing training data";
            idMap[i] = identity;
            existedIDs << identity;
        }
    }
    db.clearTraining(existedIDs, trainingContext);

    // Create training set, test set
    QMap<unsigned, QStringList> testset, trainingset;
    if(monotholicDB)
    {
        float ratio = parser.value(QLatin1String("rs")).toFloat();

        prepareForTrain(facedb, 
                        testset, trainingset, 
                        nbOfSamples, nbOfIdentities,
                        ratio);

    }
    else
    {
        QString testsetFolder = parser.value(QLatin1String("ts"));
        QString trainingsetFoler = parser.value(QLatin1String("ds"));

        // TODO: Overload of prepareForTrain() to create training set and test set here
    }

    // Start timing for benchmark training
    QTime time;
    time.start();

    // Evaluation metrics
    unsigned correct = 0, notRecognized = 0, falsePositive = 0, totalTrained = 0, totalRecognized = 0, elapsed = 0;

    for (QMap<unsigned, QStringList>::const_iterator it = trainingset.constBegin() ;
         it != trainingset.constEnd() ; ++it)
    {
        Identity identity = db.findIdentity(QString::fromLatin1("name"), QString::number(it.key()));

        if (identity.isNull())
        {
            qDebug() << "Identity management failed for person " << it.key();
        }

        QList<QImage> images = toImages(it.value());
        qDebug() << "Training directory " << it.key();

        db.activeFaceRecognizer(RecognitionDatabase::RecognizeAlgorithm::DNN);
        db.train(identity, images, trainingContext);
        totalTrained        += images.size();
    }

    elapsed = time.restart();

    if (totalTrained)
    {
        qDebug() << "Training " << trainingset.size() << "/" << nbOfIdentities << "of objects took " << elapsed << " ms, " << ((float)elapsed/totalTrained) << " ms per image";
    }

    for (QMap<unsigned, QStringList>::const_iterator it = testset.constBegin() ;
         it != testset.constEnd() ; ++it)
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
        qDebug() << "Recognition " << trainingset.size() << "/" << nbOfIdentities << "of objects took " << elapsed << " ms, " << ((float)elapsed/totalRecognized) << " ms per image";
        qDebug() << correct       << " of " << nbOfSamples*nbOfIdentities << " (" << (float(correct) / totalRecognized*100) << "%) were correctly recognized";
        qDebug() << falsePositive << " of " << nbOfSamples*nbOfIdentities << " (" << (float(falsePositive) / totalRecognized*100) << "%) were falsely assigned to an identity (false positive)";
        qDebug() << notRecognized << " of " << nbOfSamples*nbOfIdentities << " (" << (float(notRecognized) / totalRecognized*100) << "%) were not recognized";
    }
    else
    {
        qDebug() << "No face recognized";
    }

    return 0;

}
