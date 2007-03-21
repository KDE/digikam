/* ============================================================
 * Authors: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *          Gilles Caulier <caulier dot gilles at gmail dot com> 
 * Date   : 2004-02-14
 * Description : 
 * 
 * Copyright 2004-2005 by Renchi Raju
 * Copyright 2006-2007 by Gilles Caulier
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

#ifndef IMAGEPLUGIN_COLORFX_H
#define IMAGEPLUGIN_COLORFX_H

// Digikam includes.

#include "imageplugin.h"
#include "digikam_export.h"

class KAction;

class DIGIKAMIMAGEPLUGINS_EXPORT ImagePlugin_ColorFX : public Digikam::ImagePlugin
{
    Q_OBJECT

public:

    ImagePlugin_ColorFX(QObject *parent, const char* name, const QStringList &args);
    ~ImagePlugin_ColorFX();

    void setEnabledActions(bool enable);

private slots:

    void slotColorFX();

private:

    KAction *m_solarizeAction;
};

#endif /* IMAGEPLUGIN_COLORFX_H */
