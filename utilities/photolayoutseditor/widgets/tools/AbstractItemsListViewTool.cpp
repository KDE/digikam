/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-09-01
 * Description : a plugin to create photo layouts by fusion of several images.
 * 
 *
 * Copyright (C) 2011      by Lukasz Spas <lukasz dot spas at gmail dot com>
 * Copyright (C) 2009-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "AbstractItemsListViewTool.h"
#include "AbstractItemsListViewTool_p.h"

#include <QGridLayout>
#include <QLabel>
#include <QListView>
#include <QPushButton>
#include <QComboBox>
#include <QIcon>

#include <klocalizedstring.h>

#include "qtpropertybrowser.h"
#include "AbstractPhoto.h"
#include "ToolsDockWidget.h"
#include "BorderDrawersLoader.h"
#include "global.h"
#include "digikam_debug.h"

using namespace PhotoLayoutsEditor;

class ItemCreatedCommand : public QUndoCommand
{
    QObject*              item;
    int                   row;
    AbstractMovableModel* model;
    bool                  done;

public:
    ItemCreatedCommand(QObject * item, int row, AbstractMovableModel * model, QUndoCommand * parent = 0) :
        QUndoCommand(parent),
        item(item),
        row(row),
        model(model),
        done(false)
    {}

    ~ItemCreatedCommand()
    {
        if (!done)
            delete item;
    }

    virtual void redo()
    {
        done = true;
        if (model->item(model->index(row, 0)) == item)
            return;
        model->insertRow(row);
        model->setItem(item, model->index(row, 0));
    }

    virtual void undo()
    {
        done = false;
        if (model->item(model->index(row, 0)) != item)
            return;
        model->removeRow(row);
    }
};

class ItemRemovedCommand : public QUndoCommand
{
    QObject*              item;
    int                   row;
    AbstractMovableModel* model;
    bool                  done;

public:

    ItemRemovedCommand(QObject * item, int row, AbstractMovableModel * model, QUndoCommand * parent = 0) :
        QUndoCommand(parent),
        item(item),
        row(row),
        model(model),
        done(true)
    {}

    ~ItemRemovedCommand()
    {
        if (done)
            delete item;
    }

    virtual void redo()
    {
        done = true;
        if (model->item(model->index(row, 0)) != item)
            return;
        model->removeRow(row);
    }

    virtual void undo()
    {
        done = false;
        if (model->item(model->index(row, 0)) == item)
            return;
        model->insertRow(row);
        model->setItem(item, model->index(row, 0));
    }
};

class ItemMoveRowsCommand : public QUndoCommand
{
    int                   sourceStart;
    int                   count;
    int                   destinationRow;
    AbstractMovableModel* model;

public:

    ItemMoveRowsCommand(int sourceStart, int count, int destinationRow, AbstractMovableModel * model, QUndoCommand * parent = 0) :
        QUndoCommand(parent),
        sourceStart(sourceStart),
        count(count),
        destinationRow(destinationRow),
        model(model)
    {}

    virtual void redo()
    {
        model->moveRowsData(sourceStart, count, destinationRow);
        this->swap();
    }

    virtual void undo()
    {
        model->moveRowsData(sourceStart, count, destinationRow);
        this->swap();
    }

    void swap()
    {
        int temp       = sourceStart;
        sourceStart    = destinationRow;
        destinationRow = temp;

        if (destinationRow > sourceStart)
            destinationRow += count;
        else
            sourceStart -= count;
    }
};

class PhotoLayoutsEditor::AbstractItemsListViewToolPrivate
{
    AbstractItemsListViewToolPrivate() :
        m_list_widget(0),
        m_add_button(0),
        m_remove_button(0),
        m_down_button(0),
        m_up_button(0),
        m_delegate(0)
//        , m_editors_object(0)
    {}
    AbstractListToolView * m_list_widget;
    QPushButton * m_add_button;
    QPushButton * m_remove_button;
    QPushButton * m_down_button;
    QPushButton * m_up_button;
    AbstractListToolViewDelegate * m_delegate;
//    QObject * m_editors_object;

    void closeChooser()
    {
        if (m_delegate)
            m_delegate->deleteLater();
        m_delegate = 0;
    }

    void setButtonsEnabled(bool isEnabled)
    {
        m_add_button->setEnabled(isEnabled);
        QModelIndex index = m_list_widget->selectedIndex();
        m_remove_button->setEnabled(isEnabled && index.isValid());
        m_down_button->setEnabled(isEnabled && index.isValid() && index.row() < index.model()->rowCount()-1);
        m_up_button->setEnabled(isEnabled && index.isValid() && index.row() > 0);
    }

    friend class AbstractItemsListViewTool;
};

