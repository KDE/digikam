/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-04-30
 * Description : rating icon view item at mouse hover
 *
 * Copyright (C) 2009      by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef IMAGERATINGOVERLAY_H
#define IMAGERATINGOVERLAY_H

// Qt includes

#include <QAbstractItemView>

// Local includes

#include "imagedelegateoverlay.h"
#include "itemviewimagedelegate.h"

namespace Digikam
{

class RatingWidget;

class ImageRatingOverlay : public AbstractWidgetDelegateOverlay
{
    Q_OBJECT
    REQUIRE_DELEGATE(ItemViewImageDelegate)

public:

    explicit ImageRatingOverlay(QObject* const parent);

    RatingWidget* ratingWidget() const;

Q_SIGNALS:

    void ratingEdited(const QList<QModelIndex>& indexes, int rating);

protected Q_SLOTS:

    void slotRatingChanged(int);
    void slotDataChanged(const QModelIndex&, const QModelIndex&);

protected:

    void updatePosition();
    void updateRating();

protected:

    virtual QWidget* createWidget();
    virtual void setActive(bool);
    virtual void visualChange();
    virtual void hide();
    virtual void slotEntered(const QModelIndex& index);
    virtual void widgetEnterEvent();
    virtual void widgetLeaveEvent();

protected:

    QPersistentModelIndex m_index;
};

} // namespace Digikam

#endif /* IMAGERATINGOVERLAY_H */
