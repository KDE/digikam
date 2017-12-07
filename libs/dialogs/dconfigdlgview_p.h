/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-11-03
 * Description : A dialog base class which can handle multiple pages.
 *
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2007      by Rafael Fernández López <ereslibre at kde dot org>
 * Copyright (C) 2006      by Tobias Koenig <tokoe at kde dot org>
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

#ifndef DBCONFIGDLG_VIEW_P_H
#define DBCONFIGDLG_VIEW_P_H

#include "dconfigdlgview.h"

// Qt includes

#include <QAbstractItemDelegate>
#include <QGridLayout>
#include <QStackedWidget>
#include <QAbstractProxyModel>
#include <QListView>
#include <QTabBar>
#include <QTreeView>

// Local includes

#include "dconfigdlgwidgets.h"

namespace Digikam
{

class DConfigDlgStackedWidget : public QStackedWidget
{
public:

    DConfigDlgStackedWidget(QWidget* const parent = 0)
        : QStackedWidget(parent)
    {
    }

    void setMinimumSize(const QSize& size)
    {
        mMinimumSize = size;
    }

    QSize minimumSizeHint() const Q_DECL_OVERRIDE
    {
        return mMinimumSize.expandedTo(QStackedWidget::minimumSizeHint());
    }

private:

    QSize mMinimumSize;
};

// ---------------------------

class DConfigDlgViewPrivate
{
    Q_DECLARE_PUBLIC(DConfigDlgView)

protected:

    DConfigDlgViewPrivate(DConfigDlgView* const);

    void updateTitleWidget(const QModelIndex& index);

    void updateSelection();
    void cleanupPages();
    QList<QWidget*> collectPages(const QModelIndex& parent = QModelIndex());
    DConfigDlgView::FaceType detectAutoFace() const;

    // private slots
    void _k_rebuildGui();
    void _k_modelChanged();
    void _k_dataChanged(const QModelIndex&, const QModelIndex&);
    void _k_pageSelected(const QItemSelection&, const QItemSelection&);

protected:

    DConfigDlgView*          q_ptr;

    // data
    QAbstractItemModel*      model;
    DConfigDlgView::FaceType faceType;

    // gui
    QGridLayout*             layout;
    DConfigDlgStackedWidget* stack;
    DConfigDlgTitle*         titleWidget;
    QWidget*                 defaultWidget;

    QAbstractItemView*       view;

private:

    void init();
};

// --------------------------------------------------------------------------------------------

namespace DConfigDlgInternal
{

class DConfigDlgListViewDelegate;
class DConfigDlgListViewProxy;

class DConfigDlgPlainView : public QAbstractItemView
{
public:

    DConfigDlgPlainView(QWidget* const parent = 0);

    QModelIndex indexAt(const QPoint& point) const                           Q_DECL_OVERRIDE;
    void scrollTo(const QModelIndex& index, ScrollHint hint = EnsureVisible) Q_DECL_OVERRIDE;
    QRect visualRect(const QModelIndex& index) const                         Q_DECL_OVERRIDE;

protected:

    QModelIndex moveCursor(QAbstractItemView::CursorAction, Qt::KeyboardModifiers) Q_DECL_OVERRIDE;
    int horizontalOffset() const                                                   Q_DECL_OVERRIDE;
    int verticalOffset() const                                                     Q_DECL_OVERRIDE;
    bool isIndexHidden(const QModelIndex&) const                                   Q_DECL_OVERRIDE;
    void setSelection(const QRect& , QFlags<QItemSelectionModel::SelectionFlag>)   Q_DECL_OVERRIDE;
    QRegion visualRegionForSelection(const QItemSelection&) const                  Q_DECL_OVERRIDE;
};

// ---------------------------

class DConfigDlgListView : public QListView
{
    Q_OBJECT

public:

    DConfigDlgListView(QWidget* const parent = 0);
    virtual ~DConfigDlgListView();

    void setModel(QAbstractItemModel* model) Q_DECL_OVERRIDE;

private Q_SLOTS:

    void updateWidth();
};

// ---------------------------

class DConfigDlgTreeView : public QTreeView
{
    Q_OBJECT

public:

