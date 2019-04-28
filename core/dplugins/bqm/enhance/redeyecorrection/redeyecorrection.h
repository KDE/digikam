/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 16/08/2016
 * Description : A Red-Eye tool for automatic detection and correction filter.
 *
 * Copyright (C) 2016 by Omar Amin <Omar dot moh dot amin at gmail dot com>
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

#ifndef DIGIKAM_BQM_RED_EYE_CORRECTION_H
#define DIGIKAM_BQM_RED_EYE_CORRECTION_H

// Local includes

#include "batchtool.h"
#include "redeyecorrectionfilter.h"
#include "redeyecorrectionsettings.h"

using namespace Digikam;

namespace DigikamBqmRestorationPlugin
{

class RedEyeCorrection : public BatchTool
{
    Q_OBJECT

public:

    explicit RedEyeCorrection(QObject* const parent = nullptr);
    virtual ~RedEyeCorrection();

    BatchToolSettings defaultSettings();

    BatchTool* clone(QObject* const parent=nullptr) const { return new RedEyeCorrection(parent); };

    void registerSettingsWidget();

    void cancel();

private:

    bool toolOperations();

private Q_SLOTS:

    void slotAssignSettings2Widget();
    void slotSettingsChanged();

private:

    RedEyeCorrectionFilter*   m_redEyeCFilter;
    RedEyeCorrectionSettings* m_settingsView;
};

} // namespace DigikamBqmRestorationPlugin

#endif // DIGIKAM_BQM_RED_EYE_CORRECTION_H
