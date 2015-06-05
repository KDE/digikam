#ifndef DATESJOB_H
#define DATESJOB_H

#include "KDCRAW/RActionJob"

using namespace KDcrawIface;

namespace Digikam {

class DatesJob : public RActionJob
{
    Q_OBJECT

public:

    DatesJob();
    ~DatesJob();

    void setData(const QByteArray &data);

public Q_SLOTS:

    void slotCancel();
    void slotData(const QByteArray &data);

protected:

    void run();

private:

    class Private;
    Private *const d;
};

} // namespace Digikam

#endif // DATESJOB_H
