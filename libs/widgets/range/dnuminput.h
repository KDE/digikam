/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-08-16
 * Description : Integer and double num input widget
 *               re-implemented with a reset button to switch to
 *               a default value
 *
 * Copyright (C) 2008-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DNUM_INPUT_H
#define DNUM_INPUT_H

// Qt includes

#include <QWidget>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT DIntNumInput : public QWidget
{
    Q_OBJECT

public:

    DIntNumInput(QWidget* const parent=0);
    ~DIntNumInput();

    void setRange(int min, int max, int step);

    void setDefaultValue(int d);
    int  defaultValue() const;
    int  value()        const;

    void setSuffix(const QString& suffix);

Q_SIGNALS:

    void reset();
    void valueChanged(int);

public Q_SLOTS:

    void setValue(int d);
    void slotReset();

private Q_SLOTS:

    void slotValueChanged(int);

private:

    class Private;
    Private* const d;
};

// ---------------------------------------------------------

class DIGIKAM_EXPORT DDoubleNumInput : public QWidget
{
    Q_OBJECT

public:

    DDoubleNumInput(QWidget* const parent=0);
    ~DDoubleNumInput();

    void   setDecimals(int p);
    void   setRange(double min, double max, double step);

    void   setDefaultValue(double d);
    double defaultValue() const;
    double value()        const;

    void setSuffix(const QString& suffix);

Q_SIGNALS:

    void reset();
    void valueChanged(double);

public Q_SLOTS:

    void setValue(double d);
    void slotReset();

private Q_SLOTS:

    void slotValueChanged(double);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DNUM_INPUT_H
