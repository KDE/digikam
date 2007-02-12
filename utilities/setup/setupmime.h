/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date   : 2003-05-03
 * Description : mime types setup tab.
 * 
 * Copyright 2004-2007 by Gilles Caulier
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

#ifndef SETUPMIME_H
#define SETUPMIME_H

// Qt includes.

#include <qwidget.h>

namespace Digikam
{

class SetupMimePriv;

class SetupMime : public QWidget
{
    Q_OBJECT
    
public:

    SetupMime(QWidget* parent = 0);
    ~SetupMime();

    void applySettings();

private:

    void readSettings();

private slots:

    void slotRevertImageFileFilter();
    void slotRevertMovieFileFilter();
    void slotRevertAudioFileFilter();
    void slotRevertRawFileFilter();

private:

    SetupMimePriv* d;
};

}  // namespace Digikam

#endif // SETUPMIME_H 
