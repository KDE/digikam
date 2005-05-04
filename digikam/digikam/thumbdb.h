/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-07-21
 * Description : 
 * 
 * Copyright 2004 by Renchi Raju

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

#ifndef THUMBDB_H
#define THUMBDB_H

#include <qstring.h>

class QPixmap;
class QImage;
class ThumbDBPriv;

class ThumbDB
{
public:

    static ThumbDB* instance();
    void putThumb(const QString& path, const QImage& image);
    void getThumb(const QString& path, QPixmap& pix, int w, int h);
    bool hasThumb(const QString& path);

private:

    ThumbDB();
    ~ThumbDB();

    QCString getKey(const QString& path);

    ThumbDBPriv* d;
    static ThumbDB* m_instance;
};

#endif /* THUMBDB_H */
