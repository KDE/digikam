/* ============================================================
 *
 * This file is a part of kipi-plugins project
 * http://www.digikam.org
 *
 * Date        : 2007-02-11
 * Description : a kipi plugin to show image using an OpenGL interface.
 *
 * Copyright (C) 2007-2008 by Markus Leuthold <kusi at forum dot titlis dot org>
 * Copyright (C) 2008-2016 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef GLVIEWERPLUGIN_TIMER_H
#define GLVIEWERPLUGIN_TIMER_H

// Qt includes

#include <QString>

namespace GenericGLViewerPlugin
{

class Timer
{

public:

    Timer();
    ~Timer();

    void start();
    void at(const QString& s);

private:

    class Private;
    Private* const d;
};

} // namespace GenericGLViewerPlugin

#endif // GLVIEWERPLUGIN_TIMER_H
