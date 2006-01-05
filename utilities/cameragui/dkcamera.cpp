/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-12-21
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

// Local includes.

#include "dkcamera.h"

namespace Digikam
{

DKCamera::DKCamera(const QString& model,
               const QString& port,
               const QString& path)
{
    m_model = model;
    m_port  = port;
    m_path  = path;
}

DKCamera::~DKCamera()
{
}

QString DKCamera::model() const
{
    return m_model;    
}

QString DKCamera::port() const
{
    return m_port;
}

QString DKCamera::path() const
{
    return m_path;
}

}  // namespace Digikam
