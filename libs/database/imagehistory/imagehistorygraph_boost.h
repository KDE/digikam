/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-10-22
 * Description : Boost Graph Library: a graph class
 *
 * Copyright (C) 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef IMAGEHISTORYGRAPH_BOOST_H
#define IMAGEHISTORYGRAPH_BOOST_H

// To include pragma directives for MSVC
#include "digikam_config.h"

// Pragma directives to reduce warnings from Boost header files.
#if !defined(__APPLE__) && defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-local-typedefs"
#endif

#if defined(__APPLE__) && defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wundef"
#pragma clang diagnostic ignored "-Wunused-parameter"
#pragma clang diagnostic ignored "-Wcast-align"
#pragma clang diagnostic ignored "-Wunused-local-typedef"
#endif

// Boost includes

// prohibit boost using deprecated header files
#define BOOST_NO_HASH

#include <utility>
#include <algorithm>
#include <boost/graph/transitive_closure.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/topological_sort.hpp>
#include <boost/graph/graph_utility.hpp>
#include <boost/graph/dag_shortest_paths.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/graph/reverse_graph.hpp>
#include <boost/graph/dominator_tree.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <boost/graph/transitive_reduction.hpp>

// Local includes

#include "digikam_debug.h"

/** Install custom property ids, out-of-namespace */
enum vertex_properties_t { vertex_properties };
enum edge_properties_t   { edge_properties   };

namespace boost
{
BOOST_INSTALL_PROPERTY(vertex, properties);
BOOST_INSTALL_PROPERTY(edge,   properties);
}

namespace Digikam
{

/**
 * Adds the necessary typedefs so that associative_property_map
 * accepts a QMap, and it can be used as a Boost Property Map
 */
template <typename Key, typename Value>
class QMapForAdaptors : public QMap<Key, Value>
{
public:

    typedef Key key_type;
    typedef Value data_type;
    typedef typename std::pair<const Key, Value> value_type;

    QMapForAdaptors() {}
};

enum MeaningOfDirection
{
    /**
        * Each edge is directed: "vertex1 -> vertex2".
        * This direction has a meaning with methods such as
        * roots() or leaves().
        */

    /// Edges are directed from a parent to its child
    ParentToChild,
    /// Edges are direct from a child to its parent
    ChildToParent
};


/* the graph base class template */
template <class VertexProperties, class EdgeProperties>
class Graph
{
public:

    typedef boost::adjacency_list<
    boost::vecS, /// Standard storage. listS was desirable, but many algorithms work only with vecS
          boost::vecS,
          boost::bidirectionalS, /// directed graph
          boost::property<boost::vertex_index_t, int,
          boost::property<vertex_properties_t, VertexProperties> >,
          boost::property<edge_properties_t, EdgeProperties> /// One property for each edge: EdgeProperties
          > GraphContainer;


    /** a bunch of graph-specific typedefs that make the long boost types manageable */
    typedef typename boost::graph_traits<GraphContainer> graph_traits;

    typedef typename graph_traits::vertex_descriptor vertex_t;
    typedef typename graph_traits::edge_descriptor edge_t;

    typedef typename graph_traits::vertex_iterator vertex_iter;
    typedef typename graph_traits::edge_iterator edge_iter;
    typedef typename graph_traits::adjacency_iterator adjacency_iter;
    typedef typename graph_traits::out_edge_iterator out_edge_iter;
    typedef typename graph_traits::in_edge_iterator in_edge_iter;
    typedef typename boost::inv_adjacency_iterator_generator<GraphContainer, vertex_t, in_edge_iter>::type inv_adjacency_iter;

    typedef typename graph_traits::degree_size_type degree_t;

    typedef std::pair<adjacency_iter, adjacency_iter> adjacency_vertex_range_t;
    typedef std::pair<inv_adjacency_iter, inv_adjacency_iter> inv_adjacency_vertex_range_t;
    typedef std::pair<out_edge_iter, out_edge_iter> out_edge_range_t;
    typedef std::pair<vertex_iter, vertex_iter> vertex_range_t;
    typedef std::pair<edge_iter, edge_iter> edge_range_t;

    typedef typename boost::property_map<GraphContainer, boost::vertex_index_t>::type vertex_index_map_t;
    typedef typename boost::property_map<GraphContainer, boost::vertex_index_t>::const_type const_vertex_index_map_t;
    typedef typename boost::property_map<GraphContainer, vertex_properties_t>::type vertex_property_map_t;
    typedef typename boost::property_map<GraphContainer, vertex_properties_t>::const_type const_vertex_property_map_t;
    typedef typename boost::property_map<GraphContainer, edge_properties_t>::type edge_property_map_t;
    typedef typename boost::property_map<GraphContainer, edge_properties_t>::const_type const_edge_property_map_t;

    /**
     * These two classes provide source-compatible wrappers for the vertex and edge descriptors,
     * providing default construction to null and the isNull() method.
     */
    class Vertex
    {
    public:

        Vertex() : v(graph_traits::null_vertex()) {}
        Vertex(const vertex_t& v) : v(v) {}

        Vertex& operator=(const vertex_t& other)
        {
            v = other;
            return *this;
        }

