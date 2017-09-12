/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-05-28
 * Description : Media server manager
 *
 * Copyright (C) 2012      by Smit Mehta <smit dot meh at gmail dot com>
 * Copyright (C) 2012-2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DMEDIA_SERVER_MNGR_H
#define DMEDIA_SERVER_MNGR_H

// Qt includes

#include <QObject>
#include <QMap>
#include <QList>
#include <QString>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT DMediaServerMngr : public QObject
{
    Q_OBJECT

public:

    void checkLoadAtStartup();
    void setCollectionMap(const QMap<QString, QList<QUrl>>&);
    void startMediaServer();
    void cleanUp();
    
    // To save and load from xml data file.
    bool save();
    bool load();

public:

    static DMediaServerMngr* instance();

public Q_SLOTS:

    void slotTurnOff();
    void slotTurnOn();

private:

    explicit DMediaServerMngr();
    ~DMediaServerMngr();

private:

    friend class DMediaServerMngrCreator;

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DMEDIA_SERVER_MNGR_H
