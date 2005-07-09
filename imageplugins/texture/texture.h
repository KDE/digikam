/* ============================================================
 * File  : texture.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-05-25
 * Description : Texture threaded image filter.
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
  
#ifndef TEXTURE_H
#define TEXTURE_H

// Digikam includes.

#include <digikamheaders.h>

namespace DigikamTextureImagesPlugin
{

class Texture : public Digikam::ThreadedFilter
{

public:
    
    Texture(QImage *orgImage, QObject *parent=0, int blendGain=200, 
            QString texturePath=QString::null);
    
    ~Texture(){};
            
private:  // Texture filter data.

    int     m_blendGain;
    
    QString m_texturePath;
    
    QImage  m_textureImg;
    
private:  // Texture filter methods.

    virtual void filterImage(void);

};    

}  // NameSpace DigikamTextureImagesPlugin

#endif /* TEXTURE_H */
