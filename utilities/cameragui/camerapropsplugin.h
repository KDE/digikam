/* ============================================================
 * File  : camerapropsplugin.h
 * Author: Jörn Ahrens <kde@jokele.de>
 * Date  : 2004-07-23
 * Description : 
 * This class adds additional tab to KPropertiesDialog to show
 * the extended file attributes from the digikamcamera kioslave. 
 * 
 * Copyright 2004 by Jörn Ahrens

 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published bythe Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * ============================================================ */

#ifndef CAMERAPROPSPLUGIN_H
#define CAMERAPROPSPLUGIN_H

#include <kpropertiesdialog.h>

namespace KIO
{
class Slave;
class Job;
}

class CameraPropsPlugin : public KPropsDlgPlugin
{
    Q_OBJECT
    
public:

    CameraPropsPlugin(KPropertiesDialog *_props);
   	virtual ~CameraPropsPlugin() {};
    
};

#endif /* CAMERAPROPSPLUGIN_H */
