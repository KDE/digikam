/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-10-02
 * Description : a presentation tool.
 *
 * Copyright (C) 2008-2009 by Valerio Fuoglio <valerio dot fuoglio at gmail dot com>
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

// Local includes

#include "presentationcontainer.h"

namespace DigikamGenericPresentationPlugin
{

PresentationContainer::PresentationContainer()
{
    delayMsMaxValue               = 0;
    delayMsMinValue               = 0;
    delayMsLineStep               = 0;
    urlList                       = QList<QUrl>();
    mainPage                      = nullptr;
    captionPage                   = nullptr;
    advancedPage                  = nullptr;

#ifdef HAVE_MEDIAPLAYER
    soundtrackPage                = nullptr;
#endif

    opengl                        = false;
    openGlFullScale               = false;
    delay                         = 0;
    printFileName                 = false;
    printProgress                 = false;
    printFileComments             = false;
    loop                          = false;
    shuffle                       = false;
    commentsFontColor             = 0;
    commentsBgColor               = 0;
    commentsDrawOutline           = false;
    bgOpacity                     = 10;
    commentsLinesLength           = 0;
    captionFont                   = nullptr;

    soundtrackLoop                = false;
    soundtrackPlay                = false;
    soundtrackRememberPlaylist    = false;
    soundtrackPlayListNeedsUpdate = false;

    useMilliseconds               = false;
    enableMouseWheel              = false;
    enableCache                   = false;
    kbDisableFadeInOut            = false;
    kbDisableCrossFade            = false;
    cacheSize                     = 0;
    
    iface                         = nullptr;
}

PresentationContainer::~PresentationContainer()
{
    delete captionFont;
}

} // namespace DigikamGenericPresentationPlugin
