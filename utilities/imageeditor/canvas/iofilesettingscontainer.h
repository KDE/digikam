/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date   : 2006-01-03
 * Description : IO file Settings Container.
 * 
 * Copyright 2005-2007 by Gilles Caulier
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

#ifndef IOFILESETTINGSCONTAINER_H
#define IOFILESETTINGSCONTAINER_H

// Local includes.

#include "rawdecodingsettings.h"

namespace Digikam
{

class IOFileSettingsContainer
{

public:
    
    IOFileSettingsContainer()
    {
        JPEGCompression     = 75;
        PNGCompression      = 9;
        TIFFCompression     = false;
        JPEG2000Compression = 75;
        JPEG2000LossLess    = true;
    };

    ~IOFileSettingsContainer(){};

public:

    // JPEG quality value.
    int  JPEGCompression;

    // PNG compression value.
    int  PNGCompression;

    // TIFF deflat compression.
    bool TIFFCompression;

    // JPEG2000 quality value.
    int  JPEG2000Compression;

    // JPEG2000 lossless compression.
    bool JPEG2000LossLess;

    // ------------------------------------------------------
    // RAW File decoding options :

    RawDecodingSettings rawDecodingSettings;    
};

}  // namespace Digikam

#endif  // IOFILESETTINGSCONTAINER_H
