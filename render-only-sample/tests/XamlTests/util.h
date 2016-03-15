#ifndef _XAMLTESTS_UTIL_H_
#define _XAMLTESTS_UTIL_H_

#define LogComment(fmt, ...) WEX::Logging::Log::Comment( \
    WEX::Common::NoThrowString().Format((fmt), __VA_ARGS__))

#define LogWarning(fmt, ...) WEX::Logging::Log::Warning( \
    WEX::Common::NoThrowString().Format((fmt), __VA_ARGS__))

#define LogError(fmt, ...) WEX::Logging::Log::Error( \
    WEX::Common::NoThrowString().Format((fmt), __VA_ARGS__))

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

#endif // _XAMLTESTS_UTIL_H_
