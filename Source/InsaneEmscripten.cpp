#include <Insane/InsaneEmscripten.h>
#include <emscripten/emscripten.h>
#include <emscripten/bind.h>
#include <Insane/InsanePreprocessor.h>
#include <Insane/InsaneString.h>
#include <Insane/InsaneCryptography.h>
#include <Insane/InsaneException.h>

USING_NS_EMSCRIPTEN;
USING_NS_INSANE_EXCEPTION;
USING_NS_INSANE_CRYPTO;
USING_NS_INSANE_STR;
USING_NS_INSANE_CORE;
USING_NS_INSANE_EMSCRIPTEN;

// namespace Internal
// {
//     bool TransformN(const int &base, int &result, std::string data, const String &key, bool normal = true)
//     {
//         data = HashExtensions::ToHmac(data, key, HashAlgorithm::Sha512);
//     begin:
//         if (normal)
//         {
//             result = (~((~base) ^ (data[0] * data[2] * data[4] * data[16] * 612)) ^ data[data.length() - 1]) ^ (base / 216);
//             for (char value : data)
//             {
//                 result -= value;
//             }
//             return true;
//         }
//         else
//         {
//             for (char value : data)
//             {
//                 result += value;
//             }
//             result = ~((~(result ^ (base / 216))) ^ data[data.length() - 1]) ^ (data[0] * data[2] * data[4] * data[16] * 612);
//             if (result == base)
//             {
//                 return true;
//             }
//             else
//             {
//                 goto begin;
//             }
//         }
//         goto begin;
//         return false;
//     }

//     bool AreEquals(String stra, String strb, const String &key)
//     {
//         USING_NS_INSANE_CRYPTO;
//         const int b = (RandomExtensions::Next() / 612) * 216; // base
//         int n = b;
//         if (TransformN(b, n, stra + strb, key, true) && stra == strb)
//         {
//             TransformN(b, n, stra + strb, key, false);
//             return true;
//         }
//         else
//         {
//             TransformN(b, ++n, stra + strb, key, false);
//             return false;
//         }
//         return TransformN(b, ++n, stra + strb, key, false);
//     }
// } // namespace Internal

/* Operators */
EmscriptenVal Operator::CallOperator(const EmscriptenVal &a, const EmscriptenVal &b, const OperatorType &operatorType, const OperatorArityType &operatorArityType)
{
    val global = val::global();
    if (!VAL_INSANE)
    {
        VAL_GLOBAL.set(INSANE_STRING, val::object());
    }
    String opName = "OP"s + HexEncodingExtensions::EncodeToHex(RandomExtensions::NextBytes(16));
    String script = EMPTY_STRING;
    switch (operatorArityType)
    {
    case OperatorArityType::Unary:
        script = "Insane."s + opName + "= (a) => { return ###; };"s;
        break;
    case OperatorArityType::Binary:
        script = "Insane."s + opName + "= (a,b) => { return ###; };"s;
        break;
    default:
        break;
    }

    String op = EMPTY_STRING;
    switch (operatorType)
    {
    case OperatorType::Addition:
        op = "a + b"s;
        break;
    case OperatorType::Subtraction:
        op = "a - b"s;
        break;
    case OperatorType::Multiplication:
        op = "a * b"s;
        break;
    case OperatorType::Division:
        op = "a / b"s;
        break;
    case OperatorType::Import:
        op = "import(a)"s;
        break;
    case OperatorType::TypeOf:
        op = "typeof a";
        break;
    case OperatorType::IsNull:
        op = "a === null";
        break;
    case OperatorType::IsUndefined:
        op = "typeof a === 'undefined'";
        break;
    case OperatorType::IsNullOrUndefined:
        break;
    default:
        break;
    }

    global.call<val>("eval", StringExtensions::Replace(script, {{"###"s, op}}));
    val insane = val::global()["Insane"];

    val result = val::null();
    switch (operatorArityType)
    {
    case OperatorArityType::Unary:
        result = insane.call<val>(opName.c_str(), a);
        break;
    case OperatorArityType::Binary:
        result = insane.call<val>(opName.c_str(), a, b);
        break;
    default:
        break;
    }
    insane.delete_(opName);
    return result;
}

