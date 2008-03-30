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

#ifndef SEARCHGROUP_H
#define SEARCHGROUP_H

// Qt includes

#include <QWidget>
#include <QList>

// KDE includes

// Local includes

#include "searchview.h"
#include "searchxml.h"

class QVBoxLayout;
class QComboBox;
class QRadioButton;

namespace Digikam
{

class SearchGroupLabel;
class SearchFieldGroup;
class SearchFieldGroupLabel;

class SearchGroup : public QWidget
{
public:

    SearchGroup(SearchView *parent);

    void setup();
    void setChainSearchGroup();

    void read(SearchXmlReader &reader);
    void write(SearchXmlWriter &writer);
    void reset();

protected:

    SearchView                   *m_view;

    QList<SearchFieldGroup*>      m_fieldGroups;
    QList<SearchFieldGroupLabel*> m_fieldLabels;

    QVBoxLayout                  *m_layout;
    SearchGroupLabel             *m_label;

    bool                          m_isFirstGroup;
};

// ----------------------------------- //

class SearchGroupLabel : public QWidget
{
    Q_OBJECT

public:

    SearchGroupLabel(QWidget *parent, SearchViewThemedPartsCache *cache);
    void addGroupOperatorOption();

    void setGroupOperator(SearchXml::Operator op);
    void setDefaultFieldOperator(SearchXml::Operator op);

    SearchXml::Operator groupOperator() const;
    SearchXml::Operator defaultFieldOperator() const;

protected:

    virtual void paintEvent(QPaintEvent *ev);

private:

    QVBoxLayout                *m_layout;
    QComboBox                  *m_groupOpBox;
    QRadioButton                  *m_allBox;
    QRadioButton                  *m_anyBox;
    SearchViewThemedPartsCache *m_themeCache;
};

}

#endif


