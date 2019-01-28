/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-07-04
 * Description : Autodetect jAlbum jar archive
 *
 * Copyright (C) 2013-2019 by Andrew Goodbody <ajg zero two at elfringham dot co dot uk>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef DIGIKAM_JALBUM_JAR_H
#define DIGIKAM_JALBUM_JAR_H

// Local includes

#include "dbinaryiface.h"

using namespace Digikam;

namespace DigikamGenericJAlbumPlugin
{

class JalbumJar : public DBinaryIface
{
    Q_OBJECT

public:

    explicit JalbumJar();
    ~JalbumJar();

    bool checkDirForPath(const QString& possibleDir);
};

} // namespace DigikamGenericJAlbumPlugin

#endif // DIGIKAM_JALBUM_JAR_H
