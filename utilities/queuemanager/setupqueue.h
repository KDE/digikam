/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-02-12
 * Description : setup Queue Manager tab.
 *
 * Copyright (C) 2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef SETUPQUEUE_H
#define SETUPQUEUE_H

// Qt includes.

#include <QScrollArea>

namespace Digikam
{

class SetupQueuePriv;

class SetupQueue : public QScrollArea
{
    Q_OBJECT

public:

    enum ConflictRule 
    {
        OVERWRITE = 0,
        ASKTOUSER
    };

public:

    SetupQueue(QWidget* parent = 0);
    ~SetupQueue();

    void applySettings();

private:

    void readSettings();

private:

    SetupQueuePriv* const d;
};

}  // namespace Digikam

#endif // SETUPQUEUE_H
