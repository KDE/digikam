// local includes

#include "twittermpform.h"

// Qt includes

#include <QFile>

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

TwMPForm::TwMPForm()
{
}

TwMPForm::~TwMPForm()
{
}

bool TwMPForm::addFile(const QString& imgPath)
{
    QFile file(imgPath);

    if (!file.open(QIODevice::ReadOnly))
    {
        return false;
    }

    m_buffer = file.readAll();

    return true;
}

QByteArray TwMPForm::formData() const
{
    return m_buffer;
}

} // namespace Digikam
