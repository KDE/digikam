/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2014-09-12
 * Description : Vertical and horizontal layout widget helpers.
 *
 * Copyright (C) 2014-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DLAYOUT_BOX_H
#define DLAYOUT_BOX_H

// Qt includes

#include <QWidget>
#include <QFrame>
#include <QSize>
#include <QMargins>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

/** An Horizontal widget to host children widgets
 */
class DIGIKAM_EXPORT DHBox : public QFrame
{
    Q_OBJECT
    Q_DISABLE_COPY(DHBox)

public:

    explicit DHBox(QWidget* const parent=0);
    virtual ~DHBox();

    void setSpacing(int space);
    void setContentsMargins(const QMargins& margins);
    void setContentsMargins(int left, int top, int right, int bottom);
    void setStretchFactor(QWidget* const widget, int stretch);

    virtual QSize sizeHint() const;
    virtual QSize minimumSizeHint() const;

protected:

    DHBox(bool vertical, QWidget* const parent);

    virtual void childEvent(QChildEvent* e);
};

// ------------------------------------------------------------------------------------

/** A Vertical widget to host children widgets
 */
class DIGIKAM_EXPORT DVBox : public DHBox
{
    Q_OBJECT
    Q_DISABLE_COPY(DVBox)

  public:

    explicit DVBox(QWidget* const parent=0);
    virtual ~DVBox();
};

} // namespace Digikam

#endif // DLAYOUT_BOX_H
