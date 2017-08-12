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

#ifndef ABSTRACTITEMSLISTVIEWTOOL_P_H
#define ABSTRACTITEMSLISTVIEWTOOL_P_H

#include <QWidget>
#include <QListView>

#include <QPushButton>

namespace PhotoLayoutsEditor
{
    class AbstractItemsListViewTool;
    class AbstractMovableModel;

    class AbstractListToolViewDelegate : public QWidget
    {
            Q_OBJECT

        public:

            QPushButton*               m_acceptButton;
            AbstractItemsListViewTool* m_parent;
            AbstractMovableModel*      m_model;
            QModelIndex                m_index;
            QObject*                   m_object;

        public:

            AbstractListToolViewDelegate(AbstractMovableModel * model, QModelIndex index, AbstractItemsListViewTool * parent);

        Q_SIGNALS:

            void editorClosed();
            void showEditor(QObject * object);

        protected Q_SLOTS:

            void editorAccepted();
            void editorCancelled();
            void itemSelected(const QString & selectedItem);

        friend class AbstractItemsListViewTool;
    };

    class AbstractListToolView : public QListView
    {
            Q_OBJECT

        public:

            AbstractListToolView(QWidget * parent = 0) :
                QListView(parent)
            {
                this->setSelectionMode(QAbstractItemView::SingleSelection);
                this->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
            }

            QModelIndex selectedIndex() const
            {
                QModelIndexList indexes = selectedIndexes();

                if (indexes.count() == 1)
                    return indexes.at(0);

                return QModelIndex();
            }

        Q_SIGNALS:

            void selectedIndex(const QModelIndex & index);

        protected:

            virtual void selectionChanged(const QItemSelection & selected, const QItemSelection & /*deselected*/)
            {
                QModelIndexList indexes = selected.indexes();
                if (indexes.count())
                {
                    QModelIndex index = indexes.at(0);
                    if (index.isValid())
                    {
                        emit selectedIndex(index);
                        return;
                    }
                }

                emit selectedIndex(QModelIndex());
            }

        friend class AbstractItemsListViewTool;
    };
}

#endif // ABSTRACTITEMSLISTVIEWTOOL_P_H
