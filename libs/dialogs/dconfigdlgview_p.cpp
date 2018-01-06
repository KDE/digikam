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

#include "dconfigdlgview_p.h"

// Qt includes

#include <QApplication>
#include <QHeaderView>
#include <QPainter>
#include <QTextLayout>
#include <QVBoxLayout>

// Local includes

#include "dconfigdlgmodels.h"

namespace Digikam
{

using namespace DConfigDlgInternal;

DConfigDlgPlainView::DConfigDlgPlainView(QWidget* const parent)
    : QAbstractItemView(parent)
{
    hide();
}

QModelIndex DConfigDlgPlainView::indexAt(const QPoint&) const
{
    return QModelIndex();
}

void DConfigDlgPlainView::scrollTo(const QModelIndex&, ScrollHint)
{
}

QRect DConfigDlgPlainView::visualRect(const QModelIndex&) const
{
    return QRect();
}

QModelIndex DConfigDlgPlainView::moveCursor(QAbstractItemView::CursorAction, Qt::KeyboardModifiers)
{
    return QModelIndex();
}

int DConfigDlgPlainView::horizontalOffset() const
{
    return 0;
}

int DConfigDlgPlainView::verticalOffset() const
{
    return 0;
}

bool DConfigDlgPlainView::isIndexHidden(const QModelIndex&) const
{
    return false;
}

void DConfigDlgPlainView::setSelection(const QRect&, QFlags<QItemSelectionModel::SelectionFlag>)
{
}

QRegion DConfigDlgPlainView::visualRegionForSelection(const QItemSelection&) const
{
    return QRegion();
}

// ------------------------------------------------------------------------------------------------------

DConfigDlgListView::DConfigDlgListView(QWidget* const parent)
    : QListView(parent)
{
    setViewMode(QListView::ListMode);
    setMovement(QListView::Static);
    setVerticalScrollMode(QListView::ScrollPerPixel);

    QFont boldFont(font());
    boldFont.setBold(true);
    setFont(boldFont);

    setItemDelegate(new DConfigDlgListViewDelegate(this));
}

DConfigDlgListView::~DConfigDlgListView()
{
}

void DConfigDlgListView::setModel(QAbstractItemModel* model)
{
/*
      DConfigDlgListViewProxy* const proxy = new DConfigDlgListViewProxy( this );
      proxy->setSourceModel( model );
      proxy->rebuildMap();

      connect(model, SIGNAL(layoutChanged()),
              proxy, SLOT(rebuildMap()) );
*/
    connect(model, &QAbstractItemModel::layoutChanged,
            this, &DConfigDlgListView::updateWidth);

//  QListView::setModel( proxy );
    QListView::setModel(model);

    // Set our own selection model, which won't allow our current selection to be cleared
    setSelectionModel(new DConfigDlgInternal::SelectionModel(model, this));

    updateWidth();
}

void DConfigDlgListView::updateWidth()
{
    if (!model())
    {
        return;
    }

    int rows = model()->rowCount();

    int width = 0;

    for (int i = 0; i < rows; ++i)
    {
        width = qMax(width, sizeHintForIndex(model()->index(i, 0)).width());
    }

    setFixedWidth(width + 25);
}

// -------------------------------------------------------------------------------------------

DConfigDlgTreeView::DConfigDlgTreeView(QWidget* const parent)
    : QTreeView(parent)
{
    header()->hide();
}

void DConfigDlgTreeView::setModel(QAbstractItemModel* model)
{
    connect(model, &QAbstractItemModel::layoutChanged,
            this, &DConfigDlgTreeView::updateWidth);

    QTreeView::setModel(model);

    // Set our own selection model, which won't allow our current selection to be cleared
    setSelectionModel(new DConfigDlgInternal::SelectionModel(model, this));

    updateWidth();
}

void DConfigDlgTreeView::updateWidth()
{
    if (!model())
    {
        return;
    }

    int columns = model()->columnCount();

    expandItems();

    int width = 0;

    for (int i = 0; i < columns; ++i)
    {
        resizeColumnToContents(i);
        width = qMax(width, sizeHintForColumn(i));
    }

    setFixedWidth(width + 25);
}

void DConfigDlgTreeView::expandItems(const QModelIndex& index)
{
    setExpanded(index, true);

    const int count = model()->rowCount(index);

    for (int i = 0; i < count; ++i)
    {
        expandItems(model()->index(i, 0, index));
    }
}

// ---------------------------------------------------------------------------

DConfigDlgTabbedView::DConfigDlgTabbedView(QWidget* const parent)
    : QAbstractItemView(parent)
{
    // hide the viewport of the QAbstractScrollArea
    const QList<QWidget *> list = findChildren<QWidget *>();

    for (int i = 0; i < list.count(); ++i)
    {
        list[ i ]->hide();
    }

    setFrameShape(NoFrame);

    QVBoxLayout* const layout = new QVBoxLayout(this);
    layout->setMargin(0);

    mTabWidget = new QTabWidget(this);

    connect(mTabWidget, &QTabWidget::currentChanged, this,
            &DConfigDlgTabbedView::currentPageChanged);

    layout->addWidget(mTabWidget);
}

DConfigDlgTabbedView::~DConfigDlgTabbedView()
{
    if (model())
    {
        for (int i = 0; i < mTabWidget->count(); ++i)
        {
            QWidget* const page = qvariant_cast<QWidget *>(model()->data(model()->index(i, 0), DConfigDlgModel::WidgetRole));

            if (page)
            {
                page->setVisible(false);
                page->setParent(0); // reparent our children before they are deleted
            }
        }
    }
}

void DConfigDlgTabbedView::setModel(QAbstractItemModel* model)
{
    QAbstractItemView::setModel(model);

    connect(model, &QAbstractItemModel::layoutChanged, this,
            &DConfigDlgTabbedView::layoutChanged);

    layoutChanged();
}

QModelIndex DConfigDlgTabbedView::indexAt(const QPoint&) const
{
    if (model())
    {
        return model()->index(0, 0);
    }
    else
    {
        return QModelIndex();
    }
}

void DConfigDlgTabbedView::scrollTo(const QModelIndex& index, ScrollHint)
{
    if (!index.isValid())
    {
        return;
    }

    mTabWidget->setCurrentIndex(index.row());
}

QRect DConfigDlgTabbedView::visualRect(const QModelIndex&) const
{
    return QRect();
}

QSize DConfigDlgTabbedView::minimumSizeHint() const
{
    return mTabWidget->minimumSizeHint();
}

QModelIndex DConfigDlgTabbedView::moveCursor(QAbstractItemView::CursorAction, Qt::KeyboardModifiers)
{
    return QModelIndex();
}

int DConfigDlgTabbedView::horizontalOffset() const
{
    return 0;
}

int DConfigDlgTabbedView::verticalOffset() const
{
    return 0;
}

bool DConfigDlgTabbedView::isIndexHidden(const QModelIndex& index) const
{
    return (mTabWidget->currentIndex() != index.row());
}

void DConfigDlgTabbedView::setSelection(const QRect& , QFlags<QItemSelectionModel::SelectionFlag>)
{
}

QRegion DConfigDlgTabbedView::visualRegionForSelection(const QItemSelection&) const
{
    return QRegion();
}

void DConfigDlgTabbedView::currentPageChanged(int index)
{
    if (!model())
    {
        return;
    }

    QModelIndex modelIndex = model()->index(index, 0);

    selectionModel()->setCurrentIndex(modelIndex, QItemSelectionModel::ClearAndSelect);
}

void DConfigDlgTabbedView::layoutChanged()
{
    // save old position
    int pos = mTabWidget->currentIndex();

    // clear tab bar
    int count = mTabWidget->count();

    for (int i = 0; i < count; ++i)
    {
        mTabWidget->removeTab(0);
    }

    if (!model())
    {
        return;
    }

    // add new tabs
    for (int i = 0; i < model()->rowCount(); ++i)
    {
        const QString title = model()->data(model()->index(i, 0)).toString();
        const QIcon icon    = model()->data(model()->index(i, 0), Qt::DecorationRole).value<QIcon>();
        QWidget* const page = qvariant_cast<QWidget *>(model()->data(model()->index(i, 0), DConfigDlgModel::WidgetRole));

        if (page)
        {
            QWidget* const widget     = new QWidget(this);
            QVBoxLayout* const layout = new QVBoxLayout(widget);
            widget->setLayout(layout);
            layout->addWidget(page);
            page->setVisible(true);
            mTabWidget->addTab(widget, icon, title);
        }
    }

    mTabWidget->setCurrentIndex(pos);
}

void DConfigDlgTabbedView::dataChanged(const QModelIndex& index, const QModelIndex&, const QVector<int>& roles)
{
    if (!index.isValid())
    {
        return;
    }

    if (index.row() < 0 || index.row() >= mTabWidget->count())
    {
        return;
    }

    if (roles.isEmpty() || roles.contains(Qt::DisplayRole) || roles.contains(Qt::DecorationRole))
    {
        const QString title = model()->data(index).toString();
        const QIcon icon    = model()->data(index, Qt::DecorationRole).value<QIcon>();

        mTabWidget->setTabText(index.row(), title);
        mTabWidget->setTabIcon(index.row(), icon);
    }
}

// -----------------------------------------------------------------------------------------------

DConfigDlgListViewDelegate::DConfigDlgListViewDelegate(QObject* const parent)
    : QAbstractItemDelegate(parent)
{
}

static int layoutText(QTextLayout* layout, int maxWidth)
{
    qreal height  = 0;
    int textWidth = 0;
    layout->beginLayout();

    while (true)
    {
        QTextLine line = layout->createLine();

        if (!line.isValid())
        {
            break;
        }

        line.setLineWidth(maxWidth);
        line.setPosition(QPointF(0, height));
        height    += line.height();
        textWidth  = qMax(textWidth, qRound(line.naturalTextWidth() + 0.5));
    }

    layout->endLayout();
    return textWidth;
}

void DConfigDlgListViewDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    if (!index.isValid())
    {
        return;
    }

