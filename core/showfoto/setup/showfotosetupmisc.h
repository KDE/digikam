/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-04-02
 * Description : setup Misc tab.
 *
 * Copyright (C) 2005-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef SHOWFOTOSETUPMISC_H
#define SHOWFOTOSETUPMISC_H

// Qt includes

#include <QScrollArea>

namespace ShowFoto
{

class SetupMisc : public QScrollArea
{

public:

    enum SortOrder
    {
        SortByDate = 0,
        SortByName,
        SortByFileSize
    };

public:

    explicit SetupMisc(QWidget* const parent = 0);
    virtual ~SetupMisc();

    void applySettings();

private:

    void readSettings();

private:

    class Private;
    Private* const d;
};

} // namespace ShowFoto

#endif // SHOWFOTOSETUPMISC_H
