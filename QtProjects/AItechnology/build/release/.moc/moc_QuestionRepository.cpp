/****************************************************************************
** Meta object code from reading C++ file 'QuestionRepository.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../src/questionbank/QuestionRepository.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'QuestionRepository.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.9.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN18QuestionRepositoryE_t {};
} // unnamed namespace

template <> constexpr inline auto QuestionRepository::qt_create_metaobjectdata<qt_meta_tag_ZN18QuestionRepositoryE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "QuestionRepository",
        "dataLoaded",
        "",
        "count",
        "filterChanged",
        "currentChanged",
        "index",
        "currentQuestionChanged",
        "loadQuestions",
        "filePath",
        "getFilteredQuestions",
        "QList<Question>",
        "FilterCriteria",
        "criteria",
        "nextQuestion",
        "Question",
        "previousQuestion",
        "setShowAnswer",
        "show",
        "isShowAnswer",
        "setCurrentIndex",
        "currentIndex",
        "totalCount",
        "currentQuestion"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'dataLoaded'
        QtMocHelpers::SignalData<void(int)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 3 },
        }}),
        // Signal 'filterChanged'
        QtMocHelpers::SignalData<void(int)>(4, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 3 },
        }}),
        // Signal 'currentChanged'
        QtMocHelpers::SignalData<void(int)>(5, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 6 },
        }}),
        // Signal 'currentQuestionChanged'
        QtMocHelpers::SignalData<void()>(7, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'loadQuestions'
        QtMocHelpers::MethodData<bool(const QString &)>(8, 2, QMC::AccessPublic, QMetaType::Bool, {{
            { QMetaType::QString, 9 },
        }}),
        // Method 'getFilteredQuestions'
        QtMocHelpers::MethodData<QList<Question>(const FilterCriteria &)>(10, 2, QMC::AccessPublic, 0x80000000 | 11, {{
            { 0x80000000 | 12, 13 },
        }}),
        // Method 'nextQuestion'
        QtMocHelpers::MethodData<Question()>(14, 2, QMC::AccessPublic, 0x80000000 | 15),
        // Method 'previousQuestion'
        QtMocHelpers::MethodData<Question()>(16, 2, QMC::AccessPublic, 0x80000000 | 15),
        // Method 'setShowAnswer'
        QtMocHelpers::MethodData<void(bool)>(17, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 18 },
        }}),
        // Method 'isShowAnswer'
        QtMocHelpers::MethodData<bool() const>(19, 2, QMC::AccessPublic, QMetaType::Bool),
        // Method 'setCurrentIndex'
        QtMocHelpers::MethodData<void(int)>(20, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 6 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
        // property 'currentIndex'
        QtMocHelpers::PropertyData<int>(21, QMetaType::Int, QMC::DefaultPropertyFlags, 2),
        // property 'totalCount'
        QtMocHelpers::PropertyData<int>(22, QMetaType::Int, QMC::DefaultPropertyFlags, 1),
        // property 'currentQuestion'
        QtMocHelpers::PropertyData<Question>(23, 0x80000000 | 15, QMC::DefaultPropertyFlags | QMC::EnumOrFlag, 3),
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<QuestionRepository, qt_meta_tag_ZN18QuestionRepositoryE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject QuestionRepository::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN18QuestionRepositoryE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN18QuestionRepositoryE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN18QuestionRepositoryE_t>.metaTypes,
    nullptr
} };

void QuestionRepository::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<QuestionRepository *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->dataLoaded((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 1: _t->filterChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 2: _t->currentChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 3: _t->currentQuestionChanged(); break;
        case 4: { bool _r = _t->loadQuestions((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 5: { QList<Question> _r = _t->getFilteredQuestions((*reinterpret_cast< std::add_pointer_t<FilterCriteria>>(_a[1])));
            if (_a[0]) *reinterpret_cast< QList<Question>*>(_a[0]) = std::move(_r); }  break;
        case 6: { Question _r = _t->nextQuestion();
            if (_a[0]) *reinterpret_cast< Question*>(_a[0]) = std::move(_r); }  break;
        case 7: { Question _r = _t->previousQuestion();
            if (_a[0]) *reinterpret_cast< Question*>(_a[0]) = std::move(_r); }  break;
        case 8: _t->setShowAnswer((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 9: { bool _r = _t->isShowAnswer();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 10: _t->setCurrentIndex((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (QuestionRepository::*)(int )>(_a, &QuestionRepository::dataLoaded, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (QuestionRepository::*)(int )>(_a, &QuestionRepository::filterChanged, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (QuestionRepository::*)(int )>(_a, &QuestionRepository::currentChanged, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (QuestionRepository::*)()>(_a, &QuestionRepository::currentQuestionChanged, 3))
            return;
    }
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast<int*>(_v) = _t->currentIndex(); break;
        case 1: *reinterpret_cast<int*>(_v) = _t->totalCount(); break;
        case 2: *reinterpret_cast<Question*>(_v) = _t->currentQuestion(); break;
        default: break;
        }
    }
}

const QMetaObject *QuestionRepository::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *QuestionRepository::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN18QuestionRepositoryE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int QuestionRepository::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 11)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 11;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 11)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 11;
    }
    if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    }
    return _id;
}

// SIGNAL 0
void QuestionRepository::dataLoaded(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void QuestionRepository::filterChanged(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}

// SIGNAL 2
void QuestionRepository::currentChanged(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1);
}

// SIGNAL 3
void QuestionRepository::currentQuestionChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}
QT_WARNING_POP
