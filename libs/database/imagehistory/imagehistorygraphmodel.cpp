/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-10-27
 * Description : Model to an ImageHistoryGraph
 *
 * Copyright (C) 2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "imagehistorygraphmodel.h"

// Qt includes

#include <QAbstractItemModel>
#include <QTreeWidgetItem>

// KDE includes

#include <klocale.h>

// Local includes

#include "imagehistorygraphdata.h"
#include "imagemodel.h"

namespace Digikam
{

class HistoryTreeItem
{
public:

    enum HistoryTreeItemType
    {
        UnspecifiedType,
        VertexItemType,
        FilterActionItemType,
        BranchesItemType,
        SourcesItemType
    };

    HistoryTreeItem();
    virtual ~HistoryTreeItem();

    virtual HistoryTreeItemType type() const { return UnspecifiedType; }
    void addChild(HistoryTreeItem *child);

    int childCount() const { return children.size(); }
    HistoryTreeItem *child(int index) const { return children[index]; }

public:

    HistoryTreeItem*        parent;
    QList<HistoryTreeItem*> children;
};

// ------------------------------------------------------------------------

class VertexItem : public HistoryTreeItem
{
public:

    VertexItem() {}
    VertexItem(const HistoryGraph::Vertex& v) : vertex(v) {}
    HistoryTreeItemType type() const { return VertexItemType; }

    HistoryGraph::Vertex vertex;
    QModelIndex          index;
};

// ------------------------------------------------------------------------

class FilterActionItem : public HistoryTreeItem
{
public:

    FilterActionItem() {}
    FilterActionItem(const FilterAction& action) : action(action) {}
    HistoryTreeItemType type() const { return FilterActionItemType; }
    FilterAction action;
};

// ------------------------------------------------------------------------

class BranchesItem : public HistoryTreeItem
{
public:

    HistoryTreeItemType type() const { return BranchesItemType; }
};

// ------------------------------------------------------------------------

class SourcesItem : public HistoryTreeItem
{
public:

