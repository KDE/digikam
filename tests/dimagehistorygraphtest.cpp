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

#include "dimagehistorygraphtest.h"

// Qt includes

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTime>

// KDE includes

#include <qtest_kde.h>

#include <kio/netaccess.h>
#include <qtest_kde.h>

// Local includes

#include "albumdb.h"
#include "collectionlocation.h"
#include "collectionmanager.h"
#include "collectionscanner.h"
#include "dimginterface.h"
#include "imageinfo.h"
#include "imagehistorygraph.h"
#include "imagehistorygraphdata.h"
#include "imagehistorygraphmodel.h"

using namespace Digikam;

QTEST_KDEMAIN_CORE(DImageHistoryGraphTest)

void DImageHistoryGraphTest::initTestCase()
{
    initBaseTestCase();

    QString name = tempFileName("collection");
    collectionDir = QDir::temp();
    collectionDir.mkdir(name);
    collectionDir.cd(name);
    QVERIFY(collectionDir.exists());

    dbFile = tempFilePath("database");

    qDebug() << "Using database path for test: " << dbFile;

    DatabaseParameters params("QSQLITE", dbFile,
                              QString(), QString(), -1, false, QString(), QString());
    DatabaseAccess::setParameters(params, DatabaseAccess::MainApplication);
    QVERIFY(DatabaseAccess::checkReadyForUse(0));
    QVERIFY(QFile(dbFile).exists());
    CollectionManager::instance()->addLocation(collectionDir.path());
    CollectionManager::instance()->addLocation(imagePath());
    QList<CollectionLocation> locs = CollectionManager::instance()->allAvailableLocations();
    QVERIFY(locs.size() == 2);

    rescan();

    QList<AlbumShortInfo> albums = DatabaseAccess().db()->getAlbumShortInfos();
    QVERIFY(albums.size() >= 2);
    foreach (const AlbumShortInfo& album, albums)
    {
        //qDebug() << album.relativePath << album.id;
        //qDebug() << CollectionManager::instance()->albumRootPath(album.albumRootId);
        //qDebug() << DatabaseAccess().db()->getItemURLsInAlbum(album.id);
        readOnlyImages << DatabaseAccess().db()->getItemURLsInAlbum(album.id);
    }
}

void DImageHistoryGraphTest::cleanupTestCase()
{
    cleanupBaseTestCase();

    QFile(dbFile).remove();

    KUrl deleteUrl = KUrl::fromPath(collectionDir.path());
    KIO::NetAccess::del(deleteUrl, 0);
    qDebug() << "deleted test folder " << deleteUrl;
}

void DImageHistoryGraphTest::rescan()
{
    CollectionScanner().completeScan();
}

template <typename from, typename to>
QList<to> mapList(const QList<from>& l, const QMap<from,to> map)
{
    QList<to> r;
    foreach (const from& f, l)
        r << map.value(f);
    return r;
}

void DImageHistoryGraphTest::testGraph()
{
    QList<qlonglong> ids;
    foreach (const QString& file, readOnlyImages)
        ids << ImageInfo(file).id();
    QVERIFY(!ids.contains(-1));
    QVERIFY(ids.size() >= 24);


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
    qSort(controlLeaves);

    QList<qlonglong> controlRoots;
    controlRoots << 1;

    QList<qlonglong> controlLongestPathEighteen;
    controlLongestPathEighteen << 1 << 7 << 18;
    QList<qlonglong> controlLongestPathTwentyFour;
    controlLongestPathTwentyFour << 1 << 4 << 12 << 23 << 24;

    QList<qlonglong> controlSubgraphTwo;
    controlSubgraphTwo << 2 << 8 << 9 << 10 << 19 << 20 << 21;

    QList<qlonglong> controlSubgraphFour;
    controlSubgraphFour << 4 << 11 << 12 << 13 << 22 << 23 << 24;

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

    graph.finish();

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
    foreach (HistoryGraph::Vertex v, graph.data().vertices())
    {
        HistoryVertexProperties props = graph.data().properties(v);
        idToVertex[props.infos.first().id()] = v;
        vertexToId[v] = props.infos.first().id();
    }

    QList<qlonglong> leaves = mapList(graph.data().leaves(), vertexToId);
    qSort(leaves);
    QVERIFY(leaves == controlLeaves);

    QList<qlonglong> roots = mapList(graph.data().roots(), vertexToId);
    qSort(roots);
    QVERIFY(roots == controlRoots);

    QList<qlonglong> longestPath1 = mapList(graph.data().longestPathTouching(idToVertex.value(18)), vertexToId);
    QVERIFY(longestPath1 == controlLongestPathEighteen);
    QList<qlonglong> longestPath2 = mapList(graph.data().longestPathTouching(idToVertex.value(24)), vertexToId);
    QVERIFY(longestPath2 == controlLongestPathTwentyFour);

    // depth-first
    QList<qlonglong> subgraphTwo = mapList(graph.data().verticesDominatedBy(idToVertex.value(2), idToVertex.value(1),
                                                                            HistoryGraph::DepthFirstOrder), vertexToId);
    qSort(subgraphTwo);
    QVERIFY(subgraphTwo == controlSubgraphTwo);

    // breadth-first
    QList<qlonglong> subgraphFour = mapList(graph.data().verticesDominatedBy(idToVertex.value(4), idToVertex.value(1)), vertexToId);
    QVERIFY(subgraphFour.indexOf(22) > subgraphFour.indexOf(13));
    qSort(subgraphFour);
    QVERIFY(subgraphFour == controlSubgraphFour);

    ImageHistoryGraphModel model;
    model.setHistory(ImageInfo(16), graph);
}

void DImageHistoryGraphTest::slotImageLoaded(const QString&, bool success)
{
}

void DImageHistoryGraphTest::slotImageSaved(const QString& fileName, bool success)
{
}


