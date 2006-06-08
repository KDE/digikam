/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2003-01-29
 * Description : 
 * 
 * Copyright 2003 by Renchi Raju
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
#include <qguardedptr.h>

class KAction;

namespace Digikam
{

class CameraUI;

class CameraType
{
public:


    CameraType();
    CameraType(const QString& title,
               const QString& model,
               const QString& port,
               const QString& path,
               KAction* action=0);
    ~CameraType();

    CameraType(const CameraType& ctype);
    CameraType& operator=(const CameraType& type);


    void setTitle(const QString& title);
    void setModel(const QString& model);
    void setPort(const QString& port);
    void setPath(const QString& path);
    void setAction(KAction *action);
    void setValid(bool valid);
    void setCurrentCameraUI(CameraUI *cameraui);

    QString title() const;
    QString model() const;
    QString port()  const;
    QString path()  const;
    KAction* action() const;
    bool valid() const;
    CameraUI *currentCameraUI() const;

private:

    QString title_;
    QString model_;
    QString port_;
    QString path_;

    KAction *action_;
    bool     valid_;

    QGuardedPtr<CameraUI> currentCameraUI_;
};

}  // namespace Digikam

#endif /* CAMERATYPE_H */
