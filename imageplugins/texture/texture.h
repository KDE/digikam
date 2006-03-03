/* ============================================================
 * File  : texture.h
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
           Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Date  : 2005-05-25
 * Description : Texture threaded image filter.
 * 
 * Copyright 2005 by Gilles Caulier
 * Copyright 2006 by Gilles Caulier and Marcel Wiesweg
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

class Texture : public Digikam::DImgThreadedFilter
{

public:

    Texture(Digikam::DImg *orgImage, QObject *parent=0, int blendGain=200,
            QString texturePath=QString::null);

    ~Texture(){};

private:  // Texture filter data.

    int     m_blendGain;

    QString m_texturePath;
private:  // Texture filter methods.

    virtual void filterImage(void);

};

}  // NameSpace DigikamTextureImagesPlugin

#endif /* TEXTURE_H */
