/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-10-27
 * Description : Model to an ImageHistoryGraph
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

#include "imagehistorygraphmodel.h"

// Qt includes

#include <QAbstractItemModel>
#include <QTreeWidgetItem>
#include <QIcon>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dcategorizedsortfilterproxymodel.h"
#include "dimgfiltermanager.h"
#include "imagelistmodel.h"
#include "imagehistorygraphdata.h"

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
        HeaderItemType,
        CategoryItemType,
        SeparatorItemType
    };

public:

    HistoryTreeItem();
    virtual ~HistoryTreeItem();

    virtual HistoryTreeItemType type() const
    {
        return UnspecifiedType;
    }

    bool isType(HistoryTreeItemType t) const
    {
        return type() == t;
    }

    void addItem(HistoryTreeItem* child);

    int childCount() const
    {
        return children.size();
    }

    HistoryTreeItem* child(int index) const
    {
        return children.at(index);
    }

public:

    HistoryTreeItem*        parent;
    QList<HistoryTreeItem*> children;
};

// ------------------------------------------------------------------------

class VertexItem : public HistoryTreeItem
{
public:

    VertexItem() {}
    explicit VertexItem(const HistoryGraph::Vertex& v) : vertex(v), category(HistoryImageId::InvalidType) {}

    HistoryTreeItemType type() const
    {
        return VertexItemType;
    }

public:

    HistoryGraph::Vertex  vertex;
    QModelIndex           index;
    HistoryImageId::Types category;
};

// ------------------------------------------------------------------------

class FilterActionItem : public HistoryTreeItem
{
public:

    FilterActionItem() {}
    explicit FilterActionItem(const FilterAction& action) : action(action) {}

    HistoryTreeItemType type() const
    {
        return FilterActionItemType;
    }

public:

    FilterAction action;
};

// ------------------------------------------------------------------------

class HeaderItem : public HistoryTreeItem
{
public:

    explicit HeaderItem(const QString& title) : title(title) {}

    HistoryTreeItemType type() const
    {
        return HeaderItemType;
    }

public:

    QString title;
};

// ------------------------------------------------------------------------

class CategoryItem : public HistoryTreeItem
{
public:

    explicit CategoryItem(const QString& title) : title(title) {}

    HistoryTreeItemType type() const
    {
        return CategoryItemType;
    }

public:

    QString title;
};

// ------------------------------------------------------------------------

class SeparatorItem : public HistoryTreeItem
{
public:

    HistoryTreeItemType type() const
    {
        return SeparatorItemType;
    }
};

// ------------------------------------------------------------------------

#define if_isItem(class, name, pointer) \
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

void HistoryTreeItem::addItem(HistoryTreeItem* child)
{
    children << child;
    child->parent = this;
}

// ------------------------------------------------------------------------

static bool oldestInfoFirst(const ImageInfo&a, const ImageInfo& b) { return a.modDateTime() < b.modDateTime(); }
static bool newestInfoFirst(const ImageInfo&a, const ImageInfo& b) { return a.modDateTime() > b.modDateTime(); }

template <typename ImageInfoLessThan>

class LessThanOnVertexImageInfo
{
public:

    LessThanOnVertexImageInfo(const HistoryGraph& graph, ImageInfoLessThan imageInfoLessThan)
        : graph(graph), imageInfoLessThan(imageInfoLessThan)
    {
    }

    bool operator()(const HistoryGraph::Vertex& a, const HistoryGraph::Vertex& b) const
    {
        const HistoryVertexProperties& propsA = graph.properties(a);
        const HistoryVertexProperties& propsB = graph.properties(b);

        if (propsA.infos.isEmpty())
            return false;
        else if (propsB.infos.isEmpty())
            return true;

        return imageInfoLessThan(propsA.infos.first(), propsB.infos.first());
    }

public:

    const HistoryGraph& graph;
    ImageInfoLessThan   imageInfoLessThan;
};

// ------------------------------------------------------------------------

