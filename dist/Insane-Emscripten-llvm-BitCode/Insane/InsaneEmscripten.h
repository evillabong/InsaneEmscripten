#pragma once
#ifndef __INSANE_EMSCRIPTEN_H__
#define __INSANE_EMSCRIPTEN_H__

#include <Insane/Insane.h>
#include <Insane/InsaneString.h>
#include <Insane/InsanePreprocessor.h>
#include <emscripten/bind.h>
#include <type_traits>
#include <functional>

#define VAL_TYPE() emscripten::val
#define EMSCRIPTEN_EXPORT_FUNCTOR(arity, returnType, name, p3, p4, p5) emscripten::class_<std::function<returnType(INSANE_REPEAT_COMMA_##arity(VAL_TYPE(), 0))>>(STRINGIFY(name##arity)).constructor<>().function("opcall", &std::function<returnType(INSANE_REPEAT_COMMA_##arity(VAL_TYPE(), 0))>::operator());
#define EMSCRIPTEN_EXPORT_ALL_VOID_FUNCTORS(arity) INSANE_REPEAT_ADVANCED(EMSCRIPTEN_EXPORT_FUNCTOR, arity, VOID_TYPE(), VoidFunctor, 0, 0, 0)
#define EMSCRIPTEN_EXPORT_ALL_VAL_FUNCTORS(arity) INSANE_REPEAT_ADVANCED(EMSCRIPTEN_EXPORT_FUNCTOR, arity, VAL_TYPE(), ValFunctor, 0, 0, 0)
#define EMSCRIPTEN_EXPORT_ALL_FUNCTORS(arity) \
    EMSCRIPTEN_EXPORT_ALL_VAL_FUNCTORS(arity) \
    EMSCRIPTEN_EXPORT_ALL_VOID_FUNCTORS(arity)

#define EMSCRIPTEN_FUNCTOR_TYPE(arity, returnType) std::function<returnType(INSANE_REPEAT_COMMA_##arity(VAL_TYPE(), 0))>
#define EMSCRIPTEN_VOID_FUNCTOR_TYPE(arity) EMSCRIPTEN_FUNCTOR_TYPE(arity, VOID_TYPE())
#define EMSCRIPTEN_VAL_FUNCTOR_TYPE(arity) EMSCRIPTEN_FUNCTOR_TYPE(arity, VAL_TYPE())

#define USING_EMSCRIPTEN using namespace emscripten
#define USING_INSANE_EMSCRIPTEN using namespace Insane::Emscripten

#include <Insane/InsaneCryptography.h>
#include <Insane/InsaneException.h>

#define INSANE_PROPERTY_SUFFIX u8"Property_"s

namespace Insane::Emscripten
{

    static inline emscripten::val operator"" _valb(UInt64 value)
    {

        USING_EMSCRIPTEN;
        return val::global().call<val>("Boolean", value != 0);
    }

    static inline emscripten::val operator"" _val(UInt64 value)
    {
        USING_EMSCRIPTEN;
        return val::global().call<val>("parseInt", std::to_string(value));
    }

    static inline emscripten::val operator"" _val(long double value)
    {
        USING_EMSCRIPTEN;
        return val::global().call<val>("parseFloat", std::to_string(value));
    }

    static inline emscripten::val operator"" _val(const char *value, size_t size)
    {
        USING_EMSCRIPTEN;
        return val(String(value, size));
    }

    static inline emscripten::val operator"" _val(char value)
    {
        USING_EMSCRIPTEN;
        return val(value);
    }
    enum class ConsoleMessageType
    {
        LOG,
        INFO,
        WARN,
        ERROR
    };

    class Console
    {
    private:
        template <typename... ParamType,
                  typename = typename std::void_t<std::enable_if_t<std::is_same_v<ParamType, std::string> ||
                                                                   std::is_same_v<ParamType, emscripten::val> ||
                                                                   std::is_floating_point_v<ParamType> ||
                                                                   std::is_integral_v<ParamType>>...>>
        static inline void Print(ConsoleMessageType type, const ParamType &... args)
        {
            USING_EMSCRIPTEN;
            String method = EMPTY_STR;
            switch (type)
            {
            case ConsoleMessageType::INFO:
                method = u8"info"s;
                break;
            case ConsoleMessageType::WARN:
                method = u8"warn"s;
                break;
            case ConsoleMessageType::ERROR:
                method = u8"error"s;
                break;
            default:
                method = u8"log";
                break;
            }
            val::global("console").call<void>(method.c_str(), args...);
        };

    public:
        Console() = default;
        ~Console() = default;
        template <typename... ParamType,
                  typename = typename std::void_t<std::enable_if_t<std::is_same_v<ParamType, std::string> ||
                                                                   std::is_same_v<ParamType, emscripten::val> ||
                                                                   std::is_floating_point_v<ParamType> ||
                                                                   std::is_integral_v<ParamType>>...>>
        static inline void Log(const ParamType &... args)
        {
            USING_EMSCRIPTEN;
            Print(ConsoleMessageType::LOG, args...);
        };

