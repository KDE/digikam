/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-10-29
 * Description : extension to IccManager providing UI
 *
 * Copyright (C) 2005-2006 by F.J. Cruz <fj dot cruz at supercable dot es>
 * Copyright (C) 2005-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef ICCPOSTLOADINGMANAGER_H
#define ICCPOSTLOADINGMANAGER_H

// Local includes

#include "digikam_export.h"
#include "iccmanager.h"

namespace Digikam
{

class DIGIKAM_EXPORT IccPostLoadingManager : public IccManager
{

public:

    /**
     * Constructs an IccPostLoadingManager object.
     * The DImg will be edited. The filePath is for display only.
     */
    explicit IccPostLoadingManager(DImg& image, const QString& filePath = QString(),
                                   const ICCSettingsContainer& settings = IccSettings::instance()->settings());

    /**
     * Carries out color management asking the user for his decision.
     * Afterwards, needsPostLoadingManagement will return false.
     */
    IccTransform postLoadingManage(QWidget* const parent = 0);

protected:

    QString m_filePath;
};

}  // namespace Digikam

#endif   // ICCMANAGER_H
