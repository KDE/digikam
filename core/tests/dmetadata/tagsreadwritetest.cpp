/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2015-08-12
 * Description : DMetadata Settings Tests for getImageTagPaths and setImageTagPaths.
 *
 * Copyright (C) 2015 by Veaceslav Munteanu <veaceslav dot munteanu90 at gmail dot com>
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

#include "tagsreadwritetest.h"

// Qt includes

#include <QTest>
#include <QStringList>
#include <QString>

// Local includes

#include "dmetadata.h"

using namespace Digikam;

QTEST_GUILESS_MAIN(TagsReadWriteTest)

void TagsReadWriteTest::initTestCase()
{
    this->tagSet1  << QLatin1String("/root/child1/child2")
                   << QLatin1String("/root/extra/child2/triple")
                   << QLatin1String("/root/extra/ch223/triple");

    this->tagSet2  << QLatin1String("/root/child1/chilasdfasdf")
                   << QLatin1String("/root/exrebhtra/chsdrild2/asdfad")
                   << QLatin1String("/root/exfgvdtra/ch2gfg23/tridfgvle");

    this->tagSet3  << QLatin1String("/rowet/child1/crehild2")
                   << QLatin1String("/rsdfsoot/extsdera/chihgld2/triple")
                   << QLatin1String("/roosdst/extfamnbra/ch2hg23/triple");
}

void TagsReadWriteTest::testSimpleReadAfterWrite()
{
    DMetadata dmeta;
    QStringList tagPaths2;

    dmeta.setImageTagsPath(this->tagSet1);
    dmeta.getImageTagsPath(tagPaths2);

    QCOMPARE(tagSet1, tagPaths2);
}

void TagsReadWriteTest::testWriteToDisabledNamespaces()
{
    DMetadata dmeta;

    DMetadataSettingsContainer dmsettings;
    QStringList empty;
    QStringList secondNamespace;

    NamespaceEntry tagNs2;
    tagNs2.namespaceName = QLatin1String("Xmp.MicrosoftPhoto.LastKeywordXMP");
    tagNs2.tagPaths      = NamespaceEntry::TAGPATH;
    tagNs2.separator     = QLatin1String("/");
    tagNs2.nsType        = NamespaceEntry::TAGS;
    tagNs2.index         = 1;
    tagNs2.specialOpts   = NamespaceEntry::TAG_XMPBAG;
    tagNs2.subspace      = NamespaceEntry::XMP;
    tagNs2.isDisabled    = true;

    NamespaceEntry tagNs3;
    tagNs3.namespaceName   = QLatin1String("Xmp.lr.hierarchicalSubject");
    tagNs3.tagPaths        = NamespaceEntry::TAGPATH;
    tagNs3.separator       = QLatin1String("|");
    tagNs3.nsType          = NamespaceEntry::TAGS;
    tagNs3.index           = 2;
    tagNs3.specialOpts     = NamespaceEntry::TAG_XMPBAG;
    tagNs3.subspace        = NamespaceEntry::XMP;
    tagNs3.alternativeName = QLatin1String("Xmp.lr.HierarchicalSubject");
    tagNs3.secondNameOpts  = NamespaceEntry::TAG_XMPSEQ;

    dmsettings.getWriteMapping(QLatin1String(DM_TAG_CONTAINER)).clear();
    dmsettings.getWriteMapping(QLatin1String(DM_TAG_CONTAINER))
             << tagNs2
             << tagNs3;

    dmeta.setImageTagsPath(tagSet1, dmsettings);

    empty           = dmeta.getXmpTagStringBag("Xmp.MicrosoftPhoto.LastKeywordXMP", false);

    QCOMPARE(empty, QStringList());

    secondNamespace = dmeta.getXmpTagStringBag("Xmp.lr.hierarchicalSubject", false);

    secondNamespace = secondNamespace.replaceInStrings(QLatin1String("|"),QLatin1String("/"));

    QCOMPARE(secondNamespace, tagSet1);
}

void TagsReadWriteTest::testReadFromDisabledNamespaces()
{
    DMetadata dmeta;

    DMetadataSettingsContainer dmsettings;
    QStringList actual;

    NamespaceEntry tagNs2;
    tagNs2.namespaceName = QLatin1String("Xmp.MicrosoftPhoto.LastKeywordXMP");
    tagNs2.tagPaths      = NamespaceEntry::TAGPATH;
    tagNs2.separator     = QLatin1String("/");
    tagNs2.nsType        = NamespaceEntry::TAGS;
    tagNs2.index         = 1;
    tagNs2.specialOpts   = NamespaceEntry::TAG_XMPBAG;
    tagNs2.subspace      = NamespaceEntry::XMP;
    tagNs2.isDisabled    = true;

    NamespaceEntry tagNs3;
    tagNs3.namespaceName   = QLatin1String("Xmp.lr.hierarchicalSubject");
    tagNs3.tagPaths        = NamespaceEntry::TAGPATH;
    tagNs3.separator       = QLatin1String("|");
    tagNs3.nsType          = NamespaceEntry::TAGS;
    tagNs3.index           = 2;
    tagNs3.specialOpts     = NamespaceEntry::TAG_XMPBAG;
    tagNs3.subspace        = NamespaceEntry::XMP;
    tagNs3.alternativeName = QLatin1String("Xmp.lr.HierarchicalSubject");
    tagNs3.secondNameOpts  = NamespaceEntry::TAG_XMPSEQ;

    dmsettings.getReadMapping(QLatin1String(DM_TAG_CONTAINER)).clear();
    dmsettings.getReadMapping(QLatin1String(DM_TAG_CONTAINER))
             << tagNs2
             << tagNs3;

    dmeta.setXmpTagStringBag("Xmp.MicrosoftPhoto.LastKeywordXMP", tagSet1);

    dmeta.setXmpTagStringBag("Xmp.lr.hierarchicalSubject", tagSet2);

    dmeta.getImageTagsPath(actual, dmsettings);

    QCOMPARE(actual, tagSet2);
}

