/* ============================================================
 *
 * Date        : 2008-02-10
 * Description : a tool to fix automatically camera lens aberrations
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

// local includes

#include "dmetadata.h"
#include "digikam_export.h"
#include "lensfunfilter.h"

namespace Digikam
{

class DIGIKAM_EXPORT LensFunIface
{
    public:

/*
    typedef QMap<QString, QString> correctionData;
    correctionData getCorrectionData();
*/

public:

    LensFunIface();
    virtual ~LensFunIface();

    void setFilterSettings(const LensFunContainer& other);

    void setSettings(const LensFunContainer& other);
    LensFunContainer settings() const;

    LensFunContainer::DevicePtr findCamera(const QString& make, const QString& model) const;
    LensFunContainer::LensPtr   findLens(const QString& model) const;

    bool findFromMetadata(const DMetadata& meta, LensFunContainer& settings) const;

protected:

    bool init();

private:

    LensFunContainer::LensList findLenses(const lfCamera* camera, const QString& lensDesc,
                                          const QString& lensMaker=QString()) const;

private:

    // my configuration
    bool                   m_init;

    // Database items
    lfDatabase*            m_lfDb;
    const lfMount*         m_lfMounts;
    const lfCamera* const* m_lfCameras;
    const lfLens**         m_lfLenses;

    // To be used for modification
    LensFunContainer       m_settings;

    friend class LensFunCameraSelector;
    friend class LensFunFilter;
};

}  // namespace Digikam

#endif /* LENSFUNIFACE_H */
