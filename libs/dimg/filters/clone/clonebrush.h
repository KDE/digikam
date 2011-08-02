/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-06-08
 * Description : a digiKam image plugin to clone area .
 *
 * Copyright (C) 2011-06-08 by Zhang Jie <zhangjiehangyuan2005 dot at gmail dot com>
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

#ifndef CLONEBRUSH_H
#define CLONEBRUSH_H

#include <QPixmap>
#include <QObject>

namespace Digikam
{

class CloneBrush : public QObject
{
    Q_OBJECT

public:

    CloneBrush();
    ~CloneBrush();

public:

    QPixmap getPixmap();
    void setPixmap(QPixmap brushmap);
    int getDia();

//  void setDia(int mdia);

private:

    QPixmap brushMap;
    int     dia;
};

} // namespace Digikam

#endif // CLONEBRUSH_H