        operator const vertex_t&() const
        {
            return v;
        }
        operator vertex_t&()
        {
            return v;
        }

        bool operator==(const vertex_t& other) const
        {
            return v == other;
        }

        bool isNull() const
        {
            return v == graph_traits::null_vertex();
        }

    protected:

        vertex_t v;
    };

    class Edge
    {
    public:

        Edge() : null(true) {}
        Edge(const edge_t& e) : e(e), null(false) {}

        Edge& operator=(const edge_t& other)
        {
            e    = other;
            null = false;
            return *this;
        }

        operator const edge_t&() const
        {
            return e;
        }
        operator edge_t&()
        {
            return e;
        }

        const edge_t& toEdge() const
        {
            return e;
        }
        edge_t& toEdge()
        {
            return e;
        }

        bool operator==(const edge_t& other) const
        {
            return e == other;
        }

        bool isNull() const
        {
            return null;
        }

    protected:

        edge_t e;
        // there is not null_edge, we must emulate it
        bool   null;
    };

    typedef QPair<Vertex, Vertex>                            VertexPair;
    typedef QPair<Edge, Edge>                                EdgePair;

    typedef QMapForAdaptors<Vertex, Vertex>                  VertexVertexMap;
    typedef QMapForAdaptors<Vertex, int>                     VertexIntMap;

    typedef boost::associative_property_map<VertexVertexMap> VertexVertexMapAdaptor;
    typedef boost::associative_property_map<VertexIntMap>    VertexIntMapAdaptor;

    explicit Graph(MeaningOfDirection direction = ParentToChild)
        : direction(direction)
    {
    }

    Graph(const Graph& g)
        : graph(g.graph), direction(g.direction)
    {
    }

    virtual ~Graph()
    {
    }

    Graph& operator=(const Graph& other)
    {
        graph     = other.graph;
        direction = other.direction;
        return *this;
    }

    MeaningOfDirection meaningOfDirection() const
    {
        return direction;
    }

    void clear()
    {
        graph.clear();
    }

    Vertex addVertex()
    {
        Vertex v = boost::add_vertex(graph);
        return v;
    }

    Vertex addVertex(const VertexProperties& properties)
    {
        Vertex v = addVertex();
        setProperties(v, properties);
        return v;
    }

    void remove(const Vertex& v)
    {
        if (v.isNull())
        {
            return;
        }

        boost::clear_vertex(v, graph);
        boost::remove_vertex(v, graph);
    }

    Edge addEdge(const Vertex& v1, const Vertex& v2)
    {
        Edge e = edge(v1, v2);

        if (e.isNull())
        {
            e = boost::add_edge(v1, v2, graph).first;
        }

        return e;
    }

    Edge edge(const Vertex& v1, const Vertex& v2) const
    {
        std::pair<edge_t, bool> pair = boost::edge(v1, v2, graph);

        if (pair.second)
        {
            return pair.first;
        }

        return Edge();
    }

    bool hasEdge(const Vertex& v1, const Vertex& v2) const
    {
        return boost::edge(v1, v2, graph).second;
    }

    /// Does not care for direction
    bool isConnected(const Vertex& v1, const Vertex& v2) const
    {
        if (boost::edge(v1, v2, graph).second)
        {
            return true;
        }

        if (boost::edge(v2, v1, graph).second)
        {
            return true;
        }

        return false;
    }

    void setProperties(const Vertex& v, const VertexProperties& props)
    {
        boost::put(vertex_properties, graph, v, props);
    }

    const VertexProperties& properties(const Vertex& v) const
    {
        return boost::get(vertex_properties, graph, v);
    }

    VertexProperties& properties(const Vertex& v)
    {
        return boost::get(vertex_properties, graph, v);
    }

    void setProperties(const Edge& e, const EdgeProperties& props)
    {
        boost::put(edge_properties, graph, e, props);
    }

    template <class T>
    Vertex findVertexByProperties(const T& value) const
    {
        vertex_range_t range = boost::vertices(graph);

        for (vertex_iter it = range.first; it != range.second; ++it)
        {
            const VertexProperties& props = properties(*it);

            // must implement operator==(const T&)
            if (props == value)
            {
                return *it;
            }
        }

        return Vertex();
    }

    EdgeProperties properties(const Vertex& v1, const Vertex& v2) const
    {
        Edge e = edge(v1, v2);

        if (e.isNull())
        {
            return EdgeProperties();
        }

        return properties(e);
    }

    const EdgeProperties& properties(const Edge& e) const
    {
        return boost::get(edge_properties, graph, e);
    }

    EdgeProperties& properties(const Edge& e)
    {
        return boost::get(edge_properties, graph, e);
    }

    /** Accessing vertices and egdes */
    const GraphContainer& getGraph() const
    {
        return graph;
    }

    QList<Vertex> vertices() const
    {
        return toVertexList(boost::vertices(graph));
    }

    enum AdjacencyFlags
    {
        OutboundEdges = 1 << 0,
        InboundEdges  = 1 << 1,
        /// These resolve to one of the flags above, depending on MeaningOfDirection
        EdgesToLeaf   = 1 << 2,
        EdgesToRoot   = 1 << 3,
        AllEdges      = InboundEdges | OutboundEdges
    };

