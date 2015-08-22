#ifndef METADATAHUBMNGR_H
#define METADATAHUBMNGR_H

#include <QPointer>


namespace Digikam
{

class ImageInfo;
class MetadataHubMngr : public QObject
{
    Q_OBJECT
public:
    static MetadataHubMngr* instance();
    ~MetadataHubMngr();

    static QPointer<MetadataHubMngr> internalPtr;

    void addPending(ImageInfo& info);

public Q_SLOTS:
    void slotApplyPending();

private:
    MetadataHubMngr();

    class Private;
    Private* d;

};
}

#endif // METADATAHUBMNGR_H
