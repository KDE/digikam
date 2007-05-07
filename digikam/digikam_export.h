/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-31-01
 * Description : gcc export extension support
 * 
 * Copyright (c) 2005 Laurent Montel <montel@kde.org>
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

#ifndef _DIGIKAM_EXPORT_H
#define _DIGIKAM_EXPORT_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef __KDE_HAVE_GCC_VISIBILITY
#define DIGIKAM_EXPORT __attribute__ ((visibility("default")))
#define DIGIKAMIMAGEPLUGINS_EXPORT DIGIKAM_EXPORT
#define DIGIKAMIMAGEEDITOR_EXPORT  DIGIKAM_EXPORT
#define DIGIKAMIMAGEFILTERS_EXPORT DIGIKAM_EXPORT
#define DIGIKAMIMAGEWIDGET_EXPORT  DIGIKAM_EXPORT
#else
#define DIGIKAM_EXPORT
#define DIGIKAMIMAGEPLUGINS_EXPORT 
#define DIGIKAMIMAGEEDITOR_EXPORT 
#define DIGIKAMIMAGEFILTERS_EXPORT
#define DIGIKAMIMAGEWIDGET_EXPORT
#endif
#endif /* _DIGIKAM_EXPORT_H */

