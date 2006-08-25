/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2005-05-17
 * Copyright 2005 by Renchi Raju
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

#ifndef DIO_H
#define DIO_H

// KDE includes.

#include <kio/job.h>

namespace DIO
{

    KIO::Job* copy(const KURL& src, const KURL& dest);
    
    KIO::Job* copy(const KURL::List& srcList, const KURL& dest);
    
    KIO::Job* move(const KURL& src, const KURL& dest);
    
    KIO::Job* move(const KURL::List& srcList, const KURL& dest);
    
    KIO::Job* del(const KURL& src, bool useTrash = true);
    
    KIO::Job* del(const KURL::List& srcList, bool useTrash = true);
    
    bool      renameFile(const KURL& src, const KURL& dest);
    
    KIO::Job* scan(const KURL& albumURL);
    
    bool      running();

}  // namespace DIO

#endif /* DIO_H */