    QList<Vertex> adjacentVertices(const Vertex& v, AdjacencyFlags flags = AllEdges) const
    {
        if (flags & EdgesToLeaf)
        {
            flags = (AdjacencyFlags)(flags | (direction == ParentToChild ? OutboundEdges : InboundEdges));
        }

        if (flags & EdgesToRoot)
        {
            flags = (AdjacencyFlags)(flags | (direction == ParentToChild ? InboundEdges : OutboundEdges));
        }

        QList<Vertex> vertices;

        if (flags & OutboundEdges)
        {
            vertices << toVertexList(boost::adjacent_vertices(v, graph));
        }

        if (flags & InboundEdges)
        {
            vertices << toVertexList(boost::inv_adjacent_vertices(v, graph));
        }

        return vertices;
    }

    // for "hasAdjacentVertices", simply use hasEdges(v, flags)

    int vertexCount() const
    {
        return boost::num_vertices(graph);
    }

    bool isEmpty() const
    {
        return isEmptyRange(boost::vertices(graph));
    }

    int outDegree(const Vertex& v) const
    {
        return boost::out_degree(v, graph);
    }

    int inDegree(const Vertex& v) const
    {
        return boost::in_degree(v, graph);
    }

    bool isRoot(const Vertex& v) const
    {
        return !hasEdges(v, EdgesToRoot);
    }

    bool isLeaf(const Vertex& v) const
    {
        return !hasEdges(v, EdgesToLeaf);
    }

    Vertex source(const Edge& e) const
    {
        return boost::source(e.toEdge(), graph);
    }

    Vertex target(const Edge& e) const
    {
        return boost::target(e.toEdge(), graph);
    }

    QList<Edge> edges(const Vertex& v, AdjacencyFlags flags = AllEdges) const
    {
        if (flags & EdgesToLeaf)
        {
            flags = (AdjacencyFlags)(flags | (direction == ParentToChild ? OutboundEdges : InboundEdges));
        }

        if (flags & EdgesToRoot)
        {
            flags = (AdjacencyFlags)(flags | (direction == ParentToChild ? InboundEdges : OutboundEdges));
        }

        QList<Edge> es;

        if (flags & OutboundEdges)
        {
            es << toEdgeList(boost::out_edges(v, graph));
        }

        if (flags & InboundEdges)
        {
            es << toEdgeList(boost::in_edges(v, graph));
        }

        return es;
    }

    int edgeCount() const
    {
        return boost::num_edges(graph);
    }

    bool hasEdges(const Vertex& v, AdjacencyFlags flags = AllEdges) const
    {
        if (flags & EdgesToLeaf)
        {
            flags = (AdjacencyFlags)(flags | (direction == ParentToChild ? OutboundEdges : InboundEdges));
        }

        if (flags & EdgesToRoot)
        {
            flags = (AdjacencyFlags)(flags | (direction == ParentToChild ? InboundEdges : OutboundEdges));
        }

        if (flags & OutboundEdges)
        {
            if (!isEmptyRange(boost::out_edges(v, graph)))
            {
                return true;
            }
        }

        if (flags & InboundEdges)
        {
            if (!isEmptyRange(boost::in_edges(v, graph)))
            {
                return true;
            }
        }

        return false;
    }

    bool hasEdges() const
    {
        return !isEmptyRange(boost::edges(graph));
    }

    QList<Edge> edges() const
    {
        return toEdgeList(boost::edges(graph));
    }

    QList<VertexPair> edgePairs() const
    {
        QList<VertexPair> pairs;
        edge_range_t range = boost::edges(graph);

        for (edge_iter it = range.first; it != range.second; ++it)
        {
            pairs << VertexPair(boost::source(*it, graph), boost::target(*it, graph));
        }

        return pairs;
    }

    /* ---- Algorithms ---- */

    /**
     * Returns the vertex ids of this graph, in topological order.
     */
    QList<Vertex> topologicalSort() const
    {
        std::list<Vertex> vertices;

        try
        {
            boost::topological_sort(graph, std::back_inserter(vertices));
        }
        catch (boost::bad_graph& e)
        {
            qCDebug(DIGIKAM_DATABASE_LOG) << e.what();
            return QList<Vertex>();
        }

        typedef typename std::list<Vertex>::iterator vertex_list_iter;
        return toVertexList(std::pair<vertex_list_iter, vertex_list_iter>(vertices.begin(), vertices.end()));
    }

    enum GraphCopyFlags
    {
        CopyVertexProperties = 1 << 0,
        CopyEdgeProperties   = 1 << 1,
        CopyAllProperties    = CopyVertexProperties | CopyEdgeProperties
    };

    /**
     * Returns a copy of this graph with all edges added to form the transitive closure
     */
    Graph transitiveClosure(GraphCopyFlags flags = CopyAllProperties) const
    {
        // make_iterator_property_map:
        // 1. The second parameter, our verteX_index map, converts the key (Vertex) into an index
        // 2. The index is used to store the value (Vertex) in the first argument, which is our vector

        std::vector<vertex_t> copiedVertices(vertexCount(), Vertex());
        Graph closure;

        try
        {
            boost::transitive_closure(graph, closure.graph,
                                      orig_to_copy(make_iterator_property_map(copiedVertices.begin(), get(boost::vertex_index, graph)))
                                     );
        }
        catch (boost::bad_graph& e)
        {
            qCDebug(DIGIKAM_DATABASE_LOG) << e.what();
            return Graph();
        }

        copyProperties(closure, flags, copiedVertices);

        return closure;
    }