class ImageHistoryGraphModel::Private
{
public:

    Private()
        : mode(ImageHistoryGraphModel::CombinedTreeMode),
          rootItem(0)
    {
    }

    ImageHistoryGraphModel::Mode                       mode;

    ImageHistoryGraph                                  historyGraph;
    ImageInfo                                          info;

    HistoryTreeItem*                                   rootItem;
    QList<VertexItem*>                                 vertexItems;
    ImageListModel                                     imageModel;
    QList<HistoryGraph::Vertex>                        path;
    QHash<HistoryGraph::Vertex, HistoryImageId::Types> categories;

public:

    inline const ImageHistoryGraphData& graph() const
    {
        return historyGraph.data();
    }

    inline HistoryTreeItem* item(const QModelIndex& index) const
    {
        return index.isValid() ? static_cast<HistoryTreeItem*>(index.internalPointer()) : rootItem;
    }

    void build();
    void buildImagesList();
    void buildImagesTree();
    void buildCombinedTree(const HistoryGraph::Vertex& ref);
    void addCombinedItemCategory(HistoryTreeItem* parentItem, QList<HistoryGraph::Vertex>& vertices,
                                 const QString& title, const HistoryGraph::Vertex& showActionsFrom,
                                 QList<HistoryGraph::Vertex>& added);
    void addItemSubgroup(VertexItem* parent, const QList<HistoryGraph::Vertex>& vertices, const QString& title, bool flat = false);
    void addIdenticalItems(HistoryTreeItem* parentItem, const HistoryGraph::Vertex& vertex,
                           const QList<ImageInfo>& infos, const QString& title);

    VertexItem* createVertexItem(const HistoryGraph::Vertex& v, const ImageInfo& info = ImageInfo());
    FilterActionItem* createFilterActionItem(const FilterAction& action);

    template <typename ImageInfoLessThan> LessThanOnVertexImageInfo<ImageInfoLessThan>

    sortBy(ImageInfoLessThan imageInfoLessThan)
    {
        return LessThanOnVertexImageInfo<ImageInfoLessThan>(graph(), imageInfoLessThan);
    }
};

// ------------------------------------------------------------------------

VertexItem* ImageHistoryGraphModel::Private::createVertexItem(const HistoryGraph::Vertex& v,
                                                                                 const ImageInfo& givenInfo)
{
    const HistoryVertexProperties& props = graph().properties(v);
    ImageInfo info                       = givenInfo.isNull() ? props.firstImageInfo() : givenInfo;
    QModelIndex index                    = imageModel.indexForImageInfo(info);
    //qCDebug(DIGIKAM_DATABASE_LOG) << "Added" << info.id() << index;
    VertexItem* item                     = new VertexItem(v);
    item->index                          = index;
    item->category                       = categories.value(v);
    vertexItems << item;
    //qCDebug(DIGIKAM_DATABASE_LOG) << "Adding vertex item" << graph().properties(v).firstImageInfo().id() << index;
    return item;
}

FilterActionItem* ImageHistoryGraphModel::Private::createFilterActionItem(const FilterAction& action)
{
    //qCDebug(DIGIKAM_DATABASE_LOG) << "Adding vertex item for" << action.displayableName();
    return new FilterActionItem(action);
}

void ImageHistoryGraphModel::Private::build()
{
    delete rootItem;
    vertexItems.clear();
    rootItem = new HistoryTreeItem;

    //qCDebug(DIGIKAM_DATABASE_LOG) << historyGraph;

    HistoryGraph::Vertex ref = graph().findVertexByProperties(info);
    path                     = graph().longestPathTouching(ref, sortBy(newestInfoFirst));
    categories               = graph().categorize();

    if (path.isEmpty())
        return;

    if (mode == ImageHistoryGraphModel::ImagesListMode)
    {
        buildImagesList();
    }
    else if (mode == ImageHistoryGraphModel::ImagesTreeMode)
    {
        buildImagesTree();
    }
    else if (mode == CombinedTreeMode)
    {
        buildCombinedTree(ref);
    }
}

