#pragma once

#include "CosKmd.h"
#include "CosGpuCommand.h"

#include "CosKmdGlobal.h"
#include "CosKmdContext.h"

void
CosKmExecuteMetaCommand(
    CosKmContext *      pKmContext,
    GpuHwMetaCommand *  pMetaCommand);