EmscriptenVal Operator::Add(const EmscriptenVal &a, const EmscriptenVal &b)
{
    return CallOperator(a, b, OperatorType::Addition, OperatorArityType::Binary);
}

EmscriptenVal Operator::Subtract(const EmscriptenVal &a, const EmscriptenVal &b)
{
    return CallOperator(a, b, OperatorType::Subtraction, OperatorArityType::Binary);
}

EmscriptenVal Operator::Multiply(const EmscriptenVal &a, const EmscriptenVal &b)
{
    return CallOperator(a, b, OperatorType::Multiplication, OperatorArityType::Binary);
}

EmscriptenVal Operator::Divide(const EmscriptenVal &a, const EmscriptenVal &b)
{
    return CallOperator(a, b, OperatorType::Division, OperatorArityType::Binary);
}

EmscriptenVal Operator::ImportAsync(const EmscriptenVal &a)
{
    return CallOperator(a, EmscriptenVal::null(), OperatorType::Import, OperatorArityType::Unary);
}

EmscriptenVal Operator::TypeOf(const EmscriptenVal &a)
{
    return CallOperator(a, EmscriptenVal::null(), OperatorType::TypeOf, OperatorArityType::Unary);
}

bool Operator::IsNull(const EmscriptenVal &a)
{
    return CallOperator(a, EmscriptenVal::null(), OperatorType::IsNull, OperatorArityType::Unary).as<bool>();
}

bool Operator::IsUndefined(const EmscriptenVal &a)
{
    return CallOperator(a, EmscriptenVal::null(), OperatorType::IsUndefined, OperatorArityType::Unary).as<bool>();
}

bool Operator::IsNullOrUndefined(const emscripten::val &a)
{
    return IsNull(a) || IsUndefined(a);
}

EmscriptenVal Operator::ToString(const emscripten::val &a)
{
    return VAL_GLOBAL.call<val>("String", a);
}

/* Promise */
EmscriptenVal Promise::New(const EMSCRIPTEN_VOID_FUNCTOR_TYPE(2) & promiseCallback)
{
    return val::global()["Promise"].new_(Js::Bind(promiseCallback));
}

/* Json */
template <>
emscripten::val Json::Serialize(const emscripten::val &object)
{
    return val::global()["JSON"].call<val>("stringify", object);
}

template <>
String Json::Serialize(const emscripten::val &object)
{
    return Serialize(object).as<String>();
}

/* Converter */
// template <>
// emscripten::val Converter::ToString(const emscripten::val &value)
// {
//     USING_EMSCRIPTEN;
//     return val::global().call<val>("String", value);
// }

// template <>
// String Converter::ToString(const emscripten::val &value)
// {
//     USING_EMSCRIPTEN;
//     return ToString(value).as<String>();
// }

/* Browser */
template <>
emscripten::val Browser::GetUserAgent()
{
    return val::global()["navigator"]["userAgent"];
}

template <>
String Browser::GetUserAgent<String>()
{
    return Browser::GetUserAgent().as<String>();
}

template <>
emscripten::val Browser::GetTimezoneOffsetMinutes()
{
    return val(val::global()["Date"].new_().call<val>("getTimezoneOffset").as<int>() * -1);
}

template <>
int Browser::GetTimezoneOffsetMinutes()
{
    return GetTimezoneOffsetMinutes().as<int>();
}

template <>
emscripten::val Browser::GetTimezoneOffsetSeconds()
{
    return val(GetTimezoneOffsetMinutes<int>() * 60);
}

template <>
int Browser::GetTimezoneOffsetSeconds()
{
    return GetTimezoneOffsetSeconds().as<int>();
}

template <>
emscripten::val Browser::GetTimezoneOffsetMilliseconds()
{
    USING_NS_EMSCRIPTEN;
    return val(GetTimezoneOffsetSeconds<int>() * 1000);
}

template <>
int Browser::GetTimezoneOffsetMilliseconds()
{
    return GetTimezoneOffsetMilliseconds().as<int>();
}

