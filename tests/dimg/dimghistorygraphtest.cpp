/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-08-01
 * Description : a test for the DImageHistory
 *
 * Copyright (C) 2010 by Marcel Wiesweg <user dot wiesweg at gmx dot de>
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

#include "dimghistorygraphtest.h"

// Qt includes

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTime>
#include <QTreeView>
#include <QDebug>
#include <QTest>
#include <QUrl>

// Local includes

#include "coredb.h"
#include "collectionlocation.h"
#include "collectionmanager.h"
#include "collectionscanner.h"
#include "editorcore.h"
#include "dmetadata.h"
#include "imageinfo.h"
#include "imagehistorygraph.h"
#include "imagehistorygraphdata.h"
#include "imagehistorygraphmodel.h"
#include "iofilesettings.h"
#include "tagscache.h"
#include "../modeltest/modeltest.h"

using namespace Digikam;

QTEST_MAIN(DImgHistoryGraphTest)

void DImgHistoryGraphTest::initTestCase()
{
    initBaseTestCase();

    QString name  = tempFileName(QLatin1String("collection"));
    collectionDir = QDir::temp();
    collectionDir.mkdir(name);
    collectionDir.cd(name);
    QVERIFY(collectionDir.exists());

    dbFile = tempFilePath(QLatin1String("database"));

    qDebug() << "Using database path for test: " << dbFile;

    DbEngineParameters params(QLatin1String("QSQLITE"), dbFile, QLatin1String("QSQLITE"), dbFile);
    CoreDbAccess::setParameters(params, CoreDbAccess::MainApplication);
    QVERIFY(CoreDbAccess::checkReadyForUse(0));
    QVERIFY(QFile(dbFile).exists());
    CollectionManager::instance()->addLocation(QUrl::fromLocalFile(collectionDir.path()));
    CollectionManager::instance()->addLocation(QUrl::fromLocalFile(imagePath()));
    QList<CollectionLocation> locs = CollectionManager::instance()->allAvailableLocations();
    QVERIFY(locs.size() == 2);

    rescan();

    QList<AlbumShortInfo> albums = CoreDbAccess().db()->getAlbumShortInfos();
    QVERIFY(albums.size() >= 2);

    foreach(const AlbumShortInfo& album, albums)
    {
        //qDebug() << album.relativePath << album.id;
        //qDebug() << CollectionManager::instance()->albumRootPath(album.albumRootId);
        //qDebug() << CoreDbAccess().db()->getItemURLsInAlbum(album.id);
        readOnlyImages << CoreDbAccess().db()->getItemURLsInAlbum(album.id);
    }

    foreach(const QString& file, readOnlyImages)
    {
        ids << ImageInfo::fromLocalFile(file).id();
    }

    QVERIFY(!ids.contains(-1));
    QVERIFY(ids.size() >= 6);          // Nb of files in data sub dir.
}

void DImgHistoryGraphTest::cleanupTestCase()
{
    cleanupBaseTestCase();

    QFile(dbFile).remove();

    QDir dir(collectionDir.path());
    dir.removeRecursively();

    qDebug() << "deleted test folder " << collectionDir.path();
}

void DImgHistoryGraphTest::rescan()
{
    CollectionScanner().completeScan();
}

template <typename from, typename to>
QList<to> mapList(const QList<from>& l, const QMap<from,to> map)
{
    QList<to> r;

    foreach(const from& f, l)
    {
        r << map.value(f);
    }

    return r;
}

