/* ============================================================
 * File  : imageplugin.cpp
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-06-04
 * Description : 
 * 
 * Copyright 2004 by Renchi Raju

 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published bythe Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * ============================================================ */

#include <kinstance.h>

#include "imageplugin.h"

namespace Digikam
{

ImagePlugin::ImagePlugin(QObject *parent, const char* name)
    : QObject(parent, name)
{
    m_instance = 0;    
}

ImagePlugin::~ImagePlugin()
{
    
}

void ImagePlugin::setInstance(KInstance *instance)
{
    m_instance = instance;    
}

void ImagePlugin::setEnabledSelectionActions(bool)
{
    
}

}
