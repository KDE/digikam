/* ============================================================
 * File  : imageplugin_filmgrain.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-10-01
 * Description : 
 * 
 * Copyright 2004 by Gilles Caulier
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


#ifndef IMAGEPLUGIN_FILMGRAIN_H
#define IMAGEPLUGIN_FILMGRAIN_H

// Digikam includes.

#include <digikamheaders.h>

class ImagePlugin_FilmGrain : public Digikam::ImagePlugin
{
    Q_OBJECT
    
public:

    ImagePlugin_FilmGrain(QObject *parent, const char* name,
                         const QStringList &args);
    ~ImagePlugin_FilmGrain();

    QStringList guiDefinition() const;

private slots:

    void slotFilmGrain();

};
    
#endif /* IMAGEPLUGIN_FILMGRAIN_H */
