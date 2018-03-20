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

#ifndef CHOICESEARCHUTILITIES_H
#define CHOICESEARCHUTILITIES_H

// Qt includes

#include <QAbstractListModel>
#include <QList>
#include <QVariant>

// Local includes

#include "dexpanderbox.h"
#include "coredbsearchxml.h"
#include "comboboxutilities.h"

namespace Digikam
{

class ChoiceSearchModel : public QAbstractListModel
{
    Q_OBJECT

public:

    enum CustomRoles
    {
        IdRole = Qt::UserRole
    };

public:

    explicit ChoiceSearchModel(QObject* const parent = 0);

    /** Sets the data from the given map, with integer keys and QString user displayable value.
     */
    void setChoice(const QMap<int, QString>& data);

    /** Sets the data from the given list,
     *  taking every first entry as the key, every second as the user displayable value.
     *  Ensure that the QVariants' type is correct (identical for all even entries, QString for all odd entries).
     */
    void setChoice(const QVariantList& data);

    /** Sets the data from the given list,
     *  taking every first entry as the key, every second as the user displayable value.
     */
    void setChoice(const QStringList& data);

    /** Returns the keys of all entries that are selected (checked).
     */
    QVariantList checkedKeys() const;

    /** Returns the keys of all entries that are selected (checked), converted to
     *  a list of the template type. Supported for Int and QString types.
     */
    template <typename T> QList<T> checkedKeys() const;

    /** Returns the display text of all entries that are selected.
     */
    QStringList checkedDisplayTexts() const;

    /** Sets the check state of the entry with given key.
     */
    template <typename T> void setChecked(const T& key, bool checked = true);

    /** Sets the check state of all the entries whose key is found in the list to checked.
     */
    template <typename T> void setChecked(const QList<T>& keys, bool checked = true);

    /** Sets the check state of all entries. The check state is determined by
     *  the key of an entry, the relation, and a constant value.
     *  Think of "Set to checked if key is less than 5".
     *  Supported for Int and QString types.
     */
    template <typename T> void setChecked(const T& value, SearchXml::Relation relation);

    /** Sets all entries to unchecked.
     */
    void resetChecked();

    virtual int           rowCount(const QModelIndex& parent) const;
    virtual QVariant      data(const QModelIndex& index, int role) const;
    virtual QModelIndex   index(int row, int column = 0, const QModelIndex& parent = QModelIndex()) const;
    virtual Qt::ItemFlags flags(const QModelIndex& index) const;
    virtual bool          setData(const QModelIndex& index, const QVariant& value, int role);

Q_SIGNALS:

    void checkStateChanged(const QVariant& key, bool isChecked);

protected:

    void setChecked(int index, bool checked);

protected:

    class Entry
    {
    public:

        Entry();
        Entry(const QVariant& key, const QString& userDisplay);

        bool operator==(const Entry& other) const;

        /// @todo This function has no definition.
        bool operator==(const QVariant& other) const;

    public:

        QVariant m_key;
        QString  m_display;
        bool     m_checkState;
    };

protected:

    QList<Entry> m_entries;
};

template <typename T> void ChoiceSearchModel::setChecked(const T& key, bool checked)
{
    QVariant variantKey(key);

    for (int i = 0; i < m_entries.size(); ++i)
    {
        if (m_entries[i].m_key == variantKey)
        {
            setChecked(i, checked);
        }
    }
}

template <typename T> void ChoiceSearchModel::setChecked(const T& value, SearchXml::Relation relation)
{
    for (int i = 0; i < m_entries.size(); ++i)
    {
        setChecked(i, SearchXml::testRelation(m_entries.at(i).m_key.value<T>(), value, relation));
    }
}

template <typename T> void ChoiceSearchModel::setChecked(const QList<T>& keys, bool checked)
{
    foreach(T key, keys)
    {
        setChecked(key, checked);
    }
}

template <typename T> QList<T> ChoiceSearchModel::checkedKeys() const
{
    QList<T> list;

    for (QList<Entry>::const_iterator it = m_entries.begin(); it != m_entries.end(); ++it)
    {
        if ((*it).m_checkState)
        {
            list << (*it).m_key.value<T>();
        }
    }

    return list;
}

// -------------------------------------------------------------------------------------

class ChoiceSearchComboBox : public ListViewComboBox
{
    Q_OBJECT

public:

    /** A combo box for entering a choice of values.
     *  Operates on a ChoiceSearchModel.
     *  After constructing the object, call setModel
     *  with your model.
     */
    explicit ChoiceSearchComboBox(QWidget* const parent = 0);

    /** Sets the model and initializes the widget.
     *  Can only be called once for a widget.
     */
    void setModel(ChoiceSearchModel* model);

    /** Updates the text on the line edit area.
     */
    void setLabelText(const QString& text);

    ChoiceSearchModel*   model() const;
    DSqueezedClickLabel* label() const;

Q_SIGNALS:

    void checkStateChanged();

protected Q_SLOTS:

    void labelClicked();

protected:

    virtual void installView(QAbstractItemView* view = 0);

protected:

    DSqueezedClickLabel* m_label;
};

} // namespace Digikam

#endif // CHOICESEARCHUTILITIES_H
