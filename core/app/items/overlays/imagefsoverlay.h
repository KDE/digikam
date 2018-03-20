/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2015-08-30
 * Description : fullscreen overlay
 *
 * Copyright (C)      2015 by Luca Carlon <carlon dot luca at gmail dot com>
 * Copyright (C) 2015-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef IMAGEFSOVERLAY_H
#define IMAGEFSOVERLAY_H

// Qt includes

#include <QAbstractButton>
#include <QAbstractItemView>

// Local includes

#include "itemviewhoverbutton.h"
#include "imagedelegateoverlay.h"

namespace Digikam
{

class ImageFsOverlayButton : public ItemViewHoverButton
{
public:

    explicit ImageFsOverlayButton(QAbstractItemView* const parentView);
    virtual QSize sizeHint() const;

protected:

    virtual QIcon icon();
    virtual void updateToolTip();
};

// --------------------------------------------------------------------

class ImageFsOverlay : public HoverButtonDelegateOverlay
{
    Q_OBJECT

public:

    explicit ImageFsOverlay(QObject* const parent);
    virtual void setActive(bool active);

    static ImageFsOverlay* instance(QObject* const parent);

Q_SIGNALS:

    void signalFullscreen(const QList<QModelIndex>& indexes);

protected:

    virtual ItemViewHoverButton* createButton();
    virtual void updateButton(const QModelIndex& index);
    virtual bool checkIndex(const QModelIndex& index) const;
    virtual void widgetEnterEvent();
    virtual void widgetLeaveEvent();

private Q_SLOTS:

    void slotClicked();
};

} // namespace Digikam

#endif // IMAGEFSOVERLAY_H
