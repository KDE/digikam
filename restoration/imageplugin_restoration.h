/* ============================================================
 * File  : imageplugin_restoration.h
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2005-03-26
 * Description : 
 * 
 * Copyright 2005 by Gilles Caulier
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


#ifndef IMAGEPLUGIN_RESTORATION_H
#define IMAGEPLUGIN_RESTORATION_H

// Digikam includes.

#include <digikamheaders.h>

class KAction;

class ImagePlugin_Restoration : public Digikam::ImagePlugin
{
    Q_OBJECT
    
public:

    ImagePlugin_Restoration(QObject *parent, const char* name,
                         const QStringList &args);
    ~ImagePlugin_Restoration();

    void setEnabledActions(bool enable);

private:

    KAction *m_restorationAction;
   
private slots:

    void slotRestoration();

};
    
#endif /* IMAGEPLUGIN_RESTORATION_H */
