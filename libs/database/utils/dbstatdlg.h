/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-05-28
 * Description : database statistics dialog
 *
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

#ifndef DATABASE_STATISTICS_DIALOG_H
#define DATABASE_STATISTICS_DIALOG_H

// Local includes

#include "infodlg.h"
#include "coredbconstants.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT DBStatDlg : public InfoDlg
{
public:

    explicit DBStatDlg(QWidget* const parent);
    ~DBStatDlg();

private:

    int generateItemsList(DatabaseItem::Category category, const QString& title);
};

} // namespace Digikam

#endif // DATABASE_STATISTICS_DIALOG_H
