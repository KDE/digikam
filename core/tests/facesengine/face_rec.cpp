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
#include <QRectF>
#include <QList>

#include <cassert>

// Local includes

#include "dimg.h"
#include "facescansettings.h"
#include "facedetector.h"
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
                     double ratio)
{
    for (unsigned i = 1 ; i <= nbOfObjects ; ++i)
    {
        for (unsigned j = 1 ; j <= nbOfSamples ; ++j)
        {
            QString path = orlDir + QString::fromLatin1("s%1/%2.pgm").arg(i).arg(j);

            if (j <= qRound(nbOfSamples * ratio))
            {
                trainingset[i] << path;
                qDebug() << path;
            }
            else
            {
                testset[i] << path;
                qDebug() << path;
            }
        }
    }    
}

QImage scaleForDetection(const DImg& image, FaceDetector detector)
{
    int recommendedSize = detector.recommendedImageSize(image.size());

    if (qMax(image.width(), image.height()) > (uint)recommendedSize)
    {
        return image.smoothScale(recommendedSize, recommendedSize, Qt::KeepAspectRatio).copyQImage();
    }

    return image.copyQImage();
}

QList<QRectF> processFaceDetection(const DImg& image, FaceDetector detector)
{
    QImage detectionImage = scaleForDetection(image, detector);
    QList<QRectF> detectedFaces = detector.detectFaces(detectionImage, image.originalSize());

    qDebug() << "Found " << detectedFaces.size() << " faces";

    if(detectedFaces.size() == 0) // Found no face
    {
        detectedFaces << QRectF(QPointF(0,0), image.originalSize());
    }

    return detectedFaces;
}

QImage retrieveFace(const DImg& image, const QList<QRectF>& rects)
{
    if(rects.size() > 1)
    {
        qWarning() << "More than 1 face found in image, strange for our test set!!!";
        assert(0);
    }

    QRectF rect = rects.first();
    QImage face = image.copyQImage(rect);
    
    return face;
}

// --------------------------------------------------------------------------------------------------

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);
    app.setApplicationName(QString::fromLatin1("digikam"));          // for DB init.

    // Options for commandline parser

    QCommandLineParser parser;
    parser.addOption(QCommandLineOption(QLatin1String("db"), QLatin1String("Faces database"), QLatin1String("path to db folder")));
    parser.addOption(QCommandLineOption(QLatin1String("rs"), QLatin1String("Split ratio (test set / whole set)"), QLatin1String("decimal")));
    parser.addOption(QCommandLineOption(QLatin1String("ts"), QLatin1String("Test set folder"), QLatin1String("path relative to db folder")));
    parser.addOption(QCommandLineOption(QLatin1String("ds"), QLatin1String("Training set (dev set) folder"), QLatin1String("path relative to db folder")));
    parser.addOption(QCommandLineOption(QLatin1String("ni"), QLatin1String("Number of total objects"), QLatin1String("nbIdentities")));
    parser.addOption(QCommandLineOption(QLatin1String("ns"), QLatin1String("Number of samples per object"), QLatin1String("nbSamples")));
    parser.addHelpOption();
    parser.process(app);

    // Parse arguments

    if(parser.optionNames().empty())
    {
        parser.showHelp();
        return 0;
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
    double ratio = 0;
    if(parser.isSet(QLatin1String("rs")))
    {
        ratio = parser.value(QLatin1String("rs")).toDouble();
    }

    QString facedb = parser.value(QLatin1String("db"));

    // Init config for digiKam

    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    DbEngineParameters prm    = DbEngineParameters::parametersFromConfig(config);
    CoreDbAccess::setParameters(prm, CoreDbAccess::MainApplication);
    RecognitionDatabase db;
    db.activeFaceRecognizer(RecognitionDatabase::RecognizeAlgorithm::DNN);

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
    if(ratio > 0)
    {
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

    // Init FaceDetector used for detecting faces and bounding box
    // before recognizing

    FaceDetector detector;

    // Start timing for benchmark training

    QTime time;
    time.start();

    // Evaluation metrics
    unsigned correct = 0, notRecognized = 0, falsePositive = 0, totalTrained = 0, totalRecognized = 0;
    unsigned elapsedTraining = 0, elapsedTesting = 0;

    for (QMap<unsigned, QStringList>::const_iterator it = trainingset.constBegin() ;
         it != trainingset.constEnd() ; ++it)
    {
        Identity identity = db.findIdentity(QString::fromLatin1("name"), QString::number(it.key()));

        if (identity.isNull())
        {
            qDebug() << "Identity management failed for person " << it.key();
        }

        QList<QImage> rawImages = toImages(it.value());
        qDebug() << "Training directory " << it.key();

        foreach(const QImage& img, rawImages)
        {
            DImg image(img);
            QList<QRectF> detectedFaces = processFaceDetection(image, detector);
            QImage face = retrieveFace(image, detectedFaces);

            db.train(identity, face, trainingContext);
            totalTrained++;
        }
    }

    elapsedTraining = time.restart();

    for (QMap<unsigned, QStringList>::const_iterator it = testset.constBegin() ;
         it != testset.constEnd() ; ++it)
    {
        Identity identity       = idMap.value(it.key());
        QList<QImage> rawImages = toImages(it.value());

        QList<Identity> results;
        foreach(const QImage& img, rawImages)
        {
            DImg image(img);
            QList<QRectF> detectedFaces = processFaceDetection(image, detector);
            QImage face = retrieveFace(image, detectedFaces);

            results << db.recognizeFace(face);
            totalRecognized++;
        }

        // QList<Identity> results = db.recognizeFaces(images);

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

        // totalRecognized += images.size();
    }

    elapsedTesting = time.elapsed();

    if (totalTrained)
    {
        qDebug() << "Training " << totalTrained << "of " << nbOfIdentities << " different objects took " << elapsedTraining << " ms, " << ((float)elapsedTraining/totalTrained) << " ms per image";
    }

    if (totalRecognized)
    {
        qDebug() << "Recognition test performed on " << totalRecognized << " of " << nbOfIdentities << " different objects took " << elapsedTesting << " ms, " << ((float)elapsedTesting/totalRecognized) << " ms per image";
        qDebug() << correct       << " / " << totalRecognized << " (" << (float(correct) / totalRecognized*100) << "%) were correctly recognized";
        qDebug() << falsePositive << " / " << totalRecognized << " (" << (float(falsePositive) / totalRecognized*100) << "%) were falsely assigned to an identity (false positive)";
        qDebug() << notRecognized << " / " << totalRecognized << " (" << (float(notRecognized) / totalRecognized*100) << "%) were not recognized";
    }
    else
    {
        qDebug() << "No face recognized";
    }

    return 0;

}
