/* ============================================================
 * File  : imageguiclient_core.h
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

#ifndef IMAGEGUICLIENT_CORE_H
#define IMAGEGUICLIENT_CORE_H

#include <qobject.h>
#include <guiclient.h>

class ImageGUIClient_Core : public QObject, public Digikam::GUIClient
{
    Q_OBJECT

public:

    ImageGUIClient_Core();
    ~ImageGUIClient_Core();

    QStringList guiDefinition() const;

private slots:

    void slotBCG();
    void slotBW();
    void slotSepia();
    void slotSolarize();
    void slotRedEye();
};

#endif /* IMAGEGUICLIENT_CORE_H */
