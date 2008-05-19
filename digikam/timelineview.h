/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-12-08
 * Description : Time line sidebar tab contents.
 * 
 * Copyright (C) 2007-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef TIMELINEVIEW_H
#define TIMELINEVIEW_H

// Qt includes.

#include <qdatetime.h>
#include <qwidget.h>
#include <qstring.h>

namespace Digikam
{

class SAlbum;
class SearchTextBar;
class TimeLineFolderView;
class TimeLineViewPriv;

class TimeLineView : public QWidget
{
    Q_OBJECT

public:

    TimeLineView(QWidget *parent=0);
    ~TimeLineView();

    TimeLineFolderView *folderView() const;
    SearchTextBar* searchBar() const;

    void setActive(bool val);

private: 

    void readConfig();
    void writeConfig();
    void createNewDateSearchAlbum(const QString& name);
    bool checkName(QString& name);
    bool checkAlbum(const QString& name) const;

private slots:

    void slotInit();
    void slotScrollBarValueChanged(int);
    void slotRefDateTimeChanged();
    void slotScaleChanged(int);
    void slotTimeUnitChanged(int);
    void slotCursorPositionChanged();
    void slotSelectionChanged();
    void slotResetSelection();
    void slotSaveSelection();
    void slotUpdateCurrentDateSearchAlbum();
    void slotAlbumSelected(SAlbum*);
    void slotCheckAboutSelection();
    void slotRenameAlbum(SAlbum*);

private:

    TimeLineViewPriv* d;
};

}  // NameSpace Digikam

#endif /* TIMELINEVIEW_H */
