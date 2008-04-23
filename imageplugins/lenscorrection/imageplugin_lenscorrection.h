/* ============================================================
 *
 * Date        : 2008-02-10
 * Description : A plugin for autocorrect pictues based on lens database
 * 
 * Copyright (C) 2008 Adrian Schroeter <adrian@suse.de>
 * Copyright (C) 2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef IMAGEPLUGIN_LENSCORRECTION_H
#define IMAGEPLUGIN_LENSCORRECTION_H

// Digikam includes.

#include "imageplugin.h"
#include "digikam_export.h"

class KAction;

class DIGIKAMIMAGEPLUGINS_EXPORT ImagePlugin_LensCorrection : public Digikam::ImagePlugin
{
    Q_OBJECT

public:

    ImagePlugin_LensCorrection(QObject *parent, const QVariantList &args);
    ~ImagePlugin_LensCorrection();

    void setEnabledActions(bool enable);

private slots:

    void slotAutoCorrection();
    void slotLensDistortion();
    void slotAntiVignetting();

private:

    KAction *m_autoCorrectionAction;
    KAction *m_lensdistortionAction;
    KAction *m_antivignettingAction;
};

#endif /* IMAGEPLUGIN_LENSCORRECTION_H */
