/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-08-09
 * Description : Generic range boxes, i.e. ranges where a minimum and maximum can be given.
 *
 * Copyright (C) 2017 by Mario Frank <mario dot frank at uni minus potsdam dot de>
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

#ifndef DRANGE_BOX_H
#define DRANGE_BOX_H

// Qt includes

#include <QWidget>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT DIntRangeBox : public QWidget
{
    Q_OBJECT

public:

    explicit DIntRangeBox(QWidget* const parent=0);
    ~DIntRangeBox();

    /**
     * This method sets the lower and upper threshold of possible interval minimum and maximum values.
     * @param min the lowest value to which the interval can be expanded.
     * @param max the highest value to which the interval can be expanded.
     **/
    void setRange(int min, int max);

    /**
     * This method sets the minimum and maximum of the interval.
     * @param min The minimum value of the interval.
     * @param max The maximum value of the interval.
     **/
    void setInterval(int min, int max);

    /**
     * This method sets the suffix for the minimum and maximum value boxes.
     * @param suffix The suffix.
     **/
    void setSuffix(const QString& suffix);

    /**
     * This method enables or disables the embedded spinboxes.
     * @param enabled If the interval boxes should be enabled.
     **/
    void setEnabled(bool enabled);

    /**
     * This method returns the minimum value of the interval.
     * @returns the minimum value.
     **/
    int minValue();

    /**
     * This method returns the maximum value of the interval.
     * @returns the maximum value.
     **/
    int maxValue();

Q_SIGNALS:

    void minChanged(int);
    void maxChanged(int);

private Q_SLOTS:

    void slotMinimumChanged(int);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DRANGE_BOX_H
