@REM
@REM Copyright (c) Microsoft Corporation.  All rights reserved.
@REM
@REM
@REM Use of this sample source code is subject to the terms of the Microsoft
@REM license agreement under which you licensed this sample source code. If
@REM you did not accept the terms of the license agreement, you are not
@REM authorized to use this sample source code. For the terms of the license,
@REM please see the license agreement between you and Microsoft or, if applicable,
@REM see the LICENSE.RTF on your install media or the root of your tools installation.
@REM THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES OR INDEMNITIES.
@REM
call fxc /T vs_4_0_level_9_1 /E "ShadeDolphinVertex" /Fo DolphinTween.xvu /Fc DolphinTween.txt Dolphin.vsh

call fxc /T vs_4_0_level_9_1 /E "ShadeSeaFloorVertex" /Fo SeaFloor.xvu /Fc SeaFloor.txt Dolphin.vsh

call fxc /T ps_4_0_level_9_1 /E "ShadeCausticsPixel" /Fo ShadeCausticsPixel.xpu /Fc ShadeCausticsPixel.txt Dolphin.psh

