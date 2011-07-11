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
#include<QtDeclarative/QDeclarativeView>
#include<QDeclarativeEngine>
namespace Digikam
{
class ImageInfo;
QmlShow::QmlShow(QList<ImageInfo> list) : QMainWindow(0, Qt::FramelessWindowHint)
{
		this->ui=new QDeclarativeView;
        this->ui->setSource(QUrl::fromLocalFile("/home/dhruv/digikam/digikam-sc/core/utilities/qmlShow/qmlview/qmlview.qml"));
        setCentralWidget(this->ui);
		connect(this->ui->engine(),SIGNAL(quit()),this,SLOT(close()),Qt::DirectConnection);
        this->ui->setResizeMode(QDeclarativeView::SizeRootObjectToView);
//      this->ui->setFocus(Qt::OtherFocusReason);
		this->ui->show();
        showMaximized();
}

QmlShow::~QmlShow()
{
}
}
