/* ============================================================
 * File  : imageeffect_freerotation.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-01-04
 * Description : a Digikam image editor plugin for superimpose a 
 *               template to an image.
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

#ifndef IMAGEEFFECT_SUPERIMPOSE_H
#define IMAGEEFFECT_SUPERIMPOSE_H

// Qt include.

#include <qimage.h>

// KDE include.

#include <kdialogbase.h>
#include <kurl.h>


class QPushButton;

class DirSelectWidget;

namespace Digikam
{
class ThumbBarView;
}

namespace DigikamSuperImposeImagesPlugin
{

class SuperImposeWidget;

class ImageEffect_SuperImpose : public KDialogBase
{
    Q_OBJECT

public:

    ImageEffect_SuperImpose(QWidget* parent);
    ~ImageEffect_SuperImpose();

private:

    QWidget               *m_parent;
    
    QPushButton           *m_helpButton;
    
    SuperImposeWidget     *m_previewWidget;
    
    Digikam::ThumbBarView *m_thumbnailsBar;

    KURL                   m_templatesUrl;
    KURL                   m_templatesRootUrl;
    
    DirSelectWidget       *m_dirSelect;
    
    void populateTemplates(void);
    
private slots:

    void slotHelp();
    void slotOk();
    void slotUser1();
    void slotTemplateDirChanged(const KURL& url);
    void slotRootTemplateDirChanged(void);

};

}  // NameSpace DigikamSuperImposeImagesPlugin

#endif /* IMAGEEFFECT_SUPERIMPOSE_H */
