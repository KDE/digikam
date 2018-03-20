/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-12-08
 * Description : ICC Settings Container.
 *
 * Copyright (C) 2005-2007 by F.J. Cruz <fj dot cruz at supercable dot es>
 * Copyright (C) 2005-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef ICCSETTINGSCONTAINER_H
#define ICCSETTINGSCONTAINER_H

// Qt includes

#include <QColor>
#include <QFlags>
#include <QString>

// Local includes

#include "digikam_export.h"

class KConfigGroup;

namespace Digikam
{

class DIGIKAM_EXPORT ICCSettingsContainer
{

public:

    enum BehaviorEnum
    {
        // Note: Values are stored in config - keep them constant

        InvalidBehavior          = 0,

        /// Interpretation of the image data

        UseEmbeddedProfile       = 1 << 0,
        UseSRGB                  = 1 << 1,
        UseWorkspace             = 1 << 2,
        UseDefaultInputProfile   = 1 << 3,
        UseSpecifiedProfile      = 1 << 4,
        AutomaticColors          = 1 << 5,
        DoNotInterpret           = 1 << 6,

        /// Transformation / target profile

        KeepProfile              = 1 << 10,
        ConvertToWorkspace       = 1 << 11,

        /// Special flags and values

        LeaveFileUntagged        = 1 << 18,

        AskUser                  = 1 << 20,
        SafestBestAction         = 1 << 21,

        /// ready combinations for convenience

        PreserveEmbeddedProfile  = UseEmbeddedProfile     | KeepProfile,
        EmbeddedToWorkspace      = UseEmbeddedProfile     | ConvertToWorkspace,
        SRGBToWorkspace          = UseSRGB                | ConvertToWorkspace,
        AutoToWorkspace          = AutomaticColors        | ConvertToWorkspace,
        InputToWorkspace         = UseDefaultInputProfile | ConvertToWorkspace,
        SpecifiedToWorkspace     = UseSpecifiedProfile    | ConvertToWorkspace,
        NoColorManagement        = DoNotInterpret         | LeaveFileUntagged
    };
    Q_DECLARE_FLAGS(Behavior, BehaviorEnum)

public:

    ICCSettingsContainer();
    ~ICCSettingsContainer() {};

    void readFromConfig(KConfigGroup& group);
    void writeToConfig(KConfigGroup& group) const;
    void writeManagedViewToConfig(KConfigGroup& group) const;
    void writeManagedPreviewsToConfig(KConfigGroup& group) const;

public:

    bool     enableCM;

    QString  iccFolder;

    QString  workspaceProfile;

    Behavior defaultMismatchBehavior;
    Behavior defaultMissingProfileBehavior;
    Behavior defaultUncalibratedBehavior;

    Behavior lastMismatchBehavior;
    Behavior lastMissingProfileBehavior;
    Behavior lastUncalibratedBehavior;
    QString  lastSpecifiedAssignProfile;
    QString  lastSpecifiedInputProfile;

    bool     useManagedView;
    bool     useManagedPreviews;
    QString  monitorProfile;

    QString  defaultInputProfile;
    QString  defaultProofProfile;

    bool     useBPC;
    int      renderingIntent;

    // Settings specific for soft proofing
    int      proofingRenderingIntent;
    int      doGamutCheck;
    QColor   gamutCheckMaskColor;
};

}  // namespace Digikam

Q_DECLARE_OPERATORS_FOR_FLAGS(Digikam::ICCSettingsContainer::Behavior)

#endif  // ICCSETTINGSCONTAINER_H
