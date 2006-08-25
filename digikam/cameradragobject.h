/* ============================================================
 * Authors: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *          Caulier Gilles <caulier dot gilles at kdemail dot net>
 * Date   : 2003-02-18
 * Description : drag and drop camera management 
 * 
 * Copyright 2003-2005 by Renchi Raju
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

#ifndef CAMERADRAGOBJECT_H
#define CAMERADRAGOBJECT_H

// Qt includes.

#include <qdragobject.h>

class QMimeSource;
class QWidget;

namespace Digikam
{

class CameraType;

class CameraDragObject : public QStoredDrag
{

public:

    CameraDragObject(const CameraType& ctype, QWidget* dragSource=0);
    ~CameraDragObject();

    static bool canDecode(const QMimeSource* e);
    static bool decode(const QMimeSource* e, CameraType& ctype);

private:

    void setCameraType(const CameraType& ctype);

};

}  // namespace Digikam

#endif  // CAMERADRAGOBJECT_H
