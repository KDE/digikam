/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2015-06-10
 * Description : Progress observer for duplicate scanning
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

#ifndef DUPLICATESPROGRESSOBSERVER_H
#define DUPLICATESPROGRESSOBSERVER_H

#include "haariface.h"
#include "dbjob.h"
#include "digikam_export.h"

namespace Digikam
{

class SearchesJob;

class DIGIKAM_DATABASE_EXPORT DuplicatesProgressObserver : public HaarProgressObserver
{

public:

    DuplicatesProgressObserver(SearchesJob* const thread);
    ~DuplicatesProgressObserver();

    virtual void totalNumberToScan(int number);
    virtual void processedNumber(int number);
    virtual bool isCanceled();

private:

    SearchesJob* m_job;
};

} // namespace Digikam

#endif // DUPLICATESPROGRESSOBSERVER_H
