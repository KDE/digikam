/* ============================================================
 * File  : imageplugin_blurfx.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-02-09
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


#ifndef IMAGEPLUGIN_BLURFX_H
#define IMAGEPLUGIN_BLURFX_H

// Digikam includes.

#include <digikamheaders.h>

class KAction;

class ImagePlugin_BlurFX : public Digikam::ImagePlugin
{
    Q_OBJECT
    
public:

    ImagePlugin_BlurFX(QObject *parent, const char* name,
                         const QStringList &args);
    ~ImagePlugin_BlurFX();
    
    void setEnabledActions(bool enable);

private:

    KAction *m_blurfxAction;
    
private slots:

    void slotBlurFX();

};
    
#endif /* IMAGEPLUGIN_BLURFX_H */
