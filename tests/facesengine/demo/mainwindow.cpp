/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-06-21
 * Description : GUI test program for FacesEngine
 *
 * Copyright (C) 2010-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C)      2010 by Alex Jironkin <alexjironkin at gmail dot com>
 * Copyright (C)      2010 by Aditya Bhatt <adityabhatt1991 at gmail dot com>
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

#include "mainwindow.h"
#include "ui_mainwindow.h"

// Qt includes

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QTime>
#include <QDebug>
#include <QStandardPaths>

// Local includes

#include "recognitiondatabase.h"
#include "facedetector.h"
#include "faceitem.h"
#include "dfiledialog.h"

using namespace std;
using namespace Digikam;

// --------------------------------------------------------------------------------------------------

class MainWindow::Private
{
public:

    Private()
    {
        ui            = 0;
        myScene       = 0;
        myView        = 0;
        lastPhotoItem = 0;
        detector      = 0;
        scale         = 0.0;
    }

    Ui::MainWindow*      ui;
    QGraphicsScene*      myScene;
    QGraphicsView*       myView;
    QGraphicsPixmapItem* lastPhotoItem;
    QList<FaceItem*>     faceitems;

    RecognitionDatabase  database;
    FaceDetector*        detector;
    QImage               currentPhoto;
    double               scale;
    QString              lastFileOpenPath;
};

MainWindow::MainWindow(QWidget* const parent)
    : QMainWindow(parent),
      d(new Private)
{
    d->ui = new Ui::MainWindow;
    d->ui->setupUi(this);
    d->ui->recogniseBtn->setEnabled(false);
    d->ui->updateDatabaseBtn->setEnabled(false);
    d->ui->detectFacesBtn->setEnabled(false);
    d->ui->configLocation->setReadOnly(true);

    connect(d->ui->openImageBtn, SIGNAL(clicked()),
            this, SLOT(slotOpenImage()));

    connect(d->ui->accuracySlider, SIGNAL(valueChanged(int)),
            this, SLOT(slotUpdateAccuracy()));

    connect(d->ui->sensitivitySlider, SIGNAL(valueChanged(int)),
            this, SLOT(slotUpdateSensitivity()));

    connect(d->ui->detectFacesBtn, SIGNAL(clicked()),
            this, SLOT(slotDetectFaces()));

    connect(d->ui->recogniseBtn, SIGNAL(clicked()),
            this, SLOT(slotRecognise()));

    connect(d->ui->updateDatabaseBtn, SIGNAL(clicked()),
            this, SLOT(slotUpdateDatabase()));

    d->myScene                = new QGraphicsScene();
    QGridLayout* const layout = new QGridLayout;
    d->myView                 = new QGraphicsView(d->myScene);

    d->myView->setCacheMode(QGraphicsView::CacheBackground);
    d->myScene->setItemIndexMethod(QGraphicsScene::NoIndex);

    setMouseTracking(true);
    layout->addWidget(d->myView);

    d->ui->widget->setLayout(layout);

    d->myView->show();

    d->detector = new FaceDetector();

    d->ui->accuracySlider->setValue(80);
    d->ui->sensitivitySlider->setValue(80);

    d->lastFileOpenPath = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation).first();
}

MainWindow::~MainWindow()
{
    delete d->ui;
    delete d->detector;
    delete d;
}

void MainWindow::changeEvent(QEvent* e)
{
    QMainWindow::changeEvent(e);

    switch (e->type())
    {
        case QEvent::LanguageChange:
            d->ui->retranslateUi(this);
            break;
        default:
            break;
    }
}

void MainWindow::clearScene()
{
    QList<QGraphicsItem*> list = d->myScene->items();

    for(int i = 0 ; i < list.size() ; i++)
    {
        d->myScene->removeItem(list.at(i));
    }
}

