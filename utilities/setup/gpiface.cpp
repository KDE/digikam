/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-02-19
 * Description : Gphoto2 interface
 * 
 * Copyright (C) 2003-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2006-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <QString>
#include <QStringList>

// Local includes

#include "ddebug.h"
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

bool GPIface::findConnectedUsbCamera(int vendorId, int productId, QString& model, QString& port)
{
    CameraAbilitiesList *abilList;
    GPPortInfoList      *list;
    GPPortInfo           info;
    GPContext           *context;
    bool                 success;

    success = false;

    context = gp_context_new();

    // get list of all ports
    gp_port_info_list_new( &list );
    gp_port_info_list_load( list );

    int numPorts = gp_port_info_list_count( list );
    for (int i = 0 ; i < numPorts ; i++) 
    {
        // create a port object from info
        gp_port_info_list_get_info( list, i, &info );
        GPPort *gpport;
        gp_port_new(&gpport);
        gp_port_set_info(gpport, info);

        // check if device is connected to port
        if (gp_port_usb_find_device(gpport, vendorId, productId) == GP_OK)
        {
            CameraList          *camList;
            GPPortInfoList      *portinfo;

            // create three lists
            gp_list_new (&camList);
            gp_port_info_list_new( &portinfo );
            gp_abilities_list_new( &abilList );

            // append one port info to 
            gp_port_info_list_append(portinfo, info);
            // get list of all supported cameras
            gp_abilities_list_load(abilList, context);
            // search for all supported cameras on one port
            gp_abilities_list_detect(abilList, portinfo, camList, context);
            int count = gp_list_count(camList);
            // get name and port of detected camera
            const char *model_str, *port_str;
            if (count > 0)
            {
                gp_list_get_name(camList, i, &model_str);
                gp_list_get_value(camList, i, &port_str);

                model = QString::fromLatin1(model_str);
                port  = QString::fromLatin1(port_str);

                success = true;
            }
            if (count > 1)
            {
                DWarning() << "More than one camera detected on port " << port
                           << ". Due to restrictions in the GPhoto2 API, only the first camera is used." << endl;
            }

            gp_abilities_list_free( abilList );
            gp_port_info_list_free(portinfo);
            gp_list_free(camList);
        }
        gp_port_free(gpport);

        if (success)
            break;
    }

    gp_port_info_list_free( list );
    gp_context_unref( context );

    return success;
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
        DDebug() << "failed to get list of cameras!" << endl;
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
    i = gp_abilities_list_lookup_model(abilList, model.toLocal8Bit().data());
    gp_abilities_list_get_abilities(abilList, i, &abilities);
    gp_abilities_list_free(abilList);

    if (abilities.port & GP_PORT_SERIAL)
        plist.append("serial");
        
    if (abilities.port & GP_PORT_USB)
        plist.append("usb");

    gp_context_unref( context );
}

}  // namespace Digikam
