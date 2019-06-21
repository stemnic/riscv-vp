/****************************************************************************
** Meta object code from reading C++ file 'wrapper.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.11.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../src/wrapper.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'wrapper.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.11.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_NlviewMinimap_t {
    QByteArrayData data[9];
    char stringdata0[86];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_NlviewMinimap_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_NlviewMinimap_t qt_meta_stringdata_NlviewMinimap = {
    {
QT_MOC_LITERAL(0, 0, 13), // "NlviewMinimap"
QT_MOC_LITERAL(1, 14, 6), // "cancel"
QT_MOC_LITERAL(2, 21, 0), // ""
QT_MOC_LITERAL(3, 22, 11), // "forceUpdate"
QT_MOC_LITERAL(4, 34, 15), // "viewportChanged"
QT_MOC_LITERAL(5, 50, 10), // "mapChanged"
QT_MOC_LITERAL(6, 61, 7), // "scrollr"
QT_MOC_LITERAL(7, 69, 8), // "hiselvis"
QT_MOC_LITERAL(8, 78, 7) // "vpcolor"

    },
    "NlviewMinimap\0cancel\0\0forceUpdate\0"
    "viewportChanged\0mapChanged\0scrollr\0"
    "hiselvis\0vpcolor"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_NlviewMinimap[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       2,   44, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   34,    2, 0x0a /* Public */,
       3,    0,   35,    2, 0x0a /* Public */,
       4,    2,   36,    2, 0x08 /* Private */,
       5,    1,   41,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QRect, QMetaType::QPoint,    2,    2,
    QMetaType::Void, QMetaType::QRect,    6,

 // properties: name, type, flags
       7, QMetaType::Bool, 0x00095003,
       8, QMetaType::QColor, 0x00095003,

       0        // eod
};

void NlviewMinimap::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        NlviewMinimap *_t = static_cast<NlviewMinimap *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->cancel(); break;
        case 1: _t->forceUpdate(); break;
        case 2: _t->viewportChanged((*reinterpret_cast< const QRect(*)>(_a[1])),(*reinterpret_cast< const QPoint(*)>(_a[2]))); break;
        case 3: _t->mapChanged((*reinterpret_cast< const QRect(*)>(_a[1]))); break;
        default: ;
        }
    }
#ifndef QT_NO_PROPERTIES
    else if (_c == QMetaObject::ReadProperty) {
        NlviewMinimap *_t = static_cast<NlviewMinimap *>(_o);
        Q_UNUSED(_t)
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< bool*>(_v) = _t->getHiSelVis(); break;
        case 1: *reinterpret_cast< QColor*>(_v) = _t->getVportColor(); break;
        default: break;
        }
    } else if (_c == QMetaObject::WriteProperty) {
        NlviewMinimap *_t = static_cast<NlviewMinimap *>(_o);
        Q_UNUSED(_t)
        void *_v = _a[0];
        switch (_id) {
        case 0: _t->setHiSelVis(*reinterpret_cast< bool*>(_v)); break;
        case 1: _t->setVportColor(*reinterpret_cast< QColor*>(_v)); break;
        default: break;
        }
    } else if (_c == QMetaObject::ResetProperty) {
    }
#endif // QT_NO_PROPERTIES
}

QT_INIT_METAOBJECT const QMetaObject NlviewMinimap::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_NlviewMinimap.data,
      qt_meta_data_NlviewMinimap,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *NlviewMinimap::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *NlviewMinimap::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_NlviewMinimap.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int NlviewMinimap::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
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
#ifndef QT_NO_PROPERTIES
   else if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    } else if (_c == QMetaObject::QueryPropertyDesignable) {
        _id -= 2;
    } else if (_c == QMetaObject::QueryPropertyScriptable) {
        _id -= 2;
    } else if (_c == QMetaObject::QueryPropertyStored) {
        _id -= 2;
    } else if (_c == QMetaObject::QueryPropertyEditable) {
        _id -= 2;
    } else if (_c == QMetaObject::QueryPropertyUser) {
        _id -= 2;
    }
