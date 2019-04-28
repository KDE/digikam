/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-09-18
 * Description : lens auto-fix batch tool.
 *
 * Copyright (C) 2010-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAM_BQM_LENS_AUTO_FIX_H
#define DIGIKAM_BQM_LENS_AUTO_FIX_H

#include "batchtool.h"

using namespace Digikam;

namespace DigikamBqmLensAutoFixPlugin
{

class LensAutoFix : public BatchTool
{
    Q_OBJECT

public:

    explicit LensAutoFix(QObject* const parent = nullptr);
    ~LensAutoFix();

    BatchToolSettings defaultSettings();

    BatchTool* clone(QObject* const parent=nullptr) const { return new LensAutoFix(parent); };

    void registerSettingsWidget();

private:

    bool toolOperations();

private Q_SLOTS:

    void slotAssignSettings2Widget();
    void slotSettingsChanged();

private:

    class Private;
    Private* const d;
};

} // namespace DigikamBqmLensAutoFixPlugin

#endif // DIGIKAM_BQM_LENS_AUTO_FIX_H
