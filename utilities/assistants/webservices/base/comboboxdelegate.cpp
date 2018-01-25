/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-07-05
 * Description : A combobox delegate to display in image lists.
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

#include "comboboxdelegate.h"

// Qt includes

#include <QApplication>
#include <QComboBox>
#include <QPaintEvent>
#include <QStyleOption>

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

ComboBoxDelegate::ComboBoxDelegate(DImagesList* const parent, const QMap<int, QString>& items)
    : QAbstractItemDelegate(parent),
      m_parent(parent),
      m_items(items),
      m_rowEdited(-1)
{
    // Figure out the maximum width of a displayed item from the items list and
    // save it in the m_size parameter.
    QFontMetrics listFont = parent->fontMetrics();
    m_size                = QSize(0, listFont.height());
    int tmpWidth          = 0;
    QMapIterator<int, QString> i(m_items);

    while (i.hasNext())
    {
        i.next();
        tmpWidth = listFont.width(i.value());

        if (tmpWidth > m_size.width())
        {
            m_size.setWidth(tmpWidth);
        }
    }
}

void ComboBoxDelegate::startEditing(QTreeWidgetItem* item, int column)
{
    // Start editing the item. This is part of a hack to make sure the item text
    // doesn't get painted whenever a combobox is drawn (otherwise the text can
    // be seen around the edges of the combobox. This method breaks the OO
    // paradigm.
    m_rowEdited = m_parent->listView()->currentIndex().row();
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    m_parent->listView()->editItem(item, column);
    item->setFlags(item->flags() & ~Qt::ItemIsEditable);
}

void ComboBoxDelegate::paint(QPainter* painter,
                             const QStyleOptionViewItem& option,
                             const QModelIndex& index) const
{
    // Draw a panel item primitive element as background.
    QStyle* const style = QApplication::style();
    style->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter);

    // If the element that gets painted is not currently edited, the item text
    // should be displayed.
    // Note that this method to detect which item is edited is a horrible hack
    // to work around the fact that there's no reliable way to detect if an item
    // is being edited from the parameters (although the documentation suggests
    // QStyle::State_Editing should be set in the option.flags parameter).
    if (m_rowEdited != index.row())
    {
        // Get the currently selected index in the items list.
        int currIndex = (index.data()).value<int>();

        // PE: These values are found by trial and error. I don't have any idea
        // if it's actually correct, but it seems to work across all themes.
        QPalette::ColorRole textColor = QPalette::Text;

        if (option.state & QStyle::State_Selected)
        {
            textColor = QPalette::HighlightedText;
        }

        // Draw the text.
        style->drawItemText(painter, option.rect, option.displayAlignment,
                            option.palette, true, m_items[currIndex],
                            textColor);
    }
}

QSize ComboBoxDelegate::sizeHint(const QStyleOptionViewItem&, const QModelIndex&) const
{
    // Return the size based on the widest item in the items list.
    return m_size;
}

QWidget* ComboBoxDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option,
                                        const QModelIndex&) const
{
    // This method returns the widget that should be used to edit the current
    // element, which is in this case a QComboBox with the items supplied by
    // the user items list on construction.
    QComboBox* const cb = new QComboBox(parent);
    QMapIterator<int, QString> i(m_items);

    while (i.hasNext())
    {
        i.next();
        cb->addItem(i.value(), QVariant(i.key()));
    }

    // Set the geometry
    cb->setGeometry(option.rect);

    // If the index is changed, the editing should be finished and the editor
    // destroyed.
    connect(cb, SIGNAL(activated(int)),
            this, SLOT(slotCommitAndCloseEditor(int)));

    // To keep track of the item being edited, the m_rowEdited parameter should
    // be reset when the editor is destroyed.
    connect(cb, SIGNAL(destroyed(QObject*)),
            this, SLOT(slotResetEditedState(QObject*)));

    return cb;
}

void ComboBoxDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    // Scroll the combobox to the current selected state on initialization.
    QComboBox* const cb = qobject_cast<QComboBox*>(editor);

    for (int i = 0; i < cb->count(); ++i)
    {
        if (cb->itemData(i).toInt() == index.data().toInt())
        {
            cb->setCurrentIndex(i);
        }
    }
}

void ComboBoxDelegate::setModelData(QWidget* editor, QAbstractItemModel* model,
                                    const QModelIndex& index) const
{
    // Write the data to the model when finishing has completed.
    QComboBox* const cb = qobject_cast<QComboBox*>(editor);
    int selected        = cb->itemData(cb->currentIndex()).toInt();
    model->setData(index, selected);
}

void ComboBoxDelegate::slotCommitAndCloseEditor(int)
{
    // Emit the proper signals when editing has finished.
    QComboBox* const editor = qobject_cast<QComboBox*>(sender());
    emit commitData(editor);
    emit closeEditor(editor);
}

void ComboBoxDelegate::slotResetEditedState(QObject*)
{
    m_rowEdited = -1;
}

} // namespace Digikam
