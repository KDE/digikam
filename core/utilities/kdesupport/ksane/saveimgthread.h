/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-10-11
 * Description : save image thread
 *
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef SAVEIMGTHREAD_H
#define SAVEIMGTHREAD_H

// Qt includes

#include <QObject>
#include <QThread>
#include <QString>
#include <QByteArray>
#include <QUrl>

namespace Digikam
{

class SaveImgThread : public QThread
{
    Q_OBJECT

public:

    SaveImgThread(QObject* const parent);
    ~SaveImgThread();

    void setTargetFile(const QUrl& url, const QString& format);
    void setScannerModel(const QString& make, const QString& model);
    void setImageData(const QByteArray& ksaneData, int width, int height,
                      int bytesPerLine, int ksaneFormat);

Q_SIGNALS:

    void signalProgress(const QUrl&, int);
    void signalComplete(const QUrl&, bool);

private:

    void run();

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif // SAVEIMGTHREAD_H
