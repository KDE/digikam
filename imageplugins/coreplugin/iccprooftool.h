/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-12-21
 * Description : digiKam image editor tool to correct picture
 *               colors using an ICC color profile
 *
 * Copyright (C) 2005-2006 by F.J. Cruz <fj.cruz@supercable.es>
 * Copyright (C) 2006-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef ICCPROOFTOOL_H
#define ICCPROOFTOOL_H

// Local includes

#include "editortool.h"

class QByteArray;
class QString;

namespace Digikam
{
class DColor;
}

namespace DigikamImagesPluginCore
{

class ICCProofToolPriv;

class ICCProofTool : public Digikam::EditorTool
{
    Q_OBJECT

public:

    ICCProofTool(QObject* parent);
    ~ICCProofTool();

private:

    void readSettings();
    void writeSettings();
    void finalRendering();

    void getICCInfo(const QString&);
    void getICCInfo(const QByteArray&);

    bool useBPC();
    bool doProof();
    bool checkGamut();
    bool embedProfile();

    bool useEmbeddedProfile();
    bool useBuiltinProfile();
    bool useDefaultInProfile();
    bool useSelectedInProfile();

    bool useDefaultSpaceProfile();
    bool useSelectedSpaceProfile();

    bool useDefaultProofProfile();
    bool useSelectedProofProfile();

private Q_SLOTS:

    void slotSaveAsSettings();
    void slotLoadSettings();
    void slotResetSettings();
    void slotEffect();
    void slotSpotColorChanged(const Digikam::DColor &);
    void slotColorSelectedFromTarget(const Digikam::DColor &);
    void slotToggledWidgets(bool);
    void slotInICCInfo();
    void slotProofICCInfo();
    void slotSpaceICCInfo();
    void slotCMDisabledWarning();
    void processLCMSUrl(const QString&);

private:

    enum ICCSettingsTab
    {
        GENERALPAGE = 0,
        INPUTPAGE,
        WORKSPACEPAGE,
        PROOFINGPAGE,
        LIGHTNESSPAGE
    };

private:

    ICCProofToolPriv* const d;
};

}  // namespace DigikamImagesPluginCore

#endif  // ICCPROOFTOOL_H
