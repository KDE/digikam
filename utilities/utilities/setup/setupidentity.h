/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at gmail dot com>
 * Date  : 2006-07-04
 * Description : default IPTC identity setup tab.
 * 
 * Copyright 2006 by Gilles Caulier
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

// Qt includes.

#include <qwidget.h>

namespace Digikam
{

class SetupIdentityPriv;

class SetupIdentity : public QWidget
{
    Q_OBJECT
    
public:

    SetupIdentity(QWidget* parent = 0);
    ~SetupIdentity();

    void applySettings();

private:

    void readSettings();

private:

    SetupIdentityPriv* d;

};

}  // namespace Digikam

#endif // SETUP_IDENTITY_H 
