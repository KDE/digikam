#ifndef DBJOB_H
#define DBJOB_H

#include "KDCRAW/RActionJob"

using namespace KDcrawIface;

namespace Digikam {

class DBJob : public RActionJob
{
    Q_OBJECT

public:

    DBJob();
    ~DBJob();

Q_SIGNALS:

    void data(const QByteArray &ba);
};

}

#endif // DBJOB_H
