/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2006-06-13
 * Description : A widget stack to embedded album content view
 *               or the current image preview.
 *
 * Copyright 2006 by Gilles Caulier
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

#ifndef ALBUMWIDGETSTACK_H
#define ALBUMWIDGETSTACK_H

// KDE includes.

#include <qwidgetstack.h>
#include <qstring.h>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class AlbumIconView;
class ImagePreviewWidget;
class AlbumWidgetStackPriv;

class DIGIKAM_EXPORT AlbumWidgetStack : public QWidgetStack
{
Q_OBJECT

public:

    enum AlbumWidgetStackMode
    {
        PreviewAlbumMode=0,
        PreviewItemMode
    };

public:

    AlbumWidgetStack( QWidget *parent=0 );
    ~AlbumWidgetStack();

    AlbumIconView* albumIconView();
    ImagePreviewWidget* imagePreviewWidget();

    void setPreviewItem(const QString& path=QString::null);
    int  previewMode(void);
    void setPreviewMode(int mode);

signals:

    void backToAlbumSignal();    
    void editImageSignal();

private slots:
    
    void slotThemeChanged();
    void slotPreviewStarted();
    void slotPreviewComplete();
    void slotPreviewFailed();

private:

    AlbumWidgetStackPriv* d;
};

}  // namespace Digikam

#endif /* ALBUMWIDGETSTACK_H */
