/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-27-08
 * Description : an tool bar action object to display logo
 * 
 * Copyright (C) 2007-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DLOGO_ACTION_H
#define DLOGO_ACTION_H   

// KDE includes.

#include <kaction.h>

// Local includes.

#include "digikam_export.h"

namespace Digikam 
{

class DIGIKAM_EXPORT DLogoAction : public KAction
{
  Q_OBJECT

public:
    
    DLogoAction(QObject* parent, const char* name=0);
               
    virtual int plug(QWidget *widget, int index=-1);

private slots:

    void slotProcessURL(const QString&);
};

} // namespace Digikam

#endif /* DLOGO_ACTION_H */