void ImageHistoryGraphModel::Private::buildImagesList()
{

    QList<HistoryGraph::Vertex> verticesOrdered = graph().verticesDepthFirstSorted(path.first(),
                                                                                   sortBy(oldestInfoFirst));
    foreach(const HistoryGraph::Vertex& v, verticesOrdered)
    {
        rootItem->addItem(createVertexItem(v));
    }
}

void ImageHistoryGraphModel::Private::buildImagesTree()
{
    QList<HistoryGraph::Vertex> verticesOrdered = graph().verticesDepthFirstSorted(path.first(),
                                                                                   sortBy(oldestInfoFirst));
    QMap<HistoryGraph::Vertex, int> distances   = graph().shortestDistancesFrom(path.first());

    QList<HistoryGraph::Vertex> sources;
    int previousLevel        = 0;
    HistoryTreeItem* parent  = rootItem;
    VertexItem* item         = 0;
    VertexItem* previousItem = 0;

    foreach(const HistoryGraph::Vertex& v, verticesOrdered)
    {
        int currentLevel = distances.value(v);

        if (currentLevel == -1)
        {
            // unreachable from first root
            if (graph().isRoot(v) && parent == rootItem)
            {
                // other first-level root?
                parent->addItem(createVertexItem(v));
            }
            else
            {
                // add later as sources
                sources << v;
            }
            continue;
        }

        item = createVertexItem(v);

        if (!sources.isEmpty())
        {
            addItemSubgroup(item, sources, i18nc("@title", "Source Images"));
        }

        if (currentLevel == previousLevel)
        {
            parent->addItem(item);
        }
        else if (currentLevel > previousLevel && previousItem) // check pointer, prevent crash is distances are faulty
        {
            previousItem->addItem(item);
            parent = previousItem;
        }
        else if (currentLevel < previousLevel)
        {
            for (int level = currentLevel; level < previousLevel; ++level)
            {
                parent = parent->parent;
            }
            parent->addItem(item);
        }

        previousItem  = item;
        previousLevel = currentLevel;
    }
}

void ImageHistoryGraphModel::Private::buildCombinedTree(const HistoryGraph::Vertex& ref)
{
    VertexItem* item           = 0;
    CategoryItem *categoryItem = new CategoryItem(i18nc("@title", "Image History"));
    rootItem->addItem(categoryItem);

    QList<HistoryGraph::Vertex> added;
    QList<HistoryGraph::Vertex> currentVersions = categories.keys(HistoryImageId::Current);
    QList<HistoryGraph::Vertex> leavesFromRef   = graph().leavesFrom(ref);

    bool onePath = leavesFromRef.size() <= 1;

    for (int i=0; i<path.size(); ++i)
    {
        const HistoryGraph::Vertex& v = path.at(i);
        HistoryGraph::Vertex previous = i ? path.at(i-1) : HistoryGraph::Vertex();
//        HistoryGraph::Vertex next     = i < path.size() - 1 ? path[i+1] : HistoryGraph::Vertex();
        //qCDebug(DIGIKAM_DATABASE_LOG) << "Vertex on path" << path[i];
        // create new item
        item = createVertexItem(v);

        QList<HistoryGraph::Vertex> vertices;

        // any extra sources?
        QList<HistoryGraph::Vertex> sources = graph().adjacentVertices(item->vertex, HistoryGraph::EdgesToRoot);

        foreach(const HistoryGraph::Vertex& source, sources)
        {
            if (source != previous)
            {
                rootItem->addItem(createVertexItem(source));
            }
        }

/*
        // Any other egdes off the main path?
        QList<HistoryGraph::Vertex> branches = graph().adjacentVertices(v, HistoryGraph::EdgesToLeaf);
        QList<HistoryGraph::Vertex> subgraph;

        foreach(const HistoryGraph::Vertex& branch, branches)
        {
            if (branch != next)
            {
                subgraph << graph().verticesDominatedByDepthFirstSorted(branch, v, sortBy(oldestInfoFirst));
            }
        }

        addItemSubgroup(item, subgraph, i18nc("@title", "More Derived Images"));
*/

        // Add filter actions above item
        HistoryEdgeProperties props = graph().properties(v, previous);

        foreach(const FilterAction& action, props.actions)
        {
            rootItem->addItem(createFilterActionItem(action));
        }

        // now, add item
        rootItem->addItem(item);
        added << v;

        // If there are multiple derived images, we display them in the next section
        if (v == ref && !onePath)
            break;
    }

    foreach(const HistoryGraph::Vertex& v, added)
    {
        leavesFromRef.removeOne(v);
    }

    if (!leavesFromRef.isEmpty())
    {
        addCombinedItemCategory(rootItem, leavesFromRef, i18nc("@title", "Derived Images"), ref, added);
    }

    foreach(const HistoryGraph::Vertex& v, added)
    {
        currentVersions.removeOne(v);
    }

    if (!currentVersions.isEmpty())
    {
        addCombinedItemCategory(rootItem, currentVersions, i18nc("@title", "Related Images"), path.first(), added);
    }

    QList<ImageInfo> allInfos = graph().properties(ref).infos;

    if (allInfos.size() > 1)
    {
        addIdenticalItems(rootItem, ref, allInfos, i18nc("@title", "Identical Images"));
    }
}

