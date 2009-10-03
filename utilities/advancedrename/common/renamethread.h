/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-02-06
 * Description : renaming thread
 *
 * Copyright (C) 2009 by Andi Clemens <andi dot clemens at gmx dot net>
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

#ifndef RENAMETHREAD_H
#define RENAMETHREAD_H

// Qt includes

#include <QThread>

// KDE includes

#include <kurl.h>

// Local includes

#include "advancedrenamedialog.h"
#include "imageinfo.h"

namespace Digikam
{

class DigikamImageView;
class RenameThreadPriv;

class RenameThread : public QThread
{
    Q_OBJECT

public:

    RenameThread(QObject* parent);
    ~RenameThread();

    void addNewNames(const NewNamesList& newNames);
    void cancel();

Q_SIGNALS:

    void renameFile(const ImageInfo&, const QString&);

public Q_SLOTS:

    void processNext();

protected:

    void run();

private:

    RenameThreadPriv* const d;
};

}  // namespace Digikam

#endif /* RENAMETHREAD_H */