#endif // QT_NO_PROPERTIES
    return _id;
}
struct qt_meta_stringdata_NlvQWidget_t {
    QByteArrayData data[206];
    char stringdata0[2414];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_NlvQWidget_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_NlvQWidget_t qt_meta_stringdata_NlvQWidget = {
    {
QT_MOC_LITERAL(0, 0, 10), // "NlvQWidget"
QT_MOC_LITERAL(1, 11, 10), // "pageNotify"
QT_MOC_LITERAL(2, 22, 0), // ""
QT_MOC_LITERAL(3, 23, 7), // "details"
QT_MOC_LITERAL(4, 31, 7), // "page_no"
QT_MOC_LITERAL(5, 39, 7), // "pagecnt"
QT_MOC_LITERAL(6, 47, 14), // "viewportNotify"
QT_MOC_LITERAL(7, 62, 4), // "left"
QT_MOC_LITERAL(8, 67, 3), // "top"
QT_MOC_LITERAL(9, 71, 5), // "right"
QT_MOC_LITERAL(10, 77, 3), // "bot"
QT_MOC_LITERAL(11, 81, 14), // "progressNotify"
QT_MOC_LITERAL(12, 96, 3), // "cnt"
QT_MOC_LITERAL(13, 100, 7), // "percent"
QT_MOC_LITERAL(14, 108, 11), // "const char*"
QT_MOC_LITERAL(15, 120, 3), // "msg"
QT_MOC_LITERAL(16, 124, 15), // "selectionNotify"
QT_MOC_LITERAL(17, 140, 6), // "length"
QT_MOC_LITERAL(18, 147, 13), // "messageOutput"
QT_MOC_LITERAL(19, 161, 2), // "id"
QT_MOC_LITERAL(20, 164, 3), // "txt"
QT_MOC_LITERAL(21, 168, 13), // "hierarchyDown"
QT_MOC_LITERAL(22, 182, 8), // "instName"
QT_MOC_LITERAL(23, 191, 11), // "hierarchyUp"
QT_MOC_LITERAL(24, 203, 12), // "bindCallback"
QT_MOC_LITERAL(25, 216, 11), // "bindingType"
QT_MOC_LITERAL(26, 228, 5), // "downX"
QT_MOC_LITERAL(27, 234, 5), // "downY"
QT_MOC_LITERAL(28, 240, 3), // "upX"
QT_MOC_LITERAL(29, 244, 3), // "upY"
QT_MOC_LITERAL(30, 248, 6), // "wdelta"
QT_MOC_LITERAL(31, 255, 3), // "oid"
QT_MOC_LITERAL(32, 259, 10), // "mapChanged"
QT_MOC_LITERAL(33, 270, 7), // "scrollr"
QT_MOC_LITERAL(34, 278, 15), // "viewportChanged"
QT_MOC_LITERAL(35, 294, 8), // "viewport"
QT_MOC_LITERAL(36, 303, 6), // "origin"
QT_MOC_LITERAL(37, 310, 15), // "imageMapChanged"
QT_MOC_LITERAL(38, 326, 6), // "cancel"
QT_MOC_LITERAL(39, 333, 17), // "hSBarValueChanged"
QT_MOC_LITERAL(40, 351, 17), // "vSBarValueChanged"
QT_MOC_LITERAL(41, 369, 19), // "imageMapChangedSlot"
QT_MOC_LITERAL(42, 389, 7), // "timeout"
QT_MOC_LITERAL(43, 397, 7), // "logfile"
QT_MOC_LITERAL(44, 405, 10), // "Actionpick"
QT_MOC_LITERAL(45, 416, 11), // "Allpageconn"
QT_MOC_LITERAL(46, 428, 12), // "Analoglayout"
QT_MOC_LITERAL(47, 441, 9), // "Attrcolor"
QT_MOC_LITERAL(48, 451, 12), // "Attrfontsize"
QT_MOC_LITERAL(49, 464, 10), // "Autobundle"
QT_MOC_LITERAL(50, 475, 13), // "Autohidestyle"
QT_MOC_LITERAL(51, 489, 15), // "Backgroundcolor"
QT_MOC_LITERAL(52, 505, 9), // "Boxcolor0"
QT_MOC_LITERAL(53, 515, 9), // "Boxcolor1"
QT_MOC_LITERAL(54, 525, 9), // "Boxcolor2"
QT_MOC_LITERAL(55, 535, 9), // "Boxcolor3"
QT_MOC_LITERAL(56, 545, 11), // "Boxfontsize"
QT_MOC_LITERAL(57, 557, 11), // "Boxhierpins"
QT_MOC_LITERAL(58, 569, 12), // "Boxinstcolor"
QT_MOC_LITERAL(59, 582, 15), // "Boxinstfontsize"
QT_MOC_LITERAL(60, 598, 11), // "Boxmaxwidth"
QT_MOC_LITERAL(61, 610, 9), // "Boxmingap"
QT_MOC_LITERAL(62, 620, 12), // "Boxminheight"
QT_MOC_LITERAL(63, 633, 11), // "Boxminwidth"
QT_MOC_LITERAL(64, 645, 11), // "Boxpincolor"
QT_MOC_LITERAL(65, 657, 14), // "Boxpinfontsize"
QT_MOC_LITERAL(66, 672, 10), // "Boxpingrid"
QT_MOC_LITERAL(67, 683, 12), // "Boxpinsquare"
QT_MOC_LITERAL(68, 696, 8), // "Buscolor"
QT_MOC_LITERAL(69, 705, 13), // "Buswidthlimit"
QT_MOC_LITERAL(70, 719, 13), // "Buswireexcess"
QT_MOC_LITERAL(71, 733, 11), // "Closeenough"
QT_MOC_LITERAL(72, 745, 16), // "Createnetattrdsp"
QT_MOC_LITERAL(73, 762, 11), // "Createvconn"
QT_MOC_LITERAL(74, 774, 12), // "Createvconn2"
QT_MOC_LITERAL(75, 787, 8), // "Decorate"
QT_MOC_LITERAL(76, 796, 9), // "Elidetext"
QT_MOC_LITERAL(77, 806, 11), // "Fastpanning"
QT_MOC_LITERAL(78, 818, 10), // "Fillcolor1"
QT_MOC_LITERAL(79, 829, 10), // "Fillcolor2"
QT_MOC_LITERAL(80, 840, 10), // "Fillcolor3"
QT_MOC_LITERAL(81, 851, 13), // "Fillednegpins"
QT_MOC_LITERAL(82, 865, 7), // "Fitpage"
QT_MOC_LITERAL(83, 873, 14), // "Flyingsegments"
QT_MOC_LITERAL(84, 888, 10), // "Framecolor"
QT_MOC_LITERAL(85, 899, 13), // "Framefontsize"
QT_MOC_LITERAL(86, 913, 12), // "Gatecellname"
QT_MOC_LITERAL(87, 926, 11), // "Gatepinname"
QT_MOC_LITERAL(88, 938, 9), // "Geitarget"
QT_MOC_LITERAL(89, 948, 8), // "Greymode"
QT_MOC_LITERAL(90, 957, 4), // "Grid"
QT_MOC_LITERAL(91, 962, 9), // "Gridcolor"
QT_MOC_LITERAL(92, 972, 15), // "Groupregionpins"
QT_MOC_LITERAL(93, 988, 11), // "Hiattrvalue"
QT_MOC_LITERAL(94, 1000, 16), // "Horizontallabels"
QT_MOC_LITERAL(95, 1017, 11), // "Instattrmax"
QT_MOC_LITERAL(96, 1029, 8), // "Instdrag"
QT_MOC_LITERAL(97, 1038, 9), // "Instorder"
QT_MOC_LITERAL(98, 1048, 7), // "Ioorder"
QT_MOC_LITERAL(99, 1056, 12), // "Latchfblevel"
QT_MOC_LITERAL(100, 1069, 11), // "Mapbool2pla"
QT_MOC_LITERAL(101, 1081, 7), // "Markgap"
QT_MOC_LITERAL(102, 1089, 8), // "Marksize"
QT_MOC_LITERAL(103, 1098, 15), // "Matrixalignment"
QT_MOC_LITERAL(104, 1114, 11), // "Maxfontsize"
QT_MOC_LITERAL(105, 1126, 13), // "Maxfontsizecg"
QT_MOC_LITERAL(106, 1140, 7), // "Maxzoom"
QT_MOC_LITERAL(107, 1148, 11), // "Mergepgnets"
QT_MOC_LITERAL(108, 1160, 11), // "Mergetracks"
QT_MOC_LITERAL(109, 1172, 15), // "Minchannelwidth"
QT_MOC_LITERAL(110, 1188, 13), // "Minlevelwidth"
QT_MOC_LITERAL(111, 1202, 10), // "Netattrmax"
QT_MOC_LITERAL(112, 1213, 11), // "Netautohide"
QT_MOC_LITERAL(113, 1225, 8), // "Netcolor"
QT_MOC_LITERAL(114, 1234, 11), // "Netfontsize"
QT_MOC_LITERAL(115, 1246, 14), // "Nethidedetails"
QT_MOC_LITERAL(116, 1261, 13), // "Netstubminlen"
QT_MOC_LITERAL(117, 1275, 11), // "Nohierattrs"
QT_MOC_LITERAL(118, 1287, 13), // "Nohiernegpins"
QT_MOC_LITERAL(119, 1301, 10), // "Objectgrey"
QT_MOC_LITERAL(120, 1312, 16), // "Objecthighlight0"
QT_MOC_LITERAL(121, 1329, 16), // "Objecthighlight1"
QT_MOC_LITERAL(122, 1346, 16), // "Objecthighlight2"
QT_MOC_LITERAL(123, 1363, 16), // "Objecthighlight3"
QT_MOC_LITERAL(124, 1380, 16), // "Objecthighlight4"
QT_MOC_LITERAL(125, 1397, 16), // "Objecthighlight5"
QT_MOC_LITERAL(126, 1414, 16), // "Objecthighlight6"
QT_MOC_LITERAL(127, 1431, 16), // "Objecthighlight7"
QT_MOC_LITERAL(128, 1448, 16), // "Objecthighlight8"
QT_MOC_LITERAL(129, 1465, 16), // "Objecthighlight9"
QT_MOC_LITERAL(130, 1482, 17), // "Objecthighlight10"
QT_MOC_LITERAL(131, 1500, 17), // "Objecthighlight11"
QT_MOC_LITERAL(132, 1518, 17), // "Objecthighlight12"
QT_MOC_LITERAL(133, 1536, 17), // "Objecthighlight13"
QT_MOC_LITERAL(134, 1554, 17), // "Objecthighlight14"
QT_MOC_LITERAL(135, 1572, 17), // "Objecthighlight15"
QT_MOC_LITERAL(136, 1590, 17), // "Objecthighlight16"
QT_MOC_LITERAL(137, 1608, 17), // "Objecthighlight17"
QT_MOC_LITERAL(138, 1626, 17), // "Objecthighlight18"
QT_MOC_LITERAL(139, 1644, 17), // "Objecthighlight19"
QT_MOC_LITERAL(140, 1662, 6), // "Ongrid"
QT_MOC_LITERAL(141, 1669, 10), // "Outfblevel"
QT_MOC_LITERAL(142, 1680, 12), // "Overlapcolor"
QT_MOC_LITERAL(143, 1693, 9), // "Pbuscolor"
QT_MOC_LITERAL(144, 1703, 13), // "Pbusnamecolor"
QT_MOC_LITERAL(145, 1717, 10), // "Pinattrmax"
QT_MOC_LITERAL(146, 1728, 11), // "Pinautohide"
QT_MOC_LITERAL(147, 1740, 8), // "Pinorder"
QT_MOC_LITERAL(148, 1749, 10), // "Pinpermute"
QT_MOC_LITERAL(149, 1760, 11), // "Portattrmax"
QT_MOC_LITERAL(150, 1772, 9), // "Portcolor"
QT_MOC_LITERAL(151, 1782, 13), // "Portnamecolor"
QT_MOC_LITERAL(152, 1796, 10), // "Reducejogs"
QT_MOC_LITERAL(153, 1807, 10), // "Ripattrmax"
QT_MOC_LITERAL(154, 1818, 16), // "Ripindexfontsize"
QT_MOC_LITERAL(155, 1835, 11), // "Rippercolor"
QT_MOC_LITERAL(156, 1847, 11), // "Rippershape"
QT_MOC_LITERAL(157, 1859, 15), // "Rubberbandcolor"
QT_MOC_LITERAL(158, 1875, 18), // "Rubberbandfontsize"
QT_MOC_LITERAL(159, 1894, 10), // "Selectattr"
QT_MOC_LITERAL(160, 1905, 19), // "Selectionappearance"
QT_MOC_LITERAL(161, 1925, 14), // "Selectioncolor"
QT_MOC_LITERAL(162, 1940, 11), // "Shadowstyle"
QT_MOC_LITERAL(163, 1952, 11), // "Sheetheight"
QT_MOC_LITERAL(164, 1964, 10), // "Sheetwidth"
QT_MOC_LITERAL(165, 1975, 10), // "Shortobjid"
QT_MOC_LITERAL(166, 1986, 13), // "Showattribute"
QT_MOC_LITERAL(167, 2000, 12), // "Showcellname"
QT_MOC_LITERAL(168, 2013, 9), // "Showframe"
QT_MOC_LITERAL(169, 2023, 8), // "Showgrid"
QT_MOC_LITERAL(170, 2032, 15), // "Showhierpinname"
QT_MOC_LITERAL(171, 2048, 12), // "Showinstname"
QT_MOC_LITERAL(172, 2061, 14), // "Showinvisibles"
QT_MOC_LITERAL(173, 2076, 10), // "Showlevels"
QT_MOC_LITERAL(174, 2087, 9), // "Showmarks"
QT_MOC_LITERAL(175, 2097, 11), // "Shownetattr"
QT_MOC_LITERAL(176, 2109, 11), // "Shownetname"
QT_MOC_LITERAL(177, 2121, 15), // "Showpagenumbers"
QT_MOC_LITERAL(178, 2137, 10), // "Showpgtype"
QT_MOC_LITERAL(179, 2148, 11), // "Showpinname"
QT_MOC_LITERAL(180, 2160, 12), // "Showportname"
QT_MOC_LITERAL(181, 2173, 12), // "Showripindex"
QT_MOC_LITERAL(182, 2186, 7), // "Showval"
QT_MOC_LITERAL(183, 2194, 9), // "Showvconn"
QT_MOC_LITERAL(184, 2204, 10), // "Showvconn2"
QT_MOC_LITERAL(185, 2215, 11), // "Stubattrmax"
QT_MOC_LITERAL(186, 2227, 9), // "Timelimit"
QT_MOC_LITERAL(187, 2237, 16), // "Transistorlayout"
QT_MOC_LITERAL(188, 2254, 7), // "Valattr"
QT_MOC_LITERAL(189, 2262, 16), // "Viewnameoptional"
QT_MOC_LITERAL(190, 2279, 9), // "PrintMode"
QT_MOC_LITERAL(191, 2289, 4), // "MONO"
QT_MOC_LITERAL(192, 2294, 5), // "COLOR"
QT_MOC_LITERAL(193, 2300, 8), // "COLORINV"
QT_MOC_LITERAL(194, 2309, 9), // "COLORINV2"
QT_MOC_LITERAL(195, 2319, 10), // "PrintScale"
QT_MOC_LITERAL(196, 2330, 4), // "FULL"
QT_MOC_LITERAL(197, 2335, 7), // "FULLFIT"
QT_MOC_LITERAL(198, 2343, 7), // "VISIBLE"
QT_MOC_LITERAL(199, 2351, 4), // "VIEW"
QT_MOC_LITERAL(200, 2356, 16), // "PrintOrientation"
QT_MOC_LITERAL(201, 2373, 4), // "AUTO"
QT_MOC_LITERAL(202, 2378, 9), // "LANDSCAPE"
QT_MOC_LITERAL(203, 2388, 8), // "PORTRAIT"
QT_MOC_LITERAL(204, 2397, 6), // "ROTATE"
QT_MOC_LITERAL(205, 2404, 9) // "NOTROTATE"

    },
    "NlvQWidget\0pageNotify\0\0details\0page_no\0"
    "pagecnt\0viewportNotify\0left\0top\0right\0"
    "bot\0progressNotify\0cnt\0percent\0"
    "const char*\0msg\0selectionNotify\0length\0"
    "messageOutput\0id\0txt\0hierarchyDown\0"
    "instName\0hierarchyUp\0bindCallback\0"
    "bindingType\0downX\0downY\0upX\0upY\0wdelta\0"
    "oid\0mapChanged\0scrollr\0viewportChanged\0"
    "viewport\0origin\0imageMapChanged\0cancel\0"
    "hSBarValueChanged\0vSBarValueChanged\0"
    "imageMapChangedSlot\0timeout\0logfile\0"
    "Actionpick\0Allpageconn\0Analoglayout\0"
    "Attrcolor\0Attrfontsize\0Autobundle\0"
    "Autohidestyle\0Backgroundcolor\0Boxcolor0\0"
    "Boxcolor1\0Boxcolor2\0Boxcolor3\0Boxfontsize\0"
    "Boxhierpins\0Boxinstcolor\0Boxinstfontsize\0"
    "Boxmaxwidth\0Boxmingap\0Boxminheight\0"
    "Boxminwidth\0Boxpincolor\0Boxpinfontsize\0"
    "Boxpingrid\0Boxpinsquare\0Buscolor\0"
    "Buswidthlimit\0Buswireexcess\0Closeenough\0"
    "Createnetattrdsp\0Createvconn\0Createvconn2\0"
    "Decorate\0Elidetext\0Fastpanning\0"
    "Fillcolor1\0Fillcolor2\0Fillcolor3\0"
    "Fillednegpins\0Fitpage\0Flyingsegments\0"
    "Framecolor\0Framefontsize\0Gatecellname\0"
    "Gatepinname\0Geitarget\0Greymode\0Grid\0"
    "Gridcolor\0Groupregionpins\0Hiattrvalue\0"
    "Horizontallabels\0Instattrmax\0Instdrag\0"
    "Instorder\0Ioorder\0Latchfblevel\0"
    "Mapbool2pla\0Markgap\0Marksize\0"
    "Matrixalignment\0Maxfontsize\0Maxfontsizecg\0"
    "Maxzoom\0Mergepgnets\0Mergetracks\0"
    "Minchannelwidth\0Minlevelwidth\0Netattrmax\0"
    "Netautohide\0Netcolor\0Netfontsize\0"
    "Nethidedetails\0Netstubminlen\0Nohierattrs\0"
    "Nohiernegpins\0Objectgrey\0Objecthighlight0\0"
    "Objecthighlight1\0Objecthighlight2\0"
    "Objecthighlight3\0Objecthighlight4\0"
    "Objecthighlight5\0Objecthighlight6\0"
    "Objecthighlight7\0Objecthighlight8\0"
    "Objecthighlight9\0Objecthighlight10\0"
    "Objecthighlight11\0Objecthighlight12\0"
    "Objecthighlight13\0Objecthighlight14\0"
    "Objecthighlight15\0Objecthighlight16\0"
    "Objecthighlight17\0Objecthighlight18\0"
    "Objecthighlight19\0Ongrid\0Outfblevel\0"
    "Overlapcolor\0Pbuscolor\0Pbusnamecolor\0"
    "Pinattrmax\0Pinautohide\0Pinorder\0"
    "Pinpermute\0Portattrmax\0Portcolor\0"
    "Portnamecolor\0Reducejogs\0Ripattrmax\0"
    "Ripindexfontsize\0Rippercolor\0Rippershape\0"
    "Rubberbandcolor\0Rubberbandfontsize\0"
    "Selectattr\0Selectionappearance\0"
    "Selectioncolor\0Shadowstyle\0Sheetheight\0"
    "Sheetwidth\0Shortobjid\0Showattribute\0"
    "Showcellname\0Showframe\0Showgrid\0"
    "Showhierpinname\0Showinstname\0"
    "Showinvisibles\0Showlevels\0Showmarks\0"
    "Shownetattr\0Shownetname\0Showpagenumbers\0"
    "Showpgtype\0Showpinname\0Showportname\0"
    "Showripindex\0Showval\0Showvconn\0"
    "Showvconn2\0Stubattrmax\0Timelimit\0"
    "Transistorlayout\0Valattr\0Viewnameoptional\0"
    "PrintMode\0MONO\0COLOR\0COLORINV\0COLORINV2\0"
    "PrintScale\0FULL\0FULLFIT\0VISIBLE\0VIEW\0"
    "PrintOrientation\0AUTO\0LANDSCAPE\0"
    "PORTRAIT\0ROTATE\0NOTROTATE"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_NlvQWidget[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
      16,   14, // methods
     147,  162, // properties
       3,  603, // enums/sets
       0,    0, // constructors
       0,       // flags
      11,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    3,   94,    2, 0x06 /* Public */,
       6,    4,  101,    2, 0x06 /* Public */,
      11,    3,  110,    2, 0x06 /* Public */,
      16,    1,  117,    2, 0x06 /* Public */,
      18,    2,  120,    2, 0x06 /* Public */,
      21,    1,  125,    2, 0x06 /* Public */,
      23,    0,  128,    2, 0x06 /* Public */,
      24,    7,  129,    2, 0x06 /* Public */,
      32,    1,  144,    2, 0x06 /* Public */,
      34,    2,  147,    2, 0x06 /* Public */,
      37,    0,  152,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      38,    0,  153,    2, 0x0a /* Public */,
      39,    1,  154,    2, 0x08 /* Private */,
      40,    1,  157,    2, 0x08 /* Private */,
      41,    0,  160,    2, 0x08 /* Private */,
      42,    0,  161,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, QMetaType::UInt, QMetaType::Int, QMetaType::Int,    3,    4,    5,
    QMetaType::Void, QMetaType::Long, QMetaType::Long, QMetaType::Long, QMetaType::Long,    7,    8,    9,   10,
    QMetaType::Void, QMetaType::Int, QMetaType::Double, 0x80000000 | 14,   12,   13,   15,
    QMetaType::Void, QMetaType::UInt,   17,
    QMetaType::Void, 0x80000000 | 14, 0x80000000 | 14,   19,   20,
    QMetaType::Void, 0x80000000 | 14,   22,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 14, QMetaType::Int, QMetaType::Int, QMetaType::Int, QMetaType::Int, QMetaType::Int, 0x80000000 | 14,   25,   26,   27,   28,   29,   30,   31,
    QMetaType::Void, QMetaType::QRect,   33,
    QMetaType::Void, QMetaType::QRect, QMetaType::QPoint,   35,   36,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void,
    QMetaType::Void,

 // properties: name, type, flags
      43, QMetaType::QString, 0x00095103,
      44, QMetaType::Int, 0x00095103,
      45, QMetaType::Bool, 0x00095103,
      46, QMetaType::Int, 0x00095103,
      47, QMetaType::QColor, 0x00095103,
      48, QMetaType::Int, 0x00095103,
      49, QMetaType::Int, 0x00095103,
      50, QMetaType::Int, 0x00095103,
      51, QMetaType::QColor, 0x00095103,
      52, QMetaType::QColor, 0x00095103,
      53, QMetaType::QColor, 0x00095103,
      54, QMetaType::QColor, 0x00095103,
      55, QMetaType::QColor, 0x00095103,
      56, QMetaType::Int, 0x00095103,
      57, QMetaType::Int, 0x00095103,
      58, QMetaType::QColor, 0x00095103,
      59, QMetaType::Int, 0x00095103,
      60, QMetaType::Int, 0x00095103,
      61, QMetaType::Int, 0x00095103,
      62, QMetaType::Int, 0x00095103,
      63, QMetaType::Int, 0x00095103,
      64, QMetaType::QColor, 0x00095103,
      65, QMetaType::Int, 0x00095103,
      66, QMetaType::Int, 0x00095103,
      67, QMetaType::Int, 0x00095103,
      68, QMetaType::QColor, 0x00095103,
      69, QMetaType::Int, 0x00095103,
      70, QMetaType::Int, 0x00095103,
      71, QMetaType::Int, 0x00095103,
      72, QMetaType::Int, 0x00095103,
      73, QMetaType::Int, 0x00095103,
      74, QMetaType::Int, 0x00095103,
      75, QMetaType::Int, 0x00095103,
      76, QMetaType::Int, 0x00095103,
      77, QMetaType::Bool, 0x00095103,
      78, QMetaType::QColor, 0x00095103,
      79, QMetaType::QColor, 0x00095103,
      80, QMetaType::QColor, 0x00095103,
      81, QMetaType::Bool, 0x00095103,
      82, QMetaType::Int, 0x00095103,
      83, QMetaType::Bool, 0x00095103,
      84, QMetaType::QColor, 0x00095103,
      85, QMetaType::Int, 0x00095103,
      86, QMetaType::Int, 0x00095103,
      87, QMetaType::Int, 0x00095103,
      88, QMetaType::Int, 0x00095103,
      89, QMetaType::Int, 0x00095103,
      90, QMetaType::Int, 0x00095103,
      91, QMetaType::QColor, 0x00095103,
      92, QMetaType::Bool, 0x00095103,
      93, QMetaType::Bool, 0x00095103,
      94, QMetaType::Bool, 0x00095103,
      95, QMetaType::Int, 0x00095103,
      96, QMetaType::Int, 0x00095103,
      97, QMetaType::Int, 0x00095103,
      98, QMetaType::Int, 0x00095103,
      99, QMetaType::Int, 0x00095103,
     100, QMetaType::Bool, 0x00095103,
     101, QMetaType::Int, 0x00095103,
     102, QMetaType::Int, 0x00095103,
     103, QMetaType::Int, 0x00095103,
     104, QMetaType::Double, 0x00095103,
     105, QMetaType::Double, 0x00095103,
     106, QMetaType::Double, 0x00095103,
     107, QMetaType::Bool, 0x00095103,
     108, QMetaType::Int, 0x00095103,
     109, QMetaType::Int, 0x00095103,
     110, QMetaType::Int, 0x00095103,
     111, QMetaType::Int, 0x00095103,
     112, QMetaType::Int, 0x00095103,
     113, QMetaType::QColor, 0x00095103,
     114, QMetaType::Int, 0x00095103,
     115, QMetaType::Int, 0x00095103,
     116, QMetaType::Int, 0x00095103,
     117, QMetaType::Bool, 0x00095103,
     118, QMetaType::Bool, 0x00095103,
     119, QMetaType::QColor, 0x00095103,
     120, QMetaType::QColor, 0x00095103,
     121, QMetaType::QColor, 0x00095103,
     122, QMetaType::QColor, 0x00095103,
     123, QMetaType::QColor, 0x00095103,
     124, QMetaType::QColor, 0x00095103,
     125, QMetaType::QColor, 0x00095103,
     126, QMetaType::QColor, 0x00095103,
     127, QMetaType::QColor, 0x00095103,
     128, QMetaType::QColor, 0x00095103,
     129, QMetaType::QColor, 0x00095103,
     130, QMetaType::QColor, 0x00095103,
     131, QMetaType::QColor, 0x00095103,
     132, QMetaType::QColor, 0x00095103,
     133, QMetaType::QColor, 0x00095103,
     134, QMetaType::QColor, 0x00095103,
     135, QMetaType::QColor, 0x00095103,
     136, QMetaType::QColor, 0x00095103,
     137, QMetaType::QColor, 0x00095103,
     138, QMetaType::QColor, 0x00095103,
     139, QMetaType::QColor, 0x00095103,
     140, QMetaType::Bool, 0x00095103,
     141, QMetaType::Int, 0x00095103,
     142, QMetaType::QColor, 0x00095103,
     143, QMetaType::QColor, 0x00095103,
     144, QMetaType::QColor, 0x00095103,
     145, QMetaType::Int, 0x00095103,
     146, QMetaType::Int, 0x00095103,
     147, QMetaType::Int, 0x00095103,
     148, QMetaType::Bool, 0x00095103,
     149, QMetaType::Int, 0x00095103,
     150, QMetaType::QColor, 0x00095103,
     151, QMetaType::QColor, 0x00095103,
     152, QMetaType::Int, 0x00095103,
     153, QMetaType::Int, 0x00095103,
     154, QMetaType::Int, 0x00095103,
     155, QMetaType::QColor, 0x00095103,
     156, QMetaType::Int, 0x00095103,
     157, QMetaType::QColor, 0x00095103,
     158, QMetaType::Int, 0x00095103,
     159, QMetaType::Int, 0x00095103,
     160, QMetaType::Int, 0x00095103,
     161, QMetaType::QColor, 0x00095103,
     162, QMetaType::Int, 0x00095103,
     163, QMetaType::Double, 0x00095103,
     164, QMetaType::Double, 0x00095103,
     165, QMetaType::Bool, 0x00095103,
     166, QMetaType::Int, 0x00095103,
     167, QMetaType::Bool, 0x00095103,
     168, QMetaType::Int, 0x00095103,
     169, QMetaType::Int, 0x00095103,
     170, QMetaType::Int, 0x00095103,
     171, QMetaType::Bool, 0x00095103,
     172, QMetaType::Bool, 0x00095103,
     173, QMetaType::Int, 0x00095103,
     174, QMetaType::Int, 0x00095103,
     175, QMetaType::Int, 0x00095103,
     176, QMetaType::Bool, 0x00095103,
     177, QMetaType::Int, 0x00095103,
     178, QMetaType::Int, 0x00095103,
     179, QMetaType::Int, 0x00095103,
     180, QMetaType::Bool, 0x00095103,
     181, QMetaType::Int, 0x00095103,
     182, QMetaType::Int, 0x00095103,
     183, QMetaType::Int, 0x00095103,
     184, QMetaType::Int, 0x00095103,
     185, QMetaType::Int, 0x00095103,
     186, QMetaType::Int, 0x00095103,
     187, QMetaType::Int, 0x00095103,
     188, QMetaType::Int, 0x00095103,
     189, QMetaType::Bool, 0x00095103,

 // enums: name, flags, count, data
     190, 0x0,    4,  615,
     195, 0x0,    4,  623,
     200, 0x0,    5,  631,

 // enum data: key, value
     191, uint(NlvQWidget::MONO),
     192, uint(NlvQWidget::COLOR),
     193, uint(NlvQWidget::COLORINV),
     194, uint(NlvQWidget::COLORINV2),
     196, uint(NlvQWidget::FULL),
     197, uint(NlvQWidget::FULLFIT),
     198, uint(NlvQWidget::VISIBLE),
     199, uint(NlvQWidget::VIEW),
     201, uint(NlvQWidget::AUTO),
     202, uint(NlvQWidget::LANDSCAPE),
     203, uint(NlvQWidget::PORTRAIT),
     204, uint(NlvQWidget::ROTATE),
     205, uint(NlvQWidget::NOTROTATE),

       0        // eod
};

void NlvQWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        NlvQWidget *_t = static_cast<NlvQWidget *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->pageNotify((*reinterpret_cast< uint(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3]))); break;
        case 1: _t->viewportNotify((*reinterpret_cast< long(*)>(_a[1])),(*reinterpret_cast< long(*)>(_a[2])),(*reinterpret_cast< long(*)>(_a[3])),(*reinterpret_cast< long(*)>(_a[4]))); break;
        case 2: _t->progressNotify((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2])),(*reinterpret_cast< const char*(*)>(_a[3]))); break;
        case 3: _t->selectionNotify((*reinterpret_cast< uint(*)>(_a[1]))); break;
        case 4: _t->messageOutput((*reinterpret_cast< const char*(*)>(_a[1])),(*reinterpret_cast< const char*(*)>(_a[2]))); break;
        case 5: _t->hierarchyDown((*reinterpret_cast< const char*(*)>(_a[1]))); break;
        case 6: _t->hierarchyUp(); break;
        case 7: _t->bindCallback((*reinterpret_cast< const char*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3])),(*reinterpret_cast< int(*)>(_a[4])),(*reinterpret_cast< int(*)>(_a[5])),(*reinterpret_cast< int(*)>(_a[6])),(*reinterpret_cast< const char*(*)>(_a[7]))); break;
        case 8: _t->mapChanged((*reinterpret_cast< const QRect(*)>(_a[1]))); break;
        case 9: _t->viewportChanged((*reinterpret_cast< const QRect(*)>(_a[1])),(*reinterpret_cast< const QPoint(*)>(_a[2]))); break;
        case 10: _t->imageMapChanged(); break;
        case 11: _t->cancel(); break;
        case 12: _t->hSBarValueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 13: _t->vSBarValueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 14: _t->imageMapChangedSlot(); break;
        case 15: _t->timeout(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (NlvQWidget::*)(unsigned  , int , int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&NlvQWidget::pageNotify)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (NlvQWidget::*)(long , long , long , long );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&NlvQWidget::viewportNotify)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (NlvQWidget::*)(int , double , const char * );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&NlvQWidget::progressNotify)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (NlvQWidget::*)(unsigned  );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&NlvQWidget::selectionNotify)) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (NlvQWidget::*)(const char * , const char * );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&NlvQWidget::messageOutput)) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (NlvQWidget::*)(const char * );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&NlvQWidget::hierarchyDown)) {
                *result = 5;
                return;
            }
        }
        {
            using _t = void (NlvQWidget::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&NlvQWidget::hierarchyUp)) {
                *result = 6;
                return;
            }
        }
        {
            using _t = void (NlvQWidget::*)(const char * , int , int , int , int , int , const char * );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&NlvQWidget::bindCallback)) {
                *result = 7;
                return;
            }
        }
        {
            using _t = void (NlvQWidget::*)(const QRect & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&NlvQWidget::mapChanged)) {
                *result = 8;
                return;
            }
        }
        {
            using _t = void (NlvQWidget::*)(const QRect & , const QPoint & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&NlvQWidget::viewportChanged)) {
                *result = 9;
                return;
            }
        }
        {
            using _t = void (NlvQWidget::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&NlvQWidget::imageMapChanged)) {
                *result = 10;
                return;
            }
        }
    }
