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

namespace Digikam
{

class TagModel;
class TagModificationHelper;

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

private:
    TagFilterSideBarWidgetPriv *d;

};

}

#endif /* TAGFILTERSIDEBARWIDGET_H*/
