#include "precomp.h"

#include "util.h"
#include "XamlTests.h"

using namespace ABI::Windows::Foundation;
using namespace ABI::Windows::ApplicationModel::Core;
using namespace ABI::Windows::UI::Core;
using namespace ABI::Windows::UI::Xaml;
using namespace ABI::Windows::UI::Xaml::Markup;
using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;
using namespace WEX::TestExecution;

ComPtr<IWindow> GetCurrentWindow ()
{
    ComPtr<IWindowStatics> windowStatics;
    VERIFY_SUCCEEDED(
        Windows::Foundation::GetActivationFactory(
            HStringReference(RuntimeClass_Windows_UI_Xaml_Window).Get(),
            &windowStatics),
        L"Getting IWindowStatics");

    ComPtr<IWindow> currentWindow;
    VERIFY_SUCCEEDED(
        windowStatics->get_Current(&currentWindow),
        L"Getting current window");

    return currentWindow;
}

ComPtr<IUIElement> XamlFromString (PCWSTR XamlStr)
{
    ComPtr<IXamlReaderStatics> xamlReaderStatics;
    VERIFY_SUCCEEDED(
        Windows::Foundation::GetActivationFactory(
            HStringReference(RuntimeClass_Windows_UI_Xaml_Markup_XamlReader).Get(),
            &xamlReaderStatics),
        L"Getting IXamlReaderStatics");

    ComPtr<IInspectable> inspectable;
    VERIFY_SUCCEEDED(
        xamlReaderStatics->Load(HStringReference(XamlStr).Get(), &inspectable),
        L"Loading XAML string");

    ComPtr<IUIElement> uiElement;
    VERIFY_SUCCEEDED(
        inspectable.As(&uiElement),
        L"Doing QueryInterface for IUIElement");

    return uiElement;
}

void SetXamlContent (PCWSTR XamlString)
{
    // Load the XAML content
    auto uiElement = XamlFromString(XamlString);

    // Set it as the Window's content
    VERIFY_SUCCEEDED(
        GetCurrentWindow()->put_Content(uiElement.Get()),
        L"Setting UI element as root of window");
}

bool XamlTests::ClassSetup ()
{
    return true;
}

//
// The purpose of this test is to demonstrate how to use RunOnUIThread
// and how to attach/detach a XAML event handler. This test does no actual
// verification.
//
void XamlTests::TestResizeWindow ()
{
    // run on UI thread
    AgileRef agileWindow;
    EventRegistrationToken token;
    HRESULT hr = RunOnUIThread([&]
    {
        auto currentWindow = GetCurrentWindow();

        VERIFY_SUCCEEDED(
            currentWindow.AsAgile(&agileWindow),
            L"Getting agile window reference");

        boolean visible;
        VERIFY_SUCCEEDED(
            currentWindow->get_Visible(&visible),
            L"Getting visibility of current window");

        LogComment(L"Current window visibility: %d", visible);

        Rect bounds;
        VERIFY_SUCCEEDED(
            currentWindow->get_Bounds(&bounds),
            L"Getting bounds");

        LogComment(
            L"Current window has bounds [%f %f %f %f]",
            bounds.X,
            bounds.Y,
            bounds.Width,
            bounds.Height);


        HRESULT hr = currentWindow->add_SizeChanged(
            Callback<IWindowSizeChangedEventHandler>([&] (
                IInspectable* /*Sender*/,
                IWindowSizeChangedEventArgs* Args
                )
            {
                Size size;
                VERIFY_SUCCEEDED(Args->get_Size(&size));

                LogComment(L"Size is [%f, %f]", size.Width, size.Height);

                return S_OK;
            }).Get(),
            &token);

        VERIFY_SUCCEEDED(hr, L"Verifying add_SizeChanged succeeded");
    });
    VERIFY_SUCCEEDED(hr, L"Verifying that UI thread operation succeeded");

    Sleep(5000);

    RunOnUIThread([&] { GetCurrentWindow()->remove_SizeChanged(token); });
}

void XamlTests::TestLoadXaml ()
{
    WEX::Common::String xamlString;
    VERIFY_SUCCEEDED(
        TestData::TryGetValue(L"Xaml", xamlString),
        L"Getting XAML fragment from test data");

    HRESULT hr = RunOnUIThread([&]
    {
        SetXamlContent(xamlString.operator const wchar_t*());
    });

    VERIFY_SUCCEEDED(hr, L"Verifying that UI thread operation succeeded");

    Sleep(1000);
}
