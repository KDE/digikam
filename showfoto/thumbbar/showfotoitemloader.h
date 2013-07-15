#ifndef SHOWFOTOITEMLOADER_H
#define SHOWFOTOITEMLOADER_H

// Qt includes

#include <QThread>
#include <QString>
#include <QFileInfo>
#include <QCustomEvent>
#include <QPixmap>
#include <QObject>
#include <QFileInfo>

#include "kurl.h"

// Local includes
#include "showfotoiteminfo.h"

namespace ShowFoto {


class ShowfotoItemLoader : public QObject
{
    Q_OBJECT
public:
    ShowfotoItemLoader();
    ~ShowfotoItemLoader();

    void openFolder(const KUrl& url);
    void openFile(const KUrl& url);


Q_SIGNALS:
    void signalNoCurrentItem();
    void signalSorry();
    void signalToggleNav(int i);
    void signalToggleAction(bool);
    void signalSetSelceted(KUrl url);
    void signalSetCurrentItem();
    void signalNewThumbItem(KUrl url);
    void signalLastDirectory(KUrl::List::const_iterator);
    void signalInfoList(ShowfotoItemInfoList&);

public Q_SLOTS:
    void slotLoadCurrentItem(const KUrl::List& urlList);
    void slotOpenFolder(const KUrl& url);
    void slotOpenFile(const KUrl::List &urls);

private:

    class Private;
    Private* const d;
};
} // namespace Showfoto
#endif // ShowfotoItemLoader_H
