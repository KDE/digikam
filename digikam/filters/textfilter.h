/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-02-23
 * Description : a widget to filter album contents by text query
 *
 * Copyright (C) 2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef TEXTFILTER_H
#define TEXTFILTER_H

// KDE includes

#include <khbox.h>

namespace Digikam
{

class SearchTextBar;

class TextFilter : public KHBox
{
    Q_OBJECT

public:

    enum TypeTextFilter
    {
        AllFiles = 0,
        ImageFiles,
        NoRAWFiles,
        JPGFiles,
        PNGFiles,
        TIFFiles,
        DNGFiles,
        RAWFiles,
        MoviesFiles,
        AudioFiles,
        RasterFiles               // PSD, XCF, etc...
    };

public:

    TextFilter(QWidget* parent);
    ~TextFilter();

    SearchTextBar* searchTextBar() const;

private:

    class TextFilterPriv;
    TextFilterPriv* const d;
};

}  // namespace Digikam

#endif // TEXTFILTER_H
