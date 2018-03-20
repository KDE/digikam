/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-08-16
 * Description : a combo box widget re-implemented with a
 *               reset button to switch to a default item
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

#ifndef DCOMBO_BOX_H
#define DCOMBO_BOX_H

// Qt includes

#include <QWidget>
#include <QComboBox>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT DComboBox : public QWidget
{

    Q_OBJECT

public:

    DComboBox(QWidget* const parent=0);
    ~DComboBox();

    void setCurrentIndex(int d);
    int  currentIndex() const;

    void setDefaultIndex(int d);
    int  defaultIndex() const;

    QComboBox* combo() const;

    void addItem(const QString& t, int index = -1);
    void insertItem(int index, const QString& t);

Q_SIGNALS:

    void reset();
    void activated(int);
    void currentIndexChanged(int);

public Q_SLOTS:

    void slotReset();

private Q_SLOTS:

    void slotItemActivated(int);
    void slotCurrentIndexChanged(int);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DCOMBO_BOX_H
