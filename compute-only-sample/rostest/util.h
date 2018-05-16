#ifndef _ROSTEST_UTIL_H_
#define _ROSTEST_UTIL_H_

#define ROS_DRIVER_NAME_WSZ L"Render Only Sample Driver"

#define LogComment(fmt, ...) WEX::Logging::Log::Comment( \
    WEX::Common::NoThrowString().Format((fmt), __VA_ARGS__))

#define LogWarning(fmt, ...) WEX::Logging::Log::Warning( \
    WEX::Common::NoThrowString().Format((fmt), __VA_ARGS__))

#define LogError(fmt, ...) WEX::Logging::Log::Error( \
    WEX::Common::NoThrowString().Format((fmt), __VA_ARGS__))

template <typename Fn>
struct _Finally : public Fn
{
    _Finally (Fn&& Func) : Fn(std::forward<Fn>(Func)) {}
    _Finally (const _Finally&); // generate link error if copy constructor is called
    ~_Finally () { this->operator()(); }
};

template <typename Fn>
inline _Finally<Fn> Finally (Fn&& Func)
{
    return {std::forward<Fn>(Func)};
}
    
template <typename TFunction>
HRESULT RunOnUIThread (const TFunction& Function)
{
    using namespace ABI::Windows::Foundation;
    using namespace ABI::Windows::ApplicationModel::Core;
    using namespace ABI::Windows::UI::Core;
    using namespace Microsoft::WRL;

    // Get the ICoreApplication
    ComPtr<ICoreImmersiveApplication> application;
    HRESULT hr = Windows::Foundation::GetActivationFactory(
            Wrappers::HStringReference(
                RuntimeClass_Windows_ApplicationModel_Core_CoreApplication).Get(),
            &application);
    if (FAILED(hr))
    {
        return hr;
    }

    // Get the ICoreApplicationView
    ComPtr<ICoreApplicationView> view;
    hr = application->get_MainView(&view);
    if (FAILED(hr))
    {
        return hr;
    }

    // Get the ICoreWindow
    ComPtr<ICoreWindow> window;
    hr = view->get_CoreWindow(&window);
    if (FAILED(hr))
    {
        return hr;
    }

    // Get the dispatcher
    ComPtr<ICoreDispatcher> dispatcher;
    hr = window->get_Dispatcher(&dispatcher);
    if (FAILED(hr))
    {
        return hr;
    }

    // Create an event so we can wait for the callback to complete
    Wrappers::Event completedEvent(CreateEvent(nullptr, TRUE, FALSE, nullptr));
    if (!completedEvent.IsValid())
    {
        return HRESULT_FROM_WIN32(::GetLastError());
    }

    // Create the dispatch callback
    HRESULT invokeResult = E_FAIL;

    typedef Microsoft::WRL::Implements<
        Microsoft::WRL::RuntimeClassFlags<::Microsoft::WRL::ClassicCom>,
        ABI::Windows::UI::Core::IDispatchedHandler,
        Microsoft::WRL::FtmBase> AgileDispatcherCallback;
    auto dispatchCallback = Callback<AgileDispatcherCallback>([&] () -> HRESULT
    {
        invokeResult = WEX::SafeInvoke([&] () -> bool
        {
            Function();
            return true;
        });
        return S_OK;
    });

    if (!dispatchCallback)
    {
        return E_OUTOFMEMORY;
    }

    // Create the completion callback
    auto completionCallback = Callback<IAsyncActionCompletedHandler>(
        [&completedEvent] (IAsyncAction*, AsyncStatus) -> HRESULT
    {
        return SetEvent(completedEvent.Get()) ? S_OK : E_FAIL;
    });

    if (!completionCallback)
    {
        return E_OUTOFMEMORY;
    }

    // Dispatch the callback
    ComPtr<IAsyncAction> asyncAction;
    hr = dispatcher->RunAsync(
            CoreDispatcherPriority_Normal,
            dispatchCallback.Get(),
            &asyncAction);
    if (FAILED(hr))
    {
        return hr;
    }

    // Subscribe to its completed event
    hr = asyncAction->put_Completed(completionCallback.Get());
    if (FAILED(hr))
    {
        return hr;
    }

    // Wait for the callback to complete
    if (WaitForSingleObject(completedEvent.Get(), INFINITE) != WAIT_OBJECT_0) {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    return invokeResult;
}

//
// An exception deriving from WEX::Common::Exception that can be constructed
// with a printf-style message and explicitly thrown via the 'throw' keyword.
// The problem with VERIFY_ and WEX::Common::Throw::* is that they are functions,
// so the compiler and OACR don't know they cause a change in control flow. 
// An explicit 'throw' in the source makes it clear to both the reader
// and compiler that the current scope is being exited via an exception.
//
// Usage:
//   throw MyException::Make(E_FAIL, L"Something bad happened (%d)", value);
//
class MyException : public WEX::Common::Exception {
public:
    explicit MyException (_In_range_(<, 0) HRESULT Hr) /*noexcept*/ :
        Exception(Hr) {}
    MyException (_In_range_(<, 0) HRESULT Hr, PCWSTR Message) /*noexcept*/ :
        Exception(Hr, Message) {}

    static MyException Make (
        HRESULT HResult,
        _In_ _Printf_format_string_ PCWSTR Format,
        ...
        ) /*noexcept*/
    {
        WCHAR buf[512];

        va_list argList;
        va_start(argList, Format);

        HRESULT hr = StringCchVPrintfW(
            buf,
            ARRAYSIZE(buf),
            Format,
            argList);

        va_end(argList);

        return MyException(HResult, SUCCEEDED(hr) ? buf : Format);
    }

    MyException& operator= (const MyException&) = delete;
};

PCWSTR StringFromFeatureLevel (D3D_FEATURE_LEVEL FeatureLevel);

Microsoft::WRL::ComPtr<IDXGIAdapter2> FindAdapter (PCWSTR Description);

void CreateDevice (
    _COM_Outptr_ ID3D11Device3** DevicePPtr,
    _COM_Outptr_ ID3D11DeviceContext3** ContextPPtr
    );

void SaveTextureToBmp (PCWSTR FileName, ID3D11Texture2D* Texture);
    
#endif // _ROSTEST_UTIL_H_
