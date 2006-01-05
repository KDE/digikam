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

#ifndef DIO_P_H
#define DIO_P_H

// QT includes.

#include <qobject.h>

namespace KIO
{
class Job;
}

namespace DIO
{

class Watch : public QObject
{
    Q_OBJECT
    
public:

    Watch(KIO::Job* job);

    static uint m_runCount;
    
private slots:

   void slotDone(KIO::Job* job);
};

} // namespace DIO

#endif /* DIO_P_H */
