/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-06-16
 * Description : Face detection CLI tool
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
#include <QImage>
#include <QHBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QWidget>
#include <QDebug>

// Local includes

#include "facedetector.h"

using namespace Digikam;

void detectFaces(const QString& file)
{
    qDebug() << "Loading" << file;
    QImage img(file);
    qDebug() << "Detecting";
    FaceDetector detector;
    QList<QRectF> faces = detector.detectFaces(img);
    qDebug() << "Detected";

    if (faces.isEmpty())
    {
        qDebug() << "No faces found";
        return;
    }

    qDebug() << "Coordinates of detected faces : ";

    foreach(const QRectF& r, faces)
    {
        qDebug() << r;
    }

    QWidget* const mainWidget = new QWidget;
    mainWidget->setWindowTitle(file);
    QHBoxLayout* const layout = new QHBoxLayout(mainWidget);
    QLabel* const fullImage   = new QLabel;
    fullImage->setPixmap(QPixmap::fromImage(img.scaled(250, 250, Qt::KeepAspectRatio)));
    layout->addWidget(fullImage);

    foreach(const QRectF& rr, faces)
    {
        QLabel* const label = new QLabel;
        label->setScaledContents(false);
        QRect r             = FaceDetector::toAbsoluteRect(rr, img.size());
        QImage part         = img.copy(r);
        label->setPixmap(QPixmap::fromImage(part.scaled(200, 200, Qt::KeepAspectRatio)));
        layout->addWidget(label);
    }

    mainWidget->show();
    qApp->processEvents(); // dirty hack
}

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        qDebug() << "Bad Arguments!!!\nUsage: " << argv[0] << " <image1> <image2> ...";
        return 0;
    }

    QApplication app(argc, argv);

    for (int i = 1 ; i < argc ; i++)
    {
        detectFaces(QString::fromLocal8Bit(argv[i]));
    }

    app.exec();

    return 0;
}
