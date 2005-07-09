/* ============================================================
 * File  : imageplugin_despeckle.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-08-24
 * Description : 
 * 
 * Copyright 2004-2005 by Gilles Caulier
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


#ifndef IMAGEPLUGIN_DESPECKLE_H
#define IMAGEPLUGIN_DESPECKLE_H

// Digikam includes.

#include <digikamheaders.h>

class KAction;

class ImagePlugin_Despeckle : public Digikam::ImagePlugin
{
    Q_OBJECT
    
public:

    ImagePlugin_Despeckle(QObject *parent, const char* name,
                             const QStringList &args);
    ~ImagePlugin_Despeckle();

    void setEnabledActions(bool enable);

private:

    KAction *m_despeckleAction;
    
private slots:

    void slotDespeckle();

};
    
#endif /* IMAGEPLUGIN_DESPECKLE_H */