void MainWindow::slotOpenImage()
{
    QString file = DFileDialog::getOpenFileName(this, QLatin1String("Select Image to Open"),
                                                d->lastFileOpenPath,
                                                QString::fromLatin1("Image Files (*.png *.jpg *.bmp *.pgm)"));

    if (file.isEmpty())
        return;

    d->lastFileOpenPath = QFileInfo(file).absolutePath();

    clearScene();

    qDebug() << "Opened file " << file;

    d->currentPhoto.load(file);
    d->lastPhotoItem = new QGraphicsPixmapItem(QPixmap::fromImage(d->currentPhoto));

    if (1.0 * d->ui->widget->width() / d->currentPhoto.width() < 1.0 * d->ui->widget->height() / d->currentPhoto.height())
    {
        d->scale = 1.0 * d->ui->widget->width() / d->currentPhoto.width();
    }
    else
    {
        d->scale = 1.0 * d->ui->widget->height() / d->currentPhoto.height();
    }

    d->lastPhotoItem->setScale(d->scale);

    d->myScene->addItem(d->lastPhotoItem);
    d->ui->detectFacesBtn->setEnabled(true);
}

void MainWindow::slotDetectFaces()
{
    setCursor(Qt::WaitCursor);

    QList<QRectF> currentFaces = d->detector->detectFaces(d->currentPhoto);

    qDebug() << "FacesEngine detected : " << currentFaces.size() << " faces.";
    qDebug() << "Coordinates of detected faces : ";

    foreach(const QRectF& r, currentFaces)
    {
        qDebug() << r;
    }

    foreach(FaceItem* const item, d->faceitems)
    {
        item->setVisible(false);
    }

    d->faceitems.clear();

    for(int i = 0 ; i < currentFaces.size() ; ++i)
    {
        QRect face = d->detector->toAbsoluteRect(currentFaces[i], d->currentPhoto.size());
        d->faceitems.append(new FaceItem(0, d->myScene, face, d->scale));
        qDebug() << face;
    }

    d->ui->recogniseBtn->setEnabled(true);
    d->ui->updateDatabaseBtn->setEnabled(true);

    unsetCursor();
}

void MainWindow::slotUpdateAccuracy()
{
    int value = d->ui->accuracySlider->value();
    d->detector->setParameter(QString::fromLatin1("accuracy"), value/100.0);
}

void MainWindow::slotUpdateSensitivity()
{
    int value = d->ui->sensitivitySlider->value();
    d->detector->setParameter(QString::fromLatin1("sensitivity"), value);
}

void MainWindow::slotRecognise()
{
    setCursor(Qt::WaitCursor);

    int i = 0;

    foreach(FaceItem* const item, d->faceitems)
    {
        QTime time;
        time.start();
        Identity identity = d->database.recognizeFace(d->currentPhoto.copy(item->originalRect()));
        int elapsed       = time.elapsed();

        qDebug() << "Recognition took " << elapsed << " for Face #" << i+1;

        if (!identity.isNull())
        {
            item->suggest(identity.attribute(QString::fromLatin1("name")));

            qDebug() << "Face #" << i+1 << " is closest to the person with ID " << identity.id()
                     << " and name "<< identity.attribute(QString::fromLatin1("name"));
        }
        else
        {
            qDebug() << "Face #" << i+1 << " : no Identity match from database.";
        }

        i++;
    }

    unsetCursor();
}

void MainWindow::slotUpdateDatabase()
{
    setCursor(Qt::WaitCursor);

    int i = 0;

    foreach(FaceItem* const item, d->faceitems)
    {
        if (item->text() != QString::fromLatin1("?"))
        {
            QTime time;
            time.start();

            QString name = item->text();
            qDebug() << "Face #" << i+1 << ": training name '" << name << "'";

            Identity identity = d->database.findIdentity(QString::fromLatin1("name"), name);

            if (identity.isNull())
            {
                QMap<QString, QString> attributes;
                attributes[QString::fromLatin1("name")] = name;
                identity                                = d->database.addIdentity(attributes);
                qDebug() << "Adding new identity ID " << identity.id() << " to database for name " << name;
            }
            else
            {
                qDebug() << "Found existing identity ID " << identity.id() << " from database for name " << name;
            }

            d->database.train(identity, d->currentPhoto.copy(item->originalRect()), QString::fromLatin1("test application"));

            int elapsed = time.elapsed();

            qDebug() << "Training took " << elapsed << " for Face #" << i+1;
        }

        i++;
    }

    unsetCursor();
}
