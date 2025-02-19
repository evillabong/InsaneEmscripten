#pragma once
#ifndef __INSANE_EMSCRIPTEN_H__
#define __INSANE_EMSCRIPTEN_H__

#include <Insane/Insane.h>
#include <Insane/InsaneString.h>
#include <Insane/InsanePreprocessor.h>
#include <Insane/InsaneCryptography.h>
#include <Insane/InsaneException.h>

#include <emscripten/bind.h>

#include <type_traits>
#include <functional>

#define VAL_TYPE_EXP() emscripten::val

#define EMSCRIPTEN_EXPORT_FUNCTOR_SEQ_EXP(n, nprev, c, cnext, se, sae, returnTypeExp, name, p3, p4, p5) emscripten::class_<std::function<returnTypeExp()(INSANE_REPEAT(emscripten::val, c, COMMA_VALUE_EXP, 0))>>(CSTRINGIFY_DEFER(name##c)).constructor<>().function("opcall", &std::function<returnTypeExp()(INSANE_REPEAT(emscripten::val, c, COMMA_VALUE_EXP, 0))>::operator());

#define EMSCRIPTEN_EXPORT_ALL_VOID_FUNCTORS(arity, name) INSANE_REPEAT_SEQ(EMSCRIPTEN_EXPORT_FUNCTOR_SEQ_EXP, arity, EMPTY_VALUE_EXP, 0, VOID_TYPE_EXP, name, 3, 4, 5)
#define EMSCRIPTEN_EXPORT_ALL_VAL_FUNCTORS(arity, name) INSANE_REPEAT_SEQ(EMSCRIPTEN_EXPORT_FUNCTOR_SEQ_EXP, arity, EMPTY_VALUE_EXP, 0, VAL_TYPE_EXP, name, 3, 4, 5)
#define EMSCRIPTEN_EXPORT_ALL_FUNCTORS(arity, name) EMSCRIPTEN_EXPORT_ALL_VAL_FUNCTORS(arity, name##ValFunctor) EMSCRIPTEN_EXPORT_ALL_VOID_FUNCTORS(arity, name##VoidFunctor)

#define EMSCRIPTEN_FUNCTOR_TYPE(arity, returnTypeExp) std::function<returnTypeExp()(INSANE_REPEAT(emscripten::val, arity, COMMA_VALUE_EXP, 0))>
#define EMSCRIPTEN_VOID_FUNCTOR_TYPE(arity) EMSCRIPTEN_FUNCTOR_TYPE(arity, VOID_TYPE_EXP)
#define EMSCRIPTEN_VAL_FUNCTOR_TYPE(arity) EMSCRIPTEN_FUNCTOR_TYPE(arity, VAL_TYPE_EXP)