void TagsReadWriteTest::testTagSeparatorWrite()
{
    DMetadata dmeta;

    DMetadataSettingsContainer dmsettings;
    QStringList readResult;
    QStringList expected;

    NamespaceEntry tagNs3;
    tagNs3.namespaceName   = QLatin1String("Xmp.lr.hierarchicalSubject");
    tagNs3.tagPaths        = NamespaceEntry::TAGPATH;
    tagNs3.separator       = QLatin1String("|");
    tagNs3.nsType          = NamespaceEntry::TAGS;
    tagNs3.index           = 2;
    tagNs3.specialOpts     = NamespaceEntry::TAG_XMPBAG;
    tagNs3.subspace        = NamespaceEntry::XMP;
    tagNs3.alternativeName = QLatin1String("Xmp.lr.HierarchicalSubject");
    tagNs3.secondNameOpts  = NamespaceEntry::TAG_XMPSEQ;

    dmsettings.getWriteMapping(QLatin1String(DM_TAG_CONTAINER)).clear();
    dmsettings.getWriteMapping(QLatin1String(DM_TAG_CONTAINER))
             << tagNs3;

    dmeta.setImageTagsPath(tagSet1, dmsettings);

    readResult = dmeta.getXmpTagStringBag("Xmp.lr.hierarchicalSubject", false);

    expected   = tagSet1;
    expected   = expected.replaceInStrings(QLatin1String("/"),QLatin1String("|"));

    QCOMPARE(readResult, expected);
}

void TagsReadWriteTest::testTagSeparatorRead()
{
    DMetadata dmeta;

    DMetadataSettingsContainer dmsettings;
    QStringList toWrite;
    QStringList actual;
    QStringList reference;

    NamespaceEntry tagNs3;
    tagNs3.namespaceName   = QLatin1String("Xmp.lr.hierarchicalSubject");
    tagNs3.tagPaths        = NamespaceEntry::TAGPATH;
    tagNs3.separator       = QLatin1String("|");
    tagNs3.nsType          = NamespaceEntry::TAGS;
    tagNs3.index           = 2;
    tagNs3.specialOpts     = NamespaceEntry::TAG_XMPBAG;
    tagNs3.subspace        = NamespaceEntry::XMP;
    tagNs3.alternativeName = QLatin1String("Xmp.lr.HierarchicalSubject");
    tagNs3.secondNameOpts  = NamespaceEntry::TAG_XMPSEQ;

    dmsettings.getReadMapping(QLatin1String(DM_TAG_CONTAINER)).clear();
    dmsettings.getReadMapping(QLatin1String(DM_TAG_CONTAINER))
             << tagNs3;

    toWrite   = tagSet1;
    toWrite   = toWrite.replaceInStrings(QLatin1String("/"),QLatin1String("|"));

    dmeta.setXmpTagStringBag("Xmp.lr.hierarchicalSubject", toWrite);

    reference = dmeta.getXmpTagStringBag("Xmp.lr.hierarchicalSubject", false);

    QCOMPARE(reference, toWrite);

    dmeta.getImageTagsPath(actual, dmsettings);

    QCOMPARE(actual, tagSet1);
}

void TagsReadWriteTest::testTagReadAlternativeNameSpace()
{
    DMetadata dmeta;

    DMetadataSettingsContainer dmsettings;
    QStringList toWrite;
    QStringList actual;
    QStringList reference;

    NamespaceEntry tagNs3;
    tagNs3.namespaceName   = QLatin1String("Xmp.lr.hierarchicalSubject");
    tagNs3.tagPaths        = NamespaceEntry::TAGPATH;
    tagNs3.separator       = QLatin1String("|");
    tagNs3.nsType          = NamespaceEntry::TAGS;
    tagNs3.index           = 2;
    tagNs3.specialOpts     = NamespaceEntry::TAG_XMPBAG;
    tagNs3.subspace        = NamespaceEntry::XMP;
    tagNs3.alternativeName = QLatin1String("Xmp.lr.HierarchicalSubject");
    tagNs3.secondNameOpts  = NamespaceEntry::TAG_XMPSEQ;

    dmsettings.getReadMapping(QLatin1String(DM_TAG_CONTAINER)).clear();
    dmsettings.getReadMapping(QLatin1String(DM_TAG_CONTAINER))
             << tagNs3;

    toWrite   = tagSet1;
    toWrite   = toWrite.replaceInStrings(QLatin1String("/"),QLatin1String("|"));

    dmeta.setXmpTagStringSeq("Xmp.lr.HierarchicalSubject", toWrite);

    // We write some data to alternative namespace
    reference = dmeta.getXmpTagStringSeq("Xmp.lr.HierarchicalSubject", false);

    QCOMPARE(reference, toWrite);

    dmeta.getImageTagsPath(actual, dmsettings);

    QCOMPARE(actual, tagSet1);
}
