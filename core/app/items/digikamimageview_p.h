/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-10-03
 * Description : Private Qt item view for images
 *
 * Copyright (C) 2009-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009-2010 by Johannes Wienke <languitar at semipol do de>
 *
 * This program is free software you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef DIGIKAM_IMAGE_VIEW_P_H_
#define DIGIKAM_IMAGE_VIEW_P_H_

// Qt includes

#include <QObject>

// Local includes

#include "digikam_config.h"
#include "digikamimageview.h"
#include "digikamimagedelegate.h"
#include "imagerotationoverlay.h"
#include "imagefsoverlay.h"
#include "applicationsettings.h"
#include "facepipeline.h"

namespace Digikam
{

class DigikamImageDelegate;
class DigikamImageFaceDelegate;

class DigikamImageView::Private : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(DigikamImageView)

public:

    explicit Private(DigikamImageView* const qq);
    virtual ~Private();

    void updateOverlays();
    void triggerRotateAction(const char* actionName);

public:

    ImageViewUtilities*       utilities;

    FacePipeline              editPipeline;

    DigikamImageDelegate*     normalDelegate;
    DigikamImageFaceDelegate* faceDelegate;

    bool                      overlaysActive;
    bool                      fullscreenActive;

    ImageRotateOverlay*       rotateLeftOverlay;
    ImageRotateOverlay*       rotateRightOverlay;
    ImageFsOverlay*           fullscreenOverlay;

    bool                      faceMode;

private:

    DigikamImageView*         q_ptr;
};

} // namespace Digikam

#endif // DIGIKAM_IMAGE_VIEW_P_H_