    /**
     * Returns a copy of this graph, with edges removed so that the transitive reduction is formed.
     * Optionally, a list of edges of this graph that have been removed in the returned graph is given.
     */
    Graph transitiveReduction(QList<Edge>* removedEdges = 0, GraphCopyFlags flags = CopyAllProperties) const
    {
        std::vector<vertex_t> copiedVertices(vertexCount(), Vertex());
        Graph reduction;

        // named parameters is not implemented
        try
        {
            boost::transitive_reduction(graph, reduction.graph,
                                        make_iterator_property_map(copiedVertices.begin(), get(boost::vertex_index, graph)),
                                        get(boost::vertex_index, graph));
        }
        catch (boost::bad_graph& e)
        {
            qCDebug(DIGIKAM_DATABASE_LOG) << e.what();
            return Graph();
        }

        copyProperties(reduction, flags, copiedVertices);

        if (removedEdges)
        {
            *removedEdges = edgeDifference(reduction, copiedVertices);
        }

        return reduction;
    }

    /**
     * Returns all roots, i.e. vertices with no parents.
     * Takes the graph direction into account.
     */
    QList<Vertex> roots() const
    {
        return findZeroDegree(direction == ParentToChild ? true : false);
    }

    /**
     * Returns all roots of vertex v. Subset of roots().
     * I case any leaves have roots that are not roots of v,
     * they will not be contained in this list.
     */
    QList<Vertex> rootsOf(const Vertex& v) const
    {
        return findZeroDegreeFrom(v, direction == ParentToChild ? true : false);
    }

    /**
     * Returns all leaves, i.e. vertices with no children
     * Takes the graph direction into account.
     */
    QList<Vertex> leaves() const
    {
        return findZeroDegree(direction == ParentToChild ? false : true);
    }

    QList<Vertex> leavesFrom(const Vertex& v) const
    {
        return findZeroDegreeFrom(v, direction == ParentToChild ? false : true);
    }

    template <typename T> static bool alwaysFalse(const T&, const T&) { return false; }
    /**
     * Returns the longest path through the graph, starting from a vertex in roots(),
     * ending on a vertex in leaves(), and passing vertex v.
     * The returned list is given in that order, root - v - leave.
     * If there is more than one candidate for root or leave, lessThan is used to determine the first candidate.
     */
    QList<Vertex> longestPathTouching(const Vertex& v) const
    {
        return longestPathTouching(v, alwaysFalse<Vertex>);
    }

    template <typename LessThan>
    QList<Vertex> longestPathTouching(const Vertex& v, LessThan lessThan) const
    {
        if (v.isNull())
        {
            return QList<Vertex>();
        }

        QList<Vertex> fromRoot;
        QList<Vertex> toLeave;
        Path          path;

        path.longestPath(boost::make_reverse_graph(graph), v);

        QList<Vertex> rootCandidates = mostRemoteNodes(path.distances);

        if (!rootCandidates.isEmpty())
        {
            std::stable_sort(rootCandidates.begin(), rootCandidates.end(), lessThan);
            Vertex root = rootCandidates.first();
            fromRoot << listPath(root, v, path.predecessors, ChildToParent);
        }

        path.longestPath(graph, v);

        QList<Vertex> leaveCandidates = mostRemoteNodes(path.distances);

        if (!leaveCandidates.isEmpty())
        {
            std::stable_sort(leaveCandidates.begin(), leaveCandidates.end(), lessThan);
            Vertex leave = leaveCandidates.first();
            toLeave << listPath(leave, v, path.predecessors);
        }

        if (direction == ParentToChild)
        {
            return fromRoot << v << toLeave;
        }
        else
        {
            return toLeave  << v << fromRoot;
        }
    }

    /**
     * Returns the shortestPath between id1 and id2.
     * If s2 is not reachable from s1, the path is searched from s2 to s1.
     * The returned list always starts with s1, contains the intermediate vertices, and ends with s2.
     * If no path is available, an empty list is returned.
     */
    QList<Vertex> shortestPath(const Vertex& v1, const Vertex& v2) const
    {
        if (v1.isNull() || v2.isNull())
        {
            return QList<Vertex>();
        }

        QList<Vertex> vertices;

        Path path;
        path.shortestPath(graph, v1);

        if (path.isReachable(v2))
        {
            vertices = listPath(v2, v1, path.predecessors, ChildToParent);
            vertices.prepend(v1);
        }
        else
        {
            // assume inverted parameters
            path.shortestPath(graph, v2);

            if (path.isReachable(v1))
            {
                vertices = listPath(v1, v2, path.predecessors);
                vertices.append(v2);
            }
        }

        return vertices;
    }

