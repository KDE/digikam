/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-25
 * Description : Texture threaded image filter.
 * 
 * Copyright (C) 2005-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2007 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

// Qt includes.

#include <qstring.h>

// Digikam includes.

#include "dimgthreadedfilter.h"

namespace DigikamTextureImagesPlugin
{

class Texture : public Digikam::DImgThreadedFilter
{

public:

    Texture(Digikam::DImg *orgImage, QObject *parent=0, int blendGain=200,
            QString texturePath=QString());

    ~Texture(){};

private:  

    virtual void filterImage(void);

private:  

    int     m_blendGain;

    QString m_texturePath;
};

}  // NameSpace DigikamTextureImagesPlugin

#endif /* TEXTURE_H */
