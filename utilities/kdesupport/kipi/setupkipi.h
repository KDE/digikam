/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-02-06
 * Description : setup kipi tools.
 *
 * Copyright (C) 2007-2015 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef SETUP_KIPI_H
#define SETUP_KIPI_H

// Qt includes

#include <QScrollArea>

// Local includes

#include "searchtextbar.h"

namespace Digikam
{

class SetupKipi : public QScrollArea
{
    Q_OBJECT

public:

    explicit SetupKipi(QWidget* const parent = 0);
    ~SetupKipi();

    void applySettings();

private Q_SLOTS:

    void slotSearchTextChanged(const SearchTextSettings& settings);
    void slotSetFilter(const QString& filter, Qt::CaseSensitivity cs);
    void slotCheckAll();
    void slotClearList();
    void slotItemClicked();
    void slotSearchResult(int found);

private:

    void updateInfo();

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif // SETUP_KIPI_H
