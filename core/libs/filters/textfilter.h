/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-02-23
 * Description : a widget to filter album contents by text query
 *
 * Copyright (C) 2011-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAM_TEXT_FILTER_H
#define DIGIKAM_TEXT_FILTER_H

// Local includes

#include "dlayoutbox.h"
#include "searchtextbar.h"
#include "itemfiltersettings.h"

class QAction;

namespace Digikam
{

class SearchTextBar;

class TextFilter : public DHBox
{
    Q_OBJECT

public:

    explicit TextFilter(QWidget* const parent);
    ~TextFilter();

    SearchTextBar* searchTextBar() const;

    void reset();

    SearchTextFilterSettings::TextFilterFields searchTextFields();
    void setsearchTextFields(SearchTextFilterSettings::TextFilterFields fields);

Q_SIGNALS:

    void signalSearchTextFilterSettings(const SearchTextFilterSettings&);

private Q_SLOTS:

    void slotSearchTextFieldsChanged();
    void slotSearchFieldsChanged(QAction*);

private:

    void checkMenuActions(bool checked);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_TEXT_FILTER_H
