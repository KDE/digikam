/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date   : 2005-04-21
 * Description : slide show tool using preview of pictures.
 * 
 * Copyright 2005-2007 by Gilles Caulier
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

#ifndef SLIDE_SHOW_H
#define SLIDE_SHOW_H

// Qt includes.

#include <qwidget.h>

// KDE includes.

#include <kurl.h>

// Local includes.

#include "loadingdescription.h"

namespace Digikam
{

class SlideShowPriv;

class SlideShow : public QWidget
{
    Q_OBJECT
    
public:

    SlideShow(const KURL::List& fileList, bool exifRotate,
              int delay, bool printName, bool loop);
    ~SlideShow();

    void setCurrent(const KURL& url);

private:

    void loadNextImage();
    void loadPrevImage();
    void updatePixmap();
    
protected:

    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void keyPressEvent(QKeyEvent *);
    
private slots:

    void slotTimeOut();
    void slotMouseMoveTimeOut();
    void slotGotImagePreview(const LoadingDescription &, const QImage &);

    void slotPause();
    void slotPlay();
    void slotPrev();
    void slotNext();
    void slotClose();

private:

    SlideShowPriv *d;
};

}  // NameSpace Digikam

#endif /* SLIDE_SHOW_H */
