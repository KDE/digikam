/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-11-03
 * Description : A dialog base class which can handle multiple pages.
 *
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2007      by Rafael Fernández López <ereslibre at kde dot org>
 * Copyright (C) 2006      by Tobias Koenig <tokoe at kde dot org>
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

#include "dconfigdlgview.h"
#include "dconfigdlgview_p.h"

// Qt includes

#include <QAbstractItemView>
#include <QGridLayout>
#include <QLabel>
#include <QSize>
#include <QStackedWidget>
#include <QTimer>

// Local includes

#include "dconfigdlgmodels.h"
#include "dconfigdlgwidgets.h"

namespace Digikam
{

void DConfigDlgViewPrivate::_k_rebuildGui()
{
    // clean up old view
    Q_Q(DConfigDlgView);

    QModelIndex currentLastIndex;

    if (view && view->selectionModel())
    {
        QObject::disconnect(view->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
                            q, SLOT(_k_pageSelected(QItemSelection,QItemSelection)));

        currentLastIndex = view->selectionModel()->currentIndex();
    }

    delete view;
    view = q->createView();

    Q_ASSERT(view);

    view->setSelectionBehavior(QAbstractItemView::SelectItems);
    view->setSelectionMode(QAbstractItemView::SingleSelection);

    if (model)
    {
        view->setModel(model);
    }

    // setup new view
    if (view->selectionModel())
    {
        QObject::connect(view->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
                         q, SLOT(_k_pageSelected(QItemSelection,QItemSelection)));

        if (currentLastIndex.isValid())
        {
            view->selectionModel()->setCurrentIndex(currentLastIndex, QItemSelectionModel::Select);
        }
        else if (model)
        {
            view->selectionModel()->setCurrentIndex(model->index(0, 0), QItemSelectionModel::Select);
        }
    }

    if (faceType == DConfigDlgView::Tabbed)
    {
        stack->setVisible(false);
        layout->removeWidget(stack);
    }
    else
    {
        layout->addWidget(stack, 2, 1);
        stack->setVisible(true);
    }

    titleWidget->setVisible(q->showPageHeader());

    Qt::Alignment alignment = q->viewPosition();

    if (alignment & Qt::AlignTop)
    {
        layout->addWidget(view, 2, 1);
    }
    else if (alignment & Qt::AlignRight)
    {
        layout->addWidget(view, 1, 2, 2, 1);
    }
    else if (alignment & Qt::AlignBottom)
    {
        layout->addWidget(view, 4, 1);
    }
    else if (alignment & Qt::AlignLeft)
    {
        layout->addWidget(view, 1, 0, 2, 1);
    }
}

void DConfigDlgViewPrivate::updateSelection()
{
    /**
     * Select the first item in the view if not done yet.
     */

    if (!model)
    {
        return;
    }

    if (!view || !view->selectionModel())
    {
        return;
    }

    const QModelIndex index = view->selectionModel()->currentIndex();

    if (!index.isValid())
    {
        view->selectionModel()->setCurrentIndex(model->index(0, 0), QItemSelectionModel::Select);
    }
}

void DConfigDlgViewPrivate::cleanupPages()
{
    /**
     * Remove all orphan pages from the stacked widget.
     */

    const QList<QWidget*> widgets = collectPages();

    for (int i = 0; i < stack->count(); ++i)
    {
        QWidget *page = stack->widget(i);

        bool found = false;

        for (int j = 0; j < widgets.count(); ++j)
        {
            if (widgets[ j ] == page)
            {
                found = true;
            }
        }

        if (!found)
        {
            stack->removeWidget(page);
        }
    }
}

QList<QWidget*> DConfigDlgViewPrivate::collectPages(const QModelIndex& parentIndex)
{
    /**
     * Traverse through the model recursive and collect all widgets in
     * a list.
     */
    QList<QWidget*> retval;

    int rows = model->rowCount(parentIndex);

    for (int j = 0; j < rows; ++j)
    {
        const QModelIndex index = model->index(j, 0, parentIndex);
        retval.append(qvariant_cast<QWidget*>(model->data(index, DConfigDlgModel::WidgetRole)));

        if (model->rowCount(index) > 0)
        {
            retval += collectPages(index);
        }
    }

    return retval;
}

DConfigDlgView::FaceType DConfigDlgViewPrivate::detectAutoFace() const
{
    if (!model)
    {
        return DConfigDlgView::Plain;
    }

    /**
     * Check whether the model has sub pages.
     */
    bool hasSubPages = false;
    const int count  = model->rowCount();

    for (int i = 0; i < count; ++i)
    {
        if (model->rowCount(model->index(i, 0)) > 0)
        {
            hasSubPages = true;
            break;
        }
    }

    if (hasSubPages)
    {
        return DConfigDlgView::Tree;
    }

    if (model->rowCount() > 1)
    {
        return DConfigDlgView::List;
    }

    return DConfigDlgView::Plain;
}

void DConfigDlgViewPrivate::_k_modelChanged()
{
    if (!model)
    {
        return;
    }

    /**
     * If the face type is Auto, we rebuild the GUI whenever the layout
     * of the model changes.
     */
    if (faceType == DConfigDlgView::Auto)
    {
        _k_rebuildGui();
        // If you discover some crashes use the line below instead...
        //QTimer::singleShot(0, q, SLOT(_k_rebuildGui()));
    }

    /**
     * Set the stack to the minimum size of the largest widget.
     */
    QSize size                    = stack->size();
    const QList<QWidget*> widgets = collectPages();

    for (int i = 0; i < widgets.count(); ++i)
    {
        const QWidget* widget = widgets[ i ];

        if (widget)
        {
            size = size.expandedTo(widget->minimumSizeHint());
        }
    }

    stack->setMinimumSize(size);

    updateSelection();
}

void DConfigDlgViewPrivate::_k_pageSelected(const QItemSelection& index, const QItemSelection& previous)
{
    if (!model)
    {
        return;
    }

    // Return if the current Index is not valid
    if (index.indexes().size() != 1)
    {
        return;
    }

    QModelIndex currentIndex = index.indexes().first();
    QModelIndex previousIndex;

    // The previous index can be invalid

    if (previous.indexes().size() == 1)
    {
        previousIndex = previous.indexes().first();
    }

    if (faceType != DConfigDlgView::Tabbed)
    {
        QWidget* const widget = qvariant_cast<QWidget*>(model->data(currentIndex, DConfigDlgModel::WidgetRole));

        if (widget)
        {
            if (stack->indexOf(widget) == -1)
            {
                // not included yet
                stack->addWidget(widget);
            }

            stack->setCurrentWidget(widget);
        }
        else
        {
            stack->setCurrentWidget(defaultWidget);
        }

        updateTitleWidget(currentIndex);
    }

    Q_Q(DConfigDlgView);
    emit q->currentPageChanged(currentIndex, previousIndex);
}

void DConfigDlgViewPrivate::updateTitleWidget(const QModelIndex& index)
{
    Q_Q(DConfigDlgView);

    QString header = model->data(index, DConfigDlgModel::HeaderRole).toString();

    if (header.isNull())
    {
        header = model->data(index, Qt::DisplayRole).toString();
    }

    titleWidget->setText(header);
    titleWidget->setVisible(q->showPageHeader());
}

void DConfigDlgViewPrivate::_k_dataChanged(const QModelIndex&, const QModelIndex&)
{
    /**
     * When data has changed we update the header and icon for the currently selected
     * page.
     */
    if (!view)
    {
        return;
    }

    QModelIndex index = view->selectionModel()->currentIndex();

    if (!index.isValid())
    {
        return;
    }

    updateTitleWidget(index);
}

DConfigDlgViewPrivate::DConfigDlgViewPrivate(DConfigDlgView* const _parent)
    : q_ptr(_parent),
      model(0),
      faceType(DConfigDlgView::Auto),
      layout(0),
      stack(0),
      titleWidget(0),
      defaultWidget(0),
      view(0)
{
}

void DConfigDlgViewPrivate::init()
{
    Q_Q(DConfigDlgView);
    layout      = new QGridLayout(q);
    stack       = new DConfigDlgStackedWidget(q);
    titleWidget = new DConfigDlgTitle(q);
    QPixmap emptyPixmap(22, 22);
    emptyPixmap.fill(Qt::transparent);
    titleWidget->setPixmap(emptyPixmap);
    layout->addWidget(titleWidget, 1, 1);
    layout->addWidget(stack, 2, 1);

    defaultWidget = new QWidget(q);
    stack->addWidget(defaultWidget);

    // stack should use most space
    layout->setColumnStretch(1, 1);
    layout->setRowStretch(2, 1);
}

// ---------------------------------------------------------------------------------------

DConfigDlgView::DConfigDlgView(QWidget* const parent)
    : QWidget(parent),
      d_ptr(new DConfigDlgViewPrivate(this))
{
    d_ptr->init();
}

DConfigDlgView::DConfigDlgView(DConfigDlgViewPrivate& dd, QWidget* const parent)
    : QWidget(parent),
      d_ptr(&dd)
{
    d_ptr->init();
}

DConfigDlgView::~DConfigDlgView()
{
    delete d_ptr;
}

void DConfigDlgView::setModel(QAbstractItemModel* model)
{
    Q_D(DConfigDlgView);

    // clean up old model
    if (d->model)
    {
        disconnect(d->model, SIGNAL(layoutChanged()),
                   this, SLOT(_k_modelChanged()));

        disconnect(d->model, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                   this, SLOT(_k_dataChanged(QModelIndex,QModelIndex)));
    }

    d->model = model;

    if (d->model)
    {
        connect(d->model, SIGNAL(layoutChanged()),
                this, SLOT(_k_modelChanged()));

        connect(d->model, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                this, SLOT(_k_dataChanged(QModelIndex,QModelIndex)));

        // set new model in navigation view
        if (d->view)
        {
            d->view->setModel(model);
        }
    }

    d->_k_rebuildGui();
}

QAbstractItemModel* DConfigDlgView::model() const
{
    return d_func()->model;
}

void DConfigDlgView::setFaceType(FaceType faceType)
{
    Q_D(DConfigDlgView);
    d->faceType = faceType;
    d->_k_rebuildGui();
}

DConfigDlgView::FaceType DConfigDlgView::faceType() const
{
    return d_func()->faceType;
}

void DConfigDlgView::setCurrentPage(const QModelIndex& index)
{
    Q_D(DConfigDlgView);

    if (!d->view || !d->view->selectionModel())
    {
        return;
    }

    d->view->selectionModel()->setCurrentIndex(index, QItemSelectionModel::SelectCurrent);
}

QModelIndex DConfigDlgView::currentPage() const
{
    Q_D(const DConfigDlgView);

    if (!d->view || !d->view->selectionModel())
    {
        return QModelIndex();
    }

    return d->view->selectionModel()->currentIndex();
}

void DConfigDlgView::setItemDelegate(QAbstractItemDelegate* delegate)
{
    Q_D(DConfigDlgView);

    if (d->view)
    {
        d->view->setItemDelegate(delegate);
    }
}

QAbstractItemDelegate *DConfigDlgView::itemDelegate() const
{
    Q_D(const DConfigDlgView);

    if (d->view)
    {
        return d->view->itemDelegate();
    }
    else
    {
        return 0;
    }
}

void DConfigDlgView::setDefaultWidget(QWidget* widget)
{
    Q_D(DConfigDlgView);

    Q_ASSERT(widget);

    bool isCurrent = (d->stack->currentIndex() == d->stack->indexOf(d->defaultWidget));

    // remove old default widget
    d->stack->removeWidget(d->defaultWidget);
    delete d->defaultWidget;

    // add new default widget
    d->defaultWidget = widget;
    d->stack->addWidget(d->defaultWidget);

    if (isCurrent)
    {
        d->stack->setCurrentWidget(d->defaultWidget);
    }
}

QAbstractItemView* DConfigDlgView::createView()
{
    Q_D(DConfigDlgView);

    if (d->faceType == Auto)
    {
        const FaceType faceType = d->detectAutoFace();

        if (faceType == Plain)
        {
            return new DConfigDlgInternal::DConfigDlgPlainView(this);
        }
        else if (faceType == List)
        {
            return new DConfigDlgInternal::DConfigDlgListView(this);
        }
        else if (faceType == Tree)
        {
            return new DConfigDlgInternal::DConfigDlgTreeView(this);
        }
        else
        {
            // should never happen
            return 0;
        }
    }
    else if (d->faceType == Plain)
    {
        return new DConfigDlgInternal::DConfigDlgPlainView(this);
    }
    else if (d->faceType == List)
    {
        return new DConfigDlgInternal::DConfigDlgListView(this);
    }
    else if (d->faceType == Tree)
    {
        return new DConfigDlgInternal::DConfigDlgTreeView(this);
    }
    else if (d->faceType == Tabbed)
    {
        return new DConfigDlgInternal::DConfigDlgTabbedView(this);
    }
    else
    {
        return 0;
    }
}

bool DConfigDlgView::showPageHeader() const
{
    Q_D(const DConfigDlgView);
    FaceType faceType = d->faceType;

    if (faceType == Auto)
    {
        faceType = d->detectAutoFace();
    }

    if (faceType == Tabbed)
    {
        return false;
    }
    else
    {
        return !d->titleWidget->text().isEmpty();
    }
}

Qt::Alignment DConfigDlgView::viewPosition() const
{
    Q_D(const DConfigDlgView);
    FaceType faceType = d->faceType;

    if (faceType == Auto)
    {
        faceType = d->detectAutoFace();
    }

    if (faceType == Plain || faceType == Tabbed)
    {
        return Qt::AlignTop;
    }
    else
    {
        return Qt::AlignLeft;
    }
}

}  // namespace Digikam

#include "moc_dconfigdlgview.cpp"