void ImageHistoryGraphModel::Private::
     addCombinedItemCategory(HistoryTreeItem* parentItem, QList<HistoryGraph::Vertex>& vertices,
                             const QString& title, const HistoryGraph::Vertex& showActionsFrom,
                             QList<HistoryGraph::Vertex>& added)
{
    parentItem->addItem(new CategoryItem(title));

    std::sort(vertices.begin(), vertices.end(), sortBy(oldestInfoFirst));
    bool isFirst     = true;
    VertexItem* item = 0;

    foreach(const HistoryGraph::Vertex& v, vertices)
    {
        if (isFirst)
        {
            isFirst = false;
        }
        else
        {
            parentItem->addItem(new SeparatorItem);
        }

        item                                     = createVertexItem(v);
        QList<HistoryGraph::Vertex> shortestPath = graph().shortestPath(showActionsFrom, v);

        // add all filter actions showActionsFrom -> v above item
        for (int i=1; i<shortestPath.size(); ++i)
        {
            HistoryEdgeProperties props = graph().properties(shortestPath.at(i), shortestPath.at(i-1));

            foreach(const FilterAction& action, props.actions)
            {
                parentItem->addItem(createFilterActionItem(action));
            }
        }

        parentItem->addItem(item);
        added << v;

        // Provide access to intermediates
        shortestPath.removeOne(showActionsFrom);
        shortestPath.removeOne(v);

        foreach(const HistoryGraph::Vertex& addedVertex, added)
        {
            shortestPath.removeOne(addedVertex);
        }

        addItemSubgroup(item, shortestPath, i18nc("@title", "Intermediate Steps:"), true);
    }
}

void ImageHistoryGraphModel::Private::
     addItemSubgroup(VertexItem* parent, const QList<HistoryGraph::Vertex>& vertices, const QString& title, bool flat)
{
    if (vertices.isEmpty())
        return;

    HeaderItem* const header         = new HeaderItem(title);
    parent->addItem(header);
    HistoryTreeItem* const addToItem = flat ? static_cast<HistoryTreeItem*>(parent) : static_cast<HistoryTreeItem*>(header);

    foreach(const HistoryGraph::Vertex& v, vertices)
    {
        addToItem->addItem(createVertexItem(v));
    }
}

