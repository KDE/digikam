/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-03-03
 * Description : A widget to select Physical or virtual albums with combo-box
 *
 * Copyright (C) 2016 by Mario Frank <mario dot frank at uni minus potsdam dot de>
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

#ifndef ALBUMSELECTIONCOMPONENT_H
#define ALBUMSELECTIONCOMPONENT_H

// Qt includes

#include <QWidget>

namespace Digikam
{

class Album;
typedef QList<Album*> AlbumList;

class SelectionComponent : public QWidget
{
    Q_OBJECT

public:

    ~SelectionComponent();


    /** Return list of selected element ids
     */
    QList<int> selectedElementIds() const;

    void resetSelection();

    /** Return true if whole Albums collection option is checked.
     */
    bool wholeElementsChecked() const;

    /** Select Physical Album from list. If singleSelection is true, only this one is
     *  selected from tree-view and all others are deselected.
     */
    void setElementSelected(Album* const album, bool singleSelection=true);

    Q_SIGNALS:

    void signalSelectionChanged();

protected:
    /** Default Contructor. 'label' is front text of label which title widget. 'configName' is name used to store
     *  Albums configuration in settings file. 'parent' is parent widget.
     */
    explicit SelectionComponent(const QString& label, const QString& configName, QWidget* const parent = 0);

    virtual AlbumList selectedElements() const = 0;
    
    
private Q_SLOTS:

    void slotUpdateClearButtons();
    void slotWholeAlbums(bool);
    void slotWholeTags(bool);

private:

    QWidget* initAlbumSelection(QWidget* parent);
    QWidget* initTagSelection(QWidget* parent);

protected:

    class Private;
    Private* const d;
};

class AlbumSelectionComponent : public SelectionComponent
{
    public:
    explicit AlbumSelectionComponent(const QString& label, const QString& configName, QWidget* const parent = 0);

    AlbumList selectedElements() const;

private:

    class Private;
    Private* const da;
};

class TagSelectionComponent : public SelectionComponent
{
public:
    explicit TagSelectionComponent(const QString& label, const QString& configName, QWidget* const parent = 0);

    AlbumList selectedElements() const;

private:

    class Private;
    Private* const dt;
};

} // namespace Digikam

#endif // ALBUMSELECTIONCOMPONENT_H


