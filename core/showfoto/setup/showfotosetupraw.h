/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-02-06
 * Description : setup RAW decoding settings.
 *
 * Copyright (C) 2007-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef SHOWFOTOSETUPRAW_H
#define SHOWFOTOSETUPRAW_H

// Qt includes

#include <QScrollArea>

namespace ShowFoto
{

class SetupRaw : public QScrollArea
{
    Q_OBJECT

public:

    explicit SetupRaw(QWidget* const parent = 0);
    ~SetupRaw();

    void applySettings();

private:

    void readSettings();

private:

    class Private;
    Private* const d;
};

}  // namespace ShowFoto

#endif // SHOWFOTOSETUPRAW_H
