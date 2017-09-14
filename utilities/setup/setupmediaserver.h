/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-05-11
 * Description : setup Media Server tab.
 *
 * Copyright (C) 2007-2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef SETUP_MEDIA_SERVER_H
#define SETUP_MEDIA_SERVER_H

// Qt includes

#include <QScrollArea>

namespace Digikam
{

class SetupMediaServer : public QScrollArea
{
    Q_OBJECT

public:

    explicit SetupMediaServer(QWidget* const parent = 0);
    ~SetupMediaServer();

    void applySettings();

private:

    void readSettings();

private Q_SLOTS:

    void slotSelectionChanged();
    void slotStartMediaServer();

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif // SETUP_MEDIA_SERVER_H