    QStyleOptionViewItem opt(option);
    opt.showDecorationSelected = true;
    QStyle* const style        = opt.widget ? opt.widget->style() : QApplication::style();

    int iconSize         = style->pixelMetric(QStyle::PM_IconViewIconSize);
    const QString text   = index.model()->data(index, Qt::DisplayRole).toString();
    const QIcon icon     = index.model()->data(index, Qt::DecorationRole).value<QIcon>();
    const QPixmap pixmap = icon.pixmap(iconSize, iconSize);

    QFontMetrics fm = painter->fontMetrics();
    int wp          = pixmap.width()  / pixmap.devicePixelRatio();
    int hp          = pixmap.height() / pixmap.devicePixelRatio();

    QTextLayout iconTextLayout(text, option.font);
    QTextOption textOption(Qt::AlignHCenter);
    iconTextLayout.setTextOption(textOption);
    int maxWidth    = qMax(3 * wp, 8 * fm.height());
    layoutText(&iconTextLayout, maxWidth);

    QPen pen                = painter->pen();
    QPalette::ColorGroup cg = (option.state & QStyle::State_Enabled) ? QPalette::Normal
                                                                     : QPalette::Disabled;

    if (cg == QPalette::Normal && !(option.state & QStyle::State_Active))
    {
        cg = QPalette::Inactive;
    }

