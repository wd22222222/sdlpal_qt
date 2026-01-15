/****************************************************************************
** Meta object code from reading C++ file 'ctableview.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.16)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../s/ctableview.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ctableview.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.16. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_CTableView_t {
    QByteArrayData data[9];
    char stringdata0[83];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CTableView_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CTableView_t qt_meta_stringdata_CTableView = {
    {
QT_MOC_LITERAL(0, 0, 10), // "CTableView"
QT_MOC_LITERAL(1, 11, 12), // "SendMessages"
QT_MOC_LITERAL(2, 24, 0), // ""
QT_MOC_LITERAL(3, 25, 3), // "msg"
QT_MOC_LITERAL(4, 29, 15), // "ListScriptEntry"
QT_MOC_LITERAL(5, 45, 6), // "script"
QT_MOC_LITERAL(6, 52, 14), // "ListScriptCall"
QT_MOC_LITERAL(7, 67, 9), // "deployMap"
QT_MOC_LITERAL(8, 77, 5) // "scene"

    },
    "CTableView\0SendMessages\0\0msg\0"
    "ListScriptEntry\0script\0ListScriptCall\0"
    "deployMap\0scene"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CTableView[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   34,    2, 0x06 /* Public */,
       4,    1,   37,    2, 0x06 /* Public */,
       6,    1,   40,    2, 0x06 /* Public */,
       7,    1,   43,    2, 0x06 /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void, QMetaType::Int,    5,
    QMetaType::Void, QMetaType::Int,    5,
    QMetaType::Void, QMetaType::Int,    8,

       0        // eod
};

void CTableView::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<CTableView *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->SendMessages((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 1: _t->ListScriptEntry((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->ListScriptCall((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->deployMap((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (CTableView::*)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CTableView::SendMessages)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (CTableView::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CTableView::ListScriptEntry)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (CTableView::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CTableView::ListScriptCall)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (CTableView::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CTableView::deployMap)) {
                *result = 3;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject CTableView::staticMetaObject = { {
    QMetaObject::SuperData::link<QTableView::staticMetaObject>(),
    qt_meta_stringdata_CTableView.data,
    qt_meta_data_CTableView,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *CTableView::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CTableView::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CTableView.stringdata0))
        return static_cast<void*>(this);
    return QTableView::qt_metacast(_clname);
}

int CTableView::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QTableView::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 4)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void CTableView::SendMessages(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void CTableView::ListScriptEntry(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void CTableView::ListScriptCall(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void CTableView::deployMap(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
