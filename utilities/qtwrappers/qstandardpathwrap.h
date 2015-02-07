/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2015-07-02
 * Description : QStandardPaths wrapper, to check for /usr/share/apps
 *
 *
 * Copyright (C) 2015 by Veaceslav Munteanu <veaceslav dot munteanu90 at gmail dot com>
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
#ifndef QSTANDARTPATHSWRAP_H_
#define QSTANDARTPATHSWRAP_H_

#include <QStandardPaths>

class QStandardPathsWrap{

public:
    static QString locate(QStandardPaths::StandardLocation type,
                          const QString & fileName,
                          QStandardPaths::LocateOptions options = QStandardPaths::LocateFile)
    {
        QString str;
        str = QStandardPaths::locate(type, fileName, options);
        if(str.isEmpty())
        {
            return QStandardPaths::locate(type, QString("apps/") +fileName, options);
        } else
            return str;
    }

    static QStringList locateAll(QStandardPaths::StandardLocation type,
                                 const QString & fileName,
                                 QStandardPaths::LocateOptions options = QStandardPaths::LocateFile)
    {
        QStringList strList;
        strList = QStandardPaths::locateAll(type, fileName, options);
        if(strList.isEmpty())
        {
            return QStandardPaths::locateAll(type, QString("apps/") +fileName, options);
        } else
            return strList;
    }
};


#endif
