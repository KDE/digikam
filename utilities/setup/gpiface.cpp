/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2003-02-19
 * Description : Gphoto2 interface
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

// C Ansi includes.

extern "C"
{
#include <gphoto2.h>
}

// Qt includes.

#include <qstring.h>
#include <qstringlist.h>

// KDE includes.

#include <kdebug.h>

// Local includes

#include "gpiface.h"

namespace Digikam
{

int GPIface::autoDetect(QString& model, QString& port)
{
    CameraList          *camList;
    CameraAbilitiesList *abilList;
    GPPortInfoList      *infoList;
    const char          *camModel_, *camPort_;
    GPContext           *context;

    context = gp_context_new ();
    gp_list_new (&camList);
    
    gp_abilities_list_new(&abilList);
    gp_abilities_list_load(abilList, context);
    gp_port_info_list_new(&infoList);
    gp_port_info_list_load(infoList);
    gp_abilities_list_detect(abilList, infoList, camList, context);
    gp_abilities_list_free(abilList);
    gp_port_info_list_free(infoList);

    gp_context_unref( context );

    int count = gp_list_count(camList);

    if (count<=0) 
    {
        gp_list_free (camList);
        return -1;
    }

    camModel_ = 0;
    camPort_  = 0;
    
    for (int i = 0; i < count; i++)
    {
        if (gp_list_get_name(camList, i, &camModel_) != GP_OK)
        {
            gp_list_free(camList);
            return -1;
        }

        if (gp_list_get_value(camList, i, &camPort_) != GP_OK)
        {
            gp_list_free(camList);
            return -1;
        }

        if (camModel_ && camPort_)
        {
            model = QString::fromLatin1(camModel_);
            port  = QString::fromLatin1(camPort_);
            gp_list_free(camList);
            return 0;
        }
    }

    gp_list_free(camList);

    return -1;
}

void GPIface::getSupportedCameras(int& count, QStringList& clist)
{
    clist.clear();
    count = 0;

    CameraAbilitiesList *abilList;
    CameraAbilities      abil;
    GPContext           *context;

    context = gp_context_new();
 
    gp_abilities_list_new( &abilList );
    gp_abilities_list_load( abilList, context );

    count = gp_abilities_list_count( abilList );
    if ( count < 0) 
    {
        gp_context_unref( context );
        kdDebug() << "failed to get list of cameras!" << endl;
        return;
    }
    else 
    {
        for (int i = 0 ; i < count ; i++) 
        {
            const char *cname;
            gp_abilities_list_get_abilities( abilList, i, &abil );
            cname = abil.model;
            clist.append( QString( cname ) );
        }
    }

    gp_abilities_list_free( abilList );
    gp_context_unref( context );
}

void GPIface::getSupportedPorts(QStringList& plist)
{
    GPPortInfoList *list;
    GPPortInfo      info;

    plist.clear();

    gp_port_info_list_new( &list );
    gp_port_info_list_load( list );

    int numPorts = gp_port_info_list_count( list );

    for (int i = 0 ; i < numPorts ; i++) 
    {
        gp_port_info_list_get_info( list, i, &info );
        plist.append( info.path );
    }

    gp_port_info_list_free( list );
}

void GPIface::getCameraSupportedPorts(const QString& model, QStringList& plist)
{
    int i = 0;
    plist.clear();

    CameraAbilities      abilities;
    CameraAbilitiesList *abilList;
    GPContext           *context;

    context = gp_context_new();

    gp_abilities_list_new(&abilList);
    gp_abilities_list_load(abilList, context);
    i = gp_abilities_list_lookup_model(abilList, model.local8Bit().data());
    gp_abilities_list_get_abilities(abilList, i, &abilities);
    gp_abilities_list_free(abilList);

    if (abilities.port & GP_PORT_SERIAL)
        plist.append("serial");
        
    if (abilities.port & GP_PORT_USB)
        plist.append("usb");

    gp_context_unref( context );
}

}  // namespace Digikam
