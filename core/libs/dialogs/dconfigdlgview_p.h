/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-11-03
 * Description : A dialog base class which can handle multiple pages.
 *
 * Copyright (C) 2009-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAM_DCONFIG_DLG_VIEW_PRIVATE_H
#define DIGIKAM_DCONFIG_DLG_VIEW_PRIVATE_H

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

class Q_DECL_HIDDEN DConfigDlgStackedWidget : public QStackedWidget
{
public:

    explicit DConfigDlgStackedWidget(QWidget* const parent = nullptr)
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

class Q_DECL_HIDDEN DConfigDlgViewPrivate
{
    Q_DECLARE_PUBLIC(DConfigDlgView)

protected:

    explicit DConfigDlgViewPrivate(DConfigDlgView* const);

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

    // cppcheck-suppress unusedPrivateFunction
    void init();
};

// --------------------------------------------------------------------------------------------

namespace DConfigDlgInternal
{

class DConfigDlgListViewDelegate;
class DConfigDlgListViewProxy;

class Q_DECL_HIDDEN DConfigDlgPlainView : public QAbstractItemView
{
public:

    explicit DConfigDlgPlainView(QWidget* const parent = nullptr);

    QModelIndex indexAt(const QPoint& point) const                           override;
    void scrollTo(const QModelIndex& index, ScrollHint hint = EnsureVisible) override;
    QRect visualRect(const QModelIndex& index) const                         override;

protected:

    QModelIndex moveCursor(QAbstractItemView::CursorAction, Qt::KeyboardModifiers) override;
    int horizontalOffset() const                                                   override;
    int verticalOffset() const                                                     override;
    bool isIndexHidden(const QModelIndex&) const                                   override;
    void setSelection(const QRect& , QFlags<QItemSelectionModel::SelectionFlag>)   override;
    QRegion visualRegionForSelection(const QItemSelection&) const                  override;
};

// ---------------------------

class Q_DECL_HIDDEN DConfigDlgListView : public QListView
{
    Q_OBJECT

public:

    explicit DConfigDlgListView(QWidget* const parent = nullptr);
    virtual ~DConfigDlgListView();

    void setModel(QAbstractItemModel* model) override;

private Q_SLOTS:

    void updateWidth();
};

// ---------------------------

class Q_DECL_HIDDEN DConfigDlgTreeView : public QTreeView
{
    Q_OBJECT

public:

    explicit DConfigDlgTreeView(QWidget* const parent = nullptr);

    void setModel(QAbstractItemModel* model) override;

private Q_SLOTS:

    void updateWidth();

private:

    void expandItems(const QModelIndex& index = QModelIndex());
};

// ---------------------------

class Q_DECL_HIDDEN DConfigDlgTabbedView : public QAbstractItemView
{
    Q_OBJECT

public:

    explicit DConfigDlgTabbedView(QWidget* const parent = nullptr);
    virtual ~DConfigDlgTabbedView();

    void setModel(QAbstractItemModel* model)                                            override;
    QModelIndex indexAt(const QPoint& point) const                                      override;
    void scrollTo(const QModelIndex& index, ScrollHint hint = EnsureVisible)            override;
    QRect visualRect(const QModelIndex& index) const                                    override;
    QSize minimumSizeHint() const                                                       override;

protected:

    QModelIndex moveCursor(QAbstractItemView::CursorAction, Qt::KeyboardModifiers)      override;
    int horizontalOffset() const                                                        override;
    int verticalOffset() const                                                          override;
    bool isIndexHidden(const QModelIndex&) const                                        override;
    void setSelection(const QRect& , QFlags<QItemSelectionModel::SelectionFlag>)        override;
    QRegion visualRegionForSelection(const QItemSelection&) const                       override;

private Q_SLOTS:

    void currentPageChanged(int);
    void layoutChanged();
    void dataChanged(const QModelIndex&, const QModelIndex&, const QVector<int>& roles) override;

private:

    QTabWidget* mTabWidget;
};

// ---------------------------

class Q_DECL_HIDDEN DConfigDlgListViewDelegate : public QAbstractItemDelegate
{
    Q_OBJECT

public:

    explicit DConfigDlgListViewDelegate(QObject* const parent = nullptr);

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const                override;

private:

    void drawFocus(QPainter*, const QStyleOptionViewItem&, const QRect&) const;
};

// ---------------------------

/**
 * We need this proxy model to map the leaves of a tree-like model
 * to a one-level list model.
 */
class Q_DECL_HIDDEN DConfigDlgListViewProxy : public QAbstractProxyModel
{
    Q_OBJECT

public:

    explicit DConfigDlgListViewProxy(QObject* const parent = nullptr);
    virtual ~DConfigDlgListViewProxy();

    int rowCount(const QModelIndex& parent = QModelIndex()) const                           override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const                        override;
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex&) const                                            override;
    QVariant data(const QModelIndex& index, int role) const                                 override;
    QModelIndex mapFromSource(const QModelIndex& index) const                               override;
    QModelIndex mapToSource(const QModelIndex& index) const                                 override;

public Q_SLOTS:

    void rebuildMap();

private:

    void addMapEntry(const QModelIndex&);

private:

    QList<QModelIndex> mList;
};

// ---------------------------

class Q_DECL_HIDDEN SelectionModel : public QItemSelectionModel
{
    Q_OBJECT

public:

    SelectionModel(QAbstractItemModel* const model, QObject* const parent);

public Q_SLOTS:

    void clear() override;
    void select(const QModelIndex& index, QItemSelectionModel::SelectionFlags command)        override;
    void select(const QItemSelection& selection, QItemSelectionModel::SelectionFlags command) override;
};

} // namespace DConfigDlgInternal

} // namespace Digikam

#endif // DIGIKAM_DCONFIG_DLG_VIEW_PRIVATE_H
