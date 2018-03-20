/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-01-20
 * Description : User interface for searches
 *
 * Copyright (C) 2008-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2012-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes

#include <QList>
#include <QSet>
#include <QWidget>

// Local includes

#include "dexpanderbox.h"
#include "coredbsearchxml.h"

class QGridLayout;
class QLabel;
class QVBoxLayout;

namespace Digikam
{

class SearchFieldGroupLabel;
class SearchGroup;
class SearchField;
class VisibilityController;

class SearchFieldGroup : public QWidget
{
    Q_OBJECT

public:

    explicit SearchFieldGroup(SearchGroup* const parent);

    void addField(SearchField* const field);
    void setLabel(SearchFieldGroupLabel* const label);

    SearchField* fieldForName(const QString& fieldName) const;
    void write(SearchXmlWriter& writer);

    void reset();

    void markField(SearchField* const field);
    void clearMarkedFields();
    QList<QRect> areaOfMarkedFields() const;

public Q_SLOTS:

    void setFieldsVisible(bool visible);

protected Q_SLOTS:

    void slotLabelClicked();

protected:

    QList<SearchField*>    m_fields;
    QGridLayout*           m_layout;
    SearchFieldGroupLabel* m_label;
    VisibilityController*  m_controller;
    QSet<SearchField*>     m_markedFields;
};

// -----------------------------------------------------------------------

class SearchFieldGroupLabel : public QWidget
{
    Q_OBJECT

public:

    explicit SearchFieldGroupLabel(QWidget* const parent);

    void setTitle(const QString& title);

public Q_SLOTS:

    void displayExpanded();
    void displayFolded();

Q_SIGNALS:

    void clicked();

protected:

    QString      m_title;
    DClickLabel* m_titleLabel;
    QLabel*      m_expandLabel;
};

} // namespace Digikam

#endif // SEARCHFIELDGROUP_H