AbstractItemsListViewTool::AbstractItemsListViewTool(const QString & toolName, Scene * scene, Canvas::SelectionMode selectionMode, QWidget * parent) :
    AbstractItemsTool(scene, selectionMode, parent),
    d(new AbstractItemsListViewToolPrivate)
{
    QGridLayout * layout = new QGridLayout(this);

    // Title
    QLabel * title = new QLabel(toolName, this);
    QFont titleFont = title->font();
    titleFont.setBold(true);
    title->setFont(titleFont);
    layout->addWidget(title,0,0);

    // Move add/remove buttons
    QHBoxLayout * addLayout = new QHBoxLayout();
    d->m_add_button = new QPushButton(QIcon::fromTheme(QLatin1String(":/action_add.png")), QString());
    d->m_add_button->setIconSize(QSize(16,16));
    d->m_add_button->setFixedSize(24,24);
    d->m_remove_button = new QPushButton(QIcon::fromTheme(QLatin1String(":/action_remove.png")), QString());
    d->m_remove_button->setIconSize(QSize(16,16));
    d->m_remove_button->setFixedSize(24,24);
    addLayout->addWidget(d->m_add_button);
    addLayout->addWidget(d->m_remove_button);
    addLayout->setSpacing(0);
    layout->addLayout(addLayout,0,1);
    connect(d->m_add_button,SIGNAL(clicked()),this,SLOT(createChooser()));
    connect(d->m_remove_button,SIGNAL(clicked()),this,SLOT(removeSelected()));

    // Move up/down buttons
    QHBoxLayout * moveLayout = new QHBoxLayout();
    d->m_down_button = new QPushButton(QIcon::fromTheme(QLatin1String(":/arrow_down.png")), QString());
    d->m_down_button->setIconSize(QSize(16,16));
    d->m_down_button->setFixedSize(24,24);
    d->m_up_button = new QPushButton(QIcon::fromTheme(QLatin1String(":/arrow_top.png")), QString());

    d->m_up_button->setIconSize(QSize(16,16));
    d->m_up_button->setFixedSize(24,24);
    moveLayout->addWidget(d->m_down_button);
    moveLayout->addWidget(d->m_up_button);
    moveLayout->setSpacing(0);
    layout->addLayout(moveLayout,0,2);
    connect(d->m_down_button,SIGNAL(clicked()),this,SLOT(moveSelectedDown()));
    connect(d->m_up_button,SIGNAL(clicked()),this,SLOT(moveSelectedUp()));

    // Effects list
    d->m_list_widget = new AbstractListToolView(this);
    layout->addWidget(d->m_list_widget,1,0,1,-1);
    connect(d->m_list_widget,SIGNAL(selectedIndex(QModelIndex)),this,SLOT(viewCurrentEditor(QModelIndex)));

    this->setLayout(layout);
    this->setSizePolicy(QSizePolicy::Ignored,QSizePolicy::MinimumExpanding);
    layout->setRowStretch(2,1);
    d->setButtonsEnabled(false);
}

AbstractItemsListViewTool::~AbstractItemsListViewTool()
{
    if (d->m_delegate)
        d->m_delegate->editorAccepted();
    delete d;
}

void AbstractItemsListViewTool::currentItemAboutToBeChanged()
{
    if (d->m_delegate)
        d->m_delegate->editorAccepted();
}

void AbstractItemsListViewTool::currentItemChanged()
{
    d->m_list_widget->setModel(model());
    d->setButtonsEnabled(true);
}

void AbstractItemsListViewTool::viewCurrentEditor(const QModelIndex & index)
{
    closeEditor();
    d->setButtonsEnabled(true);
    QWidget * editor = createEditor(static_cast<QObject*>(index.internalPointer()), true);
    if (editor)
    {
        static_cast<QGridLayout*>(layout())->addWidget(editor,2,0,1,-1);
        editor->show();
    }
}

void AbstractItemsListViewTool::viewCurrentEditor(QObject * object)
{
    closeEditor();
    d->setButtonsEnabled(true);
    QWidget * editor = createEditor(object, false);
    if (editor)
    {
        static_cast<QGridLayout*>(layout())->addWidget(editor,2,0,1,-1);
        editor->show();
    }
}

void AbstractItemsListViewTool::createChooser()
{
    AbstractMovableModel * model = this->model();
    if (model)
    {
        // Calculate chooser position
        int row = 0;
        QModelIndex selectedIndex = d->m_list_widget->selectedIndex();
        if (selectedIndex.isValid())
            row = selectedIndex.row();
        model->insertRow(row);

        // Create chooser
        d->m_delegate = new AbstractListToolViewDelegate(model, model->index(row,0), this);
        d->m_list_widget->setIndexWidget(model->index(row,0),d->m_delegate);

        d->m_list_widget->setSelectionMode(QAbstractItemView::NoSelection);
        connect(d->m_delegate,SIGNAL(editorClosed()),this,SLOT(closeChooser()));
        connect(d->m_delegate,SIGNAL(showEditor(QObject*)),this,SLOT(viewCurrentEditor(QObject*)));
        d->setButtonsEnabled(false);
        d->m_list_widget->setSelection(QRect(),QItemSelectionModel::Clear);
    }
}