    style->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, opt.widget);

    if (option.state & QStyle::State_Selected)
    {
        painter->setPen(option.palette.color(cg, QPalette::HighlightedText));
    }
    else
    {
        painter->setPen(option.palette.color(cg, QPalette::Text));
    }

    painter->drawPixmap(option.rect.x() + (option.rect.width() / 2) - (wp / 2), option.rect.y() + 5, pixmap);

    if (!text.isEmpty())
    {
        iconTextLayout.draw(painter, QPoint(option.rect.x() + (option.rect.width() / 2) - (maxWidth / 2), option.rect.y() + hp + 7));
    }

    painter->setPen(pen);

    drawFocus(painter, option, option.rect);
}

QSize DConfigDlgListViewDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    if (!index.isValid())
    {
        return QSize(0, 0);
    }

    QStyleOptionViewItem opt(option);
    opt.showDecorationSelected = true;
    QStyle* const style        = opt.widget ? opt.widget->style() : QApplication::style();

    int iconSize         = style->pixelMetric(QStyle::PM_IconViewIconSize);
    const QString text   = index.model()->data(index, Qt::DisplayRole).toString();
    const QIcon icon     = index.model()->data(index, Qt::DecorationRole).value<QIcon>();
    const QPixmap pixmap = icon.pixmap(iconSize, iconSize);

    QFontMetrics fm = option.fontMetrics;
    int gap         = fm.height();
    int wp          = pixmap.width() / pixmap.devicePixelRatio();
    int hp          = pixmap.height() / pixmap.devicePixelRatio();

    if (hp == 0)
    {
        /**
         * No pixmap loaded yet, we'll use the default icon size in this case.
         */
        hp = iconSize;
        wp = iconSize;
    }

    QTextLayout iconTextLayout(text, option.font);
    int wt = layoutText(&iconTextLayout, qMax(3 * wp, 8 * fm.height()));
    int ht = iconTextLayout.boundingRect().height();

    int width, height;

    if (text.isEmpty())
    {
        height = hp;
    }
    else
    {
        height = hp + ht + 10;
    }

    width = qMax(wt, wp) + gap;

    return QSize(width, height);
}

