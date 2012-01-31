/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-01-31
 * Description : maintenance manager
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

#ifndef MAINTENANCEMNGR_H
#define MAINTENANCEMNGR_H

// Qt includes

#include <QObject>

// Local includes


namespace Digikam
{

class MaintenanceSettings;

class MaintenanceMngr : public QObject
{
    Q_OBJECT

public:

    MaintenanceMngr(QObject* parent);
    ~MaintenanceMngr();

    void setSettings(const MaintenanceSettings& settings);
    
    void start();
    void stop();
    bool isRunning() const;

Q_SIGNALS:

    void signalCompleted();

private Q_SLOTS:

    void slotStage1();  // Thumbnails
    void slotStage2();  // Fingerprints
    void slotStage3();  // Fingerprints

private:

    class MaintenanceMngrPriv;
    MaintenanceMngrPriv* const d;
};

}  // namespace Digikam

#endif /* MAINTENANCEMNGR_H */
