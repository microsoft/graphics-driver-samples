#include "CosUmd12.h"

HRESULT CosUmd12ExtendedFeatures_Ddi_GetSupportedExtendedFeatures_0020(
    D3D12DDI_HDEVICE hDevice,
    _Inout_ UINT32* pFeatureCount, 
    _Out_writes_opt_(*pFeatureCount) D3D12DDI_FEATURE_0020* pFeatureList)
{
    if (pFeatureList == nullptr)
    {
        assert(pFeatureCount != nullptr);
        *pFeatureCount = 0;
        return S_OK;
    }
    else
    {
        return E_INVALIDARG;
    }
}

HRESULT CosUmd12ExtendedFeatures_Ddi_GetSupportedExtendedFeatureVersions_0020(
    D3D12DDI_HDEVICE hDevice,
    D3D12DDI_FEATURE_0020 Feature, 
    _Inout_ UINT32* pFeatureVersionCount, 
    _Out_writes_opt_( *pFeatureVersionCount ) UINT32* pFeatureVersionList)
{
    if (pFeatureVersionList == nullptr)
    {
        assert(pFeatureVersionCount != nullptr);
        *pFeatureVersionCount = 0;
        return S_OK;
    }
    else
    {
        return E_INVALIDARG;
    }
}

HRESULT CosUmd12ExtendedFeatures_Ddi_EnableExtendedFeature_0020(
    D3D12DDI_HDEVICE hDevice, 
    D3D12DDI_FEATURE_0020 Feature, 
    UINT32 FeatureVersion)
{
    return E_NOTIMPL;
}

D3D12DDI_EXTENDED_FEATURES_FUNCS_0020 g_CosUmd12ExtendedFeatures_Ddi_0020 =
{
    CosUmd12ExtendedFeatures_Ddi_GetSupportedExtendedFeatures_0020,         // pfnGetSupportedExtendedFeatures
    CosUmd12ExtendedFeatures_Ddi_GetSupportedExtendedFeatureVersions_0020,  // pfnGetSupportedExtendedFeatureVersions
    CosUmd12ExtendedFeatures_Ddi_EnableExtendedFeature_0020,                // pfnEnableExtendedFeature
};
