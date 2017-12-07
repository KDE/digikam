/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-01-30
 * Description : maintenance dialog
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

#ifndef MAINTENANCEDLG_H
#define MAINTENANCEDLG_H

// Qt includes

#include <QDialog>

// Local includes

#include "digikam_export.h"
#include "maintenancesettings.h"

namespace Digikam
{

class MaintenanceDlg : public QDialog
{
    Q_OBJECT

public:

    explicit MaintenanceDlg(QWidget* const parent = 0);
    ~MaintenanceDlg();

    MaintenanceSettings settings() const;

private Q_SLOTS:

    void slotItemToggled(int index, bool b);
    void slotMetadataSetup();
    void slotQualitySetup();
    void slotOk();
    void slotHelp();

private:

    void writeSettings();
    void readSettings();

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif  // MAINTENANCEDLG_H
