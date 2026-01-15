/****************************************************************************
** Meta object code from reading C++ file 'CustomItemDelegrate.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.16)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../s/CustomItemDelegrate.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'CustomItemDelegrate.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.16. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_CustomItemDelegrate_t {
    QByteArrayData data[9];
    char stringdata0[96];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CustomItemDelegrate_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CustomItemDelegrate_t qt_meta_stringdata_CustomItemDelegrate = {
    {
QT_MOC_LITERAL(0, 0, 19), // "CustomItemDelegrate"
QT_MOC_LITERAL(1, 20, 12), // "SendMessages"
QT_MOC_LITERAL(2, 33, 0), // ""
QT_MOC_LITERAL(3, 34, 3), // "msg"
QT_MOC_LITERAL(4, 38, 15), // "editorFocusLost"
QT_MOC_LITERAL(5, 54, 11), // "QModelIndex"
QT_MOC_LITERAL(6, 66, 5), // "index"
QT_MOC_LITERAL(7, 72, 4), // "data"
QT_MOC_LITERAL(8, 77, 18) // "editorAboutToClose"

    },
    "CustomItemDelegrate\0SendMessages\0\0msg\0"
    "editorFocusLost\0QModelIndex\0index\0"
    "data\0editorAboutToClose"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CustomItemDelegrate[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   29,    2, 0x06 /* Public */,
       4,    2,   32,    2, 0x06 /* Public */,
       8,    1,   37,    2, 0x06 /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void, 0x80000000 | 5, QMetaType::QVariant,    6,    7,
    QMetaType::Void, 0x80000000 | 5,    6,

       0        // eod
};

void CustomItemDelegrate::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<CustomItemDelegrate *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->SendMessages((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 1: _t->editorFocusLost((*reinterpret_cast< const QModelIndex(*)>(_a[1])),(*reinterpret_cast< const QVariant(*)>(_a[2]))); break;
        case 2: _t->editorAboutToClose((*reinterpret_cast< const QModelIndex(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (CustomItemDelegrate::*)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CustomItemDelegrate::SendMessages)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (CustomItemDelegrate::*)(const QModelIndex & , const QVariant & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CustomItemDelegrate::editorFocusLost)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (CustomItemDelegrate::*)(const QModelIndex & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CustomItemDelegrate::editorAboutToClose)) {
                *result = 2;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject CustomItemDelegrate::staticMetaObject = { {
    QMetaObject::SuperData::link<QStyledItemDelegate::staticMetaObject>(),
    qt_meta_stringdata_CustomItemDelegrate.data,
    qt_meta_data_CustomItemDelegrate,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *CustomItemDelegrate::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CustomItemDelegrate::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CustomItemDelegrate.stringdata0))
        return static_cast<void*>(this);
    return QStyledItemDelegate::qt_metacast(_clname);
}

int CustomItemDelegrate::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QStyledItemDelegate::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 3)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 3;
    }
    return _id;
}

// SIGNAL 0
void CustomItemDelegrate::SendMessages(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void CustomItemDelegrate::editorFocusLost(const QModelIndex & _t1, const QVariant & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void CustomItemDelegrate::editorAboutToClose(const QModelIndex & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
