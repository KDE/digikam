/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-04-30
 * Description : Qt item view mouse hover button
 *
 * Copyright (C) 2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include <QAbstractButton>
#include <QAbstractItemView>

// KDE includes

// Local includes

#include "imagedelegateoverlay.h"

namespace Digikam
{

class RatingBox;

class ImageRatingOverlay : public AbstractWidgetDelegateOverlay
{
    Q_OBJECT

public:

    ImageRatingOverlay(QObject *parent);

    RatingBox *ratingBox() const;

protected:

    virtual QWidget *createWidget();
    virtual void visualChange();
    virtual void mouseMoved(QMouseEvent *e, const QRect& visualRect, const QModelIndex& index);
    virtual void slotEntered(const QModelIndex& index);

protected:

    void updateBox(const QModelIndex &index);

    QModelIndex m_index;
};


} // namespace Digikam

#endif /* IMAGERATINGOVERLAY_H */