void ImageHistoryGraphModel::Private::
     addIdenticalItems(HistoryTreeItem* parentItem, const HistoryGraph::Vertex& vertex,
                       const QList<ImageInfo>& infos, const QString& title)
{
    parentItem->addItem(new CategoryItem(title));

    // the properties image info list is already sorted by proximity to subject
    VertexItem* item = 0;
    bool isFirst     = true;

    for (int i=1; i<infos.size(); ++i)
    {
        if (isFirst)
        {
            isFirst = false;
        }
        else
        {
            parentItem->addItem(new SeparatorItem);
        }

        item = createVertexItem(vertex, infos.at(i));
        parentItem->addItem(item);
    }
}

// ------------------------------------------------------------------------

ImageHistoryGraphModel::ImageHistoryGraphModel(QObject* const parent)
    : QAbstractItemModel(parent), d(new Private)
{
    d->rootItem = new HistoryTreeItem;
}

ImageHistoryGraphModel::~ImageHistoryGraphModel()
{
    delete d->rootItem;
    delete d;
}

void ImageHistoryGraphModel::setMode(Mode mode)
{
    if (d->mode == mode)
        return;

    d->mode = mode;
    setHistory(d->info, d->historyGraph);
}

ImageHistoryGraphModel::Mode ImageHistoryGraphModel::mode() const
{
    return d->mode;
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
    d->imageModel.addImageInfos(d->historyGraph.allImages());

    d->build();

    endResetModel();
}

ImageInfo ImageHistoryGraphModel::subject() const
{
    return d->info;
}

bool ImageHistoryGraphModel::isImage(const QModelIndex& index) const
{
    HistoryTreeItem* const item = d->item(index);

    return item && item->isType(HistoryTreeItem::VertexItemType);
}

bool ImageHistoryGraphModel::isFilterAction(const QModelIndex& index) const
{
    HistoryTreeItem* const item = d->item(index);

    return item && item->isType(HistoryTreeItem::FilterActionItemType);
}

FilterAction ImageHistoryGraphModel::filterAction(const QModelIndex& index) const
{
    HistoryTreeItem* const item = d->item(index);

    if_isItem(FilterActionItem, filterActionItem, item)
    {
        return filterActionItem->action;
    }

    return FilterAction();
}

bool ImageHistoryGraphModel::hasImage(const ImageInfo& info)
{
    return d->imageModel.hasImage(info);
}

ImageInfo ImageHistoryGraphModel::imageInfo(const QModelIndex& index) const
{
    QModelIndex imageIndex = imageModelIndex(index);

    return ImageModel::retrieveImageInfo(imageIndex);
}

QModelIndex ImageHistoryGraphModel::indexForInfo(const ImageInfo& info) const
{
    if (info.isNull())
    {
        return QModelIndex();
    }

    // try with primary info
    foreach(VertexItem* const item, d->vertexItems)
    {
        if (ImageModel::retrieveImageInfo(item->index) == info)
        {
            return createIndex(item->parent->children.indexOf(item), 0, item);
        }
    }

    // try all associated infos
    foreach(VertexItem* const item, d->vertexItems)
    {
        if (d->graph().properties(item->vertex).infos.contains(info))
        {
            return createIndex(item->parent->children.indexOf(item), 0, item);
        }
    }

    return QModelIndex();
}

