/*
 * Copyright (C) 2021 Ligang Wang <ligangwangs@gmail.com>
 *
 * Unit tests for code generating
 */
#include "compiler/compiler.h"
#include "sema/analyzer.h"
#include "sema/sema_context.h"
#include "test_base.h"
#include "tutil.h"
#include "gtest/gtest.h"
#include <stdio.h>

TEST(testCGFunCall, testIntIdFunc)
{
    const char test_code[] = R"(
f x:int = x
main() = f 10
)";
    const char *expected_ir = R"(
define i32 @f(i32 %x) {
entry:
  %x1 = alloca i32, align 4
  store i32 %x, i32* %x1, align 4
  %x2 = load i32, i32* %x1, align 4
  ret i32 %x2
}

define i32 @main() {
entry:
  %0 = call i32 @f(i32 10)
  ret i32 %0
}
)";
    validate_m_code_with_ir_code(test_code, expected_ir);
}

TEST(testCGFunCall, testGenericIdFunc)
{
    const char test_code[] = R"(
f x = x
main () = f 'c'
)";
    const char *expected_ir = R"(
define i8 @__f_char(i8 %x) {
entry:
  %x1 = alloca i8, align 1
  store i8 %x, i8* %x1, align 1
  %x2 = load i8, i8* %x1, align 1
  ret i8 %x2
}

define i8 @main() {
entry:
  %0 = call i8 @__f_char(i8 99)
  ret i8 %0
}
)";
    validate_m_code_with_ir_code(test_code, expected_ir);
}

TEST(testCGFunCall, testTwoParamsFunc)
{
    const char test_code[] = R"(
f x y = (x + y) / 2
main () = f 10 20
)";
    const char *expected_ir = R"(
define i32 @f(i32 %x, i32 %y) {
entry:
  %y2 = alloca i32, align 4
  %x1 = alloca i32, align 4
  store i32 %x, i32* %x1, align 4
  store i32 %y, i32* %y2, align 4
  %x3 = load i32, i32* %x1, align 4
  %y4 = load i32, i32* %y2, align 4
  %0 = add i32 %x3, %y4
  %1 = sdiv i32 %0, 2
  ret i32 %1
}

define i32 @main() {
entry:
  %0 = call i32 @f(i32 10, i32 20)
  ret i32 %0
}
)";
    validate_m_code_with_ir_code(test_code, expected_ir);
}

TEST(testCGFunCall, testPassStructIndirect)
{
    const char test_code[] = R"(
type Point2D = x:double y:double
f xy:Point2D = xy.y
main() = 
  xy:Point2D = 10.0 20.0
  f xy
)";
    const char *expected_ir = R"(
%Point2D = type { double, double }

define double @f(%Point2D* %xy) {
entry:
  %y = getelementptr inbounds %Point2D, %Point2D* %xy, i32 0, i32 1
  %xy.y = load double, double* %y, align 8
  ret double %xy.y
}

define double @main() {
entry:
  %xy = alloca %Point2D, align 8
  %0 = getelementptr inbounds %Point2D, %Point2D* %xy, i32 0, i32 0
  store double 1.000000e+01, double* %0, align 8
  %1 = getelementptr inbounds %Point2D, %Point2D* %xy, i32 0, i32 1
  store double 2.000000e+01, double* %1, align 8
  %2 = call double @f(%Point2D* %xy)
  ret double %2
}
)";
    validate_m_code_with_ir_code(test_code, expected_ir);
}

TEST(testCGFunCall, testPassStructDirect)
{
    const char test_code[] = R"(
 type Point2D = x:int y:int
 f xy:Point2D = xy.x
 main() = 
  xy:Point2D = 10 20
  f xy
)";
    const char *expected_ir = R"(
%Point2D = type { i32, i32 }

define i32 @f(i64 %xy.coerce) {
entry:
  %xy = alloca %Point2D, align 4
  %0 = bitcast %Point2D* %xy to i64*
  store i64 %xy.coerce, i64* %0, align 4
  %x = getelementptr inbounds %Point2D, %Point2D* %xy, i32 0, i32 0
  %xy.x = load i32, i32* %x, align 4
  ret i32 %xy.x
}

