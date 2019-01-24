/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-07-04
 * Description : Autodetect Java binary program
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

#ifndef DIGIKAM_JALBUM_JAVA_H
#define DIGIKAM_JALBUM_JAVA_H

// Local includes

#include "dbinaryiface.h"

using namespace Digikam;

namespace GenericDigikamJAlbumPlugin
{

class JalbumJava : public DBinaryIface
{
public:

    explicit JalbumJava()
        : DBinaryIface(
                       QLatin1String("java"),
                       QLatin1String("Java"),
                       QLatin1String("https://www.java.com/"),
                       QLatin1String("jAlbum Export"),
                       QStringList(QLatin1String("-version")),
                       i18n("jAlbum Gallery Generator.")
                      )
        {
            setup();
        }

    ~JalbumJava()
    {
    }
};

} // namespace GenericDigikamJAlbumPlugin

#endif // DIGIKAM_JALBUM_JAVA_H