void DImgHistoryGraphTest::testEditing()
{
    QDir imageDir(imagePath());

    /*
    orig
        1
            2
                3
                4
    */

    IOFileSettings container;
    m_im->load(readOnlyImages.first(), &container);
    m_loop.exec();

    applyFilters1();
    m_im->saveAs(collectionDir.filePath(QLatin1String("1.jpg")), &container, true, QString(), QString());
    m_loop.exec();

    applyFilters2();
    m_im->saveAs(collectionDir.filePath(QLatin1String("2.jpg")), &container, true, QString(), QString());
    m_loop.exec();

    applyFilters3();
    m_im->saveAs(collectionDir.filePath(QLatin1String("3.jpg")), &container, true, QString(), QString());
    m_loop.exec();

    m_im->load(collectionDir.filePath(QLatin1String("2.jpg")), &container);
    m_loop.exec();

    applyFilters4();
    m_im->saveAs(collectionDir.filePath(QLatin1String("4.jpg")), &container, true, QString(), QString());
    m_loop.exec();

    CollectionScanner().completeScan();

    ImageInfo orig  = ImageInfo::fromLocalFile(readOnlyImages.first());
    ImageInfo one   = ImageInfo::fromLocalFile(collectionDir.filePath(QLatin1String("1.jpg"))),
              two   = ImageInfo::fromLocalFile(collectionDir.filePath(QLatin1String("2.jpg"))),
              three = ImageInfo::fromLocalFile(collectionDir.filePath(QLatin1String("3.jpg"))),
              four  = ImageInfo::fromLocalFile(collectionDir.filePath(QLatin1String("4.jpg")));

    typedef QPair<qlonglong, qlonglong> IdPair;
    QList<IdPair> controlCloud;
    controlCloud << IdPair(one.id(), orig.id()); //X
    controlCloud << IdPair(two.id(), one.id());  //X
    controlCloud << IdPair(three.id(), two.id());//X
    controlCloud << IdPair(four.id(), two.id()); //X
    controlCloud << IdPair(three.id(), one.id());
    controlCloud << IdPair(four.id(), one.id());
    controlCloud << IdPair(two.id(), orig.id());
    controlCloud << IdPair(three.id(), orig.id());
    controlCloud << IdPair(four.id(), orig.id());
    std::sort(controlCloud.begin(), controlCloud.end());

    ImageHistoryGraph graph1 = ImageHistoryGraph::fromInfo(three);
    qDebug() << graph1;
    ImageHistoryGraph graph2 = ImageHistoryGraph::fromInfo(four);
    qDebug() << graph2;
    ImageHistoryGraph graph3 = ImageHistoryGraph::fromInfo(one);
    qDebug() << graph3;

    // all three must have the full cloud
    QVERIFY(graph1.data().vertexCount() == 5);
    QVERIFY(graph2.data().vertexCount() == 5);
    QVERIFY(graph3.data().vertexCount() == 5);

    QList<IdPair> cloud = graph3.relationCloud();
    std::sort(cloud.begin(), cloud.end());
    QVERIFY(cloud == controlCloud);

    int needResolvingTag = TagsCache::instance()->getOrCreateInternalTag(InternalTagName::needResolvingHistory());
    int needTaggingTag   = TagsCache::instance()->getOrCreateInternalTag(InternalTagName::needTaggingHistoryGraph());

    int originalVersionTag     = TagsCache::instance()->getOrCreateInternalTag(InternalTagName::originalVersion());
    int currentVersionTag      = TagsCache::instance()->getOrCreateInternalTag(InternalTagName::currentVersion());
    int intermediateVersionTag = TagsCache::instance()->getOrCreateInternalTag(InternalTagName::intermediateVersion());

    //qDebug() << orig.tagIds() << one.tagIds() << two.tagIds() << three.tagIds() << four.tagIds();
    QVERIFY(!orig.tagIds().contains(needResolvingTag));
    QVERIFY(!orig.tagIds().contains(needTaggingTag));

    QVERIFY(orig.tagIds().contains(originalVersionTag));
    QVERIFY(one.tagIds().contains(intermediateVersionTag));
    QVERIFY(two.tagIds().contains(intermediateVersionTag));
    QVERIFY(three.tagIds().contains(currentVersionTag));
    QVERIFY(four.tagIds().contains(currentVersionTag));

    QFile fileTwo(two.filePath());
    fileTwo.remove();
    QVERIFY(!fileTwo.exists());
    CollectionScanner().completeScan();
    graph2 = ImageHistoryGraph::fromInfo(four);
    // graph is prepared for display, vertex of removed file cleared
    QVERIFY(graph2.data().vertexCount() == 4);
    qDebug() << graph2;

    // Check that removal of current version leads to resetting of current verion tag
    QFile fileThree(three.filePath());
    fileThree.remove();
    QFile fileFour(four.filePath());
    fileFour.remove();
    CollectionScanner().completeScan();
    qDebug() << originalVersionTag << currentVersionTag << intermediateVersionTag<<  orig.tagIds() << one.tagIds();
    QVERIFY(one.tagIds().contains(currentVersionTag));
    QVERIFY(!one.tagIds().contains(intermediateVersionTag));
}

