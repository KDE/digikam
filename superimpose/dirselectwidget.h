/* ============================================================
 * File   : dirselectwidget.h
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-01-11
 * Description : a directory selection widget.
 * 
 * Copyright 2005 by Gilles Caulier
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Library General
 * Public License as published bythe Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
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
     
    DirSelectWidget( KURL rootUrl=KURL::KURL("/"), KURL currentUrl=KURL::KURL(), 
                 QWidget* parent=0, const char* name=0, QString headerLabel=QString::null);
    ~DirSelectWidget();
     
    KURL path() const;
    KURL rootPath(void);
    void setRootPath(KURL rootUrl, KURL currentUrl=KURL::KURL(QString::QString::null));
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
