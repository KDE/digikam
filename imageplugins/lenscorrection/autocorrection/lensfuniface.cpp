/* ============================================================
 *
 * Date        : 2008-02-10
 * Description : a plugin to fix automatically camera lens aberrations
 *
 * Copyright (C) 2008 by Adrian Schroeter <adrian at suse dot de>
 * Copyright (C) 2008-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "lensfuniface.h"

// KDE includes

#include <kdebug.h>

namespace DigikamAutoCorrectionImagesPlugin
{

LensFunIface::LensFunIface()
{
    m_init = false;
    init();
}

LensFunIface::~LensFunIface()
{
    if (m_init)
    {
    }
}

bool LensFunIface::init()
{
    m_lfDb       = lf_db_new();
    m_lfDb->Load();
    m_lfCameras  = m_lfDb->GetCameras();
    m_init       = true;
    m_usedLens   = NULL;
    m_filterCCA  = true;
    m_filterVig  = true;
    m_filterDist = true;

    return true;
}

void LensFunIface::setCorrection(bool CCA, bool Vig, bool CCI, bool Dist, bool Geom)
{
    m_filterCCA  = CCA;
    m_filterVig  = Vig;
    m_filterCCI  = CCI;
    m_filterDist = Dist;
    m_filterGeom = Geom;
}

bool LensFunIface::supportsDistortion()
{
    if (m_usedLens == NULL) return false;

    lfLensCalibDistortion res;
    return m_usedLens->InterpolateDistortion(m_focalLength, res);
}

bool LensFunIface::supportsCCA()
{
    if (m_usedLens == NULL) return false;

    lfLensCalibTCA res;
    return m_usedLens->InterpolateTCA(m_focalLength, res);
}

bool LensFunIface::supportsVig()
{
    if (m_usedLens == NULL) return false;

    lfLensCalibVignetting res;
    return m_usedLens->InterpolateVignetting(m_focalLength, m_aperture, m_subjectDistance, res);
}

#if 0
LensFunIface::correctionData LensFunIface::getCorrectionData()
{
}
#endif

}  // namespace DigikamAutoCorrectionImagesPlugin
