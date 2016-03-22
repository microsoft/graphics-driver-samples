#ifndef _RESOURCE_TESTS_H_
#define _RESOURCE_TESTS_H_

//
// Tests related to resources (creating, copying, updating, mapping, etc..)
//
class ResourceTests {
    BEGIN_TEST_CLASS(ResourceTests)
        TEST_CLASS_PROPERTY(L"ThreadingModel", L"MTA")
    END_TEST_CLASS()

    TEST_CLASS_SETUP(ClassSetup)

    BEGIN_TEST_METHOD(TestShaderConstantBuffer)
        TEST_METHOD_PROPERTY(
            L"Description",
            L"Verifies that a constant buffer can be created and copied.")
    END_TEST_METHOD()

    Microsoft::WRL::ComPtr<ID3D11Device3> m_device;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext3> m_context;
};

#endif // _RESOURCE_TESTS_H_
