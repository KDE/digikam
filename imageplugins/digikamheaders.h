/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-12-22
 * Copyright 2004 by Renchi Raju
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
 * ============================================================ */

#ifndef DIGIKAMHEADERS_H
#define DIGIKAMHEADERS_H

#include <config.h>

#ifdef HAVE_DIGIKAM_TOPLEVEL

#include <guiclient.h>
#include <guifactory.h>
#include <colorgradientwidget.h>
#include <histogramwidget.h>
#include <imagecurves.h>
#include <imagehistogram.h>
#include <imageiface.h>
#include <imagelevels.h>
#include <imagepaniconwidget.h>
#include <imageplugin.h>
#include <imagepreviewwidget.h>
#include <imageregionwidget.h>
#include <imageselectionwidget.h>
#include <imagewidget.h>

#else

#include <digikam/guiclient.h>
#include <digikam/guifactory.h>
#include <digikam/colorgradientwidget.h>
#include <digikam/histogramwidget.h>
#include <digikam/imagecurves.h>
#include <digikam/imagehistogram.h>
#include <digikam/imageiface.h>
#include <digikam/imagelevels.h>
#include <digikam/imagepaniconwidget.h>
#include <digikam/imageplugin.h>
#include <digikam/imagepreviewwidget.h>
#include <digikam/imageregionwidget.h>
#include <digikam/imageselectionwidget.h>
#include <digikam/imagewidget.h>

#endif

#endif /* DIGIKAMHEADERS_H */
