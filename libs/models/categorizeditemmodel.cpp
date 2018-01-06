/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-12-02
 * Description : Generic, standard item based model for DCategorizedView
 *
 * Copyright (C) 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "categorizeditemmodel.h"

// Qt includes

#include <QAction>
#include <QMenu>
#include <QWidget>

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

CategorizedItemModel::CategorizedItemModel(QObject* const parent)
    : QStandardItemModel(parent)
{
}

QStandardItem* CategorizedItemModel::addItem(const QString& text, const QVariant& category, const QVariant& categorySorting)
{
    QStandardItem* const item = new QStandardItem(text);
    item->setData(category, DCategorizedSortFilterProxyModel::CategoryDisplayRole);
    item->setData(categorySorting.isNull() ? category : categorySorting, DCategorizedSortFilterProxyModel::CategorySortRole);
    item->setData(rowCount(), ItemOrderRole);
    appendRow(item);

    return item;
}

QStandardItem* CategorizedItemModel::addItem(const QString& text, const QIcon& decoration,
                                             const QVariant& category, const QVariant& categorySorting)
{
    QStandardItem* const item = addItem(text, category, categorySorting);
    item->setIcon(decoration);
    return item;
}

DCategorizedSortFilterProxyModel* CategorizedItemModel::createFilterModel()
{
    DCategorizedSortFilterProxyModel* const filterModel = new DCategorizedSortFilterProxyModel(this);
    filterModel->setCategorizedModel(true);
    filterModel->setSortRole(ItemOrderRole);
    filterModel->setSourceModel(this);
    return filterModel;
}

// -------------------------------------------------------------------------------------------------------------------------

static QString adjustedActionText(const QAction* const action)
{
    QString text = action->text();
    text.remove(QLatin1Char('&'));
    text.remove(QLatin1String(" ..."));
    text.remove(QLatin1String("..."));
    int slashPos = -1;

    while ( (slashPos = text.indexOf(QLatin1Char('/'), slashPos + 1)) != -1 )
    {
        if (slashPos == 0 || slashPos == text.length()-1)
            continue;

        if (text.at(slashPos - 1).isSpace() || text.at(slashPos + 1).isSpace())
            continue;

        text.replace(slashPos, 1, QLatin1Char(','));
        text.insert(slashPos+1, QLatin1Char(' '));
    }

    return text;
}

class ActionEnumerator
{
public:

    explicit ActionEnumerator(const QList<QAction*>& whitelist)
        : whitelist(whitelist)
    {
    }

    void enumerate(QWidget* const w)
    {
        // recurse
        enumerateActions(w, 0);
    }

    void addTo(ActionItemModel* const model, ActionItemModel::MenuCategoryMode mode)
    {
        int categorySortStartIndex = model->rowCount();

        foreach(QAction* const a, actions)
        {
            QAction* categoryAction = 0;

            if (mode & ActionItemModel::ToplevelMenuCategory)
            {
                for (QAction* p = a; p; p = parents.value(p))
                    categoryAction = p;
            }
            else
            {
                categoryAction = parents.value(a);
            }

            if (!categoryAction)
                continue;

            QVariant categorySortValue;

            if (mode & ActionItemModel::SortCategoriesByInsertionOrder)
            {
                categorySortValue = categorySortStartIndex++;
            }

            model->addAction(a, adjustedActionText(categoryAction), categorySortValue);
        }
    }

protected:

    const QList<QAction*>&   whitelist;
    QList<QAction*>          actions;
    QMap<QAction*, QAction*> parents;
    QList<QAction*>          parentsInOrder;

    void enumerateActions(const QWidget* const w, QAction* const widgetAction)
    {
        foreach(QAction* const a, w->actions())
        {
            if (a->menu())
            {
                enumerateActions(a->menu(), a->menu()->menuAction());
            }
            else if (whitelist.isEmpty() || whitelist.contains(a))
            {
                actions << a;
            }

            parents[a] = widgetAction;

            if (!parentsInOrder.contains(widgetAction))
            {
                parentsInOrder << widgetAction;
            }
        }
    }
};

// -------------------------------------------------------------------------------------------------------------------------

ActionItemModel::ActionItemModel(QObject* const parent)
    : CategorizedItemModel(parent),
      m_mode(ToplevelMenuCategory | SortCategoriesAlphabetically)
{
}

void ActionItemModel::setMode(MenuCategoryMode mode)
{
    m_mode = mode;
}

ActionItemModel::MenuCategoryMode ActionItemModel::mode() const
{
    return m_mode;
}

QStandardItem* ActionItemModel::addAction(QAction* action, const QString& category, const QVariant& categorySorting)
{
    QStandardItem* const item = addItem(QString(), category, categorySorting);
    item->setEditable(false);
    setPropertiesFromAction(item, action);

    connect(action, SIGNAL(changed()),
            this, SLOT(slotActionChanged()));

    return item;
}

void ActionItemModel::setPropertiesFromAction(QStandardItem* item, QAction* action)
{
    item->setText(adjustedActionText(action));
    item->setIcon(action->icon());
    item->setEnabled(action->isEnabled());
    item->setCheckable(action->isCheckable());

    if (action->toolTip() != action->text())
        item->setToolTip(action->toolTip());

    item->setWhatsThis(action->whatsThis());
    item->setData(QVariant::fromValue(static_cast<QObject*>(action)), ItemActionRole);
}

void ActionItemModel::addActions(QWidget* w)
{
    addActions(w, QList<QAction*>());
}

void ActionItemModel::addActions(QWidget* w, const QList<QAction*>& actionWhiteList)
{
    ActionEnumerator enumerator(actionWhiteList);
    enumerator.enumerate(w);
    enumerator.addTo(this, m_mode);
}

QAction* ActionItemModel::actionForIndex(const QModelIndex& index)
{
    return static_cast<QAction*>(index.data(ItemActionRole).value<QObject*>());
}

QStandardItem* ActionItemModel::itemForAction(QAction* action) const
{
    if (!action)
        return 0;

    for (int i = 0 ; i < rowCount() ; ++i)
    {
        QStandardItem* const it = item(i);

        if (it && (static_cast<QAction*>(it->data(ItemActionRole).value<QObject*>()) == action))
            return it;
    }

    return 0;
}

QModelIndex ActionItemModel::indexForAction(QAction *action) const
{
    return indexFromItem(itemForAction(action));
}

void ActionItemModel::hover(const QModelIndex& index)
{
    QAction* const action = actionForIndex(index);

    if (action)
        action->hover();
}

void ActionItemModel::toggle(const QModelIndex& index)
{
    QAction* const action = actionForIndex(index);

    if (action)
        action->toggle();
}

void ActionItemModel::trigger(const QModelIndex& index)
{
    QAction* const action = actionForIndex(index);

    if (action && action->isEnabled())
        action->trigger();
}

void ActionItemModel::slotActionChanged()
{
    QAction* const action     = qobject_cast<QAction*>(sender());
    QStandardItem* const item = itemForAction(action);

    if (item)
    {
        setPropertiesFromAction(item, action);
    }
}

} // namespace Digikam
