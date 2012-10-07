/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-01-08
 * Description : polling thread checks if there are digikam
 *               components registered on DBus
 *
 * Copyright (C) 2009-2011 by Holger Foerster <Hamsi2k at freenet dot de>
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

#ifndef POLLTHREAD_H_
#define POLLTHREAD_H_

// Qt includes

#include <QCoreApplication>
#include <QThread>

namespace Digikam
{

class PollThread : public QThread
{
    Q_OBJECT

public:

    explicit PollThread(QObject* const application);
    void run();
    bool checkDigikamInstancesRunning();
    bool stop;

Q_SIGNALS:

    void done();

private:

    int waitTime;
};

} // namespace Digikam

#endif /* POLLTHREAD_H_ */
