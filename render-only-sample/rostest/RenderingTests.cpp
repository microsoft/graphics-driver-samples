#include "precomp.h"

#include "util.h"
#include "RenderingTests.h"

using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;
using namespace WEX::TestExecution;

bool RenderingTests::ClassSetup ()
{
    CreateDevice(&m_device, &m_context);
    return true;
}