void DConfigDlgListViewDelegate::drawFocus(QPainter* painter, const QStyleOptionViewItem& option, const QRect& rect) const
{
    if (option.state & QStyle::State_HasFocus)
    {
        QStyleOptionFocusRect o;

        o.QStyleOption::operator = (option);
        o.rect                   = rect;
        o.state                 |= QStyle::State_KeyboardFocusChange;
        QPalette::ColorGroup cg  = (option.state & QStyle::State_Enabled)
                                   ? QPalette::Normal : QPalette::Disabled;
        o.backgroundColor        = option.palette.color(cg, (option.state & QStyle::State_Selected)
                                   ? QPalette::Highlight : QPalette::Background);

        QApplication::style()->drawPrimitive(QStyle::PE_FrameFocusRect, &o, painter);
    }
}

// ------------------------------------------------------------------------------------------------------

DConfigDlgListViewProxy::DConfigDlgListViewProxy(QObject* const parent)
    : QAbstractProxyModel(parent)
{
}

DConfigDlgListViewProxy::~DConfigDlgListViewProxy()
{
}

int DConfigDlgListViewProxy::rowCount(const QModelIndex&) const
{
    return mList.count();
}

int DConfigDlgListViewProxy::columnCount(const QModelIndex&) const
{
    return 1;
}

QModelIndex DConfigDlgListViewProxy::index(int row, int column, const QModelIndex&) const
{
    if (column > 1 || row >= mList.count())
    {
        return QModelIndex();
    }
    else
    {
        return createIndex(row, column, mList[ row ].internalPointer());
    }
}

QModelIndex DConfigDlgListViewProxy::parent(const QModelIndex&) const
{
    return QModelIndex();
}

QVariant DConfigDlgListViewProxy::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
    {
        return QVariant();
    }

    if (index.row() >= mList.count())
    {
        return QVariant();
    }

    return sourceModel()->data(mList[ index.row() ], role);
}

QModelIndex DConfigDlgListViewProxy::mapFromSource(const QModelIndex& index) const
{
    if (!index.isValid())
    {
        return QModelIndex();
    }

    for (int i = 0; i < mList.count(); ++i)
    {
        if (mList[ i ] == index)
        {
            return createIndex(i, 0, index.internalPointer());
        }
    }

    return QModelIndex();
}

QModelIndex DConfigDlgListViewProxy::mapToSource(const QModelIndex& index) const
{
    if (!index.isValid())
    {
        return QModelIndex();
    }

    return mList[ index.row() ];
}

void DConfigDlgListViewProxy::rebuildMap()
{
    mList.clear();

    const QAbstractItemModel* model = sourceModel();

    if (!model)
    {
        return;
    }

    for (int i = 0; i < model->rowCount(); ++i)
    {
        addMapEntry(model->index(i, 0));
    }

    for (int i = 0; i < mList.count(); ++i)
    {
        qDebug("%d:0 -> %d:%d", i, mList[ i ].row(), mList[ i ].column());
    }

    emit layoutChanged();
}

void DConfigDlgListViewProxy::addMapEntry(const QModelIndex& index)
{
    if (sourceModel()->rowCount(index) == 0)
    {
        mList.append(index);
    }
    else
    {
        const int count = sourceModel()->rowCount(index);
        for (int i = 0; i < count; ++i)
        {
            addMapEntry(sourceModel()->index(i, 0, index));
        }
    }
}

// ---------------------------------------------------------------------------------------

SelectionModel::SelectionModel(QAbstractItemModel* const model, QObject* const parent)
    : QItemSelectionModel(model, parent)
{
}

void SelectionModel::clear()
{
    // Don't allow the current selection to be cleared
}

void SelectionModel::select(const QModelIndex& index, QItemSelectionModel::SelectionFlags command)
{
    // Don't allow the current selection to be cleared
    if (!index.isValid() && (command & QItemSelectionModel::Clear))
    {
        return;
    }

    QItemSelectionModel::select(index, command);
}

void SelectionModel::select(const QItemSelection& selection, QItemSelectionModel::SelectionFlags command)
{
    // Don't allow the current selection to be cleared
    if (!selection.count() && (command & QItemSelectionModel::Clear))
    {
        return;
    }

    QItemSelectionModel::select(selection, command);
}

}  // namespace Digikam
