/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-11-14
 * Description : database migration dialog
 *
 * Copyright (C) 2009 by Holger Foerster <Hamsi2k at freenet dot de>
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

#ifndef MIGRATIONDLG_H
#define MIGRATIONDLG_H

// KDE includes
#include <kdialog.h>

// Local includes
#include "digikam_export.h"
#include "databasewidget.h"

namespace Digikam
{


class DIGIKAM_EXPORT MigrationDlg : public KDialog
{
    Q_OBJECT

public:

    MigrationDlg(QWidget* parent);
    ~MigrationDlg();

private Q_SLOTS:

private:

//    MigrationDlgPriv* const d;
    DatabaseWidget *fromDatabaseWidget;
    DatabaseWidget *toDatabaseWidget;

    void setupMainArea();
};

}  // namespace Digikam

#endif  // MIGRATIONDLG_H
