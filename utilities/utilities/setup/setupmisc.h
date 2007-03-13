/* ============================================================
 * Authors: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *          Gilles Caulier <caulier dot gilles at gmail dot com>
 * Date   : 2004-08-23
 * Description : mics configuration setup tab
 * 
 * Copyright 2004 by Renchi Raju
 * Copyright 2005-2006 by Gilles Caulier
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

#ifndef SETUPMISC_H
#define SETUPMISC_H

// Qt includes.

#include <qwidget.h>

namespace Digikam
{

class SetupMiscPriv;

class SetupMisc : public QWidget
{
public:

    SetupMisc(QWidget* parent);
    ~SetupMisc();

    void applySettings();

private:

    void readSettings();

private:

    SetupMiscPriv* d;

};

}  // namespace Digikam

#endif /* SETUPMISC_H */
