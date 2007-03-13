/* ============================================================
 * Authors: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date   : 2004-06-04
 * Description : 
 * 
 * Copyright 2004-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>

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
 
#include "imageplugin.h"

namespace Digikam
{

ImagePlugin::ImagePlugin(QObject *parent, const char* name)
           : QObject(parent, name)
{
    m_parentWidget = 0;
}

ImagePlugin::~ImagePlugin()
{    
}

void ImagePlugin::setEnabledSelectionActions(bool)
{
}

void ImagePlugin::setEnabledActions(bool)
{
}

void ImagePlugin::setParentWidget(QWidget* parent)
{
    m_parentWidget = parent;    
}

QWidget* ImagePlugin::parentWidget()
{
    return m_parentWidget;
}

}  // namespace Digikam

