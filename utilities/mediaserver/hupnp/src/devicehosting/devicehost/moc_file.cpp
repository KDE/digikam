/****************************************************************************
** Meta object code from reading C++ file 'hserverdevicecontroller_p.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.9.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "hserverdevicecontroller_p.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'hserverdevicecontroller_p.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.9.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_Herqq__Upnp__HServerDeviceController_t {
    QByteArrayData data[6];
    char stringdata0[93];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_Herqq__Upnp__HServerDeviceController_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_Herqq__Upnp__HServerDeviceController_t qt_meta_stringdata_Herqq__Upnp__HServerDeviceController = {
    {
QT_MOC_LITERAL(0, 0, 36), // "Herqq::Upnp::HServerDeviceCon..."
QT_MOC_LITERAL(1, 37, 13), // "statusTimeout"
QT_MOC_LITERAL(2, 51, 0), // ""
QT_MOC_LITERAL(3, 52, 24), // "HServerDeviceController*"
QT_MOC_LITERAL(4, 77, 6), // "source"
QT_MOC_LITERAL(5, 84, 8) // "timeout_"

    },
    "Herqq::Upnp::HServerDeviceController\0"
    "statusTimeout\0\0HServerDeviceController*\0"
    "source\0timeout_"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Herqq__Upnp__HServerDeviceController[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   24,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       5,    0,   27,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    4,

 // slots: parameters
    QMetaType::Void,

       0        // eod
};

void Herqq::Upnp::HServerDeviceController::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        HServerDeviceController *_t = static_cast<HServerDeviceController *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->statusTimeout((*reinterpret_cast< HServerDeviceController*(*)>(_a[1]))); break;
        case 1: _t->timeout_(); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 0:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< HServerDeviceController* >(); break;
            }
            break;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (HServerDeviceController::*_t)(HServerDeviceController * );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&HServerDeviceController::statusTimeout)) {
                *result = 0;
                return;
            }
        }
    }
}

const QMetaObject Herqq::Upnp::HServerDeviceController::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_Herqq__Upnp__HServerDeviceController.data,
      qt_meta_data_Herqq__Upnp__HServerDeviceController,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *Herqq::Upnp::HServerDeviceController::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Herqq::Upnp::HServerDeviceController::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_Herqq__Upnp__HServerDeviceController.stringdata0))
        return static_cast<void*>(const_cast< HServerDeviceController*>(this));
    return QObject::qt_metacast(_clname);
}

int Herqq::Upnp::HServerDeviceController::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    }
    return _id;
}

// SIGNAL 0
void Herqq::Upnp::HServerDeviceController::statusTimeout(HServerDeviceController * _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
