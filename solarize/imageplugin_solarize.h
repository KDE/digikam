/* ============================================================
 * File  : imageplugin_solarize.h
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-02-14
 * Description : 
 * 
 * Copyright 2004-2005 by Renchi Raju
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


#ifndef IMAGEPLUGIN_SOLARIZE_H
#define IMAGEPLUGIN_SOLARIZE_H

// Digikam includes.

#include <digikamheaders.h>

class KAction;

class ImagePlugin_Solarize : public Digikam::ImagePlugin
{
    Q_OBJECT

public:

    ImagePlugin_Solarize(QObject *parent, const char* name,
                         const QStringList &args);
    ~ImagePlugin_Solarize();

    void setEnabledActions(bool enable);

private:

    KAction *m_solarizeAction;

private slots:

    void slotSolarize();

};

#endif /* IMAGEPLUGIN_SOLARIZE_H */
