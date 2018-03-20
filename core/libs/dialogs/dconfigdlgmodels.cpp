/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-11-03
 * Description : A dialog base class which can handle multiple pages.
 *
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2007      by Matthias Kretz <kretz at kde dot org>
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

#include "dconfigdlgmodels.h"
#include "dconfigdlgmodels_p.h"

// Qt includes

#include <QPointer>
#include <QIcon>

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

DConfigDlgModelPrivate::~DConfigDlgModelPrivate()
{
}

DConfigDlgModel::DConfigDlgModel(QObject* const parent)
    : QAbstractItemModel(parent),
      d_ptr(0)
{
}

DConfigDlgModel::DConfigDlgModel(DConfigDlgModelPrivate& dd, QObject* const parent)
    : QAbstractItemModel(parent),
      d_ptr(&dd)
{
    d_ptr->q_ptr = this;
}

DConfigDlgModel::~DConfigDlgModel()
{
    delete d_ptr;
}

// ----------------------------------------------------------------------------------


class DConfigDlgWdgItem::Private
{
public:

    Private()
        : checkable(false),
          checked(false),
          enabled(true)
    {
    }

    ~Private()
    {
        delete widget;
        widget = 0;
    }

    QString           name;
    QString           header;
    QIcon             icon;
    QPointer<QWidget> widget;
    bool              checkable : 1;
    bool              checked   : 1;
    bool              enabled   : 1;
};

DConfigDlgWdgItem::DConfigDlgWdgItem(QWidget* widget)
    : QObject(0),
      d(new Private)
{
    d->widget = widget;

    /**
     * Hide the widget, otherwise when the widget has this DConfigDlgView as
     * parent the widget is shown outside the QStackedWidget if the page
     * was not selected ( and reparented ) yet.
     */
    if (d->widget)
    {
        d->widget->hide();
    }
}

DConfigDlgWdgItem::DConfigDlgWdgItem(QWidget* widget, const QString& name)
    : QObject(0),
      d(new Private)
{
    d->widget = widget;
    d->name = name;

    /**
     * Hide the widget, otherwise when the widget has this DConfigDlgView as
     * parent the widget is shown outside the QStackedWidget if the page
     * was not selected ( and reparented ) yet.
     */
    if (d->widget)
    {
        d->widget->hide();
    }
}

DConfigDlgWdgItem::~DConfigDlgWdgItem()
{
    delete d;
}

void DConfigDlgWdgItem::setEnabled(bool enabled)
{
    d->enabled = enabled;

    if (d->widget)
    {
        d->widget->setEnabled(enabled);
    }

    emit changed();
}

bool DConfigDlgWdgItem::isEnabled() const
{
    return d->enabled;
}

QWidget* DConfigDlgWdgItem::widget() const
{
    return d->widget;
}

void DConfigDlgWdgItem::setName(const QString& name)
{
    d->name = name;

    emit changed();
}

QString DConfigDlgWdgItem::name() const
{
    return d->name;
}

void DConfigDlgWdgItem::setHeader(const QString& header)
{
    d->header = header;

    emit changed();
}

QString DConfigDlgWdgItem::header() const
{
    return d->header;
}

void DConfigDlgWdgItem::setIcon(const QIcon& icon)
{
    d->icon = icon;

    emit changed();
}

QIcon DConfigDlgWdgItem::icon() const
{
    return d->icon;
}

void DConfigDlgWdgItem::setCheckable(bool checkable)
{
    d->checkable = checkable;

    emit changed();
}

bool DConfigDlgWdgItem::isCheckable() const
{
    return d->checkable;
}

void DConfigDlgWdgItem::setChecked(bool checked)
{
    d->checked = checked;

    emit toggled(checked);
    emit changed();
}

bool DConfigDlgWdgItem::isChecked() const
{
    return d->checked;
}

// ---------------------------------------------------------------------------------

PageItem::PageItem(DConfigDlgWdgItem* pageWidgetItem, PageItem* parent)
    : mPageWidgetItem(pageWidgetItem),
      mParentItem(parent)
{
}

PageItem::~PageItem()
{
    delete mPageWidgetItem;
    mPageWidgetItem = 0;

    qDeleteAll(mChildItems);
}

void PageItem::appendChild(PageItem* item)
{
    mChildItems.append(item);
}

void PageItem::insertChild(int row, PageItem* item)
{
    mChildItems.insert(row, item);
}

void PageItem::removeChild(int row)
{
    mChildItems.removeAt(row);
}

PageItem* PageItem::child(int row)
{
    return mChildItems.value(row);
}

int PageItem::childCount() const
{
    return mChildItems.count();
}

int PageItem::columnCount() const
{
    return 1;
}

PageItem* PageItem::parent()
{
    return mParentItem;
}

int PageItem::row() const
{
    if (mParentItem)
    {
        return mParentItem->mChildItems.indexOf(const_cast<PageItem *>(this));
    }

    return 0;
}

DConfigDlgWdgItem* PageItem::pageWidgetItem() const
{
    return mPageWidgetItem;
}

