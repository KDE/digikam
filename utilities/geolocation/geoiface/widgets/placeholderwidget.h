/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-12-05
 * Description : Placeholder widget for when backends are activated
 *
 * Copyright (C) 2010-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C)      2010 by Michael G. Hansen <mike at mghansen dot de>
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

#ifndef PLACE_HOLDER_WIDGET_H
#define PLACE_HOLDER_WIDGET_H

// Qt includes

#include <QFrame>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT PlaceholderWidget : public QFrame
{
    Q_OBJECT

public:

    explicit PlaceholderWidget(QWidget* const parent = 0);
    ~PlaceholderWidget();

    void setMessage(const QString& message);

private:

    Q_DISABLE_COPY(PlaceholderWidget)

    class Private;
    const QScopedPointer<Private> d;
};

} // namespace Digikam

#endif // PLACE_HOLDER_WIDGET_H
