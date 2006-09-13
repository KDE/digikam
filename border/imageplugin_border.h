/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2005-01-20
 * Description : 
 * 
 * Copyright 2005-2006 by Gilles Caulier
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


#ifndef IMAGEPLUGIN_BORDER_H
#define IMAGEPLUGIN_BORDER_H

// Digikam includes.

#include <digikamheaders.h>

class KAction;

class ImagePlugin_Border : public Digikam::ImagePlugin
{
    Q_OBJECT
    
public:

    ImagePlugin_Border(QObject *parent, const char* name,
                         const QStringList &args);
    ~ImagePlugin_Border();
    
    void setEnabledActions(bool enable);

private:

    KAction *m_borderAction;
    
private slots:

    void slotBorder();

};
    
#endif /* IMAGEPLUGIN_BORDER_H */