    /**
     * Returns the shortest distances from Vertex to all vertices in the graph.
     * If the value is -1, a vertex is not reachable from v.
     */
    QMap<Vertex, int> shortestDistancesFrom(const Vertex& v) const
    {
        QList<Vertex> vertices;
        Path          path;

        if (direction == ParentToChild)
        {
            path.shortestPath(graph, v);
        }
        else
        {
            path.shortestPath(boost::make_reverse_graph(graph), v);
        }

        // change 2147483647 to -1
        typename QMap<Vertex, int>::iterator it;

        for (it = path.distances.begin(); it != path.distances.end(); ++it)
        {
            if (it.value() == std::numeric_limits<int>::max())
                it.value() = -1;
        }

        return path.distances;
    }

    enum ReturnOrder
    {
        BreadthFirstOrder,
        DepthFirstOrder
    };

    /**
     * For a vertex v reachable from a vertex root,
     * returns, in depth-first or breadth-first order, all vertices dominated by v
     * starting from root.
     */
    QList<Vertex> verticesDominatedBy(const Vertex& v, const Vertex& root, ReturnOrder order = BreadthFirstOrder) const
    {
        if (v.isNull() || isEmpty())
        {
            return QList<Vertex>();
        }

        GraphSearch search;

        if (order == BreadthFirstOrder)
        {
            search.breadthFirstSearch(graph, root, direction == ChildToParent);
        }
        else
        {
            search.depthFirstSearch(graph, root, direction == ChildToParent);
        }

        return verticesDominatedBy(v, root, search.vertices);
    }

    /**
     * For a vertex v reachable from a vertex root all vertices dominated by v starting from root.
     * The returned list is in depth-first order, using root as starting point, and
     * when discovering a vertex, sorting the adjacent vertices with the given lessThan.
     */
    template <typename LessThan>
    QList<Vertex> verticesDominatedByDepthFirstSorted(const Vertex& v, const Vertex& root, LessThan lessThan) const
    {
        return verticesDominatedBy(v, root, verticesDepthFirstSorted(root, lessThan));
    }

    /**
     * For a vertex v reachable from a vertex root returns
     * all vertices dominated by v starting from root.
     * The order is the same as in the given, sorted list of all vertices in this graph
     * (or all vertices expected to be returned. The returned list is the intersection
     *  of the dominated vertices and presortedVertices, in order of presortedVertices)
     */
    QList<Vertex> verticesDominatedBy(const Vertex& v, const Vertex& root, const QList<Vertex> presortedVertices) const
    {
        if (v.isNull() || isEmpty())
        {
            return QList<Vertex>();
        }

        DominatorTree tree;
        tree.enter(graph, root, direction);

        QList<Vertex> dominatedTree = treeFromPredecessors(v, tree.predecessors);

        // remove all vertices from the DFS of v that are not in the dominated tree
        QList<Vertex> orderedTree;

        foreach(const Vertex& v, presortedVertices)
        {
            if (dominatedTree.contains(v))
            {
                orderedTree << v;
            }
        }

        return orderedTree;
    }

    /**
     * Orders all vertices of the graph in a breadth-first manner.
     * A single vertex is taken as reference to distinguish main root and side paths.
     * Otherwise the first root is taken as reference.
     */
    QList<Vertex> verticesBreadthFirst(const Vertex& givenRef = Vertex()) const
    {
        if (isEmpty())
        {
            return QList<Vertex>();
        }

        Vertex ref(givenRef);

        if (ref.isNull())
            ref = roots().first();

        QList<Vertex> vertices;

        vertices << rootsOf(ref);

        if (vertices.size() == vertexCount())
            return vertices;

        GraphSearch search;
        search.breadthFirstSearch(graph, vertices.first(), direction == ChildToParent);
        QList<Vertex> bfs = search.vertices;

        foreach(const Vertex& v, vertices)
        {
            bfs.removeOne(v);
        }
        vertices << bfs;

        if (vertices.size() == vertexCount())
            return vertices;

        // sort in any so far unreachable nodes
        vertex_range_t range = boost::vertices(graph);

        for (vertex_iter it = range.first; it != range.second; ++it)
        {
            if (!vertices.contains(*it))
            {
                GraphSearch childSearch;
                childSearch.breadthFirstSearch(graph, *it, direction == ChildToParent);
                QList<Vertex> childBfs = childSearch.vertices;
                QList<Vertex> toInsert;

                // any item reachable from *it should come after it
                int minIndex = vertices.size();

                foreach(const Vertex& c, childBfs)
                {
                    int foundAt = vertices.indexOf(c);

                    if (foundAt == -1)
                    {
                        toInsert << c;
                    }
                    else
                    {
                        minIndex = qMin(foundAt, minIndex);
                    }
                }

                foreach(const Vertex& c, toInsert)
                {
                    vertices.insert(minIndex++, c);
                }
            }
        }

        return vertices;
    }

    /**
     * Orders all vertices of the graph in a depth-first manner.
     * When discovering a vertex, the adjacent vertices are sorted with the given lessThan.
     * A single vertex is taken as starting point.
     * If null, the first root is taken as reference.
     */
    template <typename LessThan>
    QList<Vertex> verticesDepthFirstSorted(const Vertex& givenRef, LessThan lessThan) const
    {
        if (isEmpty())
        {
            return QList<Vertex>();
        }

        Vertex ref(givenRef);

        if (ref.isNull())
            ref = roots().first();

        QList<Vertex> vertices;

        vertices = rootsOf(ref);

        if (vertices.size() == vertexCount() || vertices.isEmpty())
            return vertices;

        GraphSearch search;
        search.depthFirstSearchSorted(graph, vertices.first(), direction == ChildToParent, lessThan);
        QList<Vertex> dfs = search.vertices;

        foreach(const Vertex& v, vertices)
        {
            dfs.removeOne(v);
        }
        vertices << dfs;

        return search.vertices;
    }

protected:

