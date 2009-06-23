/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-07-04
 * Description : identities setup page.
 *
 * Copyright (C) 2006-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef SETUP_IDENTITY_H
#define SETUP_IDENTITY_H

// Qt includes

#include <QScrollArea>

namespace Digikam
{

class SetupIdentityPriv;

class SetupIdentity : public QScrollArea
{
    Q_OBJECT

public:

    SetupIdentity(QWidget* parent = 0);
    ~SetupIdentity();

    void applySettings();

private:

    void readSettings();

private Q_SLOTS:

    void slotSelectionChanged();
    void slotAddIdentity();
    void slotDelIdentity();
    void slotRepIdentity();

private:

    SetupIdentityPriv* const d;
};

}  // namespace Digikam

#endif // SETUP_IDENTITY_H 
