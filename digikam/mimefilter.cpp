/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-10-22
 * Description : a widget to filter album contents by type mime
 *
 * Copyright (C) 2007-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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


#include "mimefilter.h"
#include "mimefilter.moc"

// KDE includes.

#include <kdebug.h>
#include <klocale.h>
#include <kglobal.h>


namespace Digikam
{

class MimeFilterPriv
{
public:

    MimeFilterPriv()
    {
    }
};

MimeFilter::MimeFilter(QWidget* parent)
          : QComboBox(parent)
{
    d = new MimeFilterPriv;
    insertItem( AllFiles,    i18n("All Files") );
    insertItem( ImageFiles,  i18n("Image files") );
    insertItem( NoRAWFiles,  i18n("No RAW files") );
    insertItem( JPGFiles,    i18n("JPEG files") );
    insertItem( PNGFiles,    i18n("PNG files") );
    insertItem( TIFFiles,    i18n("TIFF files") );
    insertItem( DNGFiles,    i18n("DNG files") );
    insertItem( RAWFiles,    i18n("RAW files") );
    insertItem( MoviesFiles, i18n("Movie files") );
    insertItem( AudioFiles,  i18n("Audio files") );

    setToolTip(i18n("Filter by file type"));
    setWhatsThis(i18n("Select the file types (mime types) that you want shown"));

    setMimeFilter(AllFiles);
}

MimeFilter::~MimeFilter()
{
    delete d;
}

void MimeFilter::setMimeFilter(int filter)
{
    setCurrentIndex(filter);
    emit activated(filter);
}

int MimeFilter::mimeFilter()
{
    return currentIndex();
}

}  // namespace Digikam
