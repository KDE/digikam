/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-12-16
 * Description : Filter combobox
 *
 * Copyright (C) 2010-2011 by Petri Damstén <petri.damsten@iki.fi>
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

#ifndef FILTERCOMBOBOX_H
#define FILTERCOMBOBOX_H

// KDE includes

#include <kcombobox.h>

namespace Digikam
{

class CamItemInfo;

class Filter
{
public:

    Filter();

    QString toString();
    void fromString(const QString& filter);

public:

    QString     name;
    bool        onlyNew;
    QStringList fileFilter;
    QStringList pathFilter;
    QString     mimeFilter;
};

typedef QList<Filter*> FilterList;

// ---------------------------------------------------------------------

class FilterComboBox : public KComboBox
{
    Q_OBJECT

public:

    explicit FilterComboBox(QWidget* const parent);
    ~FilterComboBox();

    bool matchesCurrentFilter(const CamItemInfo& item);
    void saveSettings();

    static void  defaultFilters(FilterList* const filters);
    static const QString defaultIgnoreNames;
    static const QString defaultIgnoreExtensions;

Q_SIGNALS:

    void filterChanged();

protected:

    void  fillCombo();
    const QRegExp& regexp(const QString& wildcard);
    bool  match(const QStringList& wildcards, const QString& name);
    const QStringList& mimeWildcards(const QString& mime);

protected Q_SLOTS:

    void indexChanged(int index);

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

Q_DECLARE_METATYPE(Digikam::Filter*)

#endif /* FILTERCOMBOBOX_H */
