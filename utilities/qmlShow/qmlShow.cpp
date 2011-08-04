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

#include"qmlShow.h"
#include"imageinfo.h"
#include<QtGui>
#include<QString>
#include<QtDeclarative/QDeclarativeView>
#include<QDeclarativeEngine>
#include<stdio.h>
namespace Digikam
{
class ImageInfo;
QmlShow::QmlShow(QList<QString> *list) : QMainWindow(0, Qt::FramelessWindowHint)
{
		this->ui=new QDeclarativeView;
	this->list=list;
        this->ui->setSource(QUrl::fromLocalFile("../core/utilities/qmlShow/qmlview/qmlview.qml"));
        setCentralWidget(this->ui);
		connect(this->ui->engine(),SIGNAL(quit()),this,SLOT(close()),Qt::DirectConnection);
        this->ui->setResizeMode(QDeclarativeView::SizeRootObjectToView);
	QObject *object=ui->rootObject();
	connect(object,SIGNAL(nextClicked()),this,SLOT(nextImage()));
        connect(object,SIGNAL(prevClicked()),this,SLOT(prevImage()));
        connect(object,SIGNAL(play()),this,SLOT(play()));
        connect(object,SIGNAL(pause()),this,SLOT(pause()));
	connect(object,SIGNAL(grid_view()),this,SLOT(gridview()));
	imageno=0;
	QMessageBox* n=new QMessageBox(this);
	n->setText(list->at(++imageno));
	n->show();
	this->ui->show();
        showMaximized();
	object->setProperty("text",list->at(0));
}

QmlShow::~QmlShow()
{
}
void QmlShow::nextImage()
{
	if(imageno==(list->count()-1)) return;
        QObject *object=(QObject*)ui->rootObject();
        object->setProperty("text",list->at(++imageno));
}
void QmlShow::prevImage()
{
	if(imageno==0) return;
         QObject *object=(QObject*)ui->rootObject();
        object->setProperty("text",list->at(--imageno));
}
void QmlShow::play()
{
        QObject *object=(QObject*)ui->rootObject();
        object->setProperty("bool_pp",true);
}
void QmlShow::pause()
{
        QObject *object=(QObject*)ui->rootObject();
        object->setProperty("bool_pp",false);
}
void QmlShow::gridview()
{
	QDeclarativeEngine engine;
	QDeclarativeComponent component(&engine, "qmlview/ContactModel.qml");
	QObject *object = component.create();
	
}
}
