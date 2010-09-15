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
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef LENSFUNIFACE_H
#define LENSFUNIFACE_H

// Lib LensFun includes

extern "C"
{
#include <lensfun.h>
}

// local includes

#include "dmetadata.h"
#include "digikam_export.h"
#include "lensfunfilter.h"

namespace Digikam
{

class DIGIKAM_EXPORT LensFunIface
{

public:

    LensFunIface();
    virtual ~LensFunIface();

/*
    typedef QMap<QString, QString> correctionData;
    correctionData getCorrectionData();
*/
    void setSettings(const LensFunContainer& settings);

    bool supportsDistortion();
    bool supportsCCA();
    bool supportsVig();
    bool supportsGeometry(){ return supportsDistortion(); };
    bool supportsCCI()     { return supportsVig();        };

    bool findFromMetadata(const DMetadata& meta);

protected:

    bool init();

private:

    // my configuration
    bool                   m_init;

    LensFunContainer       m_settings;

    // Database items
    lfDatabase*            m_lfDb;
    const lfCamera* const* m_lfCameras;
    const lfLens**         m_lfLenses;
    const lfMount*         m_lfMounts;

    // To be used for modification
    const lfLens*          m_usedLens;
    float                  m_cropFactor;
    float                  m_focalLength;
    float                  m_aperture;
    float                  m_subjectDistance;

    friend class LensFunCameraSelector;
    friend class LensFunFilter;
};

}  // namespace Digikam

#endif /* LENSFUNIFACE_H */
