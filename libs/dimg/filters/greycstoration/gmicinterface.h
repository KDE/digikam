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

    /**
     * @brief addImg - set single image for processing
     *
     * @param image  - image to be processed
     */
    void addImg(CImg<> image);

    /**
     * @brief addImg - set 2 images for processing, for algorithms which apply
     *                 mask
     * @param image  - main image
     * @param mask   - mask to be applied to main image
     */
    void addImg(CImg<> image, CImg<> mask);

    /**
     * @brief setCommand - set single threaded command to be applied to image.
     *                     Use only if gmic command is not compatible with
     *                     multi threads, for example algorithms that require
     *                     a mask
     * @param command    - command to be executed
     */
    void setCommand(QString command);

    /**
     * @brief setParallelCommand - set command with parallel support
     *
     * @param command - command to be run in parallel, parallel arguments are
     *                  added automatically
     */
    void setParallelCommand(QString command);

    /**
     * @brief getImg - method to retrieve image after processing
     *
     * @return - image processed
     */
    CImg<> getImg();

    /**
     * @brief getProgress - return progress value reported by gmic
     * @return -1 if no progress available and 0..1 if available
     */
    float getProgress();

    /**
     * @brief cancel - tell gmic to cancel processing
     */
    void cancel();

public slots:

    /**
     * @brief runGmic - start gmic with loaded image and command
     */
    void runGmic();

signals:
    void signalResultReady(bool);

private:
    class GMicInterfacePriv;
    GMicInterfacePriv* d;
};

}
