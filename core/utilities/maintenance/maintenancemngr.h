/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-01-31
 * Description : maintenance manager
 *
 * Copyright (C) 2012-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef MAINTENANCEMNGR_H
#define MAINTENANCEMNGR_H

// Qt includes

#include <QObject>

namespace Digikam
{

class MaintenanceSettings;
class ProgressItem;

class MaintenanceMngr : public QObject
{
    Q_OBJECT

public:

    explicit MaintenanceMngr(QObject* const parent);
    ~MaintenanceMngr();

    void setSettings(const MaintenanceSettings& settings);

    void start();
    bool isRunning() const;

Q_SIGNALS:

    void signalComplete();

private Q_SLOTS:

    void slotToolCompleted(ProgressItem*);
    void slotToolCanceled(ProgressItem*);

private:

    void stage1();  // New items
    void stage2();  // Thumbnails
    void stage3();  // Finger-prints
    void stage4();  // Duplicates
    void stage5();  // Faces Management
    void stage6();  // Image Quality Sorter
    void stage7();  // Metadata
    void stage8();  // Database cleanup

    void done();    // Called when all scheduled tools are done.
    void cancel();  // Called when a tool is canceled.

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif /* MAINTENANCEMNGR_H */
