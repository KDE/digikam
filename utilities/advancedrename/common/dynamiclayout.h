/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-10-22
 * Description : a dynamic layout manager
 *
 * Copyright (C) 2009-2012 by Andi Clemens <andi dot clemens at gmail dot com>
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

#ifndef DYNAMICLAYOUT_H
#define DYNAMICLAYOUT_H

// Qt includes

#include <QLayout>

class QLayoutItem;
class QRect;
class QWidget;

namespace Digikam
{

class DynamicLayout : public QLayout
{
public:

    explicit DynamicLayout(QWidget* parent, int margin = -1,   int hSpacing = 0, int vSpacing = 0);
    explicit DynamicLayout(int margin = -1, int hSpacing = 0,  int vSpacing = 0);
    ~DynamicLayout();

    void addItem(QLayoutItem* item);

    int  horizontalSpacing() const;
    int  verticalSpacing() const;

    Qt::Orientations expandingDirections() const;

    bool hasHeightForWidth() const;
    int  heightForWidth(int) const;

    int count() const;

    QLayoutItem* itemAt(int index) const;
    QLayoutItem* takeAt(int index);

    QSize minimumSize() const;

    void  setGeometry(const QRect& rect);
    QSize sizeHint() const;

private:

    DynamicLayout(const DynamicLayout&);
    DynamicLayout& operator=(const DynamicLayout&);

    int reLayout(const QRect& rect, bool testOnly) const;

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DYNAMICLAYOUT_H
