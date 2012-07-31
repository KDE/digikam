#ifndef IMPORTTHUMBNAILBAR_H
#define IMPORTTHUMBNAILBAR_H

// Local includes

#include "importcategorizedview.h"

namespace Digikam
{

class ImportThumbnailBar : public ImportCategorizedView
{
    Q_OBJECT

public:

    ImportThumbnailBar(QWidget* parent = 0);
    ~ImportThumbnailBar();

    /**
     * This installs a duplicate filter model, if the ImportImageModel may contain duplicates.
     * Otherwise, just use setModels().
     */
    void setModelsFiltered(ImportImageModel* model, ImportSortFilterModel* filterModel);

    QModelIndex nextIndex(const QModelIndex& index)     const;
    QModelIndex previousIndex(const QModelIndex& index) const;
    QModelIndex firstIndex() const;
    QModelIndex lastIndex()  const;

    /// Sets the policy always for the one scroll bar which is relevant, depending on orientation
    void setScrollBarPolicy(Qt::ScrollBarPolicy policy);
    void setFlow(QListView::Flow newFlow);

    //TODO: Implement rating in Import Tool
    //void installRatingOverlay();

public Q_SLOTS:

    //TODO: Implement rating in Import Tool
    //void assignRating(const QList<QModelIndex>& index, int rating);
    void slotDockLocationChanged(Qt::DockWidgetArea area);

protected:

    virtual void slotSetupChanged();
    virtual bool event(QEvent*);

private:

    class ImportThumbnailBarPriv;
    ImportThumbnailBarPriv* const d;
};

} // namespace Digikam

#endif // IMPORTTHUMBNAILBAR_H
