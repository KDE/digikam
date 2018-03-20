/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-11-18
 * Description : a tool to export items to Google web services
 *
 * Copyright (C) 2013 by Pankaj Kumar <me at panks dot me>
 * Copyright (C) 2013-2018 by Caulier Gilles <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef GD_TALKER_H
#define GD_TALKER_H

// Qt includes

#include <QList>
#include <QString>
#include <QObject>
#include <QStringList>

// Local includes

#include "gsitem.h"
#include "gstalkerbase.h"

namespace Digikam
{

class GDTalker : public GSTalkerBase
{
    Q_OBJECT

public:

    explicit GDTalker(QWidget* const parent);
    ~GDTalker();

public:

    void getUserName();
    void listFolders();
    void createFolder(const QString& title, const QString& id);
    bool addPhoto(const QString& imgPath,
                  const GSPhoto& info,
                  const QString& id,
                  bool rescale,
                  int maxDim,
                  int imageQuality);
    void cancel();

Q_SIGNALS:

    void signalListAlbumsDone(int, const QString&, const QList <GSFolder>&);
    void signalCreateFolderDone(int,const QString& msg);
    void signalAddPhotoDone(int,const QString& msg, const QString&);
    void signalSetUserName(const QString& msg);

private Q_SLOTS:

    void slotFinished(QNetworkReply* reply);

private:

    void parseResponseListFolders(const QByteArray& data);
    void parseResponseCreateFolder(const QByteArray& data);
    void parseResponseAddPhoto(const QByteArray& data);
    void parseResponseUserName(const QByteArray& data);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // GD_TALKER_H