#define EMSCRIPTEN_ENUM_EXTENSIONS_EXPORT(enumExtensionExportName, insaneEnumType) class_<insaneEnumType ## EnumExtensions>(enumExtensionExportName) \
        .class_function("ParseString", select_overload<insaneEnumType(const String &)>(&insaneEnumType ## EnumExtensions::Parse)) \
        .class_function("ParseInt", select_overload<insaneEnumType(const int &)>(&insaneEnumType ## EnumExtensions::Parse)) \
        .class_function("ToIntegral",&insaneEnumType ## EnumExtensions::ToIntegral) \
        .class_function("ToString", &insaneEnumType ## EnumExtensions::ToString) \
        .class_function("ToIntegralString", &insaneEnumType ## EnumExtensions::ToIntegralString)

#define USING_NS_EMSCRIPTEN using namespace emscripten
#define USING_NS_INSANE_EMSCRIPTEN using namespace InsaneIO::Insane::Emscripten

#define INSANE_PROPERTY_SUFFIX ("Insane_"s)
#define VAL_GLOBAL emscripten::val::global()
#define VAL_INSANE VAL_GLOBAL[INSANE_STRING]

static inline emscripten::val operator"" _valb(unsigned long long value)
{
    USING_NS_EMSCRIPTEN;
    return val::global().call<val>("Boolean", value != 0);
}

static inline emscripten::val operator"" _val(unsigned long long value)
{
    USING_NS_EMSCRIPTEN;
    return val::global().call<val>("parseInt", std::to_string(value));
}

static inline emscripten::val operator"" _val(long double value)
{
    USING_NS_EMSCRIPTEN;
    return val::global().call<val>("parseFloat", std::to_string(value));
}

static inline emscripten::val operator"" _val(const char *value, size_t size)
{
    USING_NS_EMSCRIPTEN;
    return val(String(value, size));
}

static inline emscripten::val operator"" _val(char value)
{
    USING_NS_EMSCRIPTEN;
    return val(value);
}

typedef emscripten::val EmscriptenVal;

namespace InsaneIO::Insane::Emscripten
{
    class EmscriptenValManager
    {
    public:
        template <typename ParamType, typename ReturnType = ParamType>
        static inline EmscriptenVal Transform(const ParamType &param)
        {
            return EmscriptenVal(param);
        }

        static inline EmscriptenVal Transform(const char *param)
        {

            return EmscriptenVal(String(param));
        }

        static inline EmscriptenVal Transform(char *param)
        {

            return EmscriptenVal(String(param));
        }
    };

    class Operator final
    {
    private:
        enum class OperatorArityType
        {
            Unary,
            Binary
        };

        enum class OperatorType
        {
            Addition,
            Subtraction,
            Multiplication,
            Division,
            Import,
            TypeOf,
            IsNull,
            IsUndefined,
            IsNullOrUndefined,
            ToString
        };

        static EmscriptenVal CallOperator(const EmscriptenVal &a, const EmscriptenVal &b, const OperatorType &operatorType, const OperatorArityType &operatorArityType);

    public:
        [[nodiscard]] static EmscriptenVal Add(const EmscriptenVal &a, const EmscriptenVal &b);
        [[nodiscard]] static EmscriptenVal Subtract(const EmscriptenVal &a, const EmscriptenVal &b);
        [[nodiscard]] static EmscriptenVal Multiply(const EmscriptenVal &a, const EmscriptenVal &b);
        [[nodiscard]] static EmscriptenVal Divide(const EmscriptenVal &a, const EmscriptenVal &b);
        [[nodiscard]] static EmscriptenVal ImportAsync(const EmscriptenVal &a);
        [[nodiscard]] static EmscriptenVal TypeOf(const EmscriptenVal &a);
        [[nodiscard]] static bool IsNull(const EmscriptenVal &a);
        [[nodiscard]] static bool IsUndefined(const EmscriptenVal &a);
        [[nodiscard]] static bool IsNullOrUndefined(const EmscriptenVal &a);
        [[nodiscard]] static EmscriptenVal ToString(const EmscriptenVal &a);
    };

    class Promise final
    {
    private:
    public:
        [[nodiscard]] static EmscriptenVal New(const EMSCRIPTEN_VOID_FUNCTOR_TYPE(2) & promiseCallback);

        template <typename ParamType = EmscriptenVal,
                  typename = typename std::void_t<std::enable_if_t<std::is_same_v<ParamType, String> ||
                                                                   std::is_floating_point_v<ParamType> ||
                                                                   std::is_integral_v<ParamType> ||
                                                                   std::is_same_v<ParamType, EmscriptenVal>>>>
        [[nodiscard]] static inline EmscriptenVal Resolve(const ParamType &value)
        {
            USING_NS_EMSCRIPTEN;
            return val::global()["Promise"].call<val>("resolve", EmscriptenVal(value));
        }

        template <typename ParamType = EmscriptenVal,
                  typename = typename std::void_t<std::enable_if_t<std::is_same_v<ParamType, String> ||
                                                                   std::is_floating_point_v<ParamType> ||
                                                                   std::is_integral_v<ParamType> ||
                                                                   std::is_same_v<ParamType, EmscriptenVal>>>>
        [[nodiscard]] static EmscriptenVal Reject(const ParamType &value)
        {
            USING_NS_EMSCRIPTEN;
            return val::global()["Promise"].call<val>("reject", EmscriptenVal(value));
        }
    };

    class Console final
    {
    private:
        enum class ConsoleMessageType
        {
            LOG,
            INFO,
            WARN,
            ERROR
        };
        template <typename... ParamType,
                  typename = typename std::void_t<std::enable_if_t<std::is_same_v<ParamType, std::string> ||
                                                                   std::is_same<std::decay_t<ParamType>, const char *>::value ||
                                                                   std::is_same<std::decay_t<ParamType>, char *>::value ||
                                                                   std::is_same_v<ParamType, emscripten::val> ||
                                                                   std::is_floating_point_v<ParamType> ||
                                                                   std::is_integral_v<ParamType>>...>>
        static inline void Print(ConsoleMessageType type, const ParamType &...args)
        {
            USING_NS_EMSCRIPTEN;
            String method = EMPTY_STRING;
            switch (type)
            {
            case ConsoleMessageType::INFO:
                method = "info"s;
                break;
            case ConsoleMessageType::WARN:
                method = "warn"s;
                break;
            case ConsoleMessageType::ERROR:
                method = "error"s;
                break;
            default:
                method = "log";
                break;
            }
            val::global("console").call<void>(method.c_str(), EmscriptenValManager::Transform(args)...);
        };

    public:
        Console() = default;
        ~Console() = default;

        template <typename... ParamType,
                  typename = typename std::void_t<std::enable_if_t<std::is_same_v<ParamType, std::string> ||
                                                                   std::is_same<std::decay_t<ParamType>, const char *>::value ||
                                                                   std::is_same<std::decay_t<ParamType>, char *>::value ||
                                                                   std::is_same_v<ParamType, emscripten::val> ||
                                                                   std::is_floating_point_v<ParamType> ||
                                                                   std::is_integral_v<ParamType>>...>>
        static inline void Log(const ParamType &...args)
        {
            USING_NS_EMSCRIPTEN;
            Print(ConsoleMessageType::LOG, args...);
        };

        template <typename... ParamType,
                  typename = typename std::void_t<std::enable_if_t<std::is_same_v<ParamType, std::string> ||
                                                                   std::is_same<std::decay_t<ParamType>, const char *>::value ||
                                                                   std::is_same<std::decay_t<ParamType>, char *>::value ||
                                                                   std::is_same_v<ParamType, emscripten::val> ||
                                                                   std::is_floating_point_v<ParamType> ||
                                                                   std::is_integral_v<ParamType>>...>>
        static inline void Info(const ParamType &...args)
        {
            USING_NS_EMSCRIPTEN;
            Print(ConsoleMessageType::INFO, args...);
        };

        template <typename... ParamType,
                  typename = typename std::void_t<std::enable_if_t<std::is_same_v<ParamType, std::string> ||
                                                                   std::is_same<std::decay_t<ParamType>, const char *>::value ||
                                                                   std::is_same<std::decay_t<ParamType>, char *>::value ||
                                                                   std::is_same_v<ParamType, emscripten::val> ||
                                                                   std::is_floating_point_v<ParamType> ||
                                                                   std::is_integral_v<ParamType>>...>>
        static inline void Warn(const ParamType &...args)
        {
            USING_NS_EMSCRIPTEN;
            Print(ConsoleMessageType::WARN, args...);
        };

        template <typename... ParamType,
                  typename = typename std::void_t<std::enable_if_t<std::is_same_v<ParamType, std::string> ||
                                                                   std::is_same<std::decay_t<ParamType>, const char *>::value ||
                                                                   std::is_same<std::decay_t<ParamType>, char *>::value ||
                                                                   std::is_same_v<ParamType, emscripten::val> ||
                                                                   std::is_floating_point_v<ParamType> ||
                                                                   std::is_integral_v<ParamType>>...>>
        static inline void Error(const ParamType &...args)
        {
            USING_NS_EMSCRIPTEN;
            Print(ConsoleMessageType::ERROR, args...);
        };
    };

    template <typename T>
    struct DefaultValue;

    template <>
    struct DefaultValue<String>
    {
        static String value()
        {
            return EMPTY_STRING;
        }
    };

    template <>
    struct DefaultValue<emscripten::val>
    {
        static emscripten::val value()
        {
            return emscripten::val::null();
        }
    };

    enum class OrientationType
    {
        LANDSCAPE = 0,
        PORTRAIT = 1,
        SQUARE = 2
    };

    class Browser final
    {
    private:
    public:
        template <typename ReturnType = emscripten::val,
                  typename = typename std::enable_if_t<std::is_same_v<ReturnType, String> || std::is_same_v<ReturnType, emscripten::val>>>
        [[nodiscard]] static ReturnType GetUserAgent();

        template <typename ReturnType = emscripten::val,
                  typename = typename std::enable_if_t<std::is_same_v<ReturnType, int> || std::is_same_v<ReturnType, emscripten::val>>>
        [[nodiscard]] static ReturnType GetTimezoneOffsetMinutes();

        template <typename ReturnType = emscripten::val,
                  typename = typename std::enable_if_t<std::is_same_v<ReturnType, int> || std::is_same_v<ReturnType, emscripten::val>>>
        [[nodiscard]] static ReturnType GetTimezoneOffsetSeconds();

        template <typename ReturnType = emscripten::val,
                  typename = typename std::enable_if_t<std::is_same_v<ReturnType, int> || std::is_same_v<ReturnType, emscripten::val>>>
        [[nodiscard]] static ReturnType GetTimezoneOffsetMilliseconds();

        template <typename ParamType = emscripten::val,
                  typename = typename std::enable_if_t<std::is_same_v<ParamType, String> || std::is_same_v<ParamType, emscripten::val>>>
        [[nodiscard]] static emscripten::val GetNameAsync(const ParamType &userAgent = DefaultValue<ParamType>::value());

        template <typename ReturnType = emscripten::val,
                  typename ParamType = emscripten::val,
                  typename = typename std::enable_if_t<std::is_same_v<ReturnType, String> || std::is_same_v<ReturnType, emscripten::val>>,
                  typename = typename std::enable_if_t<std::is_same_v<ParamType, String> || std::is_same_v<ParamType, emscripten::val>>>
        [[nodiscard]] static ReturnType GetOS(const ParamType &userAgent = DefaultValue<ParamType>::value());

        template <typename ReturnType = emscripten::val,
                  typename = typename std::enable_if_t<std::is_same_v<ReturnType, String> || std::is_same_v<ReturnType, emscripten::val>>>
        [[nodiscard]] static ReturnType GetWebGLVendor();

        template <typename ReturnType = emscripten::val, typename = typename std::enable_if_t<std::is_same_v<ReturnType, String> || std::is_same_v<ReturnType, emscripten::val>>>
        [[nodiscard]] static ReturnType GetWebGLRenderer();

        template <typename ReturnType = emscripten::val,
                  typename = typename std::enable_if_t<std::is_same_v<ReturnType, int> || std::is_same_v<ReturnType, emscripten::val>>>
        [[nodiscard]] static ReturnType GetScreenHeight();

        template <typename ReturnType = emscripten::val,
                  typename = typename std::enable_if_t<std::is_same_v<ReturnType, int> || std::is_same_v<ReturnType, emscripten::val>>>
        [[nodiscard]] static ReturnType GetScreenWidth();

        template <typename ReturnType = emscripten::val,
                  typename = typename std::enable_if_t<std::is_same_v<ReturnType, int> || std::is_same_v<ReturnType, emscripten::val>>>
        [[nodiscard]] static ReturnType GetScreenSize();

        template <typename ReturnType = emscripten::val,
                  typename = typename std::enable_if_t<std::is_same_v<ReturnType, OrientationType> || std::is_same_v<ReturnType, emscripten::val>>>
        [[nodiscard]] static ReturnType GetScreenOrientation();

        template <typename ReturnType = emscripten::val,
                  typename = typename std::enable_if_t<std::is_same_v<ReturnType, int> || std::is_same_v<ReturnType, emscripten::val>>>
        [[nodiscard]] static ReturnType GetAvailableScreenHeight();

        template <typename ReturnType = emscripten::val,
                  typename = typename std::enable_if_t<std::is_same_v<ReturnType, int> || std::is_same_v<ReturnType, emscripten::val>>>
        [[nodiscard]] static ReturnType GetAvailableScreenWidth();

        template <typename ReturnType = emscripten::val,
                  typename = typename std::enable_if_t<std::is_same_v<ReturnType, int> || std::is_same_v<ReturnType, emscripten::val>>>
        [[nodiscard]] static ReturnType GetAvailableScreenSize();

        template <typename ReturnType = emscripten::val,
                  typename = typename std::enable_if_t<std::is_same_v<ReturnType, OrientationType> || std::is_same_v<ReturnType, emscripten::val>>>
        [[nodiscard]] static ReturnType GetAvailableScreenOrientation();

        template <typename ReturnType = emscripten::val,
                  typename = typename std::enable_if_t<std::is_same_v<ReturnType, int> || std::is_same_v<ReturnType, emscripten::val>>>
        [[nodiscard]] static ReturnType GetViewportWidth();

        template <typename ReturnType = emscripten::val,
                  typename = typename std::enable_if_t<std::is_same_v<ReturnType, int> || std::is_same_v<ReturnType, emscripten::val>>>
        [[nodiscard]] static ReturnType GetViewportHeight();

        template <typename ReturnType = emscripten::val,
                  typename = typename std::enable_if_t<std::is_same_v<ReturnType, int> || std::is_same_v<ReturnType, emscripten::val>>>
        [[nodiscard]] static ReturnType GetViewportSize();

        template <typename ReturnType = emscripten::val,
                  typename = typename std::enable_if_t<std::is_same_v<ReturnType, OrientationType> || std::is_same_v<ReturnType, emscripten::val>>>
        [[nodiscard]] static ReturnType GetViewportOrientation();

        template <typename ReturnType = emscripten::val, typename = typename std::enable_if_t<std::is_same_v<ReturnType, String> ||
                                                                                              std::is_same_v<ReturnType, emscripten::val>>>
        [[nodiscard]] static ReturnType GetLanguage();

        template <typename ReturnType = emscripten::val,
                  typename = typename std::enable_if_t<std::is_same_v<ReturnType, std::vector<String>> ||
                                                       std::is_same_v<ReturnType, emscripten::val>>>
        [[nodiscard]] static ReturnType GetLanguages();

        template <typename ReturnType = emscripten::val, typename = typename std::enable_if_t<std::is_same_v<ReturnType, int> ||
                                                                                              std::is_same_v<ReturnType, emscripten::val>>>
        [[nodiscard]] static ReturnType GetMaxTouchPoints();

        template <typename ReturnType = emscripten::val, typename = typename std::enable_if_t<std::is_same_v<ReturnType, float> ||
                                                                                              std::is_same_v<ReturnType, emscripten::val>>>
        [[nodiscard]] static ReturnType GetDeviceMemory();

        template <typename ReturnType = emscripten::val, typename = typename std::enable_if_t<std::is_same_v<ReturnType, int> ||
                                                                                              std::is_same_v<ReturnType, emscripten::val>>>
        [[nodiscard]] static ReturnType GetHardwareConcurrency();

        struct MimeType
        {
            String Type;
            String Description;
            String Suffixes;
        };

        struct Plugin
        {
            String Name;
            std::vector<MimeType> MimeTypes;
        };

        template <typename ReturnType = emscripten::val, typename = typename std::enable_if_t<std::is_same_v<ReturnType, std::vector<Plugin>> ||
                                                                                              std::is_same_v<ReturnType, emscripten::val>>>
        [[nodiscard]] static ReturnType GetPlugins();

        template <typename ReturnType = emscripten::val, typename = typename std::enable_if_t<std::is_same_v<ReturnType, std::vector<MimeType>> ||
                                                                                              std::is_same_v<ReturnType, emscripten::val>>>
        [[nodiscard]] static ReturnType GetMimeTypes();

        template <typename ReturnType = emscripten::val, typename = typename std::enable_if_t<std::is_same_v<ReturnType, String> ||
                                                                                              std::is_same_v<ReturnType, emscripten::val>>>
        [[nodiscard]] static ReturnType GetDoNotTrack();

        template <typename ReturnType = emscripten::val, typename = typename std::enable_if_t<std::is_same_v<ReturnType, bool> ||
                                                                                              std::is_same_v<ReturnType, emscripten::val>>>
        [[nodiscard]] static ReturnType HasCookiesSupport();

        template <typename ParamType = emscripten::val,
                  typename = typename std::enable_if_t<std::is_same_v<ParamType, String> || std::is_same_v<ParamType, emscripten::val>>>
        [[nodiscard]] static EmscriptenVal GetFingerprintAsync(const ParamType &key);

        Browser() = delete;
        ~Browser() = delete;
    };

    class Json final
    {
    private:
    public:
        template <typename ReturnType = emscripten::val, typename = typename std::enable_if_t<std::is_same_v<ReturnType, String> ||
                                                                                              std::is_same_v<ReturnType, emscripten::val>>>
        static ReturnType Serialize(const emscripten::val &object);
    };

    class Js final
    {
    private:
        static emscripten::val Bind(const emscripten::val &fx);
        // static void SetVars();
        // static void CheckSignature(const String &name, const String &sname, const String &key, const String &mae);

    public:
        // static void FreeState();
        // static void CheckState() noexcept(false);
        static void SetPropertyNull(EmscriptenVal object, const String &property, const bool &replaceIfExists = true);
        static void SetPropertyObject(EmscriptenVal object, const String &property, const bool &replaceIfExists = true);
        static void SetPropertyArray(EmscriptenVal object, const String &property, const bool &replaceIfExists = true);
        static void SetProperty(EmscriptenVal object, const String &property, const EmscriptenVal &value, const bool &replaceIfExists = true);
        static String GetPropertyName(const String &name, const String &key = EMPTY_STRING, const String &suffix = INSANE_PROPERTY_SUFFIX);
        template <typename ReturnType,
                  typename... ParamType,
                  typename = typename std::void_t<std::enable_if_t<std::is_same_v<ReturnType, void> ||
                                                                   std::is_same_v<ReturnType, emscripten::val>>>,
                  typename = typename std::void_t<std::enable_if_t<std::is_same_v<ParamType, emscripten::val>>...>>
        static inline emscripten::val Bind(const std::function<ReturnType(ParamType...)> &fx)
        {
            USING_NS_EMSCRIPTEN;
            return Bind(val(fx));
        }

        template <typename ParamType = EmscriptenVal,
                  typename = typename std::void_t<std::enable_if_t<std::is_same_v<ParamType, EmscriptenVal> ||
                                                                   std::is_same_v<ParamType, String>>>>
        static emscripten::val LoadScriptAsync(const ParamType &scriptpath);

        static void ThrowError(const String &message = "An error has ocurred.");
        // static emscripten::val Init(const String &key);
    };

    class LocalStorage final
    {
    private:
    public:
        template <typename ReturnType = EmscriptenVal,
                  typename ParamsType = EmscriptenVal,
                  typename = typename std::enable_if_t<std::is_same_v<ReturnType, String> ||
                                                       std::is_same_v<ReturnType, EmscriptenVal>>,
                  typename = typename std::enable_if_t<std::is_same_v<ParamsType, String> ||
                                                       std::is_same_v<ParamsType, emscripten::val>>>
        [[nodiscard]] static ReturnType GetValue(const ParamsType &key, const ParamsType &password = DefaultValue<ParamsType>::value()) noexcept;

        template <typename ParamsType = EmscriptenVal,
                  typename = typename std::enable_if_t<std::is_same_v<ParamsType, String> ||
                                                       std::is_same_v<ParamsType, emscripten::val>>>
        static void SetValue(const ParamsType &key, const ParamsType &value, const ParamsType &password = DefaultValue<ParamsType>::value()) noexcept;

        template <typename ParamType,
                  typename = typename std::void_t<std::enable_if_t<std::is_same_v<ParamType, std::string> ||
                                                                   std::is_same_v<ParamType, emscripten::val>>>>
        static void RemoveValue(const ParamType &key) noexcept;

        static void Clear();

        template <typename ParamType,
                  typename = typename std::void_t<std::enable_if_t<std::is_same_v<ParamType, std::string> ||
                                                                   std::is_same_v<ParamType, emscripten::val>>>>
        static void RemoveValuesStartingWith(const ParamType &preffix);
    };

    class InteropExtensions
    {
    public:
    [[nodiscard]] static StdVectorUint8 JsUint8ArrayToStdVectorUint8(const EmscriptenVal& array);
    [[nodiscard]] static StdVectorUint8 JsStringToStdVectorUint8(const EmscriptenVal& str);
    [[nodiscard]] static EmscriptenVal StdVectorUint8ToJsUint8Array(const StdVectorUint8& vector);
    static void PrintStdVectorUint8(StdVectorUint8 vector);
    };

} // namespace Insane::Emscripten

#endif // __INSANE_EMSCRIPTEN_H__
