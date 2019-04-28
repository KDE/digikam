/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-03-02
 * Description : Curves Adjust batch tool.
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

#ifndef DIGIKAM_BQM_CURVES_ADJUST_H
#define DIGIKAM_BQM_CURVES_ADJUST_H

// Local includes

#include "batchtool.h"
#include "curvessettings.h"

class QComboBox;

using namespace Digikam;

namespace DigikamBqmCurvesAdjustPlugin
{

class CurvesAdjust : public BatchTool
{
    Q_OBJECT

public:

    explicit CurvesAdjust(QObject* const parent = nullptr);
    ~CurvesAdjust();

    BatchToolSettings defaultSettings();

    BatchTool* clone(QObject* const parent=nullptr) const { return new CurvesAdjust(parent); };

    void registerSettingsWidget();

public Q_SLOTS:

    void slotResetSettingsToDefault();

private:

    bool toolOperations();

private Q_SLOTS:

    void slotChannelChanged();
    void slotAssignSettings2Widget();
    void slotSettingsChanged();
    void slotSettingsLoad();

private:

    DImg            m_preview;
    QComboBox*      m_channelCB;
    CurvesSettings* m_settingsView;
};

} // namespace DigikamBqmCurvesAdjustPlugin

#endif // DIGIKAM_BQM_CURVES_ADJUST_H
