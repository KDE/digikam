#ifndef DBJOBSMANAGER_H
#define DBJOBSMANAGER_H

#include <QObject>
#include "dbjobinfo.h"

namespace Digikam {

class DBJobsManager : public QObject
{
    Q_OBJECT
public:
    explicit DBJobsManager(DBJobInfo &jobInfo);
    ~DBJobsManager();

    void start();
    void cancel();

Q_SIGNALS:

public Q_SLOTS:
    void data(const QByteArray &ba);

private:
    class Private;
    Private *const d;
};

} // namespace Digikam

#endif // DBJOBSMANAGER_H
