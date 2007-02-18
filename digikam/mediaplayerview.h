/* ============================================================
 * Authors: Gilles Caulier 
 * Date   : 2006-20-12
 * Description : a view to embed a KPart media player.
 * 
 * Copyright 2006-2007 Gilles Caulier
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

#ifndef MEDIAPLAYERVIEW_H
#define MEDIAPLAYERVIEW_H

// Qt includes.

#include <qwidgetstack.h>

// KDE includes.

#include <kurl.h>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class MediaPlayerViewPriv;

class DIGIKAM_EXPORT MediaPlayerView : public QWidgetStack
{
Q_OBJECT

public:

    MediaPlayerView(QWidget *parent=0);
    ~MediaPlayerView();    

    void setMediaPlayerFromUrl(const KURL& url);
    void escapePreview();
           
private slots:

    void slotThemeChanged();

private:

    int  previewMode(void);
    void setPreviewMode(int mode);

private:

    MediaPlayerViewPriv* d;        
};

}  // NameSpace Digikam

#endif /* MEDIAPLAYERVIEW_H */
