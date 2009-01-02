/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-01-02
 * Description : album family setup tab.
 *
 * Copyright (C) 2004-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef SETUPFAMILY_H
#define SETUPFAMILY_H

// Qt includes.

#include <QWidget>

namespace Digikam
{

class SetupFamilyPriv;

class SetupFamily : public QWidget
{
    Q_OBJECT

public:

    SetupFamily(QWidget* parent = 0);
    ~SetupFamily();

    void applySettings();

private:

    void readSettings();

private slots:

    void slotFamilySelectionChanged();
    void slotAddFamily();
    void slotDelFamily();
    void slotRepFamily();

private:

    SetupFamilyPriv* const d;
};

}  // namespace Digikam

#endif // SETUPALBUMTYPE_H 
