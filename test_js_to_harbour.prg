#include "fileio.ch"
#include "hbclass.ch"
#include "common.ch"
#include "hbapi.ch"
#include "fivewin.ch"

// Harbour 函數定義
FUNCTION harbourAdd(n1, n2)
   RETURN n1 + n2

FUNCTION harbourUpperCase(cStr)
   RETURN Upper(cStr)

FUNCTION harbourArraySum(aArray)
   LOCAL nSum := 0, i
   FOR i := 1 TO Len(aArray)
      nSum += aArray[i]
   NEXT
   RETURN nSum

FUNCTION harbourGetUserInfo()
   LOCAL hUser := {=>}
   hUser["name"] := "John"
   hUser["age"] := 30
   hUser["email"] := "john@example.com"
   RETURN hUser

FUNCTION Main()
   LOCAL p, cJS, cResult

   // 初始化 Duktape
   p := DUK_INIT()
   IF ValType(p) <> 'P'
      msginfo("Failed to initialize Duktape")
      RETURN
   ENDIF

   // 註冊 Harbour 函數
   DUK_REGISTER_FUNCTION("harbourAdd", @harbourAdd())
   DUK_REGISTER_FUNCTION("harbourUpperCase", @harbourUpperCase())
   DUK_REGISTER_FUNCTION("harbourArraySum", @harbourArraySum())
   DUK_REGISTER_FUNCTION("harbourGetUserInfo", @harbourGetUserInfo())

   // 測試 1: 基本數值計算
   cJS := "harbourAdd(5, 3);"
   cResult := DUK_EVAL(cJS)
   msginfo("Test 1 - Basic calculation: " + cResult)  // 應該輸出 8

   // 測試 2: 字符串處理
   cJS := "harbourUpperCase('hello world');"
   cResult := DUK_EVAL(cJS)
   msginfo("Test 2 - String processing: " + cResult)  // 應該輸出 HELLO WORLD

   // 測試 3: 數組處理
   cJS := "var numbers = [1, 2, 3, 4, 5];" + ;
          "harbourArraySum(numbers);"
   cResult := DUK_EVAL(cJS)
   msginfo("Test 3 - Array processing: " + cResult)  // 應該輸出 15

   // 測試 4: 對象處理
   cJS := "var user = harbourGetUserInfo();" + ;
          "user.name + ' is ' + user.age + ' years old';"
   cResult := DUK_EVAL(cJS)
   msginfo("Test 4 - Object processing: " + cResult)  // 應該輸出 John is 30 years old

   // 測試 5: 混合使用
   cJS := "var user = harbourGetUserInfo();" + ;
          "harbourUpperCase(user.name) + ' - ' + user.email;"
   cResult := DUK_EVAL(cJS)
   msginfo("Test 5 - Mixed usage: " + cResult)  // 應該輸出 JOHN - john@example.com

   // 釋放資源
   p := NIL

RETURN 