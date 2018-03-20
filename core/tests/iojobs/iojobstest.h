/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2015-08-02
 * Description : Test the functions for dealing with DatabaseFields
 *
 * Copyright (C) 2015 by Mohamed Anwer <m dot anwer at gmx dot com>
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

#ifndef IOJOBSTEST_H
#define IOJOBSTEST_H

// Qt includes

#include <QObject>

class IOJobsTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void init();
    void cleanup();

    void copyAndMove();
    void copyAndMove_data();

    void permanentDel();
    void permanentDel_data();
//    void rename();
};

#endif // IOJOBSTEST_H
