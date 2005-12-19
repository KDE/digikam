/* ============================================================
 * File  : imagetabdialog.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-07-23
 * Description : simple plugins dialog based on 
 *               ImageDlgBase using ImageTabWidget 
 *               for preview.
 * 
 * Copyright 2005 by Gilles Caulier
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

#ifndef IMAGETABDIALOG_H
#define IMAGETABDIALOG_H

// Digikam includes.

#include <digikamheaders.h>

namespace Digikam
{
class ImageGuideWidget;
}

namespace DigikamImagePlugins
{

class ImageTabWidget;

class ImageTabDialog : public Digikam::ImageDlgBase
{
    Q_OBJECT

public:

    ImageTabDialog(QWidget *parent, QString title, QString name, bool loadFileSettings=true,
                   bool orgGuideVisible=false, bool targGuideVisible=false, QFrame* bannerFrame=0);
    ~ImageTabDialog();

    Digikam::ImageGuideWidget *previewOriginalWidget(void);
    Digikam::ImageGuideWidget *previewTargetWidget(void);
    
private:

    ImageTabWidget *m_imageTabPreviewWidget;
};

}  // NameSpace DigikamImagePlugins

#endif /* IMAGETABDIALOG */