void AbstractItemsListViewTool::closeChooser()
{
    closeEditor();
    d->closeChooser();
    d->m_list_widget->setSelectionMode(QAbstractItemView::SingleSelection);
    d->setButtonsEnabled(true);
}

void AbstractItemsListViewTool::removeSelected()
{
    if (!d->m_list_widget)
        return;
    QModelIndex index = d->m_list_widget->selectedIndex();
    AbstractMovableModel * model = this->model();
    if (model && index.isValid())
    {
        if (index.internalPointer())
        {
            ItemRemovedCommand * command = new ItemRemovedCommand(static_cast<QObject*>(index.internalPointer()), index.row(), model);
            PLE_PostUndoCommand(command);
        }
        else
            model->removeRow(index.row());
    }
}

void AbstractItemsListViewTool::moveSelectedDown()
{
    if (!d->m_list_widget)
        return;
    QModelIndex index = d->m_list_widget->selectedIndex();
    AbstractMovableModel * model = this->model();
    if (model && index.row() < model->rowCount()-1)
    {
        if (index.internalPointer())
        {
            QUndoCommand * command = new ItemMoveRowsCommand(index.row(),1,index.row()+2,model);
            PLE_PostUndoCommand(command);
        }
        else
            model->moveRowsData(index.row(),1,index.row()+2);
    }
    d->setButtonsEnabled(true);
}

void AbstractItemsListViewTool::moveSelectedUp()
{
    if (!d->m_list_widget)
        return;
    QModelIndex index = d->m_list_widget->selectedIndex();
    AbstractMovableModel * model = this->model();
    if (model && index.row() > 0)
    {
        if (index.internalPointer())
        {
            QUndoCommand * command = new ItemMoveRowsCommand(index.row(),1,index.row()-1,model);
            PLE_PostUndoCommand(command);
        }
        else
            model->moveRowsData(index.row(),1,index.row()-1);
    }
    d->setButtonsEnabled(true);
}

void AbstractItemsListViewTool::closeEditor()
{
    QLayoutItem * itemBrowser = static_cast<QGridLayout*>(layout())->itemAtPosition(2,0);
    if (!itemBrowser)
        return;
    QWidget * browser = itemBrowser->widget();
    if (!browser)
        return;
    static_cast<QGridLayout*>(layout())->removeWidget(browser);
    browser->deleteLater();
}

AbstractListToolViewDelegate::AbstractListToolViewDelegate(AbstractMovableModel * model, QModelIndex index, AbstractItemsListViewTool * parent) :
    QWidget(parent),
    m_parent(parent),
    m_model(model),
    m_index(index),
    m_object(0)
{
    // GUI setup
    QHBoxLayout * layout = new QHBoxLayout();
    layout->setContentsMargins(QMargins());
    layout->setSpacing(0);
    this->setLayout(layout);
    QStringList registeredDrawers = parent->options();
    QComboBox * comboBox = new QComboBox(this);
    comboBox->addItems(registeredDrawers);
    comboBox->setCurrentIndex(-1);
    connect(comboBox,SIGNAL(currentIndexChanged(QString)),this,SLOT(itemSelected(QString)));
    layout->addWidget(comboBox,1);
    m_acceptButton = new QPushButton(QIcon::fromTheme(QLatin1String(":/action_check.png")), QString(), this);
    m_acceptButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
    m_acceptButton->setEnabled(false);
    connect(m_acceptButton,SIGNAL(clicked()),this,SLOT(editorAccepted()));
    layout->addWidget(m_acceptButton);
    QPushButton * cancelButton = new QPushButton(QIcon::fromTheme(QLatin1String(":/action_delete.png")), QString(), this);
    cancelButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);
    connect(cancelButton,SIGNAL(clicked()),this,SLOT(editorCancelled()));
    layout->addWidget(cancelButton);
}

void AbstractListToolViewDelegate::editorCancelled()
{
    if (m_index.isValid() && !m_index.internalPointer())
        m_model->removeRow(m_index.row());
    if (m_object)
        m_object->deleteLater();
    m_object = 0;
    emit editorClosed();
}

void AbstractListToolViewDelegate::editorAccepted()
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "isAccepted sent" << m_object << m_model;
    if (!m_object || !m_model)
        return;
    qCDebug(DIGIKAM_GENERAL_LOG) << "isAccepted sent";
    ItemCreatedCommand * command = new ItemCreatedCommand(m_object, m_index.row(), m_model);
    PLE_PostUndoCommand(command);
    emit editorClosed();
}

void AbstractListToolViewDelegate::itemSelected(const QString & selectedItem)
{
    if (m_model)
    {
        if ((m_object =  m_parent->createItem(selectedItem)))
        {
            m_model->setItem(m_object, m_index);
            emit showEditor(m_object);
        }
    }

    m_acceptButton->setEnabled(!selectedItem.isEmpty());
}
