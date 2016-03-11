/* ============================================================
 *
 * This file is a part of kipi-plugins project
 * http://www.digikam.org
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

#ifndef PRESENTATIONCONTAINER_H
#define PRESENTATIONCONTAINER_H

// Qt includes

#include <QString>
#include <QFont>
#include <QUrl>

// Local includes

#include "digikam_config.h"

namespace Digikam
{

class PresentationMainPage;
class PresentationCaptionPage;
class PresentationAdvPage;

#ifdef HAVE_AUDIO
class PresentationAudioPage;
#endif

class PresentationContainer
{

public:

    PresentationContainer();
    ~PresentationContainer();

public:

    int                  delayMsMaxValue;
    int                  delayMsMinValue;
    int                  delayMsLineStep;

    bool                 ImagesHasComments;
    QList<QUrl>          urlList;

    PresentationMainPage*            mainPage;
    PresentationCaptionPage*         captionPage;
#ifdef HAVE_AUDIO
    PresentationAudioPage*      soundtrackPage;
#endif
    PresentationAdvPage*        advancedPage;

    // Config file data

    // --> Main page
    bool                 opengl;
    bool                 openGlFullScale;
    int                  delay;
    bool                 printFileName;
    bool                 printProgress;
    bool                 printFileComments;
    bool                 loop;
    bool                 shuffle;
    bool                 showSelectedFilesOnly;
    QString              effectName;
    QString              effectNameGL;

    // --> Captions page
    uint                 commentsFontColor;
    uint                 commentsBgColor;
    bool                 commentsDrawOutline;
    uint                 bgOpacity;
    int                  commentsLinesLength;
    QFont*               captionFont;

    // --> Soundtrack page
    bool                 soundtrackLoop;
    bool                 soundtrackRememberPlaylist;
    bool                 soundtrackPlayListNeedsUpdate;
    QUrl                 soundtrackPath;
    QList<QUrl>          soundtrackUrls;

    // --> Advanced page
    bool                 useMilliseconds;
    bool                 enableMouseWheel;
    bool                 enableCache;
    bool                 kbDisableFadeInOut;
    bool                 kbDisableCrossFade;
    uint                 cacheSize;
};

} // namespace Digikam

#endif // PRESENTATIONCONTAINER_H
