/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-01-04
 * Description : a Digikam image editor plugin for superimpose a 
 *               template to an image.
 *
 * Copyright (C) 2005-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2008 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef DIRSELECTWIDGET_H
#define DIRSELECTWIDGET_H

// Qt includes.

#include <qwidget.h>
#include <qstring.h>

// KDE includes.

#include <kfiletreeview.h>
#include <kurl.h>

namespace DigikamSuperImposeImagesPlugin
{

class DirSelectWidget : public KFileTreeView 
{
Q_OBJECT

public:
     
    DirSelectWidget(QWidget* parent, const char* name=0, QString headerLabel=QString());

    DirSelectWidget(KURL rootUrl=KURL("/"), KURL currentUrl=KURL(), 
                    QWidget* parent=0, const char* name=0, QString headerLabel=QString());

    ~DirSelectWidget();
     
    KURL path() const;
    KURL rootPath(void);
    void setRootPath(KURL rootUrl, KURL currentUrl=KURL(QString()));
    void setCurrentPath(KURL currentUrl);

signals :
    
    void folderItemSelected(const KURL &url);
        
protected slots:

    void load();
    void slotFolderSelected(QListViewItem *);

private:
    
    struct Private;
    Private* d;
};

}  // NameSpace DigikamSuperImposeImagesPlugin

#endif /* DIRSELECTWIDGET_H */
