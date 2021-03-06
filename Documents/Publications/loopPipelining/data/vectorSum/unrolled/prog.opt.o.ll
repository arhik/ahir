; ModuleID = 'prog.opt.o'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:32:64-f32:32:32-f64:32:64-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i386-pc-linux-gnu"

@.str = private constant [13 x i8] c"in_data_pipe\00"
@A = common global [64 x float] zeroinitializer, align 4
@B = common global [64 x float] zeroinitializer, align 4
@C = common global [64 x float] zeroinitializer, align 4
@.str1 = private constant [14 x i8] c"out_data_pipe\00"

define void @getData() nounwind {
  %idx = alloca i32, align 4
  %val = alloca float, align 4
  store i32 0, i32* %idx, align 4
  br label %1

; <label>:1                                       ; preds = %12, %0
  %2 = load i32* %idx, align 4
  %3 = icmp slt i32 %2, 64
  br i1 %3, label %4, label %15

; <label>:4                                       ; preds = %1
  %5 = call float @read_float32(i8* getelementptr inbounds ([13 x i8]* @.str, i32 0, i32 0))
  store float %5, float* %val, align 4
  %6 = load float* %val, align 4
  %7 = load i32* %idx, align 4
  %8 = getelementptr inbounds [64 x float]* @A, i32 0, i32 %7
  store float %6, float* %8
  %9 = load float* %val, align 4
  %10 = load i32* %idx, align 4
  %11 = getelementptr inbounds [64 x float]* @B, i32 0, i32 %10
  store float %9, float* %11
  br label %12

; <label>:12                                      ; preds = %4
  %13 = load i32* %idx, align 4
  %14 = add nsw i32 %13, 1
  store i32 %14, i32* %idx, align 4
  br label %1

; <label>:15                                      ; preds = %1
  ret void
}

declare float @read_float32(i8*)

define void @sendResult() nounwind {
  %idx = alloca i32, align 4
  %val = alloca float, align 4
  store i32 0, i32* %idx, align 4
  br label %1

; <label>:1                                       ; preds = %9, %0
  %2 = load i32* %idx, align 4
  %3 = icmp slt i32 %2, 64
  br i1 %3, label %4, label %12

; <label>:4                                       ; preds = %1
  %5 = load i32* %idx, align 4
  %6 = getelementptr inbounds [64 x float]* @C, i32 0, i32 %5
  %7 = load float* %6
  store float %7, float* %val, align 4
  %8 = load float* %val, align 4
  call void @write_float32(i8* getelementptr inbounds ([14 x i8]* @.str1, i32 0, i32 0), float %8)
  br label %9

; <label>:9                                       ; preds = %4
  %10 = load i32* %idx, align 4
  %11 = add nsw i32 %10, 1
  store i32 %11, i32* %idx, align 4
  br label %1

; <label>:12                                      ; preds = %1
  ret void
}

declare void @write_float32(i8*, float)

define void @vectorSum() nounwind {
  br label %1

; <label>:1                                       ; preds = %1, %0
  call void @getData()
  call void @_vectorSum_()
  call void @sendResult()
  br label %1
                                                  ; No predecessors!
  ret void
}

