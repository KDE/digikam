/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date   : 2006-04-21
 * Description : main photograph informations container
 *
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

#ifndef PHOTOINFOCONTAINER_H
#define PHOTOINFOCONTAINER_H

// QT includes.

#include <qstring.h>
#include <qdatetime.h>

namespace Digikam
{

class PhotoInfoContainer
{

public:

    PhotoInfoContainer(){}

    QString   make;
    QString   model;
    QString   exposureTime;
    QString   aperture;
    QString   focalLenght;
    QString   focalLenght35mm;
    QString   sensitivity;
    QString   flash;

    QDateTime dateTime;
};

} // namespace Digikam

#endif /* PHOTOINFOCONTAINER_H */
