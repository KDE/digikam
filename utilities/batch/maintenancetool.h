/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-01-16
 * Description : Maintenance tool class
 *
 * Copyright (C) 2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef MAINTENANCETOOL_H
#define MAINTENANCETOOL_H

// Qt includes

#include <QPixmap>

// Local includes

#include "progressmanager.h"

namespace Digikam
{

class LoadingDescription;
class ThumbnailLoadThread;

class MaintenanceTool : public ProgressItem
{
    Q_OBJECT

public:

    enum Mode
    {
        AllItems = 0,
        MissingItems,
        AlbumItems
    };

public:

    MaintenanceTool(Mode mode=AllItems, int albumId=-1);
    virtual ~MaintenanceTool();

    void setTitle(const QString& title);

Q_SIGNALS:

    void signalProcessDone();

protected:

    QStringList&         allPicturePath();
    Mode                 mode();
    ThumbnailLoadThread* thumbsLoadThread() const;

    /** Customize these method in child class
     */
    virtual void listItemstoProcess() = 0;
    virtual void processOne();

private Q_SLOTS:

    /** This slot call listItemstoProcess()
     */
    void slotRun();

    void slotCancel();
    void slotGotThumbnail(const LoadingDescription&, const QPixmap&);

private:

    void complete();

private:

    class MaintenanceToolPriv;
    MaintenanceToolPriv* const d;
};

}  // namespace Digikam

#endif /* MAINTENANCETOOL_H */
