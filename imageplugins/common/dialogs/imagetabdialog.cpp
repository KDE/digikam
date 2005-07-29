/* ============================================================
 * File  : imagetabdialog.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-07-23
 * Description : simple plugins dialog based on 
 *               ImageDialogBase using ImageTabWidget 
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

// Digikam includes.

#include <digikamheaders.h>

// Local includes.

#include "version.h"
#include "imagetabwidget.h"
#include "imagetabdialog.h"

namespace DigikamImagePlugins
{

ImageTabDialog::ImageTabDialog(QWidget* parent, QString title, QString name, bool loadFileSettings,
                               bool orgGuideVisible, bool targGuideVisible)
              : ImageDialogBase(parent, title, name, loadFileSettings)
{
    m_imageTabPreviewWidget = new DigikamImagePlugins::ImageTabWidget(plainPage(), 
                              orgGuideVisible, targGuideVisible);
    setPreviewAreaWidget(m_imageTabPreviewWidget);                              
}

ImageTabDialog::~ImageTabDialog()
{
}

Digikam::ImageGuideWidget *ImageTabDialog::previewOriginalWidget(void)
{
    return m_imageTabPreviewWidget->previewOriginal(); 
}

Digikam::ImageGuideWidget *ImageTabDialog::previewTargetWidget(void) 
{ 
    return m_imageTabPreviewWidget->previewTarget();   
}

}  // NameSpace DigikamImagePlugins

#include "imagetabdialog.moc"