QVariant ImageHistoryGraphModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
    {
        return QVariant();
    }

    HistoryTreeItem* const item = d->item(index);

    if_isItem(VertexItem, vertexItem, item)
    {
        if (vertexItem->index.isValid())
        {
            QVariant data = vertexItem->index.data(role);

            switch (role)
            {
                case IsImageItemRole:
                {
                    return true;
                }
                case IsSubjectImageRole:
                {
                    return (bool)d->graph().properties(vertexItem->vertex).infos.contains(d->info);
                }
                case Qt::DisplayRole:
                {
                    if (vertexItem->category & HistoryImageId::Original)
                    {
                        return i18nc("@item filename", "%1\n(Original Image)", data.toString());
                    }

                    if (vertexItem->category & HistoryImageId::Source)
                    {
                        return i18nc("@item filename", "%1\n(Source Image)", data.toString());
                    }

                    break;
                }
            }

            return data;
        }

        // else: read HistoryImageId from d->graph().properties(vertexItem->vertex)?
    }
    else if_isItem(FilterActionItem, filterActionItem, item)
    {
        switch (role)
        {
            case IsFilterActionItemRole:
            {
                return true;
            }
            case Qt::DisplayRole:
            {
                return DImgFilterManager::instance()->i18nDisplayableName(filterActionItem->action);
            }
            case Qt::DecorationRole:
            {
                QString iconName = DImgFilterManager::instance()->filterIcon(filterActionItem->action);
                return QIcon::fromTheme(iconName);
            }
            case FilterActionRole:
            {
                return QVariant::fromValue(filterActionItem->action);
            }
            default:
            {
                break;
            }
        }
    }
    else if_isItem(HeaderItem, headerItem, item)
    {
        switch (role)
        {
            case IsHeaderItemRole:
                return true;
            case Qt::DisplayRole:
            //case Qt::ToolTipRole:
                return headerItem->title;
                break;
        }
    }
    else if_isItem(CategoryItem, categoryItem, item)
    {
        switch (role)
        {
            case IsCategoryItemRole:
                return true;
            case Qt::DisplayRole:
            case DCategorizedSortFilterProxyModel::CategoryDisplayRole:
            //case Qt::ToolTipRole:
                return categoryItem->title;
        }
    }
    else if_isItem(SeparatorItem, separatorItem, item)
    {
        switch (role)
        {
            case IsSeparatorItemRole:
                return true;
        }
    }

    switch (role)
    {
        case IsImageItemRole:
        case IsFilterActionItemRole:
        case IsHeaderItemRole:
        case IsCategoryItemRole:
        case IsSubjectImageRole:
            return false;
        default:
            return QVariant();
    }
}

bool ImageHistoryGraphModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    HistoryTreeItem* const item = d->item(index);

    if_isItem(VertexItem, vertexItem, item)
    {
        if (vertexItem->index.isValid())
        {
            return d->imageModel.setData(vertexItem->index, value, role);
        }
    }

    return false;
}

ImageListModel* ImageHistoryGraphModel::imageModel() const
{
    return &d->imageModel;
}

QModelIndex ImageHistoryGraphModel::imageModelIndex(const QModelIndex& index) const
{
    HistoryTreeItem* const item = d->item(index);

    if_isItem(VertexItem, vertexItem, item)
    {
        return vertexItem->index;
    }

    return QModelIndex();
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
    {
        return 0;
    }

    HistoryTreeItem* const item = d->item(index);

    if_isItem(VertexItem, vertexItem, item)
    {
        return d->imageModel.flags(vertexItem->index);
    }

    switch (item->type())
    {
        case HistoryTreeItem::FilterActionItemType:
            return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
        case HistoryTreeItem::HeaderItemType:
        case HistoryTreeItem::CategoryItemType:
        case HistoryTreeItem::SeparatorItemType:
        default:
            return Qt::ItemIsEnabled;
    }
}

QModelIndex ImageHistoryGraphModel::index(int row, int column , const QModelIndex& parent) const
{
    if (column != 0 || row < 0)
    {
        return QModelIndex();
    }

    HistoryTreeItem* item = d->item(parent);

    if (row >= item->childCount())
    {
        return QModelIndex();
    }

    return createIndex(row, 0, item->child(row));
}

bool ImageHistoryGraphModel::hasChildren(const QModelIndex& parent) const
{
    return d->item(parent)->childCount();
}

QModelIndex ImageHistoryGraphModel::parent(const QModelIndex& index) const
{
    HistoryTreeItem* const item   = d->item(index);
    HistoryTreeItem* const parent = item->parent;

    if (!parent)
    {
        return QModelIndex();    // index was an invalid index
    }

    HistoryTreeItem* const grandparent = parent->parent;

    if (!grandparent)
    {
        return QModelIndex();    // index was a top-level index, was the invisible rootItem as parent
    }

    return createIndex(grandparent->children.indexOf(parent), 0, parent);
}

} // namespace Digikam
