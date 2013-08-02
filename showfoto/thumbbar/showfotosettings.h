#ifndef SHOWFOTOSETTINGS_H
#define SHOWFOTOSETTINGS_H

// Qt includes

#include <QObject>
#include <QFont>

namespace ShowFoto
{

class ShowfotoSettings : public QObject
{
    Q_OBJECT

public:

    enum ItemLeftClickAction
    {
        ShowPreview = 0,
        StartEditor
    };

Q_SIGNALS:

    void setupChanged();

public:

    static ShowfotoSettings* instance();

    void readSettings();
    void saveSettings();

    void emitSetupChanged();

    bool showToolTipsIsValid() const;

    void setShowThumbbar(bool val);
    bool getShowThumbbar() const;

    void setPreviewLoadFullImageSize(bool val);
    bool getPreviewLoadFullImageSize() const;

    void setPreviewItemsWhileDownload(bool val);
    bool getPreviewItemsWhileDownload() const;

    void setPreviewShowIcons(bool val);
    bool getPreviewShowIcons() const;

    void setImageSortOrder(int order);
    int getImageSortOrder() const;

    void setImageSorting(int sorting);
    int getImageSorting() const;

    void setImageGroupMode(int mode);
    int getImageGroupMode() const;

    void setItemLeftClickAction(const ItemLeftClickAction action);
    ItemLeftClickAction getItemLeftClickAction() const;

    void setDefaultIconSize(int val);
    int getDefaultIconSize() const;

    void setIconViewFont(const QFont& font);
    QFont getIconViewFont() const;

    void setIconShowName(bool val);
    bool getIconShowName() const;

    void setIconShowSize(bool val);
    bool getIconShowSize() const;

    void setIconShowTitle(bool val);
    bool getIconShowTitle() const;

    void setIconShowTags(bool val);
    bool getIconShowTags() const;

    void setIconShowDate(bool val);
    bool getIconShowDate() const;

    void setIconShowRating(bool val);
    bool getIconShowRating() const;

    void setIconShowImageFormat(bool val);
    bool getIconShowImageFormat() const;

    void setIconShowOverlays(bool val);
    bool getIconShowOverlays() const;

    void setToolTipsFont(const QFont& font);
    QFont getToolTipsFont() const;

    void setShowToolTips(bool val);
    bool getShowToolTips() const;

    void setToolTipsShowFileName(bool val);
    bool getToolTipsShowFileName() const;

    void setToolTipsShowFileDate(bool val);
    bool getToolTipsShowFileDate() const;

    void setToolTipsShowFileSize(bool val);
    bool getToolTipsShowFileSize() const;

    void setToolTipsShowImageType(bool val);
    bool getToolTipsShowImageType() const;

    void setToolTipsShowImageDim(bool val);
    bool getToolTipsShowImageDim() const;

    void setToolTipsShowPhotoMake(bool val);
    bool getToolTipsShowPhotoMake() const;

    void setToolTipsShowPhotoFocal(bool val);
    bool getToolTipsShowPhotoFocal() const;

    void setToolTipsShowPhotoExpo(bool val);
    bool getToolTipsShowPhotoExpo() const;

    void setToolTipsShowPhotoFlash(bool val);
    bool getToolTipsShowPhotoFlash() const;

    void setToolTipsShowPhotoWB(bool val);
    bool getToolTipsShowPhotoWB() const;

    void setToolTipsShowTags(bool val);
    bool getToolTipsShowTags() const;

    void setToolTipsShowLabelRating(bool val);
    bool getToolTipsShowLabelRating() const;

private:

    ShowfotoSettings();
    ~ShowfotoSettings();

    void init();

private:

    class Private;
    Private* const d;

    friend class ShowfotoSettingsCreator;
};

} // namespace ShowFoto

#endif // SHOWFOTOSETTINGS_H
