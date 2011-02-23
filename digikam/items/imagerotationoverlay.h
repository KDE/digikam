/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-05-31
 * Description : rotate icon view item at mouse hover
 *
 * Copyright (C) 2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef IMAGEROTATIONOVERLAY_H
#define IMAGEROTATIONOVERLAY_H

// Qt includes

#include <QAbstractButton>
#include <QAbstractItemView>

// Local includes

#include "itemviewhoverbutton.h"
#include "imagedelegateoverlay.h"

namespace Digikam
{

class ImageRotateLeftOverlayButton : public ItemViewHoverButton
{
public:

    ImageRotateLeftOverlayButton(QAbstractItemView* parentView);
    virtual QSize sizeHint() const;

protected:

    virtual QPixmap icon();
    virtual void updateToolTip();
};

// --------------------------------------------------------------------

class ImageRotateRightOverlayButton : public ItemViewHoverButton
{
public:

    ImageRotateRightOverlayButton(QAbstractItemView* parentView);
    virtual QSize sizeHint() const;

protected:

    virtual QPixmap icon();
    virtual void updateToolTip();
};

// --------------------------------------------------------------------

class ImageRotateLeftOverlay : public HoverButtonDelegateOverlay
{
    Q_OBJECT

public:

    ImageRotateLeftOverlay(QObject* parent);
    virtual void setActive(bool active);

Q_SIGNALS:

    void signalRotateLeft();

protected:

    virtual ItemViewHoverButton* createButton();
    virtual void updateButton(const QModelIndex& index);
    virtual bool checkIndex(const QModelIndex& index) const;

private Q_SLOTS:

    void slotClicked();
};

// --------------------------------------------------------------------

class ImageRotateRightOverlay : public HoverButtonDelegateOverlay
{
    Q_OBJECT

public:

    ImageRotateRightOverlay(QObject* parent);
    virtual void setActive(bool active);

Q_SIGNALS:

    void signalRotateRight();

protected:

    virtual ItemViewHoverButton* createButton();
    virtual void updateButton(const QModelIndex& index);
    virtual bool checkIndex(const QModelIndex& index) const;

private Q_SLOTS:

    void slotClicked();
};

} // namespace Digikam

#endif /* IMAGEROTATIONOVERLAY_H */
