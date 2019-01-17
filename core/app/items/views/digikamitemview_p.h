/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-10-03
 * Description : Private Qt model-view for items
 *
 * Copyright (C) 2009-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2009-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAM_DIGIKAMITEM_VIEW_P_H
#define DIGIKAM_DIGIKAMITEM_VIEW_P_H

// Qt includes

#include <QObject>

// Local includes

#include "digikam_config.h"
#include "digikamitemview.h"
#include "digikamitemdelegate.h"
#include "itemrotationoverlay.h"
#include "itemfullscreenoverlay.h"
#include "applicationsettings.h"
#include "facepipeline.h"

namespace Digikam
{

class DigikamItemDelegate;
class ItemFaceDelegate;

class Q_DECL_HIDDEN DigikamItemView::Private : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(DigikamItemView)

public:

    explicit Private(DigikamItemView* const qq);
    ~Private();

    void updateOverlays();
    void triggerRotateAction(const char* actionName);

public:

    ItemViewUtilities*       utilities;

    FacePipeline             editPipeline;

    DigikamItemDelegate*     normalDelegate;
    ItemFaceDelegate*        faceDelegate;

    bool                     overlaysActive;
    bool                     fullscreenActive;

    ItemRotateOverlay*       rotateLeftOverlay;
    ItemRotateOverlay*       rotateRightOverlay;
    ItemFullScreenOverlay*   fullscreenOverlay;

    bool                     faceMode;

private:

    DigikamItemView*         q_ptr;

private:

    Private() {};  // disable default constructor.
};

} // namespace Digikam

#endif // DIGIKAM_DIGIKAMITEM_VIEW_P_H
