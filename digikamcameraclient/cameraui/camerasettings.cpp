/* ============================================================
 * File  : camerasettings.cpp
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2003-02-12
 * Description : 
 * 
 * Copyright 2003 by Renchi Raju

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
 * =
 =========================================================== */

#include <kconfig.h>
#include <qstring.h>

#include "camerasettings.h"
#include "thumbnailsize.h"

CameraSettings::CameraSettings()
{
    config = new KConfig("digikamrc");
    init();
}

CameraSettings::~CameraSettings()
{
    delete config;
}

void CameraSettings::init()
{
    showFolders_ = false;
    iconSize_    = ThumbnailSize::Medium;
}

void CameraSettings::readSettings()
{
    config->setGroup("Camera Settings");

    if (config->hasKey("Show Folders"))
        showFolders_ = config->readBoolEntry("Show Folders",
                                          false);

    // Best to leave this as app wide config
    config->setGroup("Album Settings");

    if (config->hasKey("Default Icon Size"))
        iconSize_ = config->readNumEntry("Default Icon Size",
                                         ThumbnailSize::Medium);

}

void CameraSettings::saveSettings()
{
    config->setGroup("Camera Settings");

    //     config->writeEntry("Default Icon Size",
    //                        QString::number(iconSize_));

    config->writeEntry("Show Folders", showFolders_);
    config->sync();

}

void CameraSettings::setShowFolders(bool val)
{
    showFolders_ = val;    
}

bool CameraSettings::getShowFolders() const
{
    return showFolders_;
}

void CameraSettings::setDefaultIconSize(int val)
{
    iconSize_ = val;
}

int CameraSettings::getDefaultIconSize() const
{
    return iconSize_;
}
