/* ============================================================
 * File  : camerasettings.h
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
 * ============================================================ */

#ifndef CAMERASETTINGS_H
#define CAMERASETTINGS_H

class KConfig;

class CameraSettings
{
public:

    CameraSettings();
    ~CameraSettings();

    void readSettings();
    void saveSettings();

    void setShowFolders(bool val);
    bool getShowFolders() const;

    void setDefaultIconSize(int val);
    int  getDefaultIconSize() const;
    
private:

    void init();

    bool showFolders_;
    int  iconSize_;

    KConfig* config;
};

#endif /* CAMERASETTINGS_H */