define i32 @main() {
entry:
  %xy = alloca %Point2D, align 4
  %0 = getelementptr inbounds %Point2D, %Point2D* %xy, i32 0, i32 0
  store i32 10, i32* %0, align 4
  %1 = getelementptr inbounds %Point2D, %Point2D* %xy, i32 0, i32 1
  store i32 20, i32* %1, align 4
  %2 = bitcast %Point2D* %xy to i64*
  %3 = load i64, i64* %2, align 4
  %4 = call i32 @f(i64 %3)
  ret i32 %4
}
)";
    validate_m_code_with_ir_code(test_code, expected_ir);
}

TEST(testCGFunCall, testReturnStructDirect)
{
    const char test_code[] = R"(
 type Point2D = x:int y:int
 f () = 
   xy:Point2D = 10 20
   xy
 main() = 
   xy = f()
   xy.x
 )";
    const char *expected_ir = R"(
%Point2D = type { i32, i32 }

define i64 @f() {
entry:
  %xy = alloca %Point2D, align 4
  %0 = getelementptr inbounds %Point2D, %Point2D* %xy, i32 0, i32 0
  store i32 10, i32* %0, align 4
  %1 = getelementptr inbounds %Point2D, %Point2D* %xy, i32 0, i32 1
  store i32 20, i32* %1, align 4
  %2 = bitcast %Point2D* %xy to i64*
  %3 = load i64, i64* %2, align 4
  ret i64 %3
}

define i32 @main() {
entry:
  %xy = alloca %Point2D, align 4
  %0 = call i64 @f()
  %1 = bitcast %Point2D* %xy to i64*
  store i64 %0, i64* %1, align 4
  %x = getelementptr inbounds %Point2D, %Point2D* %xy, i32 0, i32 0
  %xy.x = load i32, i32* %x, align 4
  ret i32 %xy.x
}
)";
    validate_m_code_with_ir_code(test_code, expected_ir);
}

TEST(testCGFunCall, testReturnStructDirectWithoutName)
{
    const char test_code[] = R"(
 type Point2D = x:int y:int
 f () = Point2D 10 20
   
 main() = 
   xy = f()
   xy.x
 )";
    const char *expected_ir = R"(
%Point2D = type { i32, i32 }

define i64 @f() {
entry:
  %tmp = alloca %Point2D, align 4
  %0 = getelementptr inbounds %Point2D, %Point2D* %tmp, i32 0, i32 0
  store i32 10, i32* %0, align 4
  %1 = getelementptr inbounds %Point2D, %Point2D* %tmp, i32 0, i32 1
  store i32 20, i32* %1, align 4
  %2 = bitcast %Point2D* %tmp to i64*
  %3 = load i64, i64* %2, align 4
  ret i64 %3
}

define i32 @main() {
entry:
  %xy = alloca %Point2D, align 4
  %0 = call i64 @f()
  %1 = bitcast %Point2D* %xy to i64*
  store i64 %0, i64* %1, align 4
  %x = getelementptr inbounds %Point2D, %Point2D* %xy, i32 0, i32 0
  %xy.x = load i32, i32* %x, align 4
  ret i32 %xy.x
}
)";
    validate_m_code_with_ir_code(test_code, expected_ir);
}

TEST(testCGFunCall, testReturnStructInDirect)
{
    const char test_code[] = R"(
 type Point2D = x:double y:double
 f () = 
   xy:Point2D = 10.0 20.0
   xy
 main() = 
   xy = f()
   xy.x
)";
    const char *expected_ir = R"(
%Point2D = type { double, double }

define void @f(%Point2D* noalias sret(%Point2D) %agg.result) {
entry:
  %0 = getelementptr inbounds %Point2D, %Point2D* %agg.result, i32 0, i32 0
  store double 1.000000e+01, double* %0, align 8
  %1 = getelementptr inbounds %Point2D, %Point2D* %agg.result, i32 0, i32 1
  store double 2.000000e+01, double* %1, align 8
  ret void
}

