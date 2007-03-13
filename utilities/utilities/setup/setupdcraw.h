/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at gmail dot com>
 * Date   : 2007-02-06
 * Description : setup RAW decoding settings.
 * 
 * Copyright 2007 by Gilles Caulier
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

#ifndef SETUPDCRAW_H
#define SETUPDCRAW_H

// Qt includes.

#include <qwidget.h>

// Local Includes.

#include "digikam_export.h"

namespace Digikam
{

class SetupDcrawPriv;

class DIGIKAM_EXPORT SetupDcraw : public QWidget
{
    Q_OBJECT
    
public:

    SetupDcraw(QWidget* parent = 0);
    ~SetupDcraw();

    void applySettings();

private:

    void readSettings();

private:

    SetupDcrawPriv* d;    
};

}  // namespace Digikam

#endif // SETUPDCRAW_H 
