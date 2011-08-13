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

//Qt includes

#include <QPixmap>
#include <QObject>

//Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT CloneBrush : public QObject
{
    Q_OBJECT

public:

    CloneBrush();
    CloneBrush(const CloneBrush& cb);
    ~CloneBrush();

    /** Equivalent to the copy constructor
     */
    CloneBrush& operator=(const CloneBrush& cb);

public:

    QPixmap getPixmap() const;
    void    setPixmap(const QPixmap& brushmap);

    int  getDia() const;
    void setDia(int mdia);

private:

    QPixmap m_brushMap;
    int     m_dia;
};

} // namespace Digikam

#endif // CLONEBRUSH_H
