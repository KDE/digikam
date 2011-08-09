/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-04-21
 * Description : slide show tool using preview of pictures.
 *
 * Copyright (C) 2005-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Local includes

#include"qmlShow.h"
#include"imageinfo.h"

namespace Digikam
{

class QmlShowPriv
{
public:

    QmlShowPriv()
        : ui(0),
          imageno(0)
    {
    }

    QDeclarativeView *ui;
    int imageno;
    QList<QString> list;
};

QmlShow::QmlShow(const QStringList& list)
    : QMainWindow(0, Qt::FramelessWindowHint),
      d(new QmlShowPriv)
{
    d->list = list;

    ui   = new QDeclarativeView;
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
    connect(object, SIGNAL(grid_view()),
            this, SLOT(gridview()));
    imageno=0;

    // FIXME: This is for debugging?
    QMessageBox* n = new QMessageBox(this);
    n->setText(d->list[++imageno]);
    n->show();

    d->ui->show();

    showMaximized();
    // FIXME: Before accessing a list, you must be sure it is not empty -> crashes otherwise
    object->setProperty("text", d->list[0]);
}

QmlShow::~QmlShow()
{
    delete d;
}

void QmlShow::nextImage()
{
    if (imageno == d->list.count()-1)
    {
        return;
    }

    QObject *object = d->ui->rootObject();
    object->setProperty("text", d->list[++imageno]);
}

void QmlShow::prevImage()
{
    if(imageno==0) return;

    QObject *object = d->ui->rootObject();
    object->setProperty("text", d->list[--imageno]);
}

void QmlShow::play()
{
    QObject *object = d->ui->rootObject();
    object->setProperty("bool_pp", true);
}

void QmlShow::pause()
{
    QObject *object = d->ui->rootObject();
    object->setProperty("bool_pp", false);
}

void QmlShow::gridview()
{
    QDeclarativeEngine engine;
    QDeclarativeComponent component(&engine, "qmlview/ContactModel.qml");
    QObject *object = component.create();

}

} // namespace Digikam
