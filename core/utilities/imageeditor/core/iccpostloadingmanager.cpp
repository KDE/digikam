/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-08-12
 * Description : methods that implement color management tasks
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

#include "iccpostloadingmanager.h"

// Qt includes

#include <QPointer>

// Local includes

#include "digikam_debug.h"
#include "colorcorrectiondlg.h"
#include "iccprofile.h"
#include "icctransform.h"

namespace Digikam
{

IccPostLoadingManager::IccPostLoadingManager(DImg& image, const QString& filePath, const ICCSettingsContainer& settings)
    : IccManager(image, settings), m_filePath(filePath)
{
}

IccTransform IccPostLoadingManager::postLoadingManage(QWidget* const parent)
{
    if (image().hasAttribute(QLatin1String("missingProfileAskUser")))
    {
        image().removeAttribute(QLatin1String("missingProfileAskUser"));
        DImg preview                     = image().smoothScale(240, 180, Qt::KeepAspectRatio);
        QPointer<ColorCorrectionDlg> dlg = new ColorCorrectionDlg(ColorCorrectionDlg::MissingProfile, preview,
                                                                  m_filePath, parent);
        dlg->exec();

        IccTransform trans;
        getTransform(trans, dlg->behavior(), dlg->specifiedProfile());
        delete dlg;
        return trans;
    }
    else if (image().hasAttribute(QLatin1String("profileMismatchAskUser")))
    {
        image().removeAttribute(QLatin1String("profileMismatchAskUser"));
        DImg preview                     = image().smoothScale(240, 180, Qt::KeepAspectRatio);
        QPointer<ColorCorrectionDlg> dlg = new ColorCorrectionDlg(ColorCorrectionDlg::ProfileMismatch, preview,
                                                                  m_filePath, parent);
        dlg->exec();

        IccTransform trans;
        getTransform(trans, dlg->behavior(), dlg->specifiedProfile());
        delete dlg;
        return trans;
    }
    else if (image().hasAttribute(QLatin1String("uncalibratedColorAskUser")))
    {
        image().removeAttribute(QLatin1String("uncalibratedColorAskUser"));
        DImg preview                     = image().smoothScale(240, 180, Qt::KeepAspectRatio);
        QPointer<ColorCorrectionDlg> dlg = new ColorCorrectionDlg(ColorCorrectionDlg::UncalibratedColor, preview,
                                                                  m_filePath, parent);
        dlg->exec();

        IccTransform trans;
        getTransform(trans, dlg->behavior(), dlg->specifiedProfile());
        delete dlg;
        return trans;
    }

    return IccTransform();
}

}  // namespace Digikam
