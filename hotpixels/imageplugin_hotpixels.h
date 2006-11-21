/* ============================================================
 * File  : imageplugin_hotpixels.h
 * Author: Unai Garro <ugarro at users dot sourceforge dot net>
 * Date  : 2005-03-27
 * Description : 
 * 
 * Copyright 2005 by Unai Garro
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


#ifndef IMAGEPLUGIN_HOTPIXELS_H
#define IMAGEPLUGIN_HOTPIXELS_H

// Digikam includes.

#include <digikamheaders.h>

class DIGIKAMIMAGEPLUGINS_EXPORT ImagePlugin_HotPixels : public Digikam::ImagePlugin
{
    Q_OBJECT
    
public:

    ImagePlugin_HotPixels(QObject *parent, const char* name,
                          const QStringList &args);
    ~ImagePlugin_HotPixels();

    void setEnabledActions(bool enable);
        
private:

    KAction *m_hotpixelsAction;
        
private slots:

    void slotHotPixels();

};
    
#endif /* IMAGEPLUGIN_HOTPIXELS_H */