    QList<Vertex> treeFromPredecessors(const Vertex& v, const VertexVertexMap& predecessors) const
    {
        QList<Vertex> vertices;
        vertices << v;
        treeFromPredecessorsRecursive(v, vertices, predecessors);
        return vertices;
    }

    void treeFromPredecessorsRecursive(const Vertex& v, QList<Vertex>& vertices, const VertexVertexMap& predecessors) const
    {
        QList<Vertex> children = predecessors.keys(v);
        vertices << children;

        foreach(const Vertex& child, children)
        {
            treeFromPredecessorsRecursive(child, vertices, predecessors);
        }
    }

    /**
     * Returns a list of vertex ids of vertices in the given range
     */
    template <typename Value, typename range_t>
    static QList<Value> toList(const range_t& range)
    {
        typedef typename range_t::first_type iterator_t;
        QList<Value> list;

        for (iterator_t it = range.first; it != range.second; ++it)
        {
            list << *it;
        }

        return list;
    }

    template <typename range_t> static QList<Vertex> toVertexList(const range_t& range)
    {
        return toList<Vertex, range_t>(range);
    }

    template <typename range_t> static QList<Edge> toEdgeList(const range_t& range)
    {
        return toList<Edge, range_t>(range);
    }

    template <typename range_t>
    static bool isEmptyRange(const range_t& range)
    {
        return range.first == range.second;
    }

    /**
     * According to the given flags and based on the map,
     * copies vertex and edge properties from this to the other graph
     */
    void copyProperties(Graph& other, GraphCopyFlags flags, const std::vector<vertex_t>& copiedVertices) const
    {
        other.direction = direction;

        if (flags & CopyVertexProperties)
        {
            vertex_index_map_t indexMap = boost::get(boost::vertex_index, graph);
            vertex_range_t range = boost::vertices(graph);

            for (vertex_iter it = range.first; it != range.second; ++it)
            {
                Vertex copiedVertex = copiedVertices[boost::get(indexMap, *it)];

                if (copiedVertex.isNull())
                {
                    continue;
                }

                other.setProperties(copiedVertex, properties(*it));
            }
        }

        if (flags & CopyEdgeProperties)
        {
            vertex_index_map_t indexMap = boost::get(boost::vertex_index, graph);
            edge_range_t range          = boost::edges(graph);

            for (edge_iter it = range.first; it != range.second; ++it)
            {
                Vertex s       = boost::source(*it, graph);
                Vertex t       = boost::target(*it, graph);
                Vertex copiedS = copiedVertices[boost::get(indexMap, s)];
                Vertex copiedT = copiedVertices[boost::get(indexMap, t)];

                if (copiedS.isNull() || copiedT.isNull())
                {
                    continue;
                }

                Edge copiedEdge = other.edge(copiedS, copiedT);

                if (!copiedEdge.isNull())
                {
                    other.setProperties(copiedEdge, properties(s, t));
                }
            }
        }
    }

    /**
     * Returns a list of edges of this graph that have been removed in other.
     * copiedVertices maps the vertices of this graph to other.
     */
    QList<Edge> edgeDifference(const Graph& other, const std::vector<vertex_t>& copiedVertices) const
    {
        QList<Edge> removed;
        vertex_index_map_t indexMap = boost::get(boost::vertex_index, graph);
        edge_range_t range          = boost::edges(graph);

        for (edge_iter it = range.first; it != range.second; ++it)
        {
            Vertex s       = boost::source(*it, graph);
            Vertex t       = boost::target(*it, graph);
            Vertex copiedS = copiedVertices[boost::get(indexMap, s)];
            Vertex copiedT = copiedVertices[boost::get(indexMap, t)];

            if (copiedS.isNull() || copiedT.isNull())
            {
                continue;
            }

            Edge copiedEdge = other.edge(copiedS, copiedT);

            if (copiedEdge.isNull())
            {
                removed << *it;
            }
        }

        return removed;
    }

    /**
     * Finds vertex ids of all vertices with zero in- our out-degree.
     */
    QList<Vertex> findZeroDegree(bool inOrOut) const
    {
        QList<Vertex> vertices;
        vertex_range_t range = boost::vertices(graph);

        for (vertex_iter it = range.first; it != range.second; ++it)
        {
            if ( (inOrOut ? in_degree(*it, graph) : out_degree(*it, graph)) == 0)
            {
                vertices << *it;
            }
        }

        return vertices;
    }

    QList<Vertex> findZeroDegreeFrom(const Vertex& v, bool inOrOut) const
    {
        bool invertGraph = (direction == ChildToParent);

        if (!inOrOut)
            invertGraph = !invertGraph;

        GraphSearch search;
        search.breadthFirstSearch(graph, v, invertGraph);

        QList<Vertex> vertices;

        foreach(const Vertex& candidate, search.vertices)
        {
            if ( (inOrOut ? in_degree(candidate, graph) : out_degree(candidate, graph)) == 0)
            {
                vertices << candidate;
            }
        }

        return vertices;
     }


