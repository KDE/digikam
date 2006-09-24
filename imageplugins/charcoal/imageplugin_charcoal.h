/* ============================================================
 * File  : imageplugin_charcoal.h
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2004-08-26
 * Description : 
 * 
 * Copyright 2004-2006 by Gilles Caulier
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


#ifndef IMAGEPLUGIN_CHARCOAL_H
#define IMAGEPLUGIN_CHARCOAL_H

// Digikam includes.

#include <digikamheaders.h>

class KAction;

class DIGIKAMIMAGEPLUGINS_EXPORT ImagePlugin_Charcoal : public Digikam::ImagePlugin
{
    Q_OBJECT
    
public:

    ImagePlugin_Charcoal(QObject *parent, const char* name,
                         const QStringList &args);
    ~ImagePlugin_Charcoal();
    
    void setEnabledActions(bool enable);

private:

    KAction *m_charcoalAction;
    
private slots:

    void slotCharcoal();

};
    
#endif /* IMAGEPLUGIN_CHARCOAL_H */