void DImgHistoryGraphTest::testHistory()
{
    ImageHistoryGraph graph;
    ImageInfo subject(ids.first());
    graph.addHistory(history1(), subject);
    graph.reduceEdges();

    QCOMPARE(graph.data().vertexCount(), 3);
    QCOMPARE(graph.data().edges().size(), 2);
}

class lessThanById
{
public:

    lessThanById(const QMap<HistoryGraph::Vertex, qlonglong>& vertexToId)
        : vertexToId(vertexToId)
    {
    }

    bool operator()(const HistoryGraph::Vertex& a, const HistoryGraph::Vertex& b)
    {
        return vertexToId.value(a) < vertexToId.value(b);
    }

public:

    QMap<HistoryGraph::Vertex, qlonglong> vertexToId;
};

void DImgHistoryGraphTest::testGraph()
{
    /*
    1
        2
            8
            9
                19
                20
                21
            10
        3
        4
            11
            12
                22
                23
                |
            13-- 24
        5
            14
        6
            15
        7
            16
            17
            18


    */

    QList<qlonglong> controlLeaves;
    controlLeaves << 8 << 19 << 20 << 21 << 10 << 3 << 11 << 22 << 24 << 14 << 15 << 16 << 17 << 18;
    std::sort(controlLeaves.begin(), controlLeaves.end());

    QList<qlonglong> controlRoots;
    controlRoots << 1;

    QList<qlonglong> controlLongestPathEighteen;
    controlLongestPathEighteen << 1 << 7 << 18;
    QList<qlonglong> controlLongestPathTwentyFour;
    controlLongestPathTwentyFour << 1 << 4 << 12 << 23 << 24;

    QList<qlonglong> controlSubgraphTwo;
    controlSubgraphTwo << 2 << 8 << 9 << 10 << 19 << 20 << 21;

    QList<qlonglong> controlSubgraphTwoSorted;
    controlSubgraphTwoSorted << 2 << 8 << 9 << 19 << 20 << 21 << 10;

    QList<qlonglong> controlSubgraphFour;
    controlSubgraphFour << 4 << 11 << 12 << 13 << 22 << 23 << 24;

    QList<qlonglong> controlRootsOfEighteen;
    controlRootsOfEighteen << 1;

    QList<qlonglong> controlLeavesFromTwo;
    controlLeavesFromTwo << 8 << 10 << 19 << 20 << 21;

    typedef QPair<qlonglong, qlonglong> IdPair;
    QList<IdPair> pairs;

    /**
     * The following description of the tree-like graph above (24 breaks (poly)tree definition)
     * is longer than needed (transitive reduction) and less than possible (transitive closure):
     * Pairs marked with "X" must remain when building the transitive reduction.
     * The transitive closure must additionally contain all pairs not marked,
     * and the pairs commented out.
     */
    pairs << IdPair(2,1); //X
    pairs << IdPair(3,1); //X
    pairs << IdPair(4,1); //X
    pairs << IdPair(5,1); //X
    pairs << IdPair(6,1); //X
    pairs << IdPair(7,1); //X
    pairs << IdPair(8,1);
    //pairs << IdPair(9,1);
    pairs << IdPair(10,1);
    pairs << IdPair(11,1);
    pairs << IdPair(12,1);
    pairs << IdPair(13,1);
    pairs << IdPair(14,1);
    pairs << IdPair(15,1);
    pairs << IdPair(16,1);
    pairs << IdPair(17,1);
    pairs << IdPair(18,1);

    pairs << IdPair(22,4);
    pairs << IdPair(23,4);
    pairs << IdPair(24,4);
    pairs << IdPair(14,5); //X
    pairs << IdPair(15,6); //X

    //pairs << IdPair(19,1);
    //pairs << IdPair(20,1);
    //pairs << IdPair(21,1);
    pairs << IdPair(22,1);
    pairs << IdPair(23,1);
    pairs << IdPair(24,1);
    pairs << IdPair(8,2); //X
    pairs << IdPair(9,2); //X
    pairs << IdPair(10,2); //X
    //pairs << IdPair(19,2);
    //pairs << IdPair(20,2);
    //pairs << IdPair(21,2);
    pairs << IdPair(11,4); //X
    pairs << IdPair(12,4); //X
    pairs << IdPair(13,4); //X

    pairs << IdPair(16,7); //X
    pairs << IdPair(17,7); //X
    pairs << IdPair(18,7); //X
    pairs << IdPair(19,9); //X
    pairs << IdPair(20,9); //X
    pairs << IdPair(21,9); //X
    pairs << IdPair(22,12); //X
    pairs << IdPair(23,12); //X

    // no more a polytree
    pairs << IdPair(24,13); //X
    pairs << IdPair(24,23); //X
    pairs << IdPair(24,4);
    pairs << IdPair(24,1);
    pairs << IdPair(24,12);

    ImageHistoryGraph graph;
    graph.addRelations(pairs);

    qDebug() << "Initial graph:" << graph;

    graph.reduceEdges();

    qDebug() << "Transitive reduction:" << graph;

    QList<IdPair> cloud = graph.relationCloud();
    qDebug() << "Transitive closure:" << cloud;

    QVERIFY(cloud.contains(IdPair(7,1)));
    QVERIFY(cloud.contains(IdPair(8,1)));
    QVERIFY(cloud.contains(IdPair(9,1)));

    /*
    QBENCHMARK
    {
        ImageHistoryGraph benchGraph;
        graph.addRelations(pairs);
        graph.finish();
        graph.relationCloud();
    }
    */

    QMap<qlonglong,HistoryGraph::Vertex> idToVertex;
    QMap<HistoryGraph::Vertex, qlonglong> vertexToId;

    foreach(const HistoryGraph::Vertex& v, graph.data().vertices())
    {
        HistoryVertexProperties props = graph.data().properties(v);
        idToVertex[props.infos.first().id()] = v;
        vertexToId[v] = props.infos.first().id();
    }

    QList<qlonglong> leaves = mapList(graph.data().leaves(), vertexToId);
    std::sort(leaves.begin(), leaves.end());
    QVERIFY(leaves == controlLeaves);

    QList<qlonglong> roots = mapList(graph.data().roots(), vertexToId);
    std::sort(roots.begin(), roots.end());
    QVERIFY(roots == controlRoots);

    QList<qlonglong> longestPath1 = mapList(graph.data().longestPathTouching(idToVertex.value(18)), vertexToId);
    QVERIFY(longestPath1 == controlLongestPathEighteen);
    QList<qlonglong> longestPath2 = mapList(graph.data().longestPathTouching(idToVertex.value(24)), vertexToId);
    QVERIFY(longestPath2 == controlLongestPathTwentyFour);

    // depth-first
    QList<qlonglong> subgraphTwo = mapList(graph.data().verticesDominatedBy(idToVertex.value(2), idToVertex.value(1),
                                           HistoryGraph::DepthFirstOrder), vertexToId);
    std::sort(subgraphTwo.begin(), subgraphTwo.end());
    QVERIFY(subgraphTwo == controlSubgraphTwo);

    // breadth-first
    QList<qlonglong> subgraphFour = mapList(graph.data().verticesDominatedBy(idToVertex.value(4), idToVertex.value(1)), vertexToId);
    QVERIFY(subgraphFour.indexOf(22) > subgraphFour.indexOf(13));
    std::sort(subgraphFour.begin(), subgraphFour.end());
    QVERIFY(subgraphFour == controlSubgraphFour);

    // depth-first
    QList<qlonglong> subgraphTwoSorted = mapList(
            graph.data().verticesDominatedByDepthFirstSorted(idToVertex.value(2), idToVertex.value(1),lessThanById(vertexToId)),
            vertexToId);
    // no sorting this time
    QVERIFY(subgraphTwoSorted == controlSubgraphTwoSorted);

    QList<qlonglong> rootsOfEighteen = mapList(graph.data().rootsOf(idToVertex.value(18)), vertexToId);
    std::sort(rootsOfEighteen.begin(), rootsOfEighteen.end());
    QVERIFY(rootsOfEighteen == controlRootsOfEighteen);
    QList<qlonglong> leavesFromTwo   = mapList(graph.data().leavesFrom(idToVertex.value(2)), vertexToId);
    std::sort(leavesFromTwo.begin(), leavesFromTwo.end());
    QVERIFY(leavesFromTwo == controlLeavesFromTwo);
}

void DImgHistoryGraphTest::slotImageLoaded(const QString& fileName, bool success)
{
    QVERIFY(success);
    qDebug() << "Loaded" << fileName;
    m_loop.quit();
}

void DImgHistoryGraphTest::slotImageSaved(const QString& fileName, bool success)
{
    QVERIFY(success);
    m_im->setLastSaved(fileName);
    qDebug() << "Saved to" << fileName;
    m_loop.quit();
}
