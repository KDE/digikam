/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-07-05
 * Description : A combobox delegate to display in Web service image lists.
 *
 * Copyright (C) 2009 by Pieter Edelman <pieter dot edelman at gmx dot net>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef COMBO_BOX_DELEGATE_H
#define COMBO_BOX_DELEGATE_H

// Qt includes

#include <QAbstractItemDelegate>
#include <QAbstractItemModel>
#include <QMap>
#include <QModelIndex>
#include <QPainter>
#include <QSize>
#include <QString>
#include <QStyleOptionViewItem>
#include <QWidget>

// Local includes

#include "dimageslist.h"

namespace Digikam
{

class ComboBoxDelegate : public QAbstractItemDelegate
{
    Q_OBJECT

public:

    explicit ComboBoxDelegate(DImagesList* const, const QMap<int, QString>&);
    ~ComboBoxDelegate();
    
    /* Whenever an element needs to be edited, this method should be called.
     * It's actually a hack to prevent the item text shining through whenever
     * editing occurs.
     */
    void startEditing(QTreeWidgetItem*, int);

    /* Overloaded functions to provide the delegate functionality.
     */
    void     paint(QPainter*, const QStyleOptionViewItem&, const QModelIndex&) const       Q_DECL_OVERRIDE;
    QSize    sizeHint(const QStyleOptionViewItem&, const QModelIndex&) const               Q_DECL_OVERRIDE;
    QWidget* createEditor(QWidget*, const QStyleOptionViewItem&, const QModelIndex&) const Q_DECL_OVERRIDE;
    void     setEditorData(QWidget*, const QModelIndex&) const                             Q_DECL_OVERRIDE;
    void     setModelData(QWidget*, QAbstractItemModel*, const QModelIndex&) const         Q_DECL_OVERRIDE;

private Q_SLOTS:

    void slotCommitAndCloseEditor(int);
    void slotResetEditedState(QObject*);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // COMBO_BOX_DELEGATE_H
