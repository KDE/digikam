/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-07-23
 * Description : a tabulate image previews widget
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

#ifndef IMAGETABWIDGET_H
#define IMAGETABWIDGET_H

// Qt includes.

#include <qtabwidget.h>

// Digikam includes.

#include <digikamheaders.h>

namespace DigikamImagePlugins
{

class ImageTabWidget : public QTabWidget
{
Q_OBJECT

public:

    ImageTabWidget(QWidget *parent=0,
                   bool orgGuideVisible=false, bool targGuideVisible=false,
                   int orgGuideMode=Digikam::ImageGuideWidget::PickColorMode,
                   int targGuideMode=Digikam::ImageGuideWidget::PickColorMode);
    ~ImageTabWidget();
    
    Digikam::ImageGuideWidget *previewOriginal(void){ return m_previewOriginalWidget; };
    Digikam::ImageGuideWidget *previewTarget(void)  { return m_previewTargetWidget;   };
    
private:
    
    Digikam::ImageGuideWidget *m_previewOriginalWidget;
    Digikam::ImageGuideWidget *m_previewTargetWidget;    
};

}  // namespace DigikamImagePlugins

#endif /* IMAGETABWIDGET_H */
