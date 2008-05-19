/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-05-19
 * Description : Fuzzy search sidebar tab contents.
 *
 * Copyright (C) 2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef FUZZYSEARCHVIEW_H
#define FUZZYSEARCHVIEW_H

// Qt includes.

#include <QFrame>
#include <QImage>

namespace Digikam
{

class FuzzySearchViewPriv;

class FuzzySearchView : public QFrame
{
    Q_OBJECT

public:

    FuzzySearchView(QWidget *parent=0);
    ~FuzzySearchView();

    void setActive(bool val);

private: 

    void readConfig();
    void writeConfig();

private slots:

    void slotHSChanged(int h, int s);
    void slotVChanged();
    void slotResultsChanged();
    void slotSketchChanged(const QImage&);

private:

    FuzzySearchViewPriv *d;
};

}  // NameSpace Digikam

#endif /* FUZZYSEARCHVIEW_H */