    HistoryTreeItemType type() const { return SourcesItemType; }
};

#define HANDLE_ITEM(class, name, pointer) \
    if (pointer && static_cast<HistoryTreeItem*>(pointer)->type() == HistoryTreeItem:: class##Type) \
        for (class *name = static_cast<class*>(pointer); name; name=0)

// ------------------------------------------------------------------------

HistoryTreeItem::HistoryTreeItem()
               : parent(0)
{
}

HistoryTreeItem::~HistoryTreeItem()
{
    qDeleteAll(children);
}

void HistoryTreeItem::addChild(HistoryTreeItem *child)
{
    children << child;
    child->parent = this;
}

// ------------------------------------------------------------------------

class ImageHistoryGraphModel::ImageHistoryGraphModelPriv
{
public:

    ImageHistoryGraphModelPriv()
    {
        rootItem = 0;
    }

    ImageHistoryGraph historyGraph;
    ImageInfo         info;

    /*
    int                         indexLimit;
    QList<HistoryGraph::Edge>   edges;
    QList<HistoryGraph::Vertex> path;
    */
    HistoryTreeItem*  rootItem;
    ImageModel        imageModel;

    inline const HistoryGraph& graph() const
        { return historyGraph.data(); }

    inline HistoryTreeItem* item(const QModelIndex& index) const
        { return index.isValid() ? static_cast<HistoryTreeItem*>(index.internalPointer()) : rootItem; }

    void buildPath();
    void buildVertexInfo(VertexItem* item, VertexItem* previousItem);
    void addBranches(VertexItem* previousItem, const QList<HistoryGraph::Vertex>& branches);
    void addSources(VertexItem* item, const QList<HistoryGraph::Vertex>& sources);

    VertexItem* createVertexItem(const HistoryGraph::Vertex& v);
    FilterActionItem* createFilterActionItem(const FilterAction& action);
};

VertexItem* ImageHistoryGraphModel::ImageHistoryGraphModelPriv::createVertexItem(const HistoryGraph::Vertex& v)
{
    QModelIndex index = imageModel.indexForImageInfo(graph().properties(v).firstImageInfo());
    VertexItem* item  = new VertexItem(v);
    item->index       = index;
    //kDebug() << "Adding vertex item" << graph().properties(v).firstImageInfo().id() << index;
    return item;
}

FilterActionItem* ImageHistoryGraphModel::ImageHistoryGraphModelPriv::createFilterActionItem(const FilterAction& action)
{
    //kDebug() << "Adding vertex item for" << action.displayableName();
    return new FilterActionItem(action);
}

void ImageHistoryGraphModel::ImageHistoryGraphModelPriv::buildPath()
{
    delete rootItem;
    rootItem = new HistoryTreeItem;

    HistoryGraph::Vertex v = graph().findVertexByProperties(info);
    QList<HistoryGraph::Vertex> path = graph().longestPathTouching(v);

    VertexItem *item, *previousItem = 0;
    for (int i=0; i<path.size(); i++)
    {
        // create new item
        item = createVertexItem(path[i]);
        // add children between last item and new item (if it's not the first item)
        if (previousItem)
            buildVertexInfo(item, previousItem);
        // now, add item
        rootItem->addChild(item);
        previousItem = item;
    }
}

void ImageHistoryGraphModel::ImageHistoryGraphModelPriv::buildVertexInfo(VertexItem* item, VertexItem* previousItem)
{
    QList<HistoryGraph::Vertex> vertices;

    // any extra sources?
    vertices = graph().adjacentVertices(item->vertex, HistoryGraph::EdgesToRoot);
    vertices.removeOne(previousItem->vertex);
    if (!vertices.isEmpty())
        addSources(item, vertices);

    // Any other egdes off the main path?
    vertices = graph().adjacentVertices(previousItem->vertex, HistoryGraph::EdgesToLeave);
    vertices.removeOne(item->vertex);
    if (!vertices.isEmpty())
        addBranches(previousItem, vertices);

    // Listing filter actions
    HistoryEdgeProperties props = graph().properties(item->vertex, previousItem->vertex);
    foreach (const FilterAction& action, props.actions)
    {
        previousItem->addChild(createFilterActionItem(action));
    }
}

void ImageHistoryGraphModel::ImageHistoryGraphModelPriv::addBranches(VertexItem* previousItem,
                                                                     const QList<HistoryGraph::Vertex>& branches)
{
    //kDebug() << "Adding branches for items" << branches.size();
    QList<HistoryGraph::Vertex> subgraph;
    foreach (const HistoryGraph::Vertex& v, branches)
    {
        subgraph << graph().verticesDominatedBy(v, previousItem->vertex, HistoryGraph::DepthFirstOrder); // or better breadth-first?
    }

    if (!subgraph.isEmpty())
    {
        BranchesItem *branches = new BranchesItem;
        previousItem->addChild(branches);
        foreach (const HistoryGraph::Vertex& v, subgraph)
        {
            branches->addChild(createVertexItem(v));
        }
    }
}

void ImageHistoryGraphModel::ImageHistoryGraphModelPriv::addSources(VertexItem* item,
                                                                    const QList<HistoryGraph::Vertex>& sources)
{
    Q_UNUSED(item);
    Q_UNUSED(sources);
}

// ------------------------------------------------------------------------

ImageHistoryGraphModel::ImageHistoryGraphModel(QObject* parent)
                      : QAbstractItemModel(parent), d(new ImageHistoryGraphModelPriv)
{
    d->rootItem = new HistoryTreeItem;
}

ImageHistoryGraphModel::~ImageHistoryGraphModel()
{
    delete d->rootItem;
    delete d;
}

void ImageHistoryGraphModel::setHistory(const ImageInfo& subject, const ImageHistoryGraph& graph)
{
    beginResetModel();

    d->info = subject;

    if (graph.isNull())
    {
        d->historyGraph = ImageHistoryGraph::fromInfo(subject);
    }
    else
    {
        d->historyGraph = graph;
        d->historyGraph.prepareForDisplay(subject);
    }

    // fill helper model
    d->imageModel.clearImageInfos();
    d->imageModel.addImageInfos(d->historyGraph.allImageInfos());

    d->buildPath();

    endResetModel();
}

QVariant ImageHistoryGraphModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    HistoryTreeItem *item = d->item(index);

    HANDLE_ITEM(VertexItem, vertexItem, item)
    {
        if (vertexItem->index.isValid())
            return vertexItem->index.data(role);
        // else: read HistoryImageId from d->graph().properties(vertexItem->vertex)?
    }
    else HANDLE_ITEM(FilterActionItem, filterActionItem, item)
    {
        switch (role)
        {
            case Qt::DisplayRole:
            case Qt::ToolTipRole:
                return filterActionItem->action.displayableName();
                break;
        }
    }
    else HANDLE_ITEM(BranchesItem, branchesItem, item)
    {
        switch (role)
        {
            case Qt::DisplayRole:
            case Qt::ToolTipRole:
                return i18n("More Versions");
                break;
        }
    }
    else HANDLE_ITEM(SourcesItem, sourcesItem, item)
    {
        switch (role)
        {
            case Qt::DisplayRole:
            case Qt::ToolTipRole:
                return i18n("More Source Images");
                break;
        }
    }

    return QVariant();
}

QVariant ImageHistoryGraphModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(section)
    Q_UNUSED(orientation)
    Q_UNUSED(role)
    return QVariant();
}

int ImageHistoryGraphModel::rowCount(const QModelIndex& parent) const
{
    return d->item(parent)->childCount();
}

int ImageHistoryGraphModel::columnCount(const QModelIndex&) const
{
    return 1;
}

Qt::ItemFlags ImageHistoryGraphModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return 0;

    Qt::ItemFlags f = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    /*
    if (d->itemDrag)
        f |= Qt::ItemIsDragEnabled;
    if (d->itemDrop)
        f |= Qt::ItemIsDropEnabled;
    */
    return f;
}

QModelIndex ImageHistoryGraphModel::index(int row, int column , const QModelIndex& parent) const
{
    if (column != 0 || row < 0)
        return QModelIndex();

    HistoryTreeItem* item = d->item(parent);
    if (row >= item->childCount())
        return QModelIndex();

    return createIndex(row, 0, item->child(row));
}

bool ImageHistoryGraphModel::hasChildren(const QModelIndex& parent) const
{
    return d->item(parent)->childCount();
}

QModelIndex ImageHistoryGraphModel::parent(const QModelIndex& index) const
{
    HistoryTreeItem* item   = d->item(index);
    HistoryTreeItem* parent = item->parent;
    if (!parent)
        return QModelIndex(); // index was an invalid index

    HistoryTreeItem* grandparent = parent->parent;
    if (!grandparent)
        return QModelIndex(); // index was a top-level index, was the invisible rootItem as parent

    return createIndex(grandparent->children.indexOf(parent), 0, parent);
}

} // namespace Digikam
