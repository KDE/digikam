/***************************************************************************
                          gpmessages.h  -  description
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

#ifndef GPMESSAGES_H
#define GPMESSAGES_H

#include <qobject.h>

class GPMessages : public QObject {

    Q_OBJECT

    friend class GPStatus;

public:

    static GPMessages* gpMessagesWrapper();
    static void deleteMessagesWrapper();

signals:

    void errorMessage(const QString&);
    void statusChanged(const QString&);
    void progressChanged(int);

private:

    GPMessages() : QObject() { };

    static GPMessages* gpMessages;


};

#endif
