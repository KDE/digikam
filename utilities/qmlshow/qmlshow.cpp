/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-04-21
 * Description : slide show tool using preview of pictures.
 *
 * Copyright (C) 2005-2011 by Dhruv Patel <dhruvkumarr dot patel51 at gmail dot com>
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

#include "qmlshow.moc"

// C++ includes

#include <stdio.h>

// Qt includes

#include <QtGui>
#include <QString>
#include <QtDeclarative/QDeclarativeView>
#include <QDeclarativeEngine>
#include <QGraphicsObject>
#include <QDeclarativeContext>
#include <QDeclarativeView>

// KDE includes

#include <kstandarddirs.h>

// Local includes

#include "imageinfo.h"
#include "imageinfolist.h"
#include "metadatahub.h"

namespace Digikam
{

class QmlShow::QmlShowPriv
{
public:

    QmlShowPriv()
        : ui(0),
          imageno(0),
          list(0)
    {
    }

    QDeclarativeView* ui;
    int               imageno;
    int               screen_height;
    int               screen_width;
    ImageInfoList*    list;
    SlideShowSettings settings;
};

QmlShow::QmlShow(const ImageInfoList& list, const SlideShowSettings& settings)
    : QMainWindow(0, Qt::FramelessWindowHint),
      d(new QmlShowPriv)
{
    d->ui                     = new QDeclarativeView;
    d->list                   = new ImageInfoList(list);
    d->settings               = settings;
    QDeclarativeContext* ctxt = d->ui->rootContext();

//    ctxt->setContextProperty("myModel", QVariant::fromValue(d->list));

    QStringList nameslist;
    foreach (const ImageInfo& info, list)
    {
        nameslist << info.filePath();
    }
    ctxt->setContextProperty("myModel", QVariant::fromValue(nameslist));

    // FIXME: Use KStandardDirs and install qml file properly (see data/database/dbconfig.xml as example)
    QString dir(KStandardDirs::installPath("data") + QString("digikam/qmlshow/qmlview.qml"));
    d->ui->setSource(QUrl::fromLocalFile(QFile::encodeName(dir).data()));
    setCentralWidget(d->ui);
    d->screen_height = qApp->desktop()->screenGeometry(-1).height();
    d->screen_width  = qApp->desktop()->screenGeometry(-1).width();

    setAttribute(Qt::WA_DeleteOnClose,true);

    connect(d->ui->engine(), SIGNAL(quit()),
            this, SLOT(close()));

    //d->ui->setResizeMode(QDeclarativeView::SizeRootObjectToView);

    QObject* object = d->ui->rootObject();

    connect(object, SIGNAL(nextClicked()),
            this, SLOT(nextImage()));

    connect(object, SIGNAL(prevClicked()),
            this, SLOT(prevImage()));

    connect(object, SIGNAL(play()),
            this, SLOT(play()));

    connect(object, SIGNAL(pause()),
            this, SLOT(pause()));

    connect(object, SIGNAL(gridChanged(int)),
            this, SLOT(changePicture(int)));

    connect(object, SIGNAL(loadMetaData()),
            this, SLOT(setMetaData()));

    d->imageno = 0;
    d->ui->show();
    d->ui->setResizeMode(QDeclarativeView::SizeRootObjectToView);
    showMaximized();

    if(!list.isEmpty())
    {
        changePicture(d->imageno);
    }
    else
    {
        close();
    }
}

QmlShow::~QmlShow()
{
    delete d->ui;
    delete d->list;
    delete d;
}

void QmlShow::nextImage()
{
    if (d->imageno == (d->list->count()-1))
    {
        return;
    }

    d->imageno += 1;
    changePicture(d->imageno);
}

void QmlShow::prevImage()
{
    if (d->imageno == 0)
    {
        return;
    }

    d->imageno-=1;
    changePicture(d->imageno);
}

void QmlShow::play()
{
    QObject* object = d->ui->rootObject();
    object->setProperty("bool_pp", true);
}

void QmlShow::pause()
{
    QObject* object = d->ui->rootObject();
    object->setProperty("bool_pp", false);
}

void QmlShow::changePicture(int index)
{
    QObject* object  = d->ui->rootObject();
    int image_height = d->list->at(index).imageCommonContainer().height;
    int image_width  = d->list->at(index).imageCommonContainer().width;

    if(d->screen_height < image_height)
    {
        double ratio = (double)(d->screen_height)/(double)(image_height);
        image_height = (image_height*ratio);
        image_width  = (image_width*ratio);
    }

    if(d->screen_width < image_width)
    {
        double ratio = (double)(d->screen_width)/(double)(image_width);
        image_height = (int)(image_height*ratio);
        image_width  = (int)(image_width*ratio);
    }

    object->setProperty("source_scale", 1.0);
    object->setProperty("imageheight",  image_height);
    object->setProperty("imagewidth",   image_width);
    object->setProperty("text",         d->list->at(index).filePath());

    d->imageno = index;

    setMetaData();
}

void QmlShow::setMetaData()
{
    QDeclarativeContext* ctxt = d->ui->rootContext();
    QObject* object           = d->ui->rootObject();
    QObject* editBox          = 0;
    MetadataHubOnTheRoad hub  = MetadataHub();
    hub.load(d->list->at(d->imageno));

    foreach(editBox, object->children())
    {
        if(editBox->objectName().compare("editbox") == 0) break;
    }

    editBox->setProperty("name",        d->list->at(d->imageno).name());
    editBox->setProperty("data_time",   d->list->at(d->imageno).dateTime().toString("dd.MM.yyyy"));
    editBox->setProperty("pick_label",  hub.pickLabel());
    editBox->setProperty("color_label", hub.colorLabel());
    editBox->setProperty("rating",      hub.rating());
    QString imagedata;

    if(d->settings.printName)
    {
        imagedata.append(d->list->at(d->imageno).imageCommonContainer().fileName+"\n");
    }

    if(d->settings.printDate)
    {
//        imagedata.append("\n");
        imagedata.append(d->list->at(d->imageno).imageCommonContainer().creationDate.toString("MMMM d yy hh:mm:ss"));
    }

    if(d->settings.printApertureFocal)
    {
        ImageMetadataContainer photoInfo = d->list->at(d->imageno).imageMetadataContainer();
        QString str;
        imagedata.append("\n");

        if (!photoInfo.aperture.isEmpty())
        {
            str = photoInfo.aperture;
        }

        if (photoInfo.focalLength35.isEmpty())
        {
            if (!photoInfo.focalLength.isEmpty())
            {
                if (!photoInfo.aperture.isEmpty())
                {
                    str += QString(" / ");
                }

                str += photoInfo.focalLength;
            }
        }
        else
        {
            if (!photoInfo.aperture.isEmpty())
            {
                str += QString(" / ");
            }

            if (!photoInfo.focalLength.isEmpty())
            {
                str += QString("%1 (35mm: %2)").arg(photoInfo.focalLength).arg(photoInfo.focalLength35);
            }
            else
            {
                str += QString("35mm: %1)").arg(photoInfo.focalLength35);
            }
        }

        imagedata.append(str);
    }

    if(d->settings.printExpoSensitivity)
    {
        // TODO
    }

    //printf("\n\n\n%s\n\n\n",imagedata.toAscii().constData());
    ctxt->setContextProperty("imagedata", imagedata);
    //QString date=d->list->at(d->imageno).imageCommonContainer().creationDate.toString("MMMM d yyyy");
    //ctxt->setContextProperty("imagedate", date);

/*
    QObject* editBox = d->ui->findChild<QObject *>("editbox");
    QMessageBox* msg=new QMessageBox(0);
    if(editBox==NULL) msg->setText("Screw you!");
    else msg->setText(d->list->at(d->imageno).name());
    msg->show();
*/
/*
    QObject* object = d->ui->rootObject();
    QObject* editBox = d->ui->findChild<QObject *>("editbox");
    editBox->setProperty("name",d->list->at(d->imageno).name());
*/
/*
    d->editBox = new QDeclarativeView;
    d->editBox->setSource(QUrl::fromLocalFile("../core/utilities/qmlShow/qmlview/editbox.qml"));
    QObject *object = d->editBox->rootObject();
    object->setProperty("name","Hello world");
    int height = QApplication::desktop()->height();
    int width = QApplication::desktop()->width();
    object->setProperty("x",width/4);
    object->setProperty("y",height/4);
    d->editBox->show();
*/
}

} // namespace Digikam