        template <typename... ParamType,
                  typename = typename std::void_t<std::enable_if_t<std::is_same_v<ParamType, std::string> ||
                                                                   std::is_same_v<ParamType, emscripten::val> ||
                                                                   std::is_floating_point_v<ParamType> ||
                                                                   std::is_integral_v<ParamType>>...>>
        static inline void Info(const ParamType &... args)
        {
            USING_EMSCRIPTEN;
            Print(ConsoleMessageType::INFO, args...);
        };

        template <typename... ParamType,
                  typename = typename std::void_t<std::enable_if_t<std::is_same_v<ParamType, std::string> ||
                                                                   std::is_same_v<ParamType, emscripten::val> ||
                                                                   std::is_floating_point_v<ParamType> ||
                                                                   std::is_integral_v<ParamType>>...>>
        static inline void Warn(const ParamType &... args)
        {
            USING_EMSCRIPTEN;
            Print(ConsoleMessageType::WARN, args...);
        };

        template <typename... ParamType,
                  typename = typename std::void_t<std::enable_if_t<std::is_same_v<ParamType, std::string> ||
                                                                   std::is_same_v<ParamType, emscripten::val> ||
                                                                   std::is_floating_point_v<ParamType> ||
                                                                   std::is_integral_v<ParamType>>...>>
        static inline void Error(const ParamType &... args)
        {
            USING_EMSCRIPTEN;
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
            return EMPTY_STR;
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

    class Browser
    {
    private:
    public:
        template <typename ReturnType = emscripten::val,
                  typename = typename std::enable_if_t<std::is_same_v<ReturnType, String> || std::is_same_v<ReturnType, emscripten::val>>>
        [[nodiscard]] static ReturnType GetUserAgent();

        // template <typename ReturnType = emscripten::val,
        //           typename = typename std::enable_if_t<std::is_same_v<ReturnType, String> || std::is_same_v<ReturnType, emscripten::val>>>
        // [[nodiscard]] static ReturnType GetName();

        template <typename ReturnType = emscripten::val,
                  typename ParamType = emscripten::val,
                  typename = typename std::enable_if_t<std::is_same_v<ReturnType, String> || std::is_same_v<ReturnType, emscripten::val>>,
                  typename = typename std::enable_if_t<std::is_same_v<ReturnType, String> || std::is_same_v<ReturnType, emscripten::val>>>
        [[nodiscard]] static ReturnType GetName(const ParamType &userAgent = DefaultValue<ParamType>::value());

        template <typename ReturnType = emscripten::val,
                  typename = typename std::enable_if_t<std::is_same_v<ReturnType, String> || std::is_same_v<ReturnType, emscripten::val>>>
        [[nodiscard]] static ReturnType GetWebGLVendor();

        template <typename ReturnType = emscripten::val, typename = typename std::enable_if_t<std::is_same_v<ReturnType, String> || std::is_same_v<ReturnType, emscripten::val>>>
        [[nodiscard]] static ReturnType GetWebGLRenderer();

        // template <typename ReturnType = emscripten::val, typename = typename std::enable_if_t<std::is_same_v<ReturnType, String> ||
        //                                                                                       std::is_same_v<ReturnType, emscripten::val>>>
        // [[nodiscard]] static ReturnType GetFingerprint();

        // template <typename ReturnType = emscripten::val, typename = typename std::enable_if_t<std::is_same_v<ReturnType, String> ||
        //                                                                                       std::is_same_v<ReturnType, emscripten::val>>>
        // [[nodiscard]] static ReturnType GetNavigatorVendor();

        // template <typename ReturnType = emscripten::val, typename = typename std::enable_if_t<std::is_same_v<ReturnType, String> ||
        //                                                                                       std::is_same_v<ReturnType, emscripten::val>>>
        // [[nodiscard]] static ReturnType GetNavigatorPlatform();

        // template <typename ReturnType = emscripten::val, typename = typename std::enable_if_t<std::is_same_v<ReturnType, String> ||
        //                                                                                       std::is_same_v<ReturnType, emscripten::val>>>
        // [[nodiscard]] static ReturnType GetNavigatorLanguage();

        // template <typename ReturnType = emscripten::val, typename = typename std::enable_if_t<std::is_same_v<ReturnType, String> ||
        //                                                                                       std::is_same_v<ReturnType, emscripten::val>>>
        // [[nodiscard]] static ReturnType GetNavigatorDoNotTrack();

        // template <typename ReturnType = emscripten::val, typename = typename std::enable_if_t<std::is_same_v<ReturnType, bool> ||
        //                                                                                       std::is_same_v<ReturnType, emscripten::val>>>
        // [[nodiscard]] static ReturnType GetNavigatorCookieEnabled();

        template <typename ReturnType = emscripten::val,
                  typename = typename std::enable_if_t<std::is_same_v<ReturnType, std::vector<String>> || std::is_same_v<ReturnType, emscripten::val>>>
        [[nodiscard]] static ReturnType GetLanguages();

        Browser() = delete;
        ~Browser() = delete;
    };

    class Json
    {
    private:
    public:
        template <typename ReturnType = emscripten::val, typename = typename std::enable_if_t<std::is_same_v<ReturnType, String> ||
                                                                                              std::is_same_v<ReturnType, emscripten::val>>>
        static ReturnType ToString(const emscripten::val &object);
    };

    class Js
    {
    private:
        static emscripten::val Bind(const emscripten::val &fx);
        static void SetVars();
        static void CheckSignature(const String &name, const String &sname, const String &key, const String &mae);

    public:
        static void FreeState();
        static void CheckState() noexcept(false);
        static String GetProperty(const String &name, const String &key = EMPTY_STRING, const String &suffix = INSANE_PROPERTY_SUFFIX);
        template <typename ReturnType,
                  typename... ParamType,
                  typename = typename std::void_t<std::enable_if_t<std::is_same_v<ReturnType, void> ||
                                                                   std::is_same_v<ReturnType, emscripten::val>>>,
                  typename = typename std::void_t<std::enable_if_t<std::is_same_v<ParamType, emscripten::val>>...>>
        static inline emscripten::val Bind(const std::function<ReturnType(ParamType...)> &fx)
        {
            USING_EMSCRIPTEN;
            return Bind(val(fx));
        }

        static bool ValueIsNullOrUndefined(emscripten::val value);

        template <typename ParamType,
                  typename = typename std::void_t<std::enable_if_t<std::is_same_v<ParamType, emscripten::val> ||
                                                                   std::is_same_v<ParamType, String>>>>
        static emscripten::val LoadScript(const ParamType &scriptpath, bool log);

        template <typename ParamType,
                  typename = typename std::void_t<std::enable_if_t<std::is_same_v<ParamType, emscripten::val> ||
                                                                   std::is_same_v<ParamType, String>>>>
        static emscripten::val ImportModule(const ParamType &scriptpath);

        template <typename ReturnType = emscripten::val,
                  typename ParamType = emscripten::val,
                  typename = typename std::void_t<std::enable_if_t<std::is_same_v<ParamType, std::string> ||
                                                                   std::is_same_v<ParamType, emscripten::val> ||
                                                                   std::is_floating_point_v<ParamType> ||
                                                                   std::is_integral_v<ParamType>>>,
                  typename = typename std::enable_if_t<std::is_same_v<ReturnType, String> ||
                                                       std::is_same_v<ReturnType, emscripten::val>>>
        static ReturnType ToString(const ParamType &value);
        static void ThrowError(const String &message);
        static emscripten::val Init(const String &key);
    };

    class LocalStorage
    {
    private:
    public:
        template <typename ReturnType = String,
                  typename = typename std::enable_if_t<std::is_same_v<ReturnType, String> ||
                                                       std::is_same_v<ReturnType, emscripten::val>>>
        [[nodiscard]] static ReturnType GetValue(const String &key, const String &password = EMPTY_STR) noexcept;

        static bool SetValue(const String &key, const String &value, const String &password = EMPTY_STR) noexcept;

        [[nodiscard]] static String GetVolatileValue(const String &key, const String &password = EMPTY_STR) noexcept;
        static bool SetVolatileValue(const String &key, const String &value, const String &password = EMPTY_STR) noexcept;

        template <typename ParamType,
                  typename = typename std::void_t<std::enable_if_t<std::is_same_v<ParamType, std::string> ||
                                                                   std::is_same_v<ParamType, emscripten::val>>>>
        static void RemoveValue(const ParamType &key) noexcept;

        static void Clear();
        static void RemoveValuesStartingWith(const String &preffix);
    };

} // namespace Insane::Emscripten

#endif // __INSANE_EMSCRIPTEN_H__

//  template <typename Type>
//         static inline constexpr std::string_view
//         type_name()
//         {
//             std::string_view name, prefix, suffix;
// #ifdef __clang__
//             name = __PRETTY_FUNCTION__;
//             prefix = "std::string_view type_name() [T = ";
//             suffix = "]";
// #elif defined(__GNUC__)
//             name = __PRETTY_FUNCTION__;
//             prefix = "constexpr std::string_view type_name() [with T = ";
//             suffix = "; std::string_view = std::basic_string_view<char>]";
// #elif defined(_MSC_VER)
//             name = __FUNCSIG__;
//             prefix = "class std::basic_string_view<char,struct std::char_traits<char> > __cdecl type_name<";
//             suffix = ">(void)";
// #endif
//             //name.remove_prefix(prefix.size()-2);
//             //name.remove_suffix(suffix.size());
//             return name;
//         }

//         template <typename T>
//         static inline void Print(std::ostream &out, const T &head)
//         {
//             out << head << std::endl;
//         }

//         template <typename T, typename... Args>
//         static inline void Print(std::ostream &out, const T &head, const Args &... args)
//         {
//             out << head << "," << std::endl;
//             Print(out, args...);
//         }