PageItem* PageItem::findChild(const DConfigDlgWdgItem* item)
{
    if (mPageWidgetItem == item)
    {
        return this;
    }

    for (int i = 0; i < mChildItems.count(); ++i)
    {
        PageItem* const pageItem = mChildItems[ i ]->findChild(item);

        if (pageItem)
        {
            return pageItem;
        }
    }

    return 0;
}

void PageItem::dump(int indent)
{
    QString prefix;

    for (int i = 0; i < indent; ++i)
    {
        prefix.append(QLatin1String(" "));
    }

    const QString name = (mPageWidgetItem ? mPageWidgetItem->name() : QLatin1String("root"));
    qDebug("%s (%p)", qPrintable(QString(QString::fromLatin1("%1%2")).arg(prefix, name)), (void *)this);

    for (int i = 0; i < mChildItems.count(); ++i)
    {
        mChildItems[ i ]->dump(indent + 2);
    }
}

// ---------------------------------------------------------------------------------

DConfigDlgWdgModel::DConfigDlgWdgModel(QObject* parent)
    : DConfigDlgModel(*new DConfigDlgWdgModelPrivate, parent)
{
}

DConfigDlgWdgModel::~DConfigDlgWdgModel()
{
}

int DConfigDlgWdgModel::columnCount(const QModelIndex&) const
{
    return 1;
}

QVariant DConfigDlgWdgModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
    {
        return QVariant();
    }

    PageItem* const item = static_cast<PageItem *>(index.internalPointer());

    if (role == Qt::DisplayRole)
    {
        return QVariant(item->pageWidgetItem()->name());
    }
    else if (role == Qt::DecorationRole)
    {
        return QVariant(item->pageWidgetItem()->icon());
    }
    else if (role == HeaderRole)
    {
        return QVariant(item->pageWidgetItem()->header());
    }
    else if (role == WidgetRole)
    {
        return QVariant::fromValue(item->pageWidgetItem()->widget());
    }
    else if (role == Qt::CheckStateRole)
    {
        if (item->pageWidgetItem()->isCheckable())
        {
            return (item->pageWidgetItem()->isChecked() ? Qt::Checked : Qt::Unchecked);
        }
        else
        {
            return QVariant();
        }
    }
    else
    {
        return QVariant();
    }
}

bool DConfigDlgWdgModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid())
    {
        return false;
    }

    if (role != Qt::CheckStateRole)
    {
        return false;
    }

    PageItem* const item = static_cast<PageItem*>(index.internalPointer());

    if (!item)
    {
        return false;
    }

    if (!item->pageWidgetItem()->isCheckable())
    {
        return false;
    }

    if (value.toInt() == Qt::Checked)
    {
        item->pageWidgetItem()->setChecked(true);
    }
    else
    {
        item->pageWidgetItem()->setChecked(false);
    }

    return true;
}

Qt::ItemFlags DConfigDlgWdgModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
    {
        return 0;
    }

    Qt::ItemFlags flags = Qt::ItemIsSelectable;

    PageItem* const item = static_cast<PageItem *>(index.internalPointer());

    if (item->pageWidgetItem()->isCheckable())
    {
        flags |= Qt::ItemIsUserCheckable;
    }

    if (item->pageWidgetItem()->isEnabled())
    {
        flags |= Qt::ItemIsEnabled;
    }

    return flags;
}

QModelIndex DConfigDlgWdgModel::index(int row, int column, const QModelIndex& parent) const
{
    PageItem* parentItem = 0;

    if (parent.isValid())
    {
        parentItem = static_cast<PageItem*>(parent.internalPointer());
    }
    else
    {
        parentItem = d_func()->rootItem;
    }

    PageItem* const childItem = parentItem->child(row);

    if (childItem)
    {
        return createIndex(row, column, childItem);
    }
    else
    {
        return QModelIndex();
    }
}

QModelIndex DConfigDlgWdgModel::parent(const QModelIndex& index) const
{
    if (!index.isValid())
    {
        return QModelIndex();
    }

    PageItem* const item       = static_cast<PageItem *>(index.internalPointer());
    PageItem* const parentItem = item->parent();

    if (parentItem == d_func()->rootItem)
    {
        return QModelIndex();
    }
    else
    {
        return createIndex(parentItem->row(), 0, parentItem);
    }
}

int DConfigDlgWdgModel::rowCount(const QModelIndex& parent) const
{
    PageItem* parentItem = 0;

    if (!parent.isValid())
    {
        parentItem = d_func()->rootItem;
    }
    else
    {
        parentItem = static_cast<PageItem *>(parent.internalPointer());
    }

    return parentItem->childCount();
}

DConfigDlgWdgItem* DConfigDlgWdgModel::addPage(QWidget* widget, const QString& name)
{
    DConfigDlgWdgItem* const item = new DConfigDlgWdgItem(widget, name);
    addPage(item);

    return item;
}

