/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at kdemail dot net> 
 * Date   : 2007-02-13
 * Description : slide show settings container.
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

#ifndef SLIDESHOWSETTINGSCONTAINER_H
#define SLIDESHOWSETTINGSCONTAINER_H

// KDE includes.

#include <kurl.h>

namespace Digikam
{

class SlideShowSettings
{

public:
    
    SlideShowSettings()
    {
        exifRotate = true;
        printName  = true;
        loop       = false;
        delay      = 5;
    };
    
    ~SlideShowSettings(){};

public:

    /** Auto-rotate image accordinly with Exif Rotation tag */
    bool exifRotate;

    /** Print file name during slide */
    bool printName;

    /** Slide pictures in loop */
    bool loop;
    
    /** Delay in seconds */
    int delay;

    /** List of files to slide */ 
    KURL::List fileList; 
};

}  // namespace Digikam

#endif  // SLIDESHOWSETTINGSCONTAINER_H
