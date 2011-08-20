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


// C++ includes

#include <stdio.h>

// Qt includes

#include <QtGui>
#include <QString>
#include <QtDeclarative/QDeclarativeView>
#include <QDeclarativeEngine>
#include <QGraphicsObject>
#include<QDeclarativeContext>

#include"qmlShow.h"
#include"imageinfo.h"
namespace Digikam
{

class QmlShow::QmlShowPriv
{
public:

    QmlShowPriv()
        : ui(0),
          imageno(0)
    {
    }

    QDeclarativeView *ui;
    int imageno;
    QStringList list;
};

QmlShow::QmlShow(const QStringList& list)
    : QMainWindow(0, Qt::FramelessWindowHint),
      d(new QmlShowPriv)
{
    d->ui   = new QDeclarativeView;
    d->list = list;
    QDeclarativeContext *ctxt = d->ui->rootContext();
    ctxt->setContextProperty("myModel", QVariant::fromValue(d->list));

    // FIXME: Use KStandardDirs and install qml file properly (see data/database/dbconfig.xml as example)
    d->ui->setSource(QUrl::fromLocalFile("../core/utilities/qmlShow/qmlview/qmlview.qml"));
    setCentralWidget(d->ui);

    connect(d->ui->engine(), SIGNAL(quit()),
            this, SLOT(close()));

    d->ui->setResizeMode(QDeclarativeView::SizeRootObjectToView);

    QObject *object=d->ui->rootObject();
    connect(object, SIGNAL(nextClicked()),
            this, SLOT(nextImage()));
    connect(object, SIGNAL(prevClicked()),
            this, SLOT(prevImage()));
    connect(object, SIGNAL(play()),
            this, SLOT(play()));
    connect(object, SIGNAL(pause()),
            this, SLOT(pause()));
    d->imageno=0;

    d->ui->show();
    showMaximized();

    if(!list.isEmpty())
    {
        object->setProperty("text", d->list[0]);
    }
    else
    {
        close();
    }
}

QmlShow::~QmlShow()
{
    delete d;
}

void QmlShow::nextImage()
{
    if (d->imageno == (d->list.count()-1))
    {
        return;
    }

    QObject* object = d->ui->rootObject();
    object->setProperty("text", d->list[++(d->imageno)]);
}

void QmlShow::prevImage()
{
    if (d->imageno == 0)
    {
        return;
    }

    QObject* object = d->ui->rootObject();
    object->setProperty("text", d->list[--(d->imageno)]);
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

}
