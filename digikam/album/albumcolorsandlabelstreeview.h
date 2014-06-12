/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2014-05-17
 * Description : Colors and Labels Tree View.
 *
 * Copyright (C) 2014 Mohamed Anwer <mohammed dot ahmed dot anwer at gmail dot com>
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

#ifndef COLORSANDLABELSTREEVIEW_H
#define COLORSANDLABELSTREEVIEW_H

// Qt includes

#include <QTreeWidget>

// Local includes

#include "databaseconstants.h"
#include "album.h"

namespace Digikam
{

class ColorsAndLabelsTreeView : public QTreeWidget
{
    Q_OBJECT

public:
    ColorsAndLabelsTreeView(QWidget *parent = 0, bool setCheckable = false);
    virtual ~ColorsAndLabelsTreeView();

    Album* currentAlbumFromCheckedItems();
    QString generatedAlbumName();

private:
    void initTreeView();
    void initRatingsTree();
    void initLabelsTree();
    void initColorsTree();

    QList<int> selectedRatings();
    QList<int> selectedLabels();

    QString createXMLForCurrentSelection();
    SAlbum* search(const QString& xml);

    void generateAlbumName();

private Q_SLOTS:
    void slotSelectionChanged();
    void slotItemClicked();

Q_SIGNALS:
    void checkStateChanged(Album* album, Qt::CheckState checkState);

private:
    class Private;
    Private* const d;
};

}
#endif // COLORSANDLABELSTREEVIEW_H
