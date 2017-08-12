/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-09-01
 * Description : a plugin to create photo layouts by fusion of several images.
 * 
 *
 * Copyright (C) 2011      by Lukasz Spas <lukasz dot spas at gmail dot com>
 * Copyright (C) 2009-2015 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef PLE_CONFIG_SKELETON_H
#define PLE_CONFIG_SKELETON_H

#include <kconfigskeleton.h>
#include "digikam_debug.h"

namespace PhotoLayoutsEditor
{

class PLEConfigSkeleton : public KConfigSkeleton
{
  public:

    static PLEConfigSkeleton *self();
    ~PLEConfigSkeleton();

    /**
      Set Embed full-sized imaged.
    */
    static
    void setEmbedImages( bool v )
    {
      if (!self()->isImmutable( QString::fromLatin1( "EmbedImages" ) ))
        self()->mEmbedImages = v;
    }

    /**
      Get Embed full-sized imaged.
    */
    static
    bool embedImages()
    {
      return self()->mEmbedImages;
    }

    /**
      Set Horizontal distance
    */
    static
    void setHorizontalGrid( double v )
    {
      if (!self()->isImmutable( QString::fromLatin1( "HorizontalGrid" ) ))
        self()->mHorizontalGrid = v;
    }

    /**
      Get Horizontal distance
    */
    static
    double horizontalGrid()
    {
      return self()->mHorizontalGrid;
    }

    /**
      Set Vertical distance
    */
    static
    void setVerticalGrid( double v )
    {
      if (!self()->isImmutable( QString::fromLatin1( "VerticalGrid" ) ))
        self()->mVerticalGrid = v;
    }

    /**
      Get Vertical distance
    */
    static
    double verticalGrid()
    {
      return self()->mVerticalGrid;
    }

  protected:

    PLEConfigSkeleton();
    friend class PLEConfigSkeletonHelper;


    // Saving
    bool mEmbedImages;

    // View
    double mHorizontalGrid;
    double mVerticalGrid;
};

} // namespace PhotoLayoutsEditor

#endif // PLE_CONFIG_SKELETON_H
