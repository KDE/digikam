/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-09-19
 * Description : Scanning a single item - private containers.
 *
 * Copyright (C) 2007-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2013-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "itemscanner.h"

// Qt includes

#include <QImageReader>
#include <QTime>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "coredburl.h"
#include "coredbaccess.h"
#include "coredb.h"
#include "similaritydbaccess.h"
#include "similaritydb.h"
#include "collectionlocation.h"
#include "collectionmanager.h"
#include "facetagseditor.h"
#include "itemcomments.h"
#include "itemcopyright.h"
#include "itemextendedproperties.h"
#include "itemhistorygraph.h"
#include "metaenginesettings.h"
#include "tagregion.h"
#include "tagscache.h"
#include "iostream"
#include "dimagehistory.h"
#include "itemhistorygraphdata.h"

namespace Digikam
{

class Q_DECL_HIDDEN ItemScannerCommit
{

public:

    enum Operation
    {
        NoOp,
        AddItem,
        UpdateItem
    };

public:

    ItemScannerCommit();

public:

    Operation                        operation;

    qlonglong                        copyImageAttributesId;

    bool                             commitImageInformation;
    bool                             commitImageMetadata;
    bool                             commitVideoMetadata;
    bool                             commitItemPosition;
    bool                             commitItemComments;
    bool                             commitItemCopyright;
    bool                             commitFaces;
    bool                             commitIPTCCore;
    bool                             hasColorTag;
    bool                             hasPickTag;

    DatabaseFields::ImageInformation imageInformationFields;
    QVariantList                     imageInformationInfos;

    QVariantList                     imageMetadataInfos;
    QVariantList                     imagePositionInfos;

    CaptionsMap                      captions;
    QString                          headline;
    QString                          title;

    Template                         copyrightTemplate;
    QMap<QString,QVariant>           metadataFacesMap;

    QVariantList                     iptcCoreMetadataInfos;

    QList<int>                       tagIds;
    QString                          historyXml;
    QString                          uuid;
};

// ---------------------------------------------------------------------------------------

class Q_DECL_HIDDEN LessThanByProximityToSubject
{
public:

    explicit LessThanByProximityToSubject(const ImageInfo& subject);

    bool operator()(const ImageInfo& a, const ImageInfo& b);

public:

    ImageInfo subject;
};

// ---------------------------------------------------------------------------

class Q_DECL_HIDDEN ItemScanner::Private
{
public:

    explicit Private();

public:

    bool                   hasImage;
    bool                   hasMetadata;
    bool                   loadedFromDisk;

    QFileInfo              fileInfo;

    DMetadata              metadata;
    DImg                   img;
    ItemScanInfo           scanInfo;
    ItemScanner::ScanMode scanMode;

    bool                   hasHistoryToResolve;

    ItemScannerCommit     commit;

    QTime                  time;
};

} // namespace Digikam
