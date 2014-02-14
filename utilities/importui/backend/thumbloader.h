/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2014-02-13
 * Description : Thumbnail loader with with QThread support
 *
 * Copyright (C) 2014 by Veaceslav Munteanu <veaceslav dot munteanu90  at gmail dot com>
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

#ifndef THUMBLOADER_H
#define THUMBLOADER_H

#include<QWidget>


namespace Digikam
{

class PixWorker : public QObject
{
    Q_OBJECT
public:
    PixWorker();

public slots:
    void doWork(QString result);

signals:
    void resultReady(const QString result);
};

class ThumbLoader : public QObject
{
    Q_OBJECT
public:
    ThumbLoader(QObject* const parent, QString title, QString model, QString port, QString path);
    ~ThumbLoader();

    void addToWork(QString data);

private slots:
    void handleResult(QString result);

signals:
    void signalStartWork(QString todo);
private:
    class ThumbLoaderPriv;
    ThumbLoaderPriv* d;
};

} // namespace Digikam

#endif
