/* ============================================================
 * File  : plugin.cpp
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2003-01-30
 * Description : 
 * 
 * Copyright 2003 by Renchi Raju

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

#include <kparts/componentfactory.h>

#include "plugin.h"

namespace Digikam
{

Plugin::Plugin(QObject *parent, const char* name)
    : QObject(parent, name), KXMLGUIClient()
{
}


Plugin::~Plugin()
{

}

void Plugin::enableView()
{
    
}
void Plugin::disableView()
{
    
}

};


#include "plugin.moc"