    /**
     * Helper class to find paths through the graph.
     * Call one of the methods and then read the maps.
     */
    class Path
    {
    public:

        template <class GraphType>
        void shortestPath(const GraphType& graph, const Vertex& v)
        {
            int weight = 1;

            try
            {
                boost::dag_shortest_paths(graph, v,
                                          // we provide a constant weight of 1
                                          weight_map(boost::ref_property_map<typename boost::graph_traits<GraphType>::edge_descriptor,int>(weight)).
                                          // Store distance and predecessors in QMaps, wrapped to serve as property maps
                                          distance_map(VertexIntMapAdaptor(distances)).
                                          predecessor_map(VertexVertexMapAdaptor(predecessors))
                                         );
            }
            catch (boost::bad_graph& e)
            {
                qCDebug(DIGIKAM_DATABASE_LOG) << e.what();
            }
        }

        template <class GraphType>
        void longestPath(const GraphType& graph, const Vertex& v)
        {
            int weight = 1;

            try
            {
                boost::dag_shortest_paths(graph, v,
                                          // we provide a constant weight of 1
                                          weight_map(boost::ref_property_map<typename boost::graph_traits<GraphType>::edge_descriptor,int>(weight)).
                                          // Invert the default compare method: With greater, we get the longest path
                                          distance_compare(std::greater<int>()).
                                          // will be returned if a node is unreachable
                                          distance_inf(-1).
                                          // Store distance and predecessors in QMaps, wrapped to serve as property maps
                                          distance_map(VertexIntMapAdaptor(distances)).
                                          predecessor_map(VertexVertexMapAdaptor(predecessors))
                                         );
            }
            catch (boost::bad_graph& e)
            {
                qCDebug(DIGIKAM_DATABASE_LOG) << e.what();
            }
        }

        bool isReachable(const Vertex& v) const
        {
            return predecessors.value(v, v) != v;
        }

        VertexVertexMap predecessors;
        VertexIntMap    distances;
    };

    class DominatorTree
    {
    public:

        template <class GraphType>
        void enter(const GraphType& graph, const Vertex& v, MeaningOfDirection direction = ParentToChild)
        {
            try
            {
                if (direction == ParentToChild)
                {
                    boost::lengauer_tarjan_dominator_tree(graph, v, VertexVertexMapAdaptor(predecessors));
                }
                else
                    boost::lengauer_tarjan_dominator_tree(boost::make_reverse_graph(graph), v,
                                                          VertexVertexMapAdaptor(predecessors));
            }
            catch (boost::bad_graph& e)
            {
                qCDebug(DIGIKAM_DATABASE_LOG) << e.what();
            }
        }

        VertexVertexMap predecessors;
    };

    class GraphSearch
    {
    public:

        template <class GraphType>
        void depthFirstSearch(const GraphType& graph, const Vertex& v, bool invertGraph)
        {
            // remember that the visitor is passed by value
            DepthFirstSearchVisitor vis(this);

            try
            {
                if (invertGraph)
                {
                    boost::depth_first_search(boost::make_reverse_graph(graph), visitor(vis).root_vertex(v));
                }
                else
                {
                    boost::depth_first_search(graph, visitor(vis).root_vertex(v));
                }
            }
            catch (boost::bad_graph& e)
            {
                qCDebug(DIGIKAM_DATABASE_LOG) << e.what();
            }
        }

        template <class GraphType, typename LessThan>
        void depthFirstSearchSorted(const GraphType& graph, const Vertex& v, bool invertGraph, LessThan lessThan)
        {
            // remember that the visitor is passed by value
            DepthFirstSearchVisitor vis(this);
            std::vector<boost::default_color_type> color_vec(boost::num_vertices(graph), boost::white_color);

            try
            {
                if (invertGraph)
                {
                    depth_first_search_sorted(boost::make_reverse_graph(graph), v, vis,
                                make_iterator_property_map(color_vec.begin(), get(boost::vertex_index, graph)), lessThan);
                }
                else
                {
                    depth_first_search_sorted(graph, v, vis,
                                make_iterator_property_map(color_vec.begin(), get(boost::vertex_index, graph)), lessThan);
                }
            }
            catch (boost::bad_graph& e)
            {
                qCDebug(DIGIKAM_DATABASE_LOG) << e.what();
            }
        }

        template <class GraphType>
        void breadthFirstSearch(const GraphType& graph, const Vertex& v, bool invertGraph)
        {
            BreadthFirstSearchVisitor vis(this);

            try
            {
                if (invertGraph)
                {
                    boost::breadth_first_search(boost::make_reverse_graph(graph), v, visitor(vis));
                }
                else
                {
                    boost::breadth_first_search(graph, v, visitor(vis));
                }
            }
            catch (boost::bad_graph& e)
            {
                qCDebug(DIGIKAM_DATABASE_LOG) << e.what();
            }
        }

        class CommonVisitor
        {
        protected:

            explicit CommonVisitor(GraphSearch* q) : q(q) {}

            void record(const Vertex& v) const
            {
                q->vertices << v;
            }