template <>
emscripten::val Browser::GetNameAsync(const emscripten::val &ua)
{
    String userAgent = !ua ? GetUserAgent<String>() : ua.as<String>();
    if (StringExtensions::Contains(userAgent, " OPR/", false) || StringExtensions::Contains(userAgent, " Opera/", false) || !!val::global()["opr"] || !!val::global()["opera"])
    {
        return Promise::Resolve(val("Opera"));
    }

    if (Operator::TypeOf(val::global()["InstallTrigger"]).as<String>() != "undefined" || StringExtensions::Contains(userAgent, " Firefox/", false))
    {
        return Promise::Resolve(val("Firefox"));
    }

    if (StringExtensions::Contains(userAgent, " Edg/", false) || StringExtensions::Contains(userAgent, " Edge/", false))
    {
        return Promise::Resolve(val("Microsoft Edge"));
    }

    if (!!val::global()["navigator"]["brave"])
    {
        std::function<val(val)> callback = [](val isBrave) -> val
        {
            return !!isBrave ? val("Brave") : val(EMPTY_STRING);
        };
        return val::global()["navigator"]["brave"].call<val>("isBrave").call<val>("then", Js::Bind(callback));
    }

    if (StringExtensions::Contains(userAgent, " Chrome/", false) && !!val::global()["chrome"])
    {
        return Promise::Resolve(val("Google Chrome"));
    }

    if (StringExtensions::Contains(userAgent, " Version/", false) && StringExtensions::Contains(userAgent, " Safari/", false))
    {
        return Promise::Resolve(val("Safari"));
    }

    return Promise::Resolve(val(EMPTY_STRING));
}

template <>
emscripten::val Browser::GetNameAsync(const String &ua)
{
    return GetNameAsync(ua.empty() ? GetUserAgent() : val(ua));
}

template <>
emscripten::val Browser::GetOS(const emscripten::val &ua)
{
    String userAgent = !ua ? GetUserAgent<String>() : ua.as<String>();
    if (StringExtensions::Contains(userAgent, "Windows"s, false) || StringExtensions::Contains(userAgent, "Win"s, false))
    {
        return val("Windows"s);
    }

    if (StringExtensions::Contains(userAgent, "Android"s, false))
    {
        return val("Android"s);
    }

    if (StringExtensions::Contains(userAgent, "Linux", false))
    {
        if (StringExtensions::Contains(userAgent, "Ubuntu", false))
        {
            return val("Ubuntu Linux"s);
        }
        return val("Linux"s);
    }

    if (StringExtensions::Contains(userAgent, "iPhone", false) ||
        StringExtensions::Contains(userAgent, "iOS", false))
    {
        if (StringExtensions::Contains(userAgent, "iPad", false))
        {
            return val("iPadOS");
        }
        if (StringExtensions::Contains(userAgent, "iPod", false))
        {
            return val("iOS iPod");
        }
        return val("iOS");
    }

    if (StringExtensions::Contains(userAgent, "Macintosh", false) ||
        StringExtensions::Contains(userAgent, "Mac", false) ||
        StringExtensions::Contains(userAgent, "Mac OS X", false) ||
        StringExtensions::Contains(userAgent, "Mac OS", false))
    {
        return val("MacOS"s);
    }

    return val(EMPTY_STRING);
}

template <>
emscripten::val Browser::GetOS(const String &ua)
{
    return GetOS(ua.empty() ? GetUserAgent() : val(ua));
}

template <>
String Browser::GetOS(const emscripten::val &ua)
{
    return GetOS(ua).as<String>();
}

template <>
String Browser::GetOS(const String &ua)
{
    return GetOS(ua.empty() ? GetUserAgent() : val(ua)).as<String>();
}

template <>
emscripten::val Browser::GetWebGLRenderer()
{
    val canvas = val::global()["document"].call<val>("createElement", "canvas"s);
    val context = canvas.call<val>("getContext", "webgl"s);
    if(!context)
    {
        canvas.call<void>("remove");
        return val(EMPTY_STRING);
    }
    val webGlRenderInfo = context.call<val>("getExtension", "WEBGL_debug_renderer_info"s);
    val result = val::null();
    if(!webGlRenderInfo)
    {
        result  = context.call<val>("getParameter", context.call<val>("RENDERER"));
    }
    else
    {
        result = context.call<val>("getParameter", webGlRenderInfo["UNMASKED_RENDERER_WEBGL"]);
    }
    val extension = context.call<val>("getExtension", "WEBGL_lose_context"s); 
    if(!!extension)
    {
        extension.call<void>("loseContext");
        context = val::null();
    }
    canvas.call<void>("remove");
    return !result ? val(EMPTY_STRING): result;
}

