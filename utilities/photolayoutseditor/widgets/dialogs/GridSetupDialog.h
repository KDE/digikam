/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-09-01
 * Description : a plugin to create photo layouts by fusion of several images.
 * 
 *
 * Copyright (C) 2011      by Lukasz Spas <lukasz dot spas at gmail dot com>
 * Copyright (C) 2009-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef GRID_SETUP_DIALOG_H
#define GRID_SETUP_DIALOG_H

#include <QDialog>
#include <QDoubleSpinBox>

namespace PhotoLayoutsEditor
{
class GridSetupDialog : public QDialog
{
    Q_OBJECT

public:

    GridSetupDialog(QWidget * parent = 0);
    void setHorizontalDistance(qreal value);
    void setVerticalDistance(qreal value);
    qreal horizontalDistance() const;
    qreal verticalDistance() const;
    virtual int exec();

private:

    QWidget * centralWidget;
    QDoubleSpinBox * x;
    QDoubleSpinBox * y;
};

} // namespace PhotoLayoutsEditor

#endif // GRID_SETUP_DIALOG_H
