/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-01-12
 * Description : a test for the searchtextbar
 *
 * Copyright (C) 2010 by Johannes Wienke <languitar at semipol dot de>
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

#ifndef SEARCHTEXTBARTEST_H
#define SEARCHTEXTBARTEST_H

// Qt includes

#include <QObject>

// Local includes

#include "searchtextbar.h"

// What a hack... otherwise I can't use connect with SearchTextBar as argument
using namespace Digikam;

class SearchTextBarTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testHighlighting();
    void testSearchTextSettings();
    void newSearchTextSettings(const SearchTextSettings& settings);

private:

    SearchTextSettings lastSearchTextSettings;
    int                callCount;
};

#endif /* SEARCHTEXTBARTEST_H_ */
