/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-02-02
 * Description : maintenance tool
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

#include <QTime>
#include <QString>

// Local includes

#include "progressmanager.h"

namespace Digikam
{

class MaintenanceTool : public ProgressItem
{
    Q_OBJECT

public:

    MaintenanceTool(ProgressItem* parent=0);
    ~MaintenanceTool();

    /** If true, show a notification message on desktop notification manager 
     * with time elpased to run process.
     */
    void setNotificationEnabled(bool b);

Q_SIGNALS:

    /** Emit when process is done (not canceled).
     */
    void signalComplete();

protected:

    /** Return true if process have been canceled.
     */
    bool isCanceled() const;

protected Q_SLOTS:

    virtual void slotStart();
    virtual void slotDone();
    virtual void slotCancel();

private:

    class MaintenanceToolPriv;
    MaintenanceToolPriv* const d;
};

} // namespace Digikam

#endif /* MAINTENANCETOOL_H */
