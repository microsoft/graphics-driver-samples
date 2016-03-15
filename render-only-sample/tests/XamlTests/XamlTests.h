#ifndef _XAMLTESTS_H_
#define _XAMLTESTS_H_

class XamlTests {
    BEGIN_TEST_CLASS(XamlTests)
        TEST_CLASS_PROPERTY(L"RunAs", L"UAP")
        TEST_CLASS_PROPERTY(L"UAP:Host", L"XAML")
    END_TEST_CLASS()

    TEST_CLASS_SETUP(ClassSetup)

    BEGIN_TEST_METHOD(TestResizeWindow)
        TEST_METHOD_PROPERTY(
            L"Description",
            L"Shows how to use RunOnUIThread and attach event handlers to "
            L"WinRT objects.")
    END_TEST_METHOD()
    
    BEGIN_TEST_METHOD(TestLoadXaml)
        TEST_METHOD_PROPERTY(L"DataSource", L"Table:TestData.xml#XamlControls")
        TEST_METHOD_PROPERTY(
            L"Description",
            L"Shows how to load XAML from a string and display in current window")
    END_TEST_METHOD()
};

#endif // _XAMLTESTS_H_
