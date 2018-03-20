/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-04-18
 * Description : User interface for searches
 *
 * Copyright (C) 2008-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "choicesearchutilities.h"

// Qt includes

#include <QTreeView>

// Local includes

#include "digikam_debug.h"
#include "searchutilities.h"

namespace Digikam
{

ChoiceSearchModel::Entry::Entry()
    : m_checkState(false)
{
}

ChoiceSearchModel::Entry::Entry(const QVariant& key, const QString& userDisplay)
    : m_key(key),
      m_display(userDisplay),
      m_checkState(false)
{
}

bool ChoiceSearchModel::Entry::operator==(const Entry& other) const
{
    return m_key == other.m_key;
}

ChoiceSearchModel::ChoiceSearchModel(QObject* const parent)
    : QAbstractListModel(parent)
{
}

void ChoiceSearchModel::setChoice(const QMap<int, QString>& data)
{
    if (m_entries.size())
    {
        beginResetModel();
        m_entries.clear();
        endResetModel();
    }

    for (QMap<int, QString>::const_iterator it = data.constBegin(); it != data.constEnd(); ++it)
    {
        m_entries << Entry(it.key(), it.value());
    }
}

void ChoiceSearchModel::setChoice(const QVariantList& data)
{
    if (m_entries.size())
    {
        beginResetModel();
        m_entries.clear();
        endResetModel();
    }

    Q_ASSERT(data.size() % 2 == 0);

    for (QVariantList::const_iterator it = data.constBegin(); it != data.constEnd();)
    {
        QVariant key  = *it;
        ++it;
        QString value = (*it).toString();
        ++it;
        m_entries << Entry(key, value);
    }
}

void ChoiceSearchModel::setChoice(const QStringList& data)
{
    if (m_entries.size())
    {
        beginResetModel();
        m_entries.clear();
        endResetModel();
    }

    Q_ASSERT(data.size() % 2 == 0);

    for (QStringList::const_iterator it = data.constBegin(); it != data.constEnd();)
    {
        QVariant key  = *it;
        ++it;
        QString value = *it;
        ++it;
        m_entries << Entry(key, value);
    }
}

QVariantList ChoiceSearchModel::checkedKeys() const
{
    QVariantList list;

    for (QList<Entry>::const_iterator it = m_entries.constBegin(); it != m_entries.constEnd(); ++it)
    {
        if ((*it).m_checkState)
        {
            list << (*it).m_key;
        }
    }

    return list;
}

QStringList ChoiceSearchModel::checkedDisplayTexts() const
{
    QStringList list;

    for (QList<Entry>::const_iterator it = m_entries.constBegin(); it != m_entries.constEnd(); ++it)
    {
        if ((*it).m_checkState)
        {
            list << (*it).m_display;
        }
    }

    return list;
}

void ChoiceSearchModel::setChecked(int i, bool checked)
{
    m_entries[i].m_checkState = checked;
    QModelIndex modelIndex    = index(i);

    emit dataChanged(modelIndex, modelIndex);
    emit checkStateChanged(m_entries.at(i).m_key, checked);
}

void ChoiceSearchModel::resetChecked()
{
    for (int i = 0; i < m_entries.size(); ++i)
    {
        if (m_entries.at(i).m_checkState)
        {
            setChecked(i, false);
        }
    }
}

int ChoiceSearchModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }

    return m_entries.count();
}

QVariant ChoiceSearchModel::data(const QModelIndex& index, int role) const
{
    if (index.isValid())
    {
        if (role == Qt::DisplayRole)
        {
            return m_entries.at(index.row()).m_display;
        }
        else if (role == Qt::CheckStateRole)
        {
            return m_entries.at(index.row()).m_checkState ? Qt::Checked : Qt::Unchecked;
        }
        else if (role == IdRole)
        {
            return m_entries.at(index.row()).m_key;
        }
    }

    return QVariant();
}

QModelIndex ChoiceSearchModel::index(int row, int column, const QModelIndex& parent) const
{
    if (parent.isValid() || column != 0 || row >= m_entries.size())
    {
        return QModelIndex();
    }

    return createIndex(row, 0);
}

Qt::ItemFlags ChoiceSearchModel::flags(const QModelIndex& index) const
{
    return QAbstractListModel::flags(index) | Qt::ItemIsUserCheckable;
}

bool ChoiceSearchModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (role == Qt::CheckStateRole)
    {
        Qt::CheckState state = (Qt::CheckState)value.toInt();
        setChecked(index.row(), state == Qt::Checked);
        return true;
    }
    else
    {
        return QAbstractListModel::setData(index, value, role);
    }
}

// --------------------------------------------------------------------------------------

ChoiceSearchComboBox::ChoiceSearchComboBox(QWidget* const parent)
    : ListViewComboBox(parent),
      m_label(0)
{
}

void ChoiceSearchComboBox::setModel(ChoiceSearchModel* model)
{
    ModelIndexBasedComboBox::setModel(model);
    installView();
}

ChoiceSearchModel* ChoiceSearchComboBox::model() const
{
    return static_cast<ChoiceSearchModel*>(ListViewComboBox::model());
}

DSqueezedClickLabel* ChoiceSearchComboBox::label() const
{
    return m_label;
}

void ChoiceSearchComboBox::setLabelText(const QString& text)
{
    m_label->setAdjustedText(text);
}

void ChoiceSearchComboBox::labelClicked()
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "labelClicked";
    showPopup();
}

void ChoiceSearchComboBox::installView(QAbstractItemView* v)
{
    // make protected again
    ListViewComboBox::installView(v);

    //view()->setHeaderHidden(true);
    view()->setAlternatingRowColors(true);

    // create the label
    m_label = new DSqueezedClickLabel;
    m_label->setElideMode(Qt::ElideRight);

    // set a line edit that carries the label
    ProxyClickLineEdit* const lineEdit = new ProxyClickLineEdit;
    lineEdit->setCursor(m_label->cursor());
    lineEdit->setWidget(m_label);
    setLineEdit(lineEdit);

    // connect clicks on upper area (both line edit and widget within) to showPopup
    connect(lineEdit, SIGNAL(leftClicked()),
            this, SLOT(labelClicked()));

    connect(m_label, SIGNAL(activated()),
            this, SLOT(labelClicked()));
}

} // namespace Digikam
