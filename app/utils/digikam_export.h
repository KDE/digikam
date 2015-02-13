/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-08-03
 * Description : digiKam config header
 *
 * Copyright (C) 2014-2015 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAM_EXPORT_H
#define DIGIKAM_EXPORT_H

#ifdef DIGIKAM_STATIC_DEFINE

#  define DIGIKAM_EXPORT
#  define DIGIKAM_NO_EXPORT
#  define DIGIKAM_DATABASE_EXPORT
#  define DIGIKAM_DATABASE_NO_EXPORT
#  define DIGIKAM_DATABASECORE_EXPORT
#  define DIGIKAM_DATABASECORE_NO_EXPORT

#else

#  ifndef DIGIKAM_EXPORT
#    ifdef KF5digikam_EXPORTS
        /* We are building this library */
#      define DIGIKAM_EXPORT __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define DIGIKAM_EXPORT __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef DIGIKAM_DATABASE_EXPORT
#    ifdef KF5digikam_DATABASE_EXPORTS
        /* We are building this library */
#      define DIGIKAM_DATABASE_EXPORT __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define DIGIKAM_DATABASE_EXPORT __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef DIGIKAM_DATABASECORE_EXPORT
#    ifdef KF5digikam_DATABASECORE_EXPORTS
        /* We are building this library */
#      define DIGIKAM_DATABASECORE_EXPORT __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define DIGIKAM_DATABASECORE_EXPORT __attribute__((visibility("default")))
#    endif
#  endif

// ---------------------------------------------------------------------------------

#  ifndef DIGIKAM_NO_EXPORT
#    define DIGIKAM_NO_EXPORT __attribute__((visibility("hidden")))
#  endif

#  ifndef DIGIKAM_DATABASE_NO_EXPORT
#    define DIGIKAM_DATABASE_NO_EXPORT __attribute__((visibility("hidden")))
#  endif

#  ifndef DIGIKAM_DATABASECORE_NO_EXPORT
#    define DIGIKAM_DATABASECORE_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

// ---------------------------------------------------------------------------------

#ifndef DIGIKAM_DEPRECATED
#  define DIGIKAM_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef DIGIKAM_DATABASE_DEPRECATED
#  define DIGIKAM_DATABASE_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef DIGIKAM_DATABASECORE_DEPRECATED
#  define DIGIKAM_DATABASECORE_DEPRECATED __attribute__ ((__deprecated__))
#endif

// ---------------------------------------------------------------------------------

#ifndef DIGIKAM_DEPRECATED_EXPORT
#  define DIGIKAM_DEPRECATED_EXPORT DIGIKAM_EXPORT DIGIKAM_DEPRECATED
#endif

#ifndef DIGIKAM_DATABASE_DEPRECATED_EXPORT
#  define DIGIKAM_DATABASE_DEPRECATED_EXPORT DIGIKAM_DATABASE_EXPORT DIGIKAM_DATABASE_DEPRECATED
#endif

#ifndef DIGIKAM_DATABASECORE_DEPRECATED_EXPORT
#  define DIGIKAM_DATABASECORE_DEPRECATED_EXPORT DIGIKAM_DATABASECORE_EXPORT DIGIKAM_DATABASECORE_DEPRECATED
#endif

// ---------------------------------------------------------------------------------

#ifndef DIGIKAM_DEPRECATED_NO_EXPORT
#  define DIGIKAM_DEPRECATED_NO_EXPORT DIGIKAM_NO_EXPORT DIGIKAM_DEPRECATED
#endif

#ifndef DIGIKAM_DATABASE_DEPRECATED_NO_EXPORT
#  define DIGIKAM_DATABASE_DEPRECATED_NO_EXPORT DIGIKAM_DATABASE_NO_EXPORT DIGIKAM_DATABASE_DEPRECATED
#endif

#ifndef DIGIKAM_DATABASECORE_DEPRECATED_NO_EXPORT
#  define DIGIKAM_DATABASECORE_DEPRECATED_NO_EXPORT DIGIKAM_DATABASECORE_NO_EXPORT DIGIKAM_DATABASECORE_DEPRECATED
#endif

// ---------------------------------------------------------------------------------

#define DEFINE_NO_DEPRECATED 0
#if DEFINE_NO_DEPRECATED
# define DIGIKAM_NO_DEPRECATED
# define DIGIKAM_DATABASE_NO_DEPRECATED
# define DIGIKAM_DATABASECORE_NO_DEPRECATED
#endif

#endif // DIGIKAM_EXPORT_H