            GraphSearch* const q;
        };

        class DepthFirstSearchVisitor : public boost::default_dfs_visitor, public CommonVisitor
        {
        public:

            explicit DepthFirstSearchVisitor(GraphSearch* q) : CommonVisitor(q) {}

            template <typename VertexType, typename GraphType>
            void discover_vertex(VertexType u, const GraphType&) const
            {
                this->record(u);
            }
        };

        class BreadthFirstSearchVisitor : public boost::default_bfs_visitor, public CommonVisitor
        {
        public:

            explicit BreadthFirstSearchVisitor(GraphSearch* q) : CommonVisitor(q) {}

            template <typename VertexType, typename GraphType>
            void discover_vertex(VertexType u, const GraphType&) const
            {
                this->record(u);
            }
        };

        QList<Vertex> vertices;

    protected:

        template <class GraphType, typename VertexLessThan>
        class lessThanMapEdgeToTarget
        {
            typedef typename boost::graph_traits<GraphType>::edge_descriptor edge_descriptor;

        public:

            lessThanMapEdgeToTarget(const GraphType& g, VertexLessThan vertexLessThan)
                : g(g), vertexLessThan(vertexLessThan) {}

            const GraphType& g;
            VertexLessThan vertexLessThan;

            bool operator()(const edge_descriptor& a, const edge_descriptor& b)
            {
                return vertexLessThan(boost::target(a, g), boost::target(b, g));
            }
        };

        // This is boost's simple, old, recursive DFS algorithm adapted with lessThan
        template <class IncidenceGraph, class DFSVisitor, class ColorMap, typename LessThan>
        void depth_first_search_sorted(const IncidenceGraph& g, Vertex u,
                                       DFSVisitor& vis, ColorMap color, LessThan lessThan)
        {
            //typedef std::pair<Vertex, QList<Edge> > VertexInfo;

            typedef typename boost::graph_traits<IncidenceGraph>::edge_descriptor edge_descriptor;
            QList<edge_descriptor> outEdges;
            //std::vector<VertexInfo> stack;

            boost::put(color, u, boost::gray_color);
            vis.discover_vertex(u, g);

            outEdges = toList<edge_descriptor>(boost::out_edges(u, g));
            // Sort edges. The lessThan we have takes vertices, so we use a lessThan which
            // maps the given edges to their targets, and calls our vertex lessThan.
            std::sort(outEdges.begin(), outEdges.end(), lessThanMapEdgeToTarget<IncidenceGraph, LessThan>(g, lessThan));

            foreach(const edge_descriptor& e, outEdges)
            {
                Vertex v = boost::target(e, g);
                vis.examine_edge(e, g);
                boost::default_color_type v_color = boost::get(color, v);
                if (v_color == boost::white_color)
                {
                    vis.tree_edge(e, g);
                    depth_first_search_sorted(g, v, vis, color, lessThan);
                }
                else if (v_color == boost::gray_color)
                {
                    vis.back_edge(e, g);
                }
                else
                {
                    vis.forward_or_cross_edge(e, g);
                }
            }
            put(color, u, boost::black_color);
            vis.finish_vertex(u, g);
        }
    };

    /** Get the list of vertices with the largest value in the given distance map */
    QList<Vertex> mostRemoteNodes(const VertexIntMap& distances) const
    {
        typename VertexIntMap::const_iterator it;
        int maxDist = 1;
        QList<Vertex> candidates;

        for (it = distances.begin(); it != distances.end(); ++it)
        {
            if (it.value() > maxDist)
            {
                maxDist = it.value();
                //qDebug() << "Increasing maxDist to" << maxDist;
                candidates.clear();
            }

            if (it.value() >= maxDist)
            {
                //qDebug() << "Adding candidate" << id(it.key()) <<  "at distance" << maxDist;
                candidates << it.key();
            }

            /*
            if (it.value() == -1)
                qDebug() << id(it.key()) << "unreachable";
            else
                qDebug() << "Distance to" << id(it.key()) << "is" << it.value();
            */
        }

        return candidates;
    }

    /**
     *  Get a list of vertex ids for the path from root to target, using the given predecessors.
     *  Depending on MeaningOfDirection, the ids are listed inverted, from target to root.
     */
    QList<Vertex> listPath(const Vertex& root, const Vertex& target,
                           const VertexVertexMap& predecessors, MeaningOfDirection dir = ParentToChild) const
    {
        QList<Vertex> vertices;

        for (Vertex v = root; v != target; v = predecessors.value(v))
        {
            //qDebug() << "Adding waypoint" << id(v);
            if (dir == ParentToChild)
            {
                vertices.append(v);
            }
            else
            {
                vertices.prepend(v);
            }

            // If a node is not reachable, it seems its entry in the predecessors map is itself
            // Avoid endless loop
            if (predecessors.value(v) == v)
            {
                break;
            }
        }

        return vertices;
    }

protected:

    GraphContainer     graph;
    MeaningOfDirection direction;
};

} // namespace Digikam

// Restore warnings
#if !defined(__APPLE__) && defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

#if defined(__APPLE__) && defined(__clang__)
#pragma clang diagnostic pop
#endif

#endif // IMAGEHISTORYGRAPH_BOOST_H