void DConfigDlgWdgModel::addPage(DConfigDlgWdgItem* item)
{
    emit layoutAboutToBeChanged();

    Q_D(DConfigDlgWdgModel);

    connect(item, SIGNAL(changed()),
            this, SLOT(_k_itemChanged()));

    connect(item, SIGNAL(toggled(bool)),
            this, SLOT(_k_itemToggled(bool)));

    // The row to be inserted
    int row = d->rootItem->childCount();

    beginInsertRows(QModelIndex(), row, row);

    PageItem* const pageItem = new PageItem(item, d->rootItem);
    d->rootItem->appendChild(pageItem);

    endInsertRows();

    emit layoutChanged();
}

DConfigDlgWdgItem* DConfigDlgWdgModel::insertPage(DConfigDlgWdgItem* before, QWidget* widget, const QString& name)
{
    DConfigDlgWdgItem* const item = new DConfigDlgWdgItem(widget, name);

    insertPage(before, item);

    return item;
}

void DConfigDlgWdgModel::insertPage(DConfigDlgWdgItem* before, DConfigDlgWdgItem* item)
{
    PageItem* const beforePageItem = d_func()->rootItem->findChild(before);

    if (!beforePageItem)
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Invalid DConfigDlgWdgItem passed!";
        return;
    }

    emit layoutAboutToBeChanged();

    connect(item, SIGNAL(changed()),
            this, SLOT(_k_itemChanged()));

    connect(item, SIGNAL(toggled(bool)),
            this, SLOT(_k_itemToggled(bool)));

    PageItem* const parent = beforePageItem->parent();
    // The row to be inserted
    int row                = beforePageItem->row();

    QModelIndex index;

    if (parent != d_func()->rootItem)
    {
        index = createIndex(parent->row(), 0, parent);
    }

    beginInsertRows(index, row, row);

    PageItem* const newPageItem = new PageItem(item, parent);
    parent->insertChild(row, newPageItem);

    endInsertRows();

    emit layoutChanged();
}

DConfigDlgWdgItem* DConfigDlgWdgModel::addSubPage(DConfigDlgWdgItem* parent, QWidget* widget, const QString& name)
{
    DConfigDlgWdgItem* const item = new DConfigDlgWdgItem(widget, name);

    addSubPage(parent, item);

    return item;
}

void DConfigDlgWdgModel::addSubPage(DConfigDlgWdgItem* parent, DConfigDlgWdgItem* item)
{
    PageItem* const parentPageItem = d_func()->rootItem->findChild(parent);

    if (!parentPageItem)
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Invalid DConfigDlgWdgItem passed!";
        return;
    }

    emit layoutAboutToBeChanged();

    connect(item, SIGNAL(changed()),
            this, SLOT(_k_itemChanged()));

    connect(item, SIGNAL(toggled(bool)),
            this, SLOT(_k_itemToggled(bool)));

    // The row to be inserted
    int row = parentPageItem->childCount();

    QModelIndex index;

    if (parentPageItem != d_func()->rootItem)
    {
        index = createIndex(parentPageItem->row(), 0, parentPageItem);
    }

    beginInsertRows(index, row, row);

    PageItem* const newPageItem = new PageItem(item, parentPageItem);
    parentPageItem->appendChild(newPageItem);

    endInsertRows();

    emit layoutChanged();
}

void DConfigDlgWdgModel::removePage(DConfigDlgWdgItem* item)
{
    if (!item)
    {
        return;
    }

    Q_D(DConfigDlgWdgModel);

    PageItem* const pageItem = d->rootItem->findChild(item);

    if (!pageItem)
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Invalid DConfigDlgWdgItem passed!";
        return;
    }

    emit layoutAboutToBeChanged();

    disconnect(item, SIGNAL(changed()),
               this, SLOT(_k_itemChanged()));

    disconnect(item, SIGNAL(toggled(bool)),
               this, SLOT(_k_itemToggled(bool)));

    PageItem* const parentPageItem = pageItem->parent();
    int row                        = parentPageItem->row();

    QModelIndex index;

    if (parentPageItem != d->rootItem)
    {
        index = createIndex(row, 0, parentPageItem);
    }

    beginRemoveRows(index, pageItem->row(), pageItem->row());

    parentPageItem->removeChild(pageItem->row());
    delete pageItem;

    endRemoveRows();

    emit layoutChanged();
}

DConfigDlgWdgItem* DConfigDlgWdgModel::item(const QModelIndex& index) const
{
    if (!index.isValid())
    {
        return 0;
    }

    PageItem* const item = static_cast<PageItem*>(index.internalPointer());

    if (!item)
    {
        return 0;
    }

    return item->pageWidgetItem();
}

QModelIndex DConfigDlgWdgModel::index(const DConfigDlgWdgItem* item) const
{
    if (!item)
    {
        return QModelIndex();
    }

    const PageItem* pageItem = d_func()->rootItem->findChild(item);

    if (!pageItem)
    {
        return QModelIndex();
    }

    return createIndex(pageItem->row(), 0, (void*)pageItem);
}

}  // namespace Digikam

#include "moc_dconfigdlgmodels.cpp"
