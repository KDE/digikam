/* ============================================================
 * File  : guiclient.h
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-02-11
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

#ifndef GUICLIENT_H
#define GUICLIENT_H

#include <qstring.h>
#include <qstringlist.h>

class KActionCollection;
class KInstance;

namespace Digikam
{

class GUIClient
{
public:

    GUIClient();
    virtual ~GUIClient();

    virtual QStringList guiDefinition() const = 0;

    void setInstance(KInstance *instance);

    KActionCollection* actionCollection();

private:

    KActionCollection     *m_actionCollection;
    KInstance             *m_instance;
};

}

#endif /* GUICLIENT_H */