#ifndef QT_NO_PROPERTIES
    else if (_c == QMetaObject::ReadProperty) {
        NlvQWidget *_t = static_cast<NlvQWidget *>(_o);
        Q_UNUSED(_t)
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< QString*>(_v) = _t->getLogfile(); break;
        case 1: *reinterpret_cast< int*>(_v) = _t->getActionpick(); break;
        case 2: *reinterpret_cast< bool*>(_v) = _t->getAllpageconn(); break;
        case 3: *reinterpret_cast< int*>(_v) = _t->getAnaloglayout(); break;
        case 4: *reinterpret_cast< QColor*>(_v) = _t->getAttrcolor(); break;
        case 5: *reinterpret_cast< int*>(_v) = _t->getAttrfontsize(); break;
        case 6: *reinterpret_cast< int*>(_v) = _t->getAutobundle(); break;
        case 7: *reinterpret_cast< int*>(_v) = _t->getAutohidestyle(); break;
        case 8: *reinterpret_cast< QColor*>(_v) = _t->getBackgroundcolor(); break;
        case 9: *reinterpret_cast< QColor*>(_v) = _t->getBoxcolor0(); break;
        case 10: *reinterpret_cast< QColor*>(_v) = _t->getBoxcolor1(); break;
        case 11: *reinterpret_cast< QColor*>(_v) = _t->getBoxcolor2(); break;
        case 12: *reinterpret_cast< QColor*>(_v) = _t->getBoxcolor3(); break;
        case 13: *reinterpret_cast< int*>(_v) = _t->getBoxfontsize(); break;
        case 14: *reinterpret_cast< int*>(_v) = _t->getBoxhierpins(); break;
        case 15: *reinterpret_cast< QColor*>(_v) = _t->getBoxinstcolor(); break;
        case 16: *reinterpret_cast< int*>(_v) = _t->getBoxinstfontsize(); break;
        case 17: *reinterpret_cast< int*>(_v) = _t->getBoxmaxwidth(); break;
        case 18: *reinterpret_cast< int*>(_v) = _t->getBoxmingap(); break;
        case 19: *reinterpret_cast< int*>(_v) = _t->getBoxminheight(); break;
        case 20: *reinterpret_cast< int*>(_v) = _t->getBoxminwidth(); break;
        case 21: *reinterpret_cast< QColor*>(_v) = _t->getBoxpincolor(); break;
        case 22: *reinterpret_cast< int*>(_v) = _t->getBoxpinfontsize(); break;
        case 23: *reinterpret_cast< int*>(_v) = _t->getBoxpingrid(); break;
        case 24: *reinterpret_cast< int*>(_v) = _t->getBoxpinsquare(); break;
        case 25: *reinterpret_cast< QColor*>(_v) = _t->getBuscolor(); break;
        case 26: *reinterpret_cast< int*>(_v) = _t->getBuswidthlimit(); break;
        case 27: *reinterpret_cast< int*>(_v) = _t->getBuswireexcess(); break;
        case 28: *reinterpret_cast< int*>(_v) = _t->getCloseenough(); break;
        case 29: *reinterpret_cast< int*>(_v) = _t->getCreatenetattrdsp(); break;
        case 30: *reinterpret_cast< int*>(_v) = _t->getCreatevconn(); break;
        case 31: *reinterpret_cast< int*>(_v) = _t->getCreatevconn2(); break;
        case 32: *reinterpret_cast< int*>(_v) = _t->getDecorate(); break;
        case 33: *reinterpret_cast< int*>(_v) = _t->getElidetext(); break;
        case 34: *reinterpret_cast< bool*>(_v) = _t->getFastpanning(); break;
        case 35: *reinterpret_cast< QColor*>(_v) = _t->getFillcolor1(); break;
        case 36: *reinterpret_cast< QColor*>(_v) = _t->getFillcolor2(); break;
        case 37: *reinterpret_cast< QColor*>(_v) = _t->getFillcolor3(); break;
        case 38: *reinterpret_cast< bool*>(_v) = _t->getFillednegpins(); break;
        case 39: *reinterpret_cast< int*>(_v) = _t->getFitpage(); break;
        case 40: *reinterpret_cast< bool*>(_v) = _t->getFlyingsegments(); break;
        case 41: *reinterpret_cast< QColor*>(_v) = _t->getFramecolor(); break;
        case 42: *reinterpret_cast< int*>(_v) = _t->getFramefontsize(); break;
        case 43: *reinterpret_cast< int*>(_v) = _t->getGatecellname(); break;
        case 44: *reinterpret_cast< int*>(_v) = _t->getGatepinname(); break;
        case 45: *reinterpret_cast< int*>(_v) = _t->getGeitarget(); break;
        case 46: *reinterpret_cast< int*>(_v) = _t->getGreymode(); break;
        case 47: *reinterpret_cast< int*>(_v) = _t->getGrid(); break;
        case 48: *reinterpret_cast< QColor*>(_v) = _t->getGridcolor(); break;
        case 49: *reinterpret_cast< bool*>(_v) = _t->getGroupregionpins(); break;
        case 50: *reinterpret_cast< bool*>(_v) = _t->getHiattrvalue(); break;
        case 51: *reinterpret_cast< bool*>(_v) = _t->getHorizontallabels(); break;
        case 52: *reinterpret_cast< int*>(_v) = _t->getInstattrmax(); break;
        case 53: *reinterpret_cast< int*>(_v) = _t->getInstdrag(); break;
        case 54: *reinterpret_cast< int*>(_v) = _t->getInstorder(); break;
        case 55: *reinterpret_cast< int*>(_v) = _t->getIoorder(); break;
        case 56: *reinterpret_cast< int*>(_v) = _t->getLatchfblevel(); break;
        case 57: *reinterpret_cast< bool*>(_v) = _t->getMapbool2pla(); break;
        case 58: *reinterpret_cast< int*>(_v) = _t->getMarkgap(); break;
        case 59: *reinterpret_cast< int*>(_v) = _t->getMarksize(); break;
        case 60: *reinterpret_cast< int*>(_v) = _t->getMatrixalignment(); break;
        case 61: *reinterpret_cast< double*>(_v) = _t->getMaxfontsize(); break;
        case 62: *reinterpret_cast< double*>(_v) = _t->getMaxfontsizecg(); break;
        case 63: *reinterpret_cast< double*>(_v) = _t->getMaxzoom(); break;
        case 64: *reinterpret_cast< bool*>(_v) = _t->getMergepgnets(); break;
        case 65: *reinterpret_cast< int*>(_v) = _t->getMergetracks(); break;
        case 66: *reinterpret_cast< int*>(_v) = _t->getMinchannelwidth(); break;
        case 67: *reinterpret_cast< int*>(_v) = _t->getMinlevelwidth(); break;
        case 68: *reinterpret_cast< int*>(_v) = _t->getNetattrmax(); break;
        case 69: *reinterpret_cast< int*>(_v) = _t->getNetautohide(); break;
        case 70: *reinterpret_cast< QColor*>(_v) = _t->getNetcolor(); break;
        case 71: *reinterpret_cast< int*>(_v) = _t->getNetfontsize(); break;
        case 72: *reinterpret_cast< int*>(_v) = _t->getNethidedetails(); break;
        case 73: *reinterpret_cast< int*>(_v) = _t->getNetstubminlen(); break;
        case 74: *reinterpret_cast< bool*>(_v) = _t->getNohierattrs(); break;
        case 75: *reinterpret_cast< bool*>(_v) = _t->getNohiernegpins(); break;
        case 76: *reinterpret_cast< QColor*>(_v) = _t->getObjectgrey(); break;
        case 77: *reinterpret_cast< QColor*>(_v) = _t->getObjecthighlight0(); break;
        case 78: *reinterpret_cast< QColor*>(_v) = _t->getObjecthighlight1(); break;
        case 79: *reinterpret_cast< QColor*>(_v) = _t->getObjecthighlight2(); break;
        case 80: *reinterpret_cast< QColor*>(_v) = _t->getObjecthighlight3(); break;
        case 81: *reinterpret_cast< QColor*>(_v) = _t->getObjecthighlight4(); break;
        case 82: *reinterpret_cast< QColor*>(_v) = _t->getObjecthighlight5(); break;
        case 83: *reinterpret_cast< QColor*>(_v) = _t->getObjecthighlight6(); break;
        case 84: *reinterpret_cast< QColor*>(_v) = _t->getObjecthighlight7(); break;
        case 85: *reinterpret_cast< QColor*>(_v) = _t->getObjecthighlight8(); break;
        case 86: *reinterpret_cast< QColor*>(_v) = _t->getObjecthighlight9(); break;
        case 87: *reinterpret_cast< QColor*>(_v) = _t->getObjecthighlight10(); break;
        case 88: *reinterpret_cast< QColor*>(_v) = _t->getObjecthighlight11(); break;
        case 89: *reinterpret_cast< QColor*>(_v) = _t->getObjecthighlight12(); break;
        case 90: *reinterpret_cast< QColor*>(_v) = _t->getObjecthighlight13(); break;
        case 91: *reinterpret_cast< QColor*>(_v) = _t->getObjecthighlight14(); break;
        case 92: *reinterpret_cast< QColor*>(_v) = _t->getObjecthighlight15(); break;
        case 93: *reinterpret_cast< QColor*>(_v) = _t->getObjecthighlight16(); break;
        case 94: *reinterpret_cast< QColor*>(_v) = _t->getObjecthighlight17(); break;
        case 95: *reinterpret_cast< QColor*>(_v) = _t->getObjecthighlight18(); break;
        case 96: *reinterpret_cast< QColor*>(_v) = _t->getObjecthighlight19(); break;
        case 97: *reinterpret_cast< bool*>(_v) = _t->getOngrid(); break;
        case 98: *reinterpret_cast< int*>(_v) = _t->getOutfblevel(); break;
        case 99: *reinterpret_cast< QColor*>(_v) = _t->getOverlapcolor(); break;
        case 100: *reinterpret_cast< QColor*>(_v) = _t->getPbuscolor(); break;
        case 101: *reinterpret_cast< QColor*>(_v) = _t->getPbusnamecolor(); break;
        case 102: *reinterpret_cast< int*>(_v) = _t->getPinattrmax(); break;
        case 103: *reinterpret_cast< int*>(_v) = _t->getPinautohide(); break;
        case 104: *reinterpret_cast< int*>(_v) = _t->getPinorder(); break;
        case 105: *reinterpret_cast< bool*>(_v) = _t->getPinpermute(); break;
        case 106: *reinterpret_cast< int*>(_v) = _t->getPortattrmax(); break;
        case 107: *reinterpret_cast< QColor*>(_v) = _t->getPortcolor(); break;
        case 108: *reinterpret_cast< QColor*>(_v) = _t->getPortnamecolor(); break;
        case 109: *reinterpret_cast< int*>(_v) = _t->getReducejogs(); break;
        case 110: *reinterpret_cast< int*>(_v) = _t->getRipattrmax(); break;
        case 111: *reinterpret_cast< int*>(_v) = _t->getRipindexfontsize(); break;
        case 112: *reinterpret_cast< QColor*>(_v) = _t->getRippercolor(); break;
        case 113: *reinterpret_cast< int*>(_v) = _t->getRippershape(); break;
        case 114: *reinterpret_cast< QColor*>(_v) = _t->getRubberbandcolor(); break;
        case 115: *reinterpret_cast< int*>(_v) = _t->getRubberbandfontsize(); break;
        case 116: *reinterpret_cast< int*>(_v) = _t->getSelectattr(); break;
        case 117: *reinterpret_cast< int*>(_v) = _t->getSelectionappearance(); break;
        case 118: *reinterpret_cast< QColor*>(_v) = _t->getSelectioncolor(); break;
        case 119: *reinterpret_cast< int*>(_v) = _t->getShadowstyle(); break;
        case 120: *reinterpret_cast< double*>(_v) = _t->getSheetheight(); break;
        case 121: *reinterpret_cast< double*>(_v) = _t->getSheetwidth(); break;
        case 122: *reinterpret_cast< bool*>(_v) = _t->getShortobjid(); break;
        case 123: *reinterpret_cast< int*>(_v) = _t->getShowattribute(); break;
        case 124: *reinterpret_cast< bool*>(_v) = _t->getShowcellname(); break;
        case 125: *reinterpret_cast< int*>(_v) = _t->getShowframe(); break;
        case 126: *reinterpret_cast< int*>(_v) = _t->getShowgrid(); break;
        case 127: *reinterpret_cast< int*>(_v) = _t->getShowhierpinname(); break;
        case 128: *reinterpret_cast< bool*>(_v) = _t->getShowinstname(); break;
        case 129: *reinterpret_cast< bool*>(_v) = _t->getShowinvisibles(); break;
        case 130: *reinterpret_cast< int*>(_v) = _t->getShowlevels(); break;
        case 131: *reinterpret_cast< int*>(_v) = _t->getShowmarks(); break;
        case 132: *reinterpret_cast< int*>(_v) = _t->getShownetattr(); break;
        case 133: *reinterpret_cast< bool*>(_v) = _t->getShownetname(); break;
        case 134: *reinterpret_cast< int*>(_v) = _t->getShowpagenumbers(); break;
        case 135: *reinterpret_cast< int*>(_v) = _t->getShowpgtype(); break;
        case 136: *reinterpret_cast< int*>(_v) = _t->getShowpinname(); break;
        case 137: *reinterpret_cast< bool*>(_v) = _t->getShowportname(); break;
        case 138: *reinterpret_cast< int*>(_v) = _t->getShowripindex(); break;
        case 139: *reinterpret_cast< int*>(_v) = _t->getShowval(); break;
        case 140: *reinterpret_cast< int*>(_v) = _t->getShowvconn(); break;
        case 141: *reinterpret_cast< int*>(_v) = _t->getShowvconn2(); break;
        case 142: *reinterpret_cast< int*>(_v) = _t->getStubattrmax(); break;
        case 143: *reinterpret_cast< int*>(_v) = _t->getTimelimit(); break;
        case 144: *reinterpret_cast< int*>(_v) = _t->getTransistorlayout(); break;
        case 145: *reinterpret_cast< int*>(_v) = _t->getValattr(); break;
        case 146: *reinterpret_cast< bool*>(_v) = _t->getViewnameoptional(); break;
        default: break;
        }
    } else if (_c == QMetaObject::WriteProperty) {
        NlvQWidget *_t = static_cast<NlvQWidget *>(_o);
        Q_UNUSED(_t)
        void *_v = _a[0];
        switch (_id) {
        case 0: _t->setLogfile(*reinterpret_cast< QString*>(_v)); break;
        case 1: _t->setActionpick(*reinterpret_cast< int*>(_v)); break;
        case 2: _t->setAllpageconn(*reinterpret_cast< bool*>(_v)); break;
        case 3: _t->setAnaloglayout(*reinterpret_cast< int*>(_v)); break;
        case 4: _t->setAttrcolor(*reinterpret_cast< QColor*>(_v)); break;
        case 5: _t->setAttrfontsize(*reinterpret_cast< int*>(_v)); break;
        case 6: _t->setAutobundle(*reinterpret_cast< int*>(_v)); break;
        case 7: _t->setAutohidestyle(*reinterpret_cast< int*>(_v)); break;
        case 8: _t->setBackgroundcolor(*reinterpret_cast< QColor*>(_v)); break;
        case 9: _t->setBoxcolor0(*reinterpret_cast< QColor*>(_v)); break;
        case 10: _t->setBoxcolor1(*reinterpret_cast< QColor*>(_v)); break;
        case 11: _t->setBoxcolor2(*reinterpret_cast< QColor*>(_v)); break;
        case 12: _t->setBoxcolor3(*reinterpret_cast< QColor*>(_v)); break;
        case 13: _t->setBoxfontsize(*reinterpret_cast< int*>(_v)); break;
        case 14: _t->setBoxhierpins(*reinterpret_cast< int*>(_v)); break;
        case 15: _t->setBoxinstcolor(*reinterpret_cast< QColor*>(_v)); break;
        case 16: _t->setBoxinstfontsize(*reinterpret_cast< int*>(_v)); break;
        case 17: _t->setBoxmaxwidth(*reinterpret_cast< int*>(_v)); break;
        case 18: _t->setBoxmingap(*reinterpret_cast< int*>(_v)); break;
        case 19: _t->setBoxminheight(*reinterpret_cast< int*>(_v)); break;
        case 20: _t->setBoxminwidth(*reinterpret_cast< int*>(_v)); break;
        case 21: _t->setBoxpincolor(*reinterpret_cast< QColor*>(_v)); break;
        case 22: _t->setBoxpinfontsize(*reinterpret_cast< int*>(_v)); break;
        case 23: _t->setBoxpingrid(*reinterpret_cast< int*>(_v)); break;
        case 24: _t->setBoxpinsquare(*reinterpret_cast< int*>(_v)); break;
        case 25: _t->setBuscolor(*reinterpret_cast< QColor*>(_v)); break;
        case 26: _t->setBuswidthlimit(*reinterpret_cast< int*>(_v)); break;
        case 27: _t->setBuswireexcess(*reinterpret_cast< int*>(_v)); break;
        case 28: _t->setCloseenough(*reinterpret_cast< int*>(_v)); break;
        case 29: _t->setCreatenetattrdsp(*reinterpret_cast< int*>(_v)); break;
        case 30: _t->setCreatevconn(*reinterpret_cast< int*>(_v)); break;
        case 31: _t->setCreatevconn2(*reinterpret_cast< int*>(_v)); break;
        case 32: _t->setDecorate(*reinterpret_cast< int*>(_v)); break;
        case 33: _t->setElidetext(*reinterpret_cast< int*>(_v)); break;
        case 34: _t->setFastpanning(*reinterpret_cast< bool*>(_v)); break;
        case 35: _t->setFillcolor1(*reinterpret_cast< QColor*>(_v)); break;
        case 36: _t->setFillcolor2(*reinterpret_cast< QColor*>(_v)); break;
        case 37: _t->setFillcolor3(*reinterpret_cast< QColor*>(_v)); break;
        case 38: _t->setFillednegpins(*reinterpret_cast< bool*>(_v)); break;
        case 39: _t->setFitpage(*reinterpret_cast< int*>(_v)); break;
        case 40: _t->setFlyingsegments(*reinterpret_cast< bool*>(_v)); break;
        case 41: _t->setFramecolor(*reinterpret_cast< QColor*>(_v)); break;
        case 42: _t->setFramefontsize(*reinterpret_cast< int*>(_v)); break;
        case 43: _t->setGatecellname(*reinterpret_cast< int*>(_v)); break;
        case 44: _t->setGatepinname(*reinterpret_cast< int*>(_v)); break;
        case 45: _t->setGeitarget(*reinterpret_cast< int*>(_v)); break;
        case 46: _t->setGreymode(*reinterpret_cast< int*>(_v)); break;
        case 47: _t->setGrid(*reinterpret_cast< int*>(_v)); break;
        case 48: _t->setGridcolor(*reinterpret_cast< QColor*>(_v)); break;
        case 49: _t->setGroupregionpins(*reinterpret_cast< bool*>(_v)); break;
        case 50: _t->setHiattrvalue(*reinterpret_cast< bool*>(_v)); break;
        case 51: _t->setHorizontallabels(*reinterpret_cast< bool*>(_v)); break;
        case 52: _t->setInstattrmax(*reinterpret_cast< int*>(_v)); break;
        case 53: _t->setInstdrag(*reinterpret_cast< int*>(_v)); break;
        case 54: _t->setInstorder(*reinterpret_cast< int*>(_v)); break;
        case 55: _t->setIoorder(*reinterpret_cast< int*>(_v)); break;
        case 56: _t->setLatchfblevel(*reinterpret_cast< int*>(_v)); break;
        case 57: _t->setMapbool2pla(*reinterpret_cast< bool*>(_v)); break;
        case 58: _t->setMarkgap(*reinterpret_cast< int*>(_v)); break;
        case 59: _t->setMarksize(*reinterpret_cast< int*>(_v)); break;
        case 60: _t->setMatrixalignment(*reinterpret_cast< int*>(_v)); break;
        case 61: _t->setMaxfontsize(*reinterpret_cast< double*>(_v)); break;
        case 62: _t->setMaxfontsizecg(*reinterpret_cast< double*>(_v)); break;
        case 63: _t->setMaxzoom(*reinterpret_cast< double*>(_v)); break;
        case 64: _t->setMergepgnets(*reinterpret_cast< bool*>(_v)); break;
        case 65: _t->setMergetracks(*reinterpret_cast< int*>(_v)); break;
        case 66: _t->setMinchannelwidth(*reinterpret_cast< int*>(_v)); break;
        case 67: _t->setMinlevelwidth(*reinterpret_cast< int*>(_v)); break;
        case 68: _t->setNetattrmax(*reinterpret_cast< int*>(_v)); break;
        case 69: _t->setNetautohide(*reinterpret_cast< int*>(_v)); break;
        case 70: _t->setNetcolor(*reinterpret_cast< QColor*>(_v)); break;
        case 71: _t->setNetfontsize(*reinterpret_cast< int*>(_v)); break;
        case 72: _t->setNethidedetails(*reinterpret_cast< int*>(_v)); break;
        case 73: _t->setNetstubminlen(*reinterpret_cast< int*>(_v)); break;
        case 74: _t->setNohierattrs(*reinterpret_cast< bool*>(_v)); break;
        case 75: _t->setNohiernegpins(*reinterpret_cast< bool*>(_v)); break;
        case 76: _t->setObjectgrey(*reinterpret_cast< QColor*>(_v)); break;
        case 77: _t->setObjecthighlight0(*reinterpret_cast< QColor*>(_v)); break;
        case 78: _t->setObjecthighlight1(*reinterpret_cast< QColor*>(_v)); break;
        case 79: _t->setObjecthighlight2(*reinterpret_cast< QColor*>(_v)); break;
        case 80: _t->setObjecthighlight3(*reinterpret_cast< QColor*>(_v)); break;
        case 81: _t->setObjecthighlight4(*reinterpret_cast< QColor*>(_v)); break;
        case 82: _t->setObjecthighlight5(*reinterpret_cast< QColor*>(_v)); break;
        case 83: _t->setObjecthighlight6(*reinterpret_cast< QColor*>(_v)); break;
        case 84: _t->setObjecthighlight7(*reinterpret_cast< QColor*>(_v)); break;
        case 85: _t->setObjecthighlight8(*reinterpret_cast< QColor*>(_v)); break;
        case 86: _t->setObjecthighlight9(*reinterpret_cast< QColor*>(_v)); break;
        case 87: _t->setObjecthighlight10(*reinterpret_cast< QColor*>(_v)); break;
        case 88: _t->setObjecthighlight11(*reinterpret_cast< QColor*>(_v)); break;
        case 89: _t->setObjecthighlight12(*reinterpret_cast< QColor*>(_v)); break;
        case 90: _t->setObjecthighlight13(*reinterpret_cast< QColor*>(_v)); break;
        case 91: _t->setObjecthighlight14(*reinterpret_cast< QColor*>(_v)); break;
        case 92: _t->setObjecthighlight15(*reinterpret_cast< QColor*>(_v)); break;
        case 93: _t->setObjecthighlight16(*reinterpret_cast< QColor*>(_v)); break;
        case 94: _t->setObjecthighlight17(*reinterpret_cast< QColor*>(_v)); break;
        case 95: _t->setObjecthighlight18(*reinterpret_cast< QColor*>(_v)); break;
        case 96: _t->setObjecthighlight19(*reinterpret_cast< QColor*>(_v)); break;
        case 97: _t->setOngrid(*reinterpret_cast< bool*>(_v)); break;
        case 98: _t->setOutfblevel(*reinterpret_cast< int*>(_v)); break;
        case 99: _t->setOverlapcolor(*reinterpret_cast< QColor*>(_v)); break;
        case 100: _t->setPbuscolor(*reinterpret_cast< QColor*>(_v)); break;
        case 101: _t->setPbusnamecolor(*reinterpret_cast< QColor*>(_v)); break;
        case 102: _t->setPinattrmax(*reinterpret_cast< int*>(_v)); break;
        case 103: _t->setPinautohide(*reinterpret_cast< int*>(_v)); break;
        case 104: _t->setPinorder(*reinterpret_cast< int*>(_v)); break;
        case 105: _t->setPinpermute(*reinterpret_cast< bool*>(_v)); break;
        case 106: _t->setPortattrmax(*reinterpret_cast< int*>(_v)); break;
        case 107: _t->setPortcolor(*reinterpret_cast< QColor*>(_v)); break;
        case 108: _t->setPortnamecolor(*reinterpret_cast< QColor*>(_v)); break;
        case 109: _t->setReducejogs(*reinterpret_cast< int*>(_v)); break;
        case 110: _t->setRipattrmax(*reinterpret_cast< int*>(_v)); break;
        case 111: _t->setRipindexfontsize(*reinterpret_cast< int*>(_v)); break;
        case 112: _t->setRippercolor(*reinterpret_cast< QColor*>(_v)); break;
        case 113: _t->setRippershape(*reinterpret_cast< int*>(_v)); break;
        case 114: _t->setRubberbandcolor(*reinterpret_cast< QColor*>(_v)); break;
        case 115: _t->setRubberbandfontsize(*reinterpret_cast< int*>(_v)); break;
        case 116: _t->setSelectattr(*reinterpret_cast< int*>(_v)); break;
        case 117: _t->setSelectionappearance(*reinterpret_cast< int*>(_v)); break;
        case 118: _t->setSelectioncolor(*reinterpret_cast< QColor*>(_v)); break;
        case 119: _t->setShadowstyle(*reinterpret_cast< int*>(_v)); break;
        case 120: _t->setSheetheight(*reinterpret_cast< double*>(_v)); break;
        case 121: _t->setSheetwidth(*reinterpret_cast< double*>(_v)); break;
        case 122: _t->setShortobjid(*reinterpret_cast< bool*>(_v)); break;
        case 123: _t->setShowattribute(*reinterpret_cast< int*>(_v)); break;
        case 124: _t->setShowcellname(*reinterpret_cast< bool*>(_v)); break;
        case 125: _t->setShowframe(*reinterpret_cast< int*>(_v)); break;
        case 126: _t->setShowgrid(*reinterpret_cast< int*>(_v)); break;
        case 127: _t->setShowhierpinname(*reinterpret_cast< int*>(_v)); break;
        case 128: _t->setShowinstname(*reinterpret_cast< bool*>(_v)); break;
        case 129: _t->setShowinvisibles(*reinterpret_cast< bool*>(_v)); break;
        case 130: _t->setShowlevels(*reinterpret_cast< int*>(_v)); break;
        case 131: _t->setShowmarks(*reinterpret_cast< int*>(_v)); break;
        case 132: _t->setShownetattr(*reinterpret_cast< int*>(_v)); break;
        case 133: _t->setShownetname(*reinterpret_cast< bool*>(_v)); break;
        case 134: _t->setShowpagenumbers(*reinterpret_cast< int*>(_v)); break;
        case 135: _t->setShowpgtype(*reinterpret_cast< int*>(_v)); break;
        case 136: _t->setShowpinname(*reinterpret_cast< int*>(_v)); break;
        case 137: _t->setShowportname(*reinterpret_cast< bool*>(_v)); break;
        case 138: _t->setShowripindex(*reinterpret_cast< int*>(_v)); break;
        case 139: _t->setShowval(*reinterpret_cast< int*>(_v)); break;
        case 140: _t->setShowvconn(*reinterpret_cast< int*>(_v)); break;
        case 141: _t->setShowvconn2(*reinterpret_cast< int*>(_v)); break;
        case 142: _t->setStubattrmax(*reinterpret_cast< int*>(_v)); break;
        case 143: _t->setTimelimit(*reinterpret_cast< int*>(_v)); break;
        case 144: _t->setTransistorlayout(*reinterpret_cast< int*>(_v)); break;
        case 145: _t->setValattr(*reinterpret_cast< int*>(_v)); break;
        case 146: _t->setViewnameoptional(*reinterpret_cast< bool*>(_v)); break;
        default: break;
        }
    } else if (_c == QMetaObject::ResetProperty) {
    }