template <>
String Browser::GetWebGLRenderer<String>()
{
    return GetWebGLRenderer().as<String>();
}

template <>
emscripten::val Browser::GetWebGLVendor()
{
    val canvas = val::global()["document"].call<val>("createElement", "canvas"s);
    val context = canvas.call<val>("getContext", "webgl"s);
    if(!context)
    {
        canvas.call<void>("remove");
        return val(EMPTY_STRING);
    }
    val webGlRenderInfo = context.call<val>("getExtension", "WEBGL_debug_renderer_info"s);
    val result = val::null();
    if(!webGlRenderInfo)
    {
        result  = context.call<val>("getParameter", context.call<val>("RENDERER"));
    }
    else
    {
        result = context.call<val>("getParameter", webGlRenderInfo["UNMASKED_VENDOR_WEBGL"]);
    }
    val extension = context.call<val>("getExtension", "WEBGL_lose_context"s);
    if(!!extension)
    {
        extension.call<void>("loseContext");
        context = val::null();
    }
    canvas.call<void>("remove");
    return !result ? val(EMPTY_STRING): result;
}

template <>
String Browser::GetWebGLVendor<String>()
{
    return GetWebGLVendor().as<String>();
}

template <>
emscripten::val Browser::GetAvailableScreenHeight()
{
    return val::global()["screen"]["availHeight"];
}

template <>
int Browser::GetAvailableScreenHeight()
{
    return GetAvailableScreenHeight().as<int>();
}

template <>
emscripten::val Browser::GetAvailableScreenWidth()
{
    return val::global()["screen"]["availWidth"];
}

template <>
int Browser::GetAvailableScreenWidth()
{
    return GetAvailableScreenWidth().as<int>();
}

template <>
emscripten::val Browser::GetAvailableScreenOrientation()
{
    if (GetAvailableScreenHeight() > GetAvailableScreenWidth())
    {
        return val(static_cast<int>(OrientationType::PORTRAIT));
    }

    if (GetAvailableScreenWidth() > GetAvailableScreenHeight())
    {
        return val(static_cast<int>(OrientationType::LANDSCAPE));
    }

    return val(static_cast<int>(OrientationType::SQUARE));
}

template <>
OrientationType Browser::GetAvailableScreenOrientation()
{
    return static_cast<OrientationType>(GetAvailableScreenOrientation().as<int>());
}

template <>
emscripten::val Browser::GetAvailableScreenSize()
{
    return val(GetAvailableScreenHeight<int>() * GetAvailableScreenWidth<int>());
}

template <>
int Browser::GetAvailableScreenSize()
{
    return GetAvailableScreenSize().as<int>();
}

template <>
emscripten::val Browser::GetScreenHeight()
{
    return val::global()["screen"]["height"];
}

template <>
int Browser::GetScreenHeight()
{
    return GetScreenHeight().as<int>();
}

template <>
emscripten::val Browser::GetScreenWidth()
{
    return val::global()["screen"]["width"];
}

template <>
int Browser::GetScreenWidth()
{
    return GetScreenWidth().as<int>();
}

template <>
emscripten::val Browser::GetScreenOrientation()
{
    if (GetScreenHeight() > GetScreenWidth())
    {
        return val(static_cast<int>(OrientationType::PORTRAIT));
    }

    if (GetScreenWidth() > GetScreenHeight())
    {
        return val(static_cast<int>(OrientationType::LANDSCAPE));
    }

    return val(static_cast<int>(OrientationType::SQUARE));
}

template <>
OrientationType Browser::GetScreenOrientation()
{
    return static_cast<OrientationType>(GetScreenOrientation().as<int>());
}

template <>
emscripten::val Browser::GetScreenSize()
{
    USING_NS_EMSCRIPTEN;
    return val(GetScreenHeight<int>() * GetScreenWidth<int>());
}

template <>
int Browser::GetScreenSize()
{
    return GetScreenSize().as<int>();
}

template <>
emscripten::val Browser::GetViewportHeight()
{
    USING_NS_EMSCRIPTEN;
    return val::global()["innerHeight"];
}

template <>
int Browser::GetViewportHeight()
{
    return GetViewportHeight().as<int>();
}

template <>
emscripten::val Browser::GetViewportWidth()
{
    return val::global()["innerWidth"];
}

