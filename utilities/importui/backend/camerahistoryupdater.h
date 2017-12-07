/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-10-16
 * Description : history updater thread for importui
 *
 * Copyright (C) 2009-2011 by Andi Clemens <andi dot clemens at gmail dot com>
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef CAMERAHISTORYUPDATER_H
#define CAMERAHISTORYUPDATER_H

// Qt includes

#include <QDateTime>
#include <QMultiMap>
#include <QPair>
#include <QThread>

// Local includes

#include "camiteminfo.h"

class QWidget;

namespace Digikam
{

typedef QMultiMap<QDateTime, CamItemInfo>  CHUpdateItemMap;
typedef QPair<QByteArray, CHUpdateItemMap> CHUpdateItem;

class CameraHistoryUpdater : public QThread
{
    Q_OBJECT

public:

    explicit CameraHistoryUpdater(QWidget* const parent);
    ~CameraHistoryUpdater();

    void addItems(const QByteArray& id, CHUpdateItemMap& map);

Q_SIGNALS:

    void signalBusy(bool val);
    void signalHistoryMap(const CHUpdateItemMap&);

public Q_SLOTS:

    void slotCancel();

protected:

    void run();

private:

    void proccessMap(const QByteArray& id, CHUpdateItemMap& map);
    void sendBusy(bool val);

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif /* CAMERAHISTORYUPDATER_H */
