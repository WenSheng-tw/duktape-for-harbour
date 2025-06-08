#include "fileio.ch"
#include "hbclass.ch"
#include "common.ch"
#include "hbapi.ch"
#include "fivewin.ch"

FUNCTION Main()
   LOCAL p, cJS, cResult

   // 初始化 Duktape
   p := DUK_INIT()
   IF ValType(p) <> 'P'
      msginfo("Failed to initialize Duktape")
      RETURN
   ENDIF

   // 測試 1: 載入並執行簡單的 JavaScript 文件
   cJS := hb_memoRead("test1.js")
   IF cJS == NIL
      msginfo("Error: Cannot read test1.js")
   ELSE
      cResult := DUK_EVAL(cJS)
      msginfo("Test 1 - Simple calculation: " + cResult)  // 應該輸出 8
   ENDIF

   // 測試 2: 載入並執行包含函數定義的 JavaScript 文件
   cJS := hb_memoRead("test2.js")
   IF cJS == NIL
      msginfo("Error: Cannot read test2.js")
   ELSE
      cResult := DUK_EVAL(cJS)
      msginfo("Test 2 - Function definition: " + cResult)
      
      // 調用定義的函數
      cResult := DUK_EVAL("add(5, 3)")
      msginfo("Test 2 - Function call result: " + cResult)  // 應該輸出 8
   ENDIF

   // 測試 3: 載入並執行包含對象定義的 JavaScript 文件
   cJS := hb_memoRead("test3.js")
   IF cJS == NIL
      msginfo("Error: Cannot read test3.js")
   ELSE
      cResult := DUK_EVAL(cJS)
      msginfo("Test 3 - Object definition: " + cResult)
      
      // 使用定義的對象
      cResult := DUK_EVAL("person.name + ' is ' + person.age + ' years old'")
      msginfo("Test 3 - Object usage: " + cResult)  // 應該輸出 "John is 30 years old"
   ENDIF

   // 釋放資源
   p := NIL

RETURN 