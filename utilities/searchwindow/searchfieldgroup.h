/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-01-20
 * Description : User interface for searches
 *
 * Copyright (C) 2008 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef SEARCHFIELDGROUP_H
#define SEARCHFIELDGROUP_H

// Qt includes.

#include <QList>
#include <QSet>
#include <QWidget>

// KDE includes.

// Local includes.

#include "searchxml.h"

class QGridLayout;
class QLabel;
class QVBoxLayout;

class SearchField;

namespace Digikam
{

class SearchFieldGroupLabel;
class SearchGroup;
class SearchField;
class VisibilityController;
class SearchClickLabel;

class SearchFieldGroup : public QWidget
{
    Q_OBJECT

public:

    SearchFieldGroup(SearchGroup *parent);

    void addField(SearchField *field);
    void setLabel(SearchFieldGroupLabel *label);

    SearchField *fieldForName(const QString &fieldName);
    void write(SearchXmlWriter &writer);

    void reset();

    void markField(SearchField *field);
    void clearMarkedFields();
    QList<QRect> areaOfMarkedFields() const;

public Q_SLOTS:

    void setFieldsVisible(bool visible);

protected Q_SLOTS:

    void slotLabelClicked();

protected:

    QList<SearchField*>    m_fields;
    QGridLayout           *m_layout;
    SearchFieldGroupLabel *m_label;
    VisibilityController  *m_controller;

    QSet<SearchField*>     m_markedFields;
};

class SearchFieldGroupLabel : public QWidget
{
    Q_OBJECT

public:

    SearchFieldGroupLabel(QWidget *parent);

    void setTitle(const QString &title);

public Q_SLOTS:

    void displayExpanded();
    void displayFolded();

Q_SIGNALS:

    void clicked();

protected:

    QString            m_title;
    SearchClickLabel  *m_titleLabel;
    QLabel            *m_expandLabel;
};

} // namespace Digikam

#endif // SEARCHFIELDGROUP_H
