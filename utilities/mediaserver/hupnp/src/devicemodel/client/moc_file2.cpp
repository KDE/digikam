/****************************************************************************
** Meta object code from reading C++ file 'hdefault_clientdevice_p.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.9.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "hdefault_clientdevice_p.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'hdefault_clientdevice_p.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.9.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_Herqq__Upnp__HDefaultClientDevice_t {
    QByteArrayData data[7];
    char stringdata0[104];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_Herqq__Upnp__HDefaultClientDevice_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_Herqq__Upnp__HDefaultClientDevice_t qt_meta_stringdata_Herqq__Upnp__HDefaultClientDevice = {
    {
QT_MOC_LITERAL(0, 0, 33), // "Herqq::Upnp::HDefaultClientDe..."
QT_MOC_LITERAL(1, 34, 13), // "statusTimeout"
QT_MOC_LITERAL(2, 48, 0), // ""
QT_MOC_LITERAL(3, 49, 21), // "HDefaultClientDevice*"
QT_MOC_LITERAL(4, 71, 6), // "source"
QT_MOC_LITERAL(5, 78, 16), // "locationsChanged"
QT_MOC_LITERAL(6, 95, 8) // "timeout_"

    },
    "Herqq::Upnp::HDefaultClientDevice\0"
    "statusTimeout\0\0HDefaultClientDevice*\0"
    "source\0locationsChanged\0timeout_"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Herqq__Upnp__HDefaultClientDevice[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   29,    2, 0x06 /* Public */,
       5,    0,   32,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       6,    0,   33,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void,

       0        // eod
};

void Herqq::Upnp::HDefaultClientDevice::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        HDefaultClientDevice *_t = static_cast<HDefaultClientDevice *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->statusTimeout((*reinterpret_cast< HDefaultClientDevice*(*)>(_a[1]))); break;
        case 1: _t->locationsChanged(); break;
        case 2: _t->timeout_(); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 0:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< HDefaultClientDevice* >(); break;
            }
            break;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (HDefaultClientDevice::*_t)(HDefaultClientDevice * );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&HDefaultClientDevice::statusTimeout)) {
                *result = 0;
                return;
            }
        }
        {
            typedef void (HDefaultClientDevice::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&HDefaultClientDevice::locationsChanged)) {
                *result = 1;
                return;
            }
        }
    }
}

const QMetaObject Herqq::Upnp::HDefaultClientDevice::staticMetaObject = {
    { &HClientDevice::staticMetaObject, qt_meta_stringdata_Herqq__Upnp__HDefaultClientDevice.data,
      qt_meta_data_Herqq__Upnp__HDefaultClientDevice,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *Herqq::Upnp::HDefaultClientDevice::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Herqq::Upnp::HDefaultClientDevice::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_Herqq__Upnp__HDefaultClientDevice.stringdata0))
        return static_cast<void*>(const_cast< HDefaultClientDevice*>(this));
    return HClientDevice::qt_metacast(_clname);
}

int Herqq::Upnp::HDefaultClientDevice::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = HClientDevice::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    }
    return _id;
}

// SIGNAL 0
void Herqq::Upnp::HDefaultClientDevice::statusTimeout(HDefaultClientDevice * _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void Herqq::Upnp::HDefaultClientDevice::locationsChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