template <>
int Browser::GetViewportWidth()
{
    return GetViewportWidth().as<int>();
}

template <>
emscripten::val Browser::GetViewportOrientation()
{
    if (GetViewportHeight() > GetViewportWidth())
    {
        return val(static_cast<int>(OrientationType::PORTRAIT));
    }

    if (GetViewportWidth() > GetViewportHeight())
    {
        return val(static_cast<int>(OrientationType::LANDSCAPE));
    }

    return val(static_cast<int>(OrientationType::SQUARE));
}

template <>
OrientationType Browser::GetViewportOrientation()
{
    return static_cast<OrientationType>(GetViewportOrientation().as<int>());
}

template <>
emscripten::val Browser::GetViewportSize()
{
    return val(GetViewportHeight<int>() * GetViewportWidth<int>());
}

template <>
int Browser::GetViewportSize()
{
    return GetViewportSize().as<int>();
}

template <>
emscripten::val Browser::GetLanguages()
{
    val lang = val::global()["navigator"]["languages"];
    return !!lang ? lang : val::array();
}

template <>
std::vector<String> Browser::GetLanguages()
{
    return vecFromJSArray<String>(GetLanguages());
}

template <>
emscripten::val Browser::GetLanguage()
{
    return val::global()["navigator"]["language"];
}

template <>
String Browser::GetLanguage()
{
    return GetLanguage().as<String>();
}

template <>
emscripten::val Browser::GetMaxTouchPoints()
{
    return val::global()["navigator"]["maxTouchPoints"];
}

template <>
int Browser::GetMaxTouchPoints()
{
    return GetMaxTouchPoints().as<int>();
}

template <>
emscripten::val Browser::GetDeviceMemory()
{
    if (val::global()["isSecureContext"].as<bool>())
    {
        val memory = val::global()["navigator"]["deviceMemory"];
        return !!memory ? memory : val(-1);
    }
    else
    {
        return val(0);
    }
}

template <>
float Browser::GetDeviceMemory()
{
    return GetDeviceMemory().as<float>();
}

template <>
emscripten::val Browser::GetHardwareConcurrency()
{
    val concurrency = val::global()["navigator"]["hardwareConcurrency"];
    return !!concurrency ? concurrency : val(-1);
}

template <>
int Browser::GetHardwareConcurrency()
{
    return GetHardwareConcurrency().as<int>();
}

template <>
emscripten::val Browser::GetPlugins()
{
    val plugins = val::global()["navigator"]["plugins"];
    val result = val::array();
    if (!plugins)
    {
        return result;
    }
    val pluginsLength = plugins["length"];
    for (int i = 0; i < plugins["length"].as<int>(); i++)
    {
        val pluginValue = plugins.call<val>("item", i);
        val plugin = val::object();
        plugin.set("Name", !!pluginValue["name"] ? pluginValue["name"] : val(EMPTY_STRING));
        plugin.set("Description", !!pluginValue["description"] ? pluginValue["description"] : val(EMPTY_STRING));
        plugin.set("MimeTypes", val::array());
        val mimeTypes = val::array();
        for (int j = 0; j < pluginValue["length"].as<int>(); j++)
        {
            val mimeValue = pluginValue.call<val>("item", j);
            val mime = val::object();
            mime.set("Description", !!mimeValue["description"] ? mimeValue["description"] : val(EMPTY_STRING));
            mime.set("Type", !!mimeValue["type"] ? mimeValue["type"] : val(EMPTY_STRING));
            mime.set("Suffixes", !!mimeValue["suffixes"] ? mimeValue["suffixes"] : val(EMPTY_STRING));
            plugin["MimeTypes"].call<val>("push", mime);
        }
        result.call<val>("push", plugin);
    }
    return result;
}

template <>
std::vector<Browser::Plugin> Browser::GetPlugins()
{
    std::vector<Plugin> result;
    val plugins = GetPlugins();
    for (int i = 0; i < plugins["length"].as<int>(); i++)
    {
        Plugin plugin;
        plugin.Name = plugins[i]["Name"].as<String>();
        val mimeTypes = plugins[i]["MimeTypes"];
        for (int j = 0; j < mimeTypes["length"].as<int>(); j++)
        {
            MimeType mime;
            mime.Description = mimeTypes[j]["Description"].as<String>();
            mime.Type = mimeTypes[j]["Type"].as<String>();
            mime.Suffixes = mimeTypes[j]["Suffixes"].as<String>();
            plugin.MimeTypes.push_back(mime);
        }
        result.push_back(plugin);
    }
    return result;
}

