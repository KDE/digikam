/***************************************************************************
                          gpmessages.cpp  -  description
                             -------------------
    begin                : Sun Dec 30 2001
    copyright            : (C) 2001 by Renchi Raju
    email                : renchi@pooh.tam.uiuc.edu
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "gpmessages.h"

GPMessages* GPMessages::gpMessages=0;

GPMessages* GPMessages::gpMessagesWrapper() {

    if (!gpMessages) {
        gpMessages = new GPMessages;
    }
    return gpMessages;
}

void GPMessages::deleteMessagesWrapper()
{
    if (gpMessages) {
        delete gpMessages;
        gpMessages=0;
    }
}
