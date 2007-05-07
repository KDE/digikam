/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-01-12
 * Description : a DCROP interface. 
 * 
 * Copyright (C) 2005 by Leonid Zeitlin <lz@europe.com> 
 * Copyright (C) 2006 Tom Albers <tomalbers@kde.nl>
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
 
#ifndef DCOPIFACE_H
#define DCOPIFACE_H

// Qt includes.

#include <qobject.h>
#include <dcopobject.h>

namespace Digikam
{

/**
* This class implements a DCOP interface for DigiKam.
* At the moment it supports only one method, @ref cameraAutoDetect to open camera dialog
*
* @short DCOP interface for DigiKam
* @author Leonid Zeitlin
*/

class DCOPIface : public QObject, public DCOPObject
{
    K_DCOP
    Q_OBJECT

public:

    /**
     * Standard constructor.
     * @param parent Parent object reference, passed to @ref QObject constructor
     * @param name Specifis the name of the object, passed to @ref QObject constructor
     */
    DCOPIface(QObject *parent = 0, const char *name = 0);

    /**
     * Standard destructor
    */
    ~DCOPIface();

signals:
    
    /**
     * This signal is emitted when @ref cameraAutoDetect() is called via DCOP
     */
    void signalCameraAutoDetect();

    /**
     * This signal is emitted when @ref downloadFrom() is called via DCOP
     * @param folder the path passed tp downloadFrom earlier
     */
    void signalDownloadImages( const QString& folder);


public:

k_dcop:
    /**
     * This method can be called via DCOP to auto-detect attached camera and show DigiKam camera dialog
     * For example, a hotplug script can call it when a USB camera is attached to the computer 
     */
    ASYNC detectCamera();

    /**
     * This method can be called via DCOP to auto-detect attached camera and 
     * show DigiKam camera dialog. For example, a hotplug script can call it 
     * when a USB camera is attached to the computer 
     */
    ASYNC downloadFrom( const QString &folder );
};

}  // namespace Digikam

#endif // DCOPIFACE_H
