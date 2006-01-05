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

// KDE includes.

#include <kaction.h>

// Local includes.

#include "cameratype.h"

namespace Digikam
{

CameraType::CameraType()
{
    action_ = 0;
    valid_  = false;    
}

CameraType::CameraType(const QString& title,
                       const QString& model,
                       const QString& port,
                       const QString& path,
                       KAction *action)
{
    title_ = title;
    model_ = model;
    port_  = port;
    path_  = path;
    action_ = action;
    valid_  = true;    
}

CameraType::~CameraType()
{
}

CameraType::CameraType(const CameraType& ctype)
{
    title_ = ctype.title_;
    model_ = ctype.model_;
    port_  = ctype.port_;
    path_  = ctype.path_;
    action_ = ctype.action_;
    valid_  = ctype.valid_;
}

CameraType& CameraType::operator=(const CameraType& ctype)
{
    if (this != &ctype) {
        title_ = ctype.title_;
        model_ = ctype.model_;
        port_  = ctype.port_;
        path_  = ctype.path_;
        action_ = ctype.action_;
        valid_  = ctype.valid_;
    }
    return *this;
}

void CameraType::setTitle(const QString& title)
{
    title_ = title;
}

void CameraType::setModel(const QString& model)
{
    model_ = model;
}

void CameraType::setPort(const QString& port)
{
    port_  = port;
}

void CameraType::setPath(const QString& path)
{
    path_  = path;
}

void CameraType::setAction(KAction *action)
{
    action_ = action;    
}

QString CameraType::title() const
{
    return title_;
}

QString CameraType::model() const
{
    return model_;
}

QString CameraType::port() const
{
    return port_;
}

QString CameraType::path() const
{
    return path_;
}

KAction* CameraType::action()
{
    return action_;    
}

bool CameraType::valid()
{
    return valid_;
}

void CameraType::setValid(bool valid)
{
    valid_ = valid;    
}

}  // namespace Digikam