template <>
emscripten::val Browser::GetMimeTypes()
{
    val mimeTypes = val::global()["navigator"]["mimeTypes"];
    val result = val::global().array();
    if (!mimeTypes)
    {
        return result;
    }
    for (int i = 0; i < mimeTypes["length"].as<int>(); i++)
    {
        val mimeValue = mimeTypes.call<val>("item", i);
        val mime = val::object();
        mime.set("Description", !!mimeValue["description"] ? mimeValue["description"] : val(EMPTY_STRING));
        mime.set("Type", !!mimeValue["type"] ? mimeValue["type"] : val(EMPTY_STRING));
        mime.set("Suffixes", !!mimeValue["suffixes"] ? mimeValue["suffixes"] : val(EMPTY_STRING));
        result.call<val>("push", mime);
    }
    return result;
}

template <>
std::vector<Browser::MimeType> Browser::GetMimeTypes()
{
    std::vector<MimeType> result;
    val mimes = GetMimeTypes();
    for (int i = 0; i < mimes["length"].as<int>(); i++)
    {
        MimeType mime;
        mime.Description = mimes[i]["Description"].as<String>();
        mime.Type = mimes[i]["Type"].as<String>();
        mime.Suffixes = mimes[i]["Suffixes"].as<String>();
        result.push_back(mime);
    }
    return result;
}

template <>
emscripten::val Browser::GetDoNotTrack()
{
    val dnt = val::global()["navigator"]["doNotTrack"];
    return !!dnt ? dnt : val("unspecified"s);
}

template <>
String Browser::GetDoNotTrack()
{
    return GetDoNotTrack().as<String>();
}

template <>
emscripten::val Browser::HasCookiesSupport()
{
    bool result = true && !!val::global()["navigator"]["cookieEnabled"];
    if (!!result)
    {
        val::global()["document"].set("cookie", "InsaneCookie=1;SameSite=Strict"_val);
        result = StringExtensions::Contains(val::global()["document"]["cookie"].as<String>(), "InsaneCookie=0;SameSite=Strict;");
        val::global()["document"].set("cookie", "InsaneCookie=1;SameSite=Strict;expires="s + val::global()["Date"].new_(1970, 1, 1, 0, 0, 0, 0).call<val>("toGMTString").as<String>());
    }
    return val(result);
}

template <>
bool Browser::HasCookiesSupport()
{
    return HasCookiesSupport().as<bool>();
}

template <>
EmscriptenVal Browser::GetFingerprintAsync(const EmscriptenVal &key)
{
    String result = Browser::GetUserAgent<String>();
    result += std::to_string(Browser::GetTimezoneOffsetMilliseconds<int>());
    result += std::to_string(Browser::GetTimezoneOffsetSeconds<int>());
    result += std::to_string(Browser::GetTimezoneOffsetMinutes<int>());
    EMSCRIPTEN_VAL_FUNCTOR_TYPE(1)
    callback = [result, key](emscripten::val name) -> emscripten::val
    {
        String ret = result + name.as<String>();
        ret += Browser::GetOS<String>();
        ret += Browser::GetWebGLRenderer<String>();
        ret += Browser::GetWebGLVendor<String>();
        ret += std::to_string(Browser::GetScreenSize<int>());
        ret += Browser::GetLanguage<String>();
        ret += Json::Serialize<String>(Browser::GetLanguages());
        ret += std::to_string(Browser::GetMaxTouchPoints<int>());
        ret += std::to_string(Browser::GetDeviceMemory<float>());
        ret += std::to_string(Browser::GetHardwareConcurrency<int>());
        ret += Json::Serialize<String>(Browser::GetMimeTypes());
        ret += Json::Serialize<String>(Browser::GetPlugins());
        ret += Browser::GetDoNotTrack<String>();
        ret += std::to_string(Browser::HasCookiesSupport<bool>());
        val objectClass = val::global()["Object"];
        val props = objectClass.call<val>("getOwnPropertyNames", objectClass.call<val>("getPrototypeOf", val::global()["navigator"])).call<val>("sort");
        ret += Json::Serialize<String>(props);
        ret += Operator::ToString(props["length"]).as<String>();
        return val("Insane"s + Base64EncodingExtensions::EncodeToBase64(HashExtensions::ComputeHmac(ConverterExtensions::StringToStdVectorUint8(ret), ConverterExtensions::StringToStdVectorUint8(key.as<String>()))));
    };
    return Browser::GetNameAsync().call<val>("then", Js::Bind(callback));
}

