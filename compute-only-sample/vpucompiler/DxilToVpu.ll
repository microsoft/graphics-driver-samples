; Thunk dxil intrinsics to VpuShaderLib implementations
;
;
target datalayout = "e-m:x-p:32:32-i64:64-f80:32-n8:16:32-a:0:32-S32"
target triple = "i686-pc-windows-msvc19.16.27025"


%dx.types.Handle = type { i8* }
%dx.types.ResRet.i32 = type { i32, i32, i32, i32, i32 }
%dx.types.ResRet.f32 = type { float, float, float, float, i32 }

declare i8* @dx_op_createHandle(i8 zeroext, i32, i32, i8 zeroext) #0
declare void @dx_op_bufferLoad_f32(%dx.types.ResRet.f32*, %dx.types.Handle, i32, i32) #0
declare void @dx_op_bufferLoad_i32(%dx.types.ResRet.i32*, %dx.types.Handle, i32, i32) #0
declare void @dx_op_bufferStore_f32(%dx.types.Handle, i32, i32, float, float, float, float, i8 zeroext) #0
declare void @dx_op_bufferStore_i32(%dx.types.Handle, i32, i32, i32, i32, i32, i32, i8 zeroext) #0
declare i32 @dx_op_threadId_i32() #0

define i8* @dx.op.createHandle(i32, i8, i32, i32, i1) #1 {
	%non_uniform = zext i1 %4 to i8
	%result = call i8* @dx_op_createHandle(i8 %1, i32 %2, i32 %3, i8 %non_uniform)
	ret i8* %result
}

define %dx.types.ResRet.i32 @dx.op.bufferLoad.i32(i32, %dx.types.Handle, i32, i32) {
  %buf = alloca %dx.types.ResRet.i32, align 4
  call void @dx_op_bufferLoad_i32(%dx.types.ResRet.i32* sret %buf, %dx.types.Handle %1, i32 %2, i32 %3) 
  %result = load %dx.types.ResRet.i32, %dx.types.ResRet.i32* %buf
  ret %dx.types.ResRet.i32 %result
}

define %dx.types.ResRet.f32 @dx.op.bufferLoad.f32(i32, %dx.types.Handle, i32, i32) {
  %buf = alloca %dx.types.ResRet.f32, align 4
  call void @dx_op_bufferLoad_f32(%dx.types.ResRet.f32* sret %buf, %dx.types.Handle %1, i32 %2, i32 %3) 
  %result = load %dx.types.ResRet.f32, %dx.types.ResRet.f32* %buf
  ret %dx.types.ResRet.f32 %result
}

define void @dx.op.bufferStore.f32(i32, %dx.types.Handle, i32, i32, float, float, float, float, i8) {
  call void @dx_op_bufferStore_f32(%dx.types.Handle %1, i32 %2, i32 %3, float %4, float %5, float %6, float %7, i8 %8)
  ret void
}

define void @dx.op.bufferStore.i32(i32, %dx.types.Handle, i32, i32, i32, i32, i32, i32, i8) {
  call void @dx_op_bufferStore_i32(%dx.types.Handle %1, i32 %2, i32 %3, i32 %4, i32 %5, i32 %6, i32 %7, i8 %8)
  ret void
}

define i32 @dx.op.threadId.i32(i32, i32) {
	%result = call i32 @dx_op_threadId_i32()
	ret i32 %result
}

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readonly }
