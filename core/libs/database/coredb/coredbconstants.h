/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-04-21
 * Description : Structures for use in CoreDB
 *
 * Copyright (C) 2007-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2010-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C)      2005 by Renchi Raju <renchi dot raju at gmail dot com>
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

#ifndef CORE_DATABASE_CONSTANTS_H
#define CORE_DATABASE_CONSTANTS_H

// Qt includes

#include <QLatin1String>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

namespace AlbumRoot
{

enum Type
{
    // Keep values constant
    UndefinedType   = 0,
    VolumeHardWired = 1,
    VolumeRemovable = 2,
    Network         = 3
};

} // namespace AlbumRoot

// ----------------------------------------------------------------------------------

namespace DatabaseSearch
{

enum Type
{
    UndefinedType,
    KeywordSearch,
    AdvancedSearch,
    LegacyUrlSearch,
    TimeLineSearch,
    HaarSearch,
    MapSearch,
    DuplicatesSearch
};

enum HaarSearchType
{
    HaarImageSearch,
    HaarSketchSearch
};

} // namespace DatabaseSearch

// ----------------------------------------------------------------------------------

namespace DatabaseItem
{

enum Status
{
    // Keep values constant
    UndefinedStatus = 0,
    Visible         = 1,
    Hidden          = 2,
    Trashed         = 3, // previously "Removed"
    Obsolete        = 4
};

enum Category
{
    // Keep values constant
    UndefinedCategory = 0,
    Image             = 1,
    Video             = 2,
    Audio             = 3,
    Other             = 4
};

} // namespace DatabaseItem

// ----------------------------------------------------------------------------------

namespace DatabaseRelation
{

enum Type
{
    UndefinedType = 0,
    /** The subject is a derivative of the object */
    DerivedFrom   = 1,
    /** The subject is grouped behind the object */
    Grouped       = 2
};

} // namespace DatabaseRelation

// ----------------------------------------------------------------------------------

namespace DatabaseComment
{

enum Type
{
    // Keep values constant
    /// UndefinedType: Shall never appear in the database
    UndefinedType   = 0,
    /**
     *  Comment: The default - a normal comment
     *  This is what the user in digikam edits as the comment.
     *  It is mapped to and from the JFIF comment,
     *  the EXIF user comment, the IPTC Caption,
     *  Dublin Core and Photoshop Description.
     */
    Comment         = 1,
    /// Headline: as with IPTC or Photoshop
    Headline        = 2,
    /// Title: as with Dublin Core Title, Photoshop Title, IPTC Object Name
    Title           = 3
    // Feel free to add here any more types that you need!
};

} // namespace DatabaseComment

// ----------------------------------------------------------------------------------

class DIGIKAM_DATABASE_EXPORT InternalTagName
{
public:

    static QLatin1String scannedForFaces();
    static QLatin1String needResolvingHistory();
    static QLatin1String needTaggingHistoryGraph();

    static QLatin1String originalVersion();
    static QLatin1String currentVersion();
    static QLatin1String intermediateVersion();
    static QLatin1String versionAlwaysVisible();

    static QLatin1String colorLabelNone();
    static QLatin1String colorLabelRed();
    static QLatin1String colorLabelOrange();
    static QLatin1String colorLabelYellow();
    static QLatin1String colorLabelGreen();
    static QLatin1String colorLabelBlue();
    static QLatin1String colorLabelMagenta();
    static QLatin1String colorLabelGray();
    static QLatin1String colorLabelBlack();
    static QLatin1String colorLabelWhite();

    static QLatin1String pickLabelNone();
    static QLatin1String pickLabelRejected();
    static QLatin1String pickLabelPending();
    static QLatin1String pickLabelAccepted();
};

// ----------------------------------------------------------------------------------

class DIGIKAM_DATABASE_EXPORT TagPropertyName
{
public:

    static QLatin1String person();
    static QLatin1String unknownPerson();
    static QLatin1String unconfirmedPerson();
    static QLatin1String faceEngineName();
    static QLatin1String tagKeyboardShortcut();
    static QLatin1String faceEngineUuid();
};

// ----------------------------------------------------------------------------------

class DIGIKAM_DATABASE_EXPORT ImageTagPropertyName
{
public:

    static QLatin1String tagRegion();
    static QLatin1String autodetectedFace();
    static QLatin1String autodetectedPerson();
    static QLatin1String faceToTrain();
};

} // namespace Digikam

#endif // CORE_DATABASE_CONSTANTS_H
