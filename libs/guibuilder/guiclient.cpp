/* ============================================================
 * File  : guiclient.cpp
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-02-11
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

#include <kaction.h>

#include "guiclient.h"

namespace Digikam
{

GUIClient::GUIClient()
{
    m_actionCollection = 0;
}

GUIClient::~GUIClient()
{
    if (m_actionCollection)
        delete m_actionCollection;
}

KActionCollection* GUIClient::actionCollection()
{
    if (!m_actionCollection)
        m_actionCollection = new KActionCollection((QWidget*)0,
                                                   (const char*)0);

    return m_actionCollection;
}

}
