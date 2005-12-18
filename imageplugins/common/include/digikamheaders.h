/* ============================================================
 * Authors: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *          Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-12-22
 *
 * Copyright 2004 by Renchi Raju
 * Copyright 2005 by Gilles Caulier
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

#include <dimg.h>
#include <dimgthreadedfilter.h>
#include <dcolor.h>
#include <imageiface.h>
#include <imagehistogram.h>
#include <imagelevels.h>
#include <imageplugin.h>
#include <imagecurves.h>
#include <imagefilters.h>
#include <thumbbar.h>
#include <colorgradientwidget.h>
#include <histogramwidget.h>
#include <curveswidget.h>
#include <imagepaniconwidget.h>
#include <imagepannelwidget.h>
#include <imageregionwidget.h>
#include <imageselectionwidget.h>
#include <imageguidewidget.h>
#include <imagewidget.h>
#include <ctrlpaneldlg.h>
#include <imagedlgbase.h>

// FIXME : Revove this line when all plugins will be ported to DImg
#include <threadedfilter.h>

#else

#include <digikam/dimg.h>
#include <digikam/dimgthreadedfilter.h>
#include <digikam/dcolor.h>
#include <digikam/imageiface.h>
#include <digikam/imagehistogram.h>
#include <digikam/imagelevels.h>
#include <digikam/imageplugin.h>
#include <digikam/imagecurves.h>
#include <digikam/imagefilters.h>
#include <digikam/thumbbar.h>
#include <digikam/colorgradientwidget.h>
#include <digikam/histogramwidget.h>
#include <digikam/curveswidget.h>
#include <digikam/imagepaniconwidget.h>
#include <digikam/imagepannelwidget.h>
#include <digikam/imageregionwidget.h>
#include <digikam/imageselectionwidget.h>
#include <digikam/imageguidewidget.h>
#include <digikam/imagewidget.h>
#include <digikam/ctrlpaneldlg.h>
#include <digikam/imagedlgbase.h>

// FIXME : Revove this line when all plugins will be ported to DImg
#include <digikam/threadedfilter.h>

#endif

#endif /* DIGIKAMHEADERS_H */
