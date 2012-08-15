/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-01-02
 * Description : setup Kipi plugins tab.
 *
 * Copyright (C) 2004-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2011-2012 by Andi Clemens <andi dot clemens at gmail dot com>
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

#ifndef SETUPPLUGINS_H
#define SETUPPLUGINS_H

// Qt includes

#include <QScrollArea>

namespace Digikam
{

class SetupPlugins : public QScrollArea
{
    Q_OBJECT

public:

    SetupPlugins(QWidget* parent = 0);
    ~SetupPlugins();

    void applyPlugins();

private Q_SLOTS:

    void slotCheckAll();
    void slotClear();

private:

    void initPlugins();

private:

    class SetupPluginsPriv;
    SetupPluginsPriv* const d;
};

}  // namespace Digikam

#endif // SETUPPLUGINS_H 