template <>
EmscriptenVal Browser::GetFingerprintAsync(const String &key)
{
    return GetFingerprintAsync(val(key));
}

/* Js */

void Js::SetProperty(EmscriptenVal object, const String &property, const EmscriptenVal &value, const bool &replaceIfExists)
{
    if (Operator::IsNullOrUndefined(object[property]))
    {
        object.set(property, value);
    }
    else
    {
        if (replaceIfExists)
        {
            object.set(property, value);
        }
    }
}

void Js::SetPropertyObject(EmscriptenVal object, const String &property, const bool &replaceIfExists)
{
    SetProperty(object, property, EmscriptenVal::global().object(), replaceIfExists);
}

void Js::SetPropertyArray(EmscriptenVal object, const String &property, const bool &replaceIfExists)
{
    SetProperty(object, property, EmscriptenVal::global().array(), replaceIfExists);
}

void Js::SetPropertyNull(EmscriptenVal object, const String &property, const bool &replaceIfExists)
{
    SetProperty(object, property, EmscriptenVal::global().null(), replaceIfExists);
}

String Js::GetPropertyName(const String &name, const String &key, const String &suffix)
{
    return (suffix.empty() ? INSANE_PROPERTY_SUFFIX : suffix) + HexEncodingExtensions::EncodeToHex(HashExtensions::ComputeHmac( name, key, HashAlgorithm::Sha512));
}

emscripten::val Js::Bind(const emscripten::val &fx)
{
    return fx["opcall"].call<val>("bind", fx);
}

template <>
emscripten::val Js::LoadScriptAsync(const emscripten::val &scriptpath)
{
    String id = Js::GetPropertyName(scriptpath.as<String>(), EMPTY_STRING, INSANE_PROPERTY_SUFFIX);
    Js::SetPropertyObject(val::global(), INSANE_STRING, false);
    String loadedName = Js::GetPropertyName("Loaded"s, EMPTY_STRING, INSANE_PROPERTY_SUFFIX);
    Js::SetPropertyObject(val::global()["Insane"], loadedName, false);
    val loaded = VAL_INSANE[loadedName];
    if (!Operator::IsNullOrUndefined(loaded[id]))
    {
        return Promise::Resolve(id);
    }
    loaded.set(id, val(true));
    EMSCRIPTEN_VOID_FUNCTOR_TYPE(2)
    promiseCallback = [scriptpath, id, loaded](val resolve, val reject) -> void
    {
        val script = val::global("document").call<val>("createElement", val("script"));
        script.set("id", val(id));
        script.set("src", scriptpath);
        script.set("type", val("text/javascript"));
        EMSCRIPTEN_VOID_FUNCTOR_TYPE(1)
        onloadCallback = [scriptpath, resolve, id]([[maybe_unused]] val a) -> void
        {
            resolve(id);
        };
        script.set("onload", Js::Bind(onloadCallback));
        EMSCRIPTEN_VOID_FUNCTOR_TYPE(1)
        onerrorCallback = [scriptpath, id, reject, loaded]([[maybe_unused]] val a) -> void
        {
            val::global("document").call<val>("getElementById", val(id)).call<void>("remove");
            loaded.delete_(id);
            reject(id);
        };
        script.set("onerror", Js::Bind(onerrorCallback));
        val::global("document")["body"].call<val>("appendChild", script);
    };
    return Promise::New(promiseCallback);
}

template <>
emscripten::val Js::LoadScriptAsync(const String &scriptpath)
{
    return LoadScriptAsync(val(scriptpath));
}

/* LocalStorage */

