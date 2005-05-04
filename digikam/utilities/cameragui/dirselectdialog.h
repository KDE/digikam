/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-07-18
 * Description : 
 * 
 * Copyright 2004 by Renchi Raju
 * 
 * Modified from kdirselectdialog
 * Copyright (C) 2001 Michael Jarrett <michaelj@corel.com>
 * Copyright (C) 2001 Carsten Pfeiffer <pfeiffer@kde.org>
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

#ifndef DIRSELECTDIALOG_H
#define DIRSELECTDIALOG_H

#include <kdialogbase.h>
#include <kurl.h>
#include <qvaluestack.h>

class KFileTreeBranch;
class KFileTreeView;
class KFileTreeViewItem;

class DirSelectDialog : public KDialogBase
{
    Q_OBJECT

public:

    DirSelectDialog(const QString& rootDir, const QString& startDir,
                    QWidget *parent = 0,
                    const QString& header=QString::null,
                    const QString& newDirString=QString::null,
                    bool allowRootSelection=false);
    ~DirSelectDialog();

    static KURL selectDir(const QString& rootDir,
                          const QString& startDir,
                          QWidget* parent=0,
                          const QString& header=QString::null,
                          const QString& newDirString=QString::null,
                          bool allowRootSelection=false);
    
private:

    void openNextDir( KFileTreeViewItem *parent );

    KFileTreeView*   m_treeView;
    KFileTreeBranch* m_branch;

    KURL              m_rootURL;
    KURL              m_startURL;
    QValueStack<KURL> m_urlsToList;
    QString           m_newDirString;
    bool              m_allowRootSelection;

private slots:

    void slotNextDirToList(KFileTreeViewItem *dirItem);
    void slotContextMenu(KListView*, QListViewItem*,
                         const QPoint& );
    void slotUser1();
    void slotSelectionChanged(QListViewItem* item);
};

#endif /* DIRSELECTDIALOG_H */
