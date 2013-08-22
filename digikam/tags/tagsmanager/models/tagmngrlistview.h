/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 20013-08-22
 * Description : Reimplemented QListView for Tags Manager, with support for
 *               drag-n-drop
 *
 * Copyright (C) 2013 by Veaceslav Munteanu <veaceslav dot munteanu90 at gmail dot com>
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

#ifndef TAGMNGRLISTVIEW_H
#define TAGMNGRLISTVIEW_H

#include <QListView>

class QPoint;

class TagMngrListView : public QListView
{
    Q_OBJECT
public:
    explicit TagMngrListView(QWidget *parent = 0);

protected:

    /**
     * Reimplemented methods to enable custom drag-n-drop in QListView
     */
    void startDrag(Qt::DropActions supportedActions);
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *e);

    QModelIndex indexVisuallyAt(const QPoint& p);

public slots:

};

#endif // TAGMNGRLISTVIEW_H
