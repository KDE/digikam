/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-10-03
 * Description : Private Qt item view for images
 *
 * Copyright (C) 2009-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2009-2014 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAMIMAGEVIEW_P_H_
#define DIGIKAMIMAGEVIEW_P_H_

// Qt includes

#include <QObject>

// Local includes

#include "config-digikam.h"
#include "digikamimageview.h"
#include "digikamimagedelegate.h"
#include "imagerotationoverlay.h"
#include "applicationsettings.h"

#ifdef HAVE_KFACE
#include "facepipeline.h"
#endif /* HAVE_KFACE */

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

#ifdef HAVE_KFACE
    FacePipeline              editPipeline;
#endif /* HAVE_KFACE */

    DigikamImageDelegate*     normalDelegate;
    DigikamImageFaceDelegate* faceDelegate;

    bool                      overlaysActive;

    ImageRotateOverlay*       rotateLeftOverlay;
    ImageRotateOverlay*       rotateRightOverlay;

    bool                      faceMode;

private:

    DigikamImageView*         q_ptr;
};

} // namespace Digikam

#endif /* DIGIKAMIMAGEVIEW_P_H_ */