template <>
emscripten::val LocalStorage::GetValue(const EmscriptenVal &key, const EmscriptenVal &password) noexcept
{
    // USING_NS_EMSCRIPTEN;
    // USING_NS_INSANE_EXCEPTION;
    // USING_NS_INSANE_CRYPTO;
    // USING_NS_INSANE_STR;
    // try
    // {
    //     String passwordStr = Operator::IsNullOrUndefined(password) ? EMPTY_STRING : Operator::ToString(password).as<String>();
    //     const val value = val::global("localStorage").call<val>("getItem", key);
    //     if (passwordStr == EMPTY_STRING)
    //     {
    //         return !value ? val(EMPTY_STRING) : value;
    //     }
    //     return !value ? val(EMPTY_STRING) : val(Base64EncodingExtensions::DecodeFromBase64(AesExtensions::DecryptAesCbc(value.as<String>(), passwordStr)));
    // }
    // catch (const std::exception &ex)
    // {
    //     return val(EMPTY_STRING);
    // }
    // catch (...)
    // {
    //     return val(EMPTY_STRING);
    // }
}

template <>
String LocalStorage::GetValue(const EmscriptenVal &key, const EmscriptenVal &password) noexcept
{
    return GetValue(key, password).as<String>();
}

template <>
EmscriptenVal LocalStorage::GetValue(const String &key, const String &password) noexcept
{
    return GetValue(val(key), val(password));
}

template <>
String LocalStorage::GetValue(const String &key, const String &password) noexcept
{
    return GetValue(val(key), val(password)).as<String>();
}

template <>
void LocalStorage::SetValue(const EmscriptenVal &key, const EmscriptenVal &value, const EmscriptenVal &password) noexcept
{
    // try
    // {
    //     StdVectorUint8 passwordStr = ConverterExtensions::  Operator::IsNullOrUndefined(password) ? EMPTY_STRING : Operator::ToString(password).as<String>();
    //     val::global("localStorage").call<void>("setItem", key, Base64EncodingExtensions::ToBase64(AesExtensions::EncryptAesCbc(Operator::ToString(value).as<String>(), passwordStr)));
    // }
    // catch (const ExceptionBase &e)
    // {
    // }
    //throw NotImplementedException(INSANE_FUNCTION_SIGNATURE, __FILE__, __LINE__);
}

template <>
void LocalStorage::SetValue(const String &key, const String &value, const String &password) noexcept
{
    SetValue(val(key), val(value), val(password));
}

void Js::ThrowError(const String &message)
{
    val::global("Error").new_(val(message)).throw_();
}

template <>
void LocalStorage::RemoveValue(const emscripten::val &key) noexcept
{
    val::global("localStorage").call<void>("removeItem", key);
}

template <>
void LocalStorage::RemoveValue(const String &key) noexcept
{
    RemoveValue(val(key));
}

void LocalStorage::Clear()
{
    val::global("localStorage").call<void>("clear");
}

template <>
void LocalStorage::RemoveValuesStartingWith(const EmscriptenVal &preffix)
{
    for (val value : emscripten::vecFromJSArray<val>(val::global("Object").call<val>("entries", val::global("localStorage"))))
    {
        if (StringExtensions::StartsWith(value[0].as<String>(), preffix.as<String>()))
        {
            LocalStorage::RemoveValue(value[0]);
        }
    }
}

template <>
void LocalStorage::RemoveValuesStartingWith(const String &preffix)
{
    RemoveValuesStartingWith(val(preffix));
}

// ███ Interop ███
StdVectorUint8 InteropExtensions::JsUint8ArrayToStdVectorUint8(const EmscriptenVal &uint8Array)
{
    size_t length = uint8Array["length"].as<size_t>();
    std::vector<uint8_t> result;
    for (size_t i = 0; i < length; i++)
    {
        result.push_back(uint8Array[i].as<uint8_t>());
    }
    return result;
}

StdVectorUint8 InteropExtensions::JsStringToStdVectorUint8(const EmscriptenVal &str)
{
    return ConverterExtensions::StringToStdVectorUint8(str.as<String>());
}

EmscriptenVal InteropExtensions::StdVectorUint8ToJsUint8Array(const StdVectorUint8 &vector)
{
    val array8 = val::array(vector);
    return val::global("Uint8Array").new_(array8);
}

void InteropExtensions::PrintStdVectorUint8(StdVectorUint8 vector)
{
    for (size_t i = 0; i < vector.size(); i++)
    {
        Console::Log(vector[i]);
    }
}