define double @main() {
entry:
  %xy = alloca %Point2D, align 8
  call void @f(%Point2D* sret(%Point2D) align 8 %xy)
  %x = getelementptr inbounds %Point2D, %Point2D* %xy, i32 0, i32 0
  %xy.x = load double, double* %x, align 8
  ret double %xy.x
}
)";
    validate_m_code_with_ir_code(test_code, expected_ir);
}

TEST(testCGFunCall, testReturnStructInDirectWithoutName)
{
    const char test_code[] = R"(
 type Point2D = x:double y:double
 f () = Point2D 10.0 20.0
 main() = 
   xy = f()
   xy.x
)";
    const char *expected_ir = R"(
%Point2D = type { double, double }

define void @f(%Point2D* noalias sret(%Point2D) %agg.result) {
entry:
  %0 = getelementptr inbounds %Point2D, %Point2D* %agg.result, i32 0, i32 0
  store double 1.000000e+01, double* %0, align 8
  %1 = getelementptr inbounds %Point2D, %Point2D* %agg.result, i32 0, i32 1
  store double 2.000000e+01, double* %1, align 8
  ret void
}

define double @main() {
entry:
  %xy = alloca %Point2D, align 8
  call void @f(%Point2D* sret(%Point2D) align 8 %xy)
  %x = getelementptr inbounds %Point2D, %Point2D* %xy, i32 0, i32 0
  %xy.x = load double, double* %x, align 8
  ret double %xy.x
}
)";
    validate_m_code_with_ir_code(test_code, expected_ir);
}

TEST(testCGFunCall, testReturnStructInDirectWithoutNameCalling)
{
    const char test_code[] = R"(
 type Point2D = x:double y:double
 f () = Point2D 10.0 20.0
 main() = f()
)";
    const char *expected_ir = R"(
%Point2D = type { double, double }

define void @f(%Point2D* noalias sret(%Point2D) %agg.result) {
entry:
  %0 = getelementptr inbounds %Point2D, %Point2D* %agg.result, i32 0, i32 0
  store double 1.000000e+01, double* %0, align 8
  %1 = getelementptr inbounds %Point2D, %Point2D* %agg.result, i32 0, i32 1
  store double 2.000000e+01, double* %1, align 8
  ret void
}

define void @main(%Point2D* noalias sret(%Point2D) %agg.result) {
entry:
  call void @f(%Point2D* sret(%Point2D) align 8 %agg.result)
  ret void
}
)";
    validate_m_code_with_ir_code(test_code, expected_ir);
}

TEST(testCGFunCall, testReturnStructInDirectWithoutNameElementAccess)
{
    const char test_code[] = R"(
 type Point2D = x:double y:double
 f () = Point2D 10.0 20.0
 main() = f().x
)";
    const char *expected_ir = R"(
%Point2D = type { double, double }

define void @f(%Point2D* noalias sret(%Point2D) %agg.result) {
entry:
  %0 = getelementptr inbounds %Point2D, %Point2D* %agg.result, i32 0, i32 0
  store double 1.000000e+01, double* %0, align 8
  %1 = getelementptr inbounds %Point2D, %Point2D* %agg.result, i32 0, i32 1
  store double 2.000000e+01, double* %1, align 8
  ret void
}

define double @main() {
entry:
  %xy = alloca %Point2D, align 8
  call void @f(%Point2D* sret(%Point2D) align 8 %xy)
  %x = getelementptr inbounds %Point2D, %Point2D* %xy, i32 0, i32 0
  %xy.x = load double, double* %x, align 8
  ret double %xy.x
}
)";
    //validate_m_code_with_ir_code(test_code, expected_ir);
}

TEST(testCGFunCall, testReturnExpressionScalar)
{
    char test_code[] = R"(
getx()=
    x = 10
    x + 1
main() = getx()
)";
    const char *expected_ir = R"(
define i32 @getx() {
entry:
  %x = alloca i32, align 4
  store i32 10, i32* %x, align 4
  %x1 = load i32, i32* %x, align 4
  %0 = add i32 %x1, 1
  ret i32 %0
}

define i32 @main() {
entry:
  %0 = call i32 @getx()
  ret i32 %0
}
)";
    validate_m_code_with_ir_code(test_code, expected_ir);
}
