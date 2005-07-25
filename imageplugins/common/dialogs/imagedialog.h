/* ============================================================
 * File  : imagedialog.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-07-23
 * Description : simple plugins dialog without threadable 
 *               filter interface.
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

#ifndef IMAGEDIALOG_H
#define IMAGEDIALOG_H

// Qt includes

#include <qstring.h>

// KDE includes.

#include <kdialogbase.h>

class QGridLayout;

class KAboutData;

namespace Digikam
{
class ImageGuideWidget;
}

namespace DigikamImagePlugins
{

class ImageTabWidget;

class ImageDialog : public KDialogBase
{
    Q_OBJECT

public:

    ImageDialog(QWidget *parent, QString title, QString name, bool loadFileSettings=true,
                bool orgGuideVisible=false, bool targGuideVisible=false);
    ~ImageDialog();

    Digikam::ImageGuideWidget *previewOriginalWidget(void);
    Digikam::ImageGuideWidget *previewTargetWidget(void);
    
    void setAboutData(KAboutData *about);
    void setUserAreaWidget(QWidget *w);
    
private:

    QGridLayout    *m_mainLayout;
    
    QWidget        *m_parent;
    
    QString         m_name;
    
    ImageTabWidget *m_imageTabPreviewWidget;
    
private slots:

    void slotHelp();
};

}  // NameSpace DigikamImagePlugins

#endif /* IMAGEDIALOG */
