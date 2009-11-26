/*
 * TagFilterSideBarWidget.h
 *
 *  Created on: 26.11.2009
 *      Author: languitar
 */

#ifndef TAGFILTERSIDEBARWIDGET_H
#define TAGFILTERSIDEBARWIDGET_H

// Qt includes
#include <qwidget.h>

// Local includes
#include "imagefiltersettings.h"
#include "tagcheckview.h"

namespace Digikam
{

class TagModel;
class TagModificationHelper;

class TagFilterViewPriv;
class TagFilterView : public TagCheckView
{
    Q_OBJECT
public:

    enum RestoreTagFilters
    {
        OffRestoreTagFilters = 0,
        OnRestoreTagFilters
    };

    TagFilterView(QWidget *parent, TagModel *tagFilterModel,
                  TagModificationHelper *tagModificationHelper);
    virtual ~TagFilterView();

    virtual void loadViewState(KConfigGroup &group, QString prefix = QString());
    virtual void saveViewState(KConfigGroup &group, QString prefix = QString());

    ImageFilterSettings::MatchingCondition getMatchingCondition() const;

Q_SIGNALS:
    void matchingConditionChanged(const ImageFilterSettings::MatchingCondition &condition);

protected:
    virtual void addCustomContextMenuActions(ContextMenuHelper &cmh, TAlbum *tag);
    virtual void handleCustomContextMenuAction(QAction *action, TAlbum *tag);

private:
    TagFilterViewPriv *d;

};

class TagFilterSideBarWidgetPriv;
class TagFilterSideBarWidget : public QWidget
{
    Q_OBJECT
public:
    TagFilterSideBarWidget(QWidget *parent, TagModel *tagFilterModel,
                    TagModificationHelper *tagModificationHelper);
    virtual ~TagFilterSideBarWidget();

Q_SIGNALS:

    /**
     * Emitted if the selected filter has changed.
     *
     * @param tags a list of selected tag ids
     * @param matchingcondition condition to join the seleted tags
     * @param showUnTagged if this is true, only photos without a tag shall be
     *                     shown
     */
    void tagFilterChanged(const QList<int>& tags,
                          ImageFilterSettings::MatchingCondition matchingCond,
                          bool showUnTagged);

public Q_SLOTS:

    /**
     * Resets all selected tag filters.
     */
    void slotResetTagFilters();

private Q_SLOTS:

    void slotMatchingConditionChanged(const ImageFilterSettings::MatchingCondition &condition);
    void slotCheckedTagsChanged(const QList<TAlbum*> &tags);
    void slotWithoutTagChanged(int newState);

private:
    void filterChanged();

private:
    TagFilterSideBarWidgetPriv *d;

};

}

#endif /* TAGFILTERSIDEBARWIDGET_H*/