#endif // QT_NO_PROPERTIES
}

QT_INIT_METAOBJECT const QMetaObject NlvQWidget::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_NlvQWidget.data,
      qt_meta_data_NlvQWidget,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *NlvQWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *NlvQWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_NlvQWidget.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int NlvQWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 16)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 16;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 16)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 16;
    }
#ifndef QT_NO_PROPERTIES
   else if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 147;
    } else if (_c == QMetaObject::QueryPropertyDesignable) {
        _id -= 147;
    } else if (_c == QMetaObject::QueryPropertyScriptable) {
        _id -= 147;
    } else if (_c == QMetaObject::QueryPropertyStored) {
        _id -= 147;
    } else if (_c == QMetaObject::QueryPropertyEditable) {
        _id -= 147;
    } else if (_c == QMetaObject::QueryPropertyUser) {
        _id -= 147;
    }
#endif // QT_NO_PROPERTIES
    return _id;
}

// SIGNAL 0
void NlvQWidget::pageNotify(unsigned  _t1, int _t2, int _t3)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void NlvQWidget::viewportNotify(long _t1, long _t2, long _t3, long _t4)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)), const_cast<void*>(reinterpret_cast<const void*>(&_t4)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void NlvQWidget::progressNotify(int _t1, double _t2, const char * _t3)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void NlvQWidget::selectionNotify(unsigned  _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void NlvQWidget::messageOutput(const char * _t1, const char * _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void NlvQWidget::hierarchyDown(const char * _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}

// SIGNAL 6
void NlvQWidget::hierarchyUp()
{
    QMetaObject::activate(this, &staticMetaObject, 6, nullptr);
}

// SIGNAL 7
void NlvQWidget::bindCallback(const char * _t1, int _t2, int _t3, int _t4, int _t5, int _t6, const char * _t7)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)), const_cast<void*>(reinterpret_cast<const void*>(&_t4)), const_cast<void*>(reinterpret_cast<const void*>(&_t5)), const_cast<void*>(reinterpret_cast<const void*>(&_t6)), const_cast<void*>(reinterpret_cast<const void*>(&_t7)) };
    QMetaObject::activate(this, &staticMetaObject, 7, _a);
}

// SIGNAL 8
void NlvQWidget::mapChanged(const QRect & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 8, _a);
}

// SIGNAL 9
void NlvQWidget::viewportChanged(const QRect & _t1, const QPoint & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 9, _a);
}

// SIGNAL 10
void NlvQWidget::imageMapChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 10, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
