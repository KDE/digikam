/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-01-04
 * Description : a Digikam image editor plugin for superimpose a 
 *               template to an image.
 *
 * Copyright (C) 2005-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2008 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef IMAGEPLUGIN_SUPERIMPOSE_H
#define IMAGEPLUGIN_SUPERIMPOSE_H

// Digikam includes.

#include "imageplugin.h"
#include "digikam_export.h"

class KAction;

class DIGIKAMIMAGEPLUGINS_EXPORT ImagePlugin_SuperImpose : public Digikam::ImagePlugin
{
    Q_OBJECT

public:

    ImagePlugin_SuperImpose(QObject *parent, const char* name,
                         const QStringList &args);
    ~ImagePlugin_SuperImpose();

    void setEnabledActions(bool enable);

private slots:

    void slotSuperImpose();

private:

    KAction *m_superimposeAction;
};

#endif /* IMAGEPLUGIN_SUPERIMPOSE_H */
