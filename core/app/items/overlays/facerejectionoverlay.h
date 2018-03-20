/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-04-30
 * Description : rejection icon view item on mouse hover
 *
 * Copyright (C) 2009-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef FACEREJECTIONOVERLAY_H
#define FACEREJECTIONOVERLAY_H

// Qt includes

#include <QAbstractItemView>

// Local includes

#include "itemviewhoverbutton.h"
#include "imagedelegateoverlay.h"
#include "imageinfo.h"

namespace Digikam
{

class FaceRejectionOverlayButton : public ItemViewHoverButton
{
public:

    explicit FaceRejectionOverlayButton(QAbstractItemView* const parentView);
    virtual QSize sizeHint() const;

protected:

    virtual QIcon icon();
    virtual void updateToolTip();
};

// --------------------------------------------------------------------

class FaceRejectionOverlay : public HoverButtonDelegateOverlay
{
    Q_OBJECT

public:

    explicit FaceRejectionOverlay(QObject* const parent);
    virtual void setActive(bool active);

Q_SIGNALS:

    void rejectFaces(const QList<QModelIndex>& indexes);

protected:

    virtual ItemViewHoverButton* createButton();
    virtual void updateButton(const QModelIndex& index);
    virtual bool checkIndex(const QModelIndex& index) const;
    virtual void widgetEnterEvent();
    virtual void widgetLeaveEvent();

protected Q_SLOTS:

    void slotClicked();
};

} // namespace Digikam

#endif /* IMAGESELECTIONOVERLAY_H */
