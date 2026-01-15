/****************************************************************************
** Meta object code from reading C++ file 'FontFinder.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.16)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../s/FontFinder.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'FontFinder.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.16. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_FontFinder_t {
    QByteArrayData data[12];
    char stringdata0[169];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_FontFinder_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_FontFinder_t qt_meta_stringdata_FontFinder = {
    {
QT_MOC_LITERAL(0, 0, 10), // "FontFinder"
QT_MOC_LITERAL(1, 11, 19), // "browseFontDirectory"
QT_MOC_LITERAL(2, 31, 0), // ""
QT_MOC_LITERAL(3, 32, 14), // "selectFontFile"
QT_MOC_LITERAL(4, 47, 11), // "searchFonts"
QT_MOC_LITERAL(5, 59, 17), // "openFontDirectory"
QT_MOC_LITERAL(6, 77, 23), // "onFontItemDoubleClicked"
QT_MOC_LITERAL(7, 101, 16), // "QListWidgetItem*"
QT_MOC_LITERAL(8, 118, 4), // "item"
QT_MOC_LITERAL(9, 123, 17), // "onFontItemClicked"
QT_MOC_LITERAL(10, 141, 18), // "showFontProperties"
QT_MOC_LITERAL(11, 160, 8) // "filePath"

    },
    "FontFinder\0browseFontDirectory\0\0"
    "selectFontFile\0searchFonts\0openFontDirectory\0"
    "onFontItemDoubleClicked\0QListWidgetItem*\0"
    "item\0onFontItemClicked\0showFontProperties\0"
    "filePath"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_FontFinder[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   49,    2, 0x08 /* Private */,
       3,    0,   50,    2, 0x08 /* Private */,
       4,    0,   51,    2, 0x08 /* Private */,
       5,    0,   52,    2, 0x08 /* Private */,
       6,    1,   53,    2, 0x08 /* Private */,
       9,    1,   56,    2, 0x08 /* Private */,
      10,    1,   59,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 7,    8,
    QMetaType::Void, 0x80000000 | 7,    8,
    QMetaType::Void, QMetaType::QString,   11,

       0        // eod
};

void FontFinder::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<FontFinder *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->browseFontDirectory(); break;
        case 1: _t->selectFontFile(); break;
        case 2: _t->searchFonts(); break;
        case 3: _t->openFontDirectory(); break;
        case 4: _t->onFontItemDoubleClicked((*reinterpret_cast< QListWidgetItem*(*)>(_a[1]))); break;
        case 5: _t->onFontItemClicked((*reinterpret_cast< QListWidgetItem*(*)>(_a[1]))); break;
        case 6: _t->showFontProperties((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject FontFinder::staticMetaObject = { {
    QMetaObject::SuperData::link<QDialog::staticMetaObject>(),
    qt_meta_stringdata_FontFinder.data,
    qt_meta_data_FontFinder,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *FontFinder::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *FontFinder::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_FontFinder.stringdata0))
        return static_cast<void*>(this);
    return QDialog::qt_metacast(_clname);
}

int FontFinder::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 7)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 7;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