    DConfigDlgTreeView(QWidget* const parent = 0);

    void setModel(QAbstractItemModel* model) Q_DECL_OVERRIDE;

private Q_SLOTS:

    void updateWidth();

private:

    void expandItems(const QModelIndex& index = QModelIndex());
};

// ---------------------------

class DConfigDlgTabbedView : public QAbstractItemView
{
    Q_OBJECT

public:

    DConfigDlgTabbedView(QWidget* const parent = 0);
    virtual ~DConfigDlgTabbedView();

    void setModel(QAbstractItemModel* model)                                            Q_DECL_OVERRIDE;
    QModelIndex indexAt(const QPoint& point) const                                      Q_DECL_OVERRIDE;
    void scrollTo(const QModelIndex& index, ScrollHint hint = EnsureVisible)            Q_DECL_OVERRIDE;
    QRect visualRect(const QModelIndex& index) const                                    Q_DECL_OVERRIDE;
    QSize minimumSizeHint() const                                                       Q_DECL_OVERRIDE;

protected:

    QModelIndex moveCursor(QAbstractItemView::CursorAction, Qt::KeyboardModifiers)      Q_DECL_OVERRIDE;
    int horizontalOffset() const                                                        Q_DECL_OVERRIDE;
    int verticalOffset() const                                                          Q_DECL_OVERRIDE;
    bool isIndexHidden(const QModelIndex&) const                                        Q_DECL_OVERRIDE;
    void setSelection(const QRect& , QFlags<QItemSelectionModel::SelectionFlag>)        Q_DECL_OVERRIDE;
    QRegion visualRegionForSelection(const QItemSelection&) const                       Q_DECL_OVERRIDE;

private Q_SLOTS:

    void currentPageChanged(int);
    void layoutChanged();
    void dataChanged(const QModelIndex&, const QModelIndex&, const QVector<int>& roles) Q_DECL_OVERRIDE;

private:

    QTabWidget* mTabWidget;
};

// ---------------------------

class DConfigDlgListViewDelegate : public QAbstractItemDelegate
{
    Q_OBJECT

public:

    DConfigDlgListViewDelegate(QObject* const parent = 0);

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const Q_DECL_OVERRIDE;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const                Q_DECL_OVERRIDE;

private:

    void drawFocus(QPainter*, const QStyleOptionViewItem&, const QRect&) const;
};

// ---------------------------

/**
 * We need this proxy model to map the leaves of a tree-like model
 * to a one-level list model.
 */
class DConfigDlgListViewProxy : public QAbstractProxyModel
{
    Q_OBJECT

public:

    DConfigDlgListViewProxy(QObject* const parent = 0);
    virtual ~DConfigDlgListViewProxy();

    int rowCount(const QModelIndex& parent = QModelIndex()) const                           Q_DECL_OVERRIDE;
    int columnCount(const QModelIndex& parent = QModelIndex()) const                        Q_DECL_OVERRIDE;
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QModelIndex parent(const QModelIndex&) const                                            Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex& index, int role) const                                 Q_DECL_OVERRIDE;
    QModelIndex mapFromSource(const QModelIndex& index) const                               Q_DECL_OVERRIDE;
    QModelIndex mapToSource(const QModelIndex& index) const                                 Q_DECL_OVERRIDE;

public Q_SLOTS:

    void rebuildMap();

private:

    void addMapEntry(const QModelIndex&);

private:

    QList<QModelIndex> mList;
};

// ---------------------------

class SelectionModel : public QItemSelectionModel
{
    Q_OBJECT

public:

    SelectionModel(QAbstractItemModel* const model, QObject* const parent);

public Q_SLOTS:

    void clear() Q_DECL_OVERRIDE;
    void select(const QModelIndex& index, QItemSelectionModel::SelectionFlags command)        Q_DECL_OVERRIDE;
    void select(const QItemSelection& selection, QItemSelectionModel::SelectionFlags command) Q_DECL_OVERRIDE;
};

}  // namespace DConfigDlgInternal

}  // namespace Digikam

#endif // DBCONFIGDLG_VIEW_P_H
