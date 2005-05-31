/* ============================================================
 * File  : folderview.h
 * Author: Joern Ahrens <joern.ahrens@kdemail.net>
 * Date  : 2005-05-28
 * Copyright 2005 by Joern Ahrens
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
 * ============================================================ */

/** @file foldeview.h */

#ifndef _FOLDERVIEW_H_
#define _FOLDERVIEW_H_

#include <qlistview.h>

class FolderViewPriv;

class FolderView : public QListView
{
    Q_OBJECT

public:

    FolderView(QWidget *parent);
    ~FolderView();

    void setActive(bool val);
    bool active();

protected:

    void contentsMouseMoveEvent(QMouseEvent *e);
 
protected slots:
    
    virtual void slotSelectionChanged() = 0;
    
private:
    
    FolderViewPriv      *d;
};

#endif // _FOLDERVIEW_H