define void @_vectorSum_() nounwind {
  %I = alloca i8, align 1
  %I1 = alloca i8, align 1
  %I2 = alloca i8, align 1
  %I3 = alloca i8, align 1
  %I4 = alloca i8, align 1
  %I5 = alloca i8, align 1
  %I6 = alloca i8, align 1
  %I7 = alloca i8, align 1
  %c0 = alloca float, align 4
  %c1 = alloca float, align 4
  %c2 = alloca float, align 4
  %c3 = alloca float, align 4
  %c4 = alloca float, align 4
  %c5 = alloca float, align 4
  %c6 = alloca float, align 4
  %c7 = alloca float, align 4
  store i8 0, i8* %I, align 1
  br label %1

; <label>:1                                       ; preds = %138, %0
  %2 = load i8* %I, align 1
  %3 = zext i8 %2 to i32
  %4 = icmp slt i32 %3, 64
  br i1 %4, label %5, label %143

; <label>:5                                       ; preds = %1
  call void @loop_pipelining_on(i32 8)
  %6 = load i8* %I, align 1
  %7 = zext i8 %6 to i32
  %8 = add nsw i32 %7, 1
  %9 = trunc i32 %8 to i8
  store i8 %9, i8* %I1, align 1
  %10 = load i8* %I, align 1
  %11 = zext i8 %10 to i32
  %12 = add nsw i32 %11, 2
  %13 = trunc i32 %12 to i8
  store i8 %13, i8* %I2, align 1
  %14 = load i8* %I, align 1
  %15 = zext i8 %14 to i32
  %16 = add nsw i32 %15, 3
  %17 = trunc i32 %16 to i8
  store i8 %17, i8* %I3, align 1
  %18 = load i8* %I, align 1
  %19 = zext i8 %18 to i32
  %20 = add nsw i32 %19, 4
  %21 = trunc i32 %20 to i8
  store i8 %21, i8* %I4, align 1
  %22 = load i8* %I, align 1
  %23 = zext i8 %22 to i32
  %24 = add nsw i32 %23, 5
  %25 = trunc i32 %24 to i8
  store i8 %25, i8* %I5, align 1
  %26 = load i8* %I, align 1
  %27 = zext i8 %26 to i32
  %28 = add nsw i32 %27, 6
  %29 = trunc i32 %28 to i8
  store i8 %29, i8* %I6, align 1
  %30 = load i8* %I, align 1
  %31 = zext i8 %30 to i32
  %32 = add nsw i32 %31, 7
  %33 = trunc i32 %32 to i8
  store i8 %33, i8* %I7, align 1
  %34 = load i8* %I, align 1
  %35 = zext i8 %34 to i32
  %36 = getelementptr inbounds [64 x float]* @A, i32 0, i32 %35
  %37 = load float* %36
  %38 = load i8* %I, align 1
  %39 = zext i8 %38 to i32
  %40 = getelementptr inbounds [64 x float]* @B, i32 0, i32 %39
  %41 = load float* %40
  %42 = fadd float %37, %41
  store float %42, float* %c0, align 4
  %43 = load i8* %I1, align 1
  %44 = zext i8 %43 to i32
  %45 = getelementptr inbounds [64 x float]* @A, i32 0, i32 %44
  %46 = load float* %45
  %47 = load i8* %I1, align 1
  %48 = zext i8 %47 to i32
  %49 = getelementptr inbounds [64 x float]* @B, i32 0, i32 %48
  %50 = load float* %49
  %51 = fadd float %46, %50
  store float %51, float* %c1, align 4
  %52 = load i8* %I2, align 1
  %53 = zext i8 %52 to i32
  %54 = getelementptr inbounds [64 x float]* @A, i32 0, i32 %53
  %55 = load float* %54
  %56 = load i8* %I2, align 1
  %57 = zext i8 %56 to i32
  %58 = getelementptr inbounds [64 x float]* @B, i32 0, i32 %57
  %59 = load float* %58
  %60 = fadd float %55, %59
  store float %60, float* %c2, align 4
  %61 = load i8* %I3, align 1
  %62 = zext i8 %61 to i32
  %63 = getelementptr inbounds [64 x float]* @A, i32 0, i32 %62
  %64 = load float* %63
  %65 = load i8* %I3, align 1
  %66 = zext i8 %65 to i32
  %67 = getelementptr inbounds [64 x float]* @B, i32 0, i32 %66
  %68 = load float* %67
  %69 = fadd float %64, %68
  store float %69, float* %c3, align 4
  %70 = load i8* %I4, align 1
  %71 = zext i8 %70 to i32
  %72 = getelementptr inbounds [64 x float]* @A, i32 0, i32 %71
  %73 = load float* %72
  %74 = load i8* %I4, align 1
  %75 = zext i8 %74 to i32
  %76 = getelementptr inbounds [64 x float]* @B, i32 0, i32 %75
  %77 = load float* %76
  %78 = fadd float %73, %77
  store float %78, float* %c4, align 4
  %79 = load i8* %I5, align 1
  %80 = zext i8 %79 to i32
  %81 = getelementptr inbounds [64 x float]* @A, i32 0, i32 %80
  %82 = load float* %81
  %83 = load i8* %I5, align 1
  %84 = zext i8 %83 to i32
  %85 = getelementptr inbounds [64 x float]* @B, i32 0, i32 %84
  %86 = load float* %85
  %87 = fadd float %82, %86
  store float %87, float* %c5, align 4
  %88 = load i8* %I6, align 1
  %89 = zext i8 %88 to i32
  %90 = getelementptr inbounds [64 x float]* @A, i32 0, i32 %89
  %91 = load float* %90
  %92 = load i8* %I6, align 1
  %93 = zext i8 %92 to i32
  %94 = getelementptr inbounds [64 x float]* @B, i32 0, i32 %93
  %95 = load float* %94
  %96 = fadd float %91, %95
  store float %96, float* %c6, align 4
  %97 = load i8* %I7, align 1
  %98 = zext i8 %97 to i32
  %99 = getelementptr inbounds [64 x float]* @A, i32 0, i32 %98
  %100 = load float* %99
  %101 = load i8* %I7, align 1
  %102 = zext i8 %101 to i32
  %103 = getelementptr inbounds [64 x float]* @B, i32 0, i32 %102
  %104 = load float* %103
  %105 = fadd float %100, %104
  store float %105, float* %c7, align 4
  %106 = load float* %c0, align 4
  %107 = load i8* %I, align 1
  %108 = zext i8 %107 to i32
  %109 = getelementptr inbounds [64 x float]* @C, i32 0, i32 %108
  store float %106, float* %109
  %110 = load float* %c1, align 4
  %111 = load i8* %I1, align 1
  %112 = zext i8 %111 to i32
  %113 = getelementptr inbounds [64 x float]* @C, i32 0, i32 %112
  store float %110, float* %113
  %114 = load float* %c2, align 4
  %115 = load i8* %I2, align 1
  %116 = zext i8 %115 to i32
  %117 = getelementptr inbounds [64 x float]* @C, i32 0, i32 %116
  store float %114, float* %117
  %118 = load float* %c3, align 4
  %119 = load i8* %I3, align 1
  %120 = zext i8 %119 to i32
  %121 = getelementptr inbounds [64 x float]* @C, i32 0, i32 %120
  store float %118, float* %121
  %122 = load float* %c4, align 4
  %123 = load i8* %I4, align 1
  %124 = zext i8 %123 to i32
  %125 = getelementptr inbounds [64 x float]* @C, i32 0, i32 %124
  store float %122, float* %125
  %126 = load float* %c5, align 4
  %127 = load i8* %I5, align 1
  %128 = zext i8 %127 to i32
  %129 = getelementptr inbounds [64 x float]* @C, i32 0, i32 %128
  store float %126, float* %129
  %130 = load float* %c6, align 4
  %131 = load i8* %I6, align 1
  %132 = zext i8 %131 to i32
  %133 = getelementptr inbounds [64 x float]* @C, i32 0, i32 %132
  store float %130, float* %133
  %134 = load float* %c7, align 4
  %135 = load i8* %I7, align 1
  %136 = zext i8 %135 to i32
  %137 = getelementptr inbounds [64 x float]* @C, i32 0, i32 %136
  store float %134, float* %137
  br label %138

; <label>:138                                     ; preds = %5
  %139 = load i8* %I, align 1
  %140 = zext i8 %139 to i32
  %141 = add nsw i32 %140, 8
  %142 = trunc i32 %141 to i8
  store i8 %142, i8* %I, align 1
  br label %1

; <label>:143                                     ; preds = %1
  ret void
}

declare void @loop_pipelining_on(i32)
