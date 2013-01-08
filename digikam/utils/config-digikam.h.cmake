/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-08-03
 * Description : digiKam config header
 *
 * Copyright (C) 2009-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef CONFIG_DIGIKAM_H
#define CONFIG_DIGIKAM_H

/* Define to 1 if you have KDEPIM shared library installed */
#cmakedefine HAVE_KDEPIMLIBS 1

/* Define to 1 if Glib2 shared library is installed */
#cmakedefine HAVE_GLIB2 1

/* Define to 1 if an external liblqr-1 shared library have been found */
#cmakedefine USE_EXT_LIBLQR-1 1

/* Define to 1 if an external libpgf shared library have been found */
#cmakedefine USE_EXT_LIBPGF 1

/* Define to 1 if an external libclapack shared library have been found */
#cmakedefine USE_EXT_LIBCLAPACK 1

/* Define to 1 if an external lensfun shared library have been found */
#cmakedefine HAVE_LENSFUN 1

/* Define to 1 if GPhoto2 shared library is installed */
#cmakedefine HAVE_GPHOTO2 1

/* Define to 1 if libgphoto2 2.5 shared library is installed */
#cmakedefine HAVE_GPHOTO25 1

/* Define to 1 if thumbnails database is used */
#cmakedefine USE_THUMBS_DB 1

/* Define to 1 if script interface is enabled */
#cmakedefine USE_SCRIPT_IFACE 1

/* Define to 1 if presentation mode is enabled */
#cmakedefine USE_PRESENTATION_MODE 1

/* Define to 1 if advanced debug messages are enabled */
#cmakedefine USE_ADVANCEDDEBUGMSG 1

/* Define to 1 if you have Nepomuk shared libraries installed */
#cmakedefine HAVE_NEPOMUK 1

/* Define to 1 if we know how to start mysqld */
#cmakedefine HAVE_INTERNALMYSQL 1

/* remember the lcms library version to use */
#cmakedefine USE_LCMS_VERSION_1000 1
#cmakedefine USE_LCMS_VERSION_2000 1

#define LIBEXEC_INSTALL_DIR "${LIBEXEC_INSTALL_DIR}"

/*
  Disable indeep warnings from Visual Studio C++ 2008 (9.0)
*/
#if defined(_MSC_VER)
// To disable warnings about no suitable definition provided for explicit template instantiation request.
#pragma warning(disable : 4661)
// To disable warnings about deprecated POSIX methods().
#pragma warning(disable : 4996)
// To disable warnings about qualifier applied to reference type ignored.
#pragma warning(disable : 4181)
#endif

#endif /* CONFIG_DIGIKAM_H */
