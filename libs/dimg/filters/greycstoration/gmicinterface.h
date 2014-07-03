/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2014-20-06
 * Description : Gmic interface for digikam.
 *
 * Copyright (C) 2014 by Veaceslav Munteanu<veaceslav dot munteanu90 at gmail dot com>
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

#include <QObject>
#include <QString>

#include "CImg.h"
using namespace cimg_library;
namespace Digikam
{

class GMicInterface : public QObject
{
    Q_OBJECT
public:
    GMicInterface();

    ~GMicInterface();

    void setImg(CImg<> image);

    void setCommand(QString command);

    CImg<> getImg();

    float getProgress();

    void cancel();

public slots:
    void runGmic();
signals:
    void signalResultReady(bool);

private:
    class GMicInterfacePriv;
    GMicInterfacePriv* d;
};

}
