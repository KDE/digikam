#ifndef ImportIconView_H
#define ImportIconView_H

// Local includes

#include "importcategorizedview.h"

namespace Digikam
{

class ImageViewUtilities;

class ImportIconView : public ImportCategorizedView
{
    Q_OBJECT

public:

    ImportIconView(QWidget* parent = 0);
    ~ImportIconView();

    void init(CameraController* controller);

    ImageViewUtilities* utilities() const;

    int fitToWidthIcons();

    virtual void setThumbnailSize(const ThumbnailSize& size);

public Q_SLOTS:

    void deleteSelected(bool permanently = false);
    void deleteSelectedDirectly(bool permanently = false);

    void createGroupFromSelection();
    void createGroupByTimeFromSelection();
    void ungroupSelected();
    void removeSelectedFromGroup();
    void rename();

Q_SIGNALS:

    void previewRequested(const CamItemInfo& info);

protected:

    virtual void activated(const CamItemInfo& info);
    virtual void showContextMenuOnInfo(QContextMenuEvent* event, const CamItemInfo& info);
    virtual void showContextMenu(QContextMenuEvent* event);
    virtual void slotSetupChanged();

    void slotRotateLeft(const QList<QModelIndex>&);
    void slotRotateRight(const QList<QModelIndex>&);
    void slotInitProgressIndicator();

private:

    class ImportIconViewPriv;
    ImportIconViewPriv* const d;
};

} // namespace Digikam

#endif // ImportIconView_H
