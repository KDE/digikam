/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-11-27
 * Description : a bar to indicate icon-view filters status
 *
 * Copyright (C) 2007-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAM_FILTER_STATUS_BAR_H
#define DIGIKAM_FILTER_STATUS_BAR_H

// Qt includes

#include <QWidget>

// Local includes

#include "itemfiltersettings.h"

namespace Digikam
{

class FilterStatusBar : public QWidget
{
    Q_OBJECT

public:

    explicit FilterStatusBar(QWidget* const parent);
    ~FilterStatusBar();

public Q_SLOTS:

    void slotFilterMatches(bool);
    void slotFilterSettingsChanged(const ItemFilterSettings& settings);

Q_SIGNALS:

    void signalResetFilters();
    void signalPopupFiltersView();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_FILTER_STATUS_BAR_H
