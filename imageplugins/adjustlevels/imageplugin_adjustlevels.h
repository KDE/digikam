/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-06-04
 * Description : image histogram adjust levels. 
 * 
 * Copyright (C) 2004-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef IMAGEPLUGIN_ADJUSTLEVELS_H
#define IMAGEPLUGIN_ADJUSTLEVELS_H

// Digikam includes.

#include "imageplugin.h"
#include "digikam_export.h"

class KAction;

class DIGIKAMIMAGEPLUGINS_EXPORT ImagePlugin_AdjustLevels : public Digikam::ImagePlugin
{
    Q_OBJECT
    
public:

    ImagePlugin_AdjustLevels(QObject *parent, const char* name,
                             const QStringList &args);
    ~ImagePlugin_AdjustLevels();
    
    void setEnabledActions(bool enable);

private slots:

    void slotLevelsAdjust();

private:

    KAction *m_levelsAction;
};
    
#endif /* IMAGEPLUGIN_ADJUSTLEVELS_H */
