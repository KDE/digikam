/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-02-02
 * Description : maintenance tool
 *
 * Copyright (C) 2012-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2012      by Andi Clemens <andi dot clemens at gmail dot com>
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

#include <QString>

// Local includes

#include "progressmanager.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT MaintenanceTool : public ProgressItem
{
    Q_OBJECT

public:

    explicit MaintenanceTool(const QString& id, ProgressItem* const parent = 0);
    virtual ~MaintenanceTool();

    /** If true, show a notification message on desktop notification manager
     * with time elpased to run process.
     */
    void setNotificationEnabled(bool b);

    /** Re-implement this method if your tool is able to use multi-core CPU to process item in parallel
     */
    virtual void setUseMultiCoreCPU(bool) {};

Q_SIGNALS:

    /** Emit when process is done (not canceled).
     */
    void signalComplete();

public Q_SLOTS:

    void start();

protected Q_SLOTS:

    virtual void slotStart();
    virtual void slotDone();
    virtual void slotCancel();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif /* MAINTENANCETOOL_H */
