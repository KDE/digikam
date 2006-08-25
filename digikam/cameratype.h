/* ============================================================
 * Authors: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *          Caulier Gilles <caulier dot gilles at kdemail dot net>
 * Date   : 2003-01-29
 * Description : Camera settings container.
 * 
 * Copyright 2003-2005 by Renchi Raju
 * Copyright 2006 by Gilles Caulier
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

#ifndef CAMERATYPE_H
#define CAMERATYPE_H

// Qt includes.

#include <qstring.h>
#include <qdatetime.h>
#include <qguardedptr.h>

class KAction;

namespace Digikam
{

class CameraUI;
class CameraTypePrivate;

class CameraType
{
public:


    CameraType();
    CameraType(const QString& title, const QString& model,
               const QString& port, const QString& path,
               const QDateTime& lastAccess, KAction* action=0);
    ~CameraType();

    CameraType(const CameraType& ctype);
    CameraType& operator=(const CameraType& type);

    void setTitle(const QString& title);
    void setModel(const QString& model);
    void setPort(const QString& port);
    void setPath(const QString& path);
    void setLastAccess(const QDateTime& lastAccess);
    void setAction(KAction *action);
    void setValid(bool valid);
    void setCurrentCameraUI(CameraUI *cameraui);

    QString   title()           const;
    QString   model()           const;
    QString   port()            const;
    QString   path()            const;
    QDateTime lastAccess()      const;
    KAction*  action()          const;
    bool      valid()           const;
    CameraUI *currentCameraUI() const;

private:

    CameraTypePrivate *d;

};

}  // namespace Digikam

#endif /* CAMERATYPE_H */
