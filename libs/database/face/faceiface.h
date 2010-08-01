/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) <year>  <name of author>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/


#ifndef FACEIFACE_H
#define FACEIFACE_H

// Qt includes

#include <QString>
#include <QMap>
#include <QList>

// Libkface includes

#include <libkface/database.h>
#include <libkface/kface.h>

// Local includes

#include "digikam_export.h"

using namespace KFaceIface;

class QImage;

namespace Digikam
{

class Dimg;
class FaceIfacePriv;

class DIGIKAM_DATABASE_EXPORT FaceIface
{

public:

    FaceIface();
    ~FaceIface();

    QList<Face> findAndTagFaces(qlonglong imageid);
    void forgetFaceTags(qlonglong imageid);
    
private:
    
    FaceIfacePriv* const d;

};

}  // Namespace Digikam


#endif // FACEIFACE_H
