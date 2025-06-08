#include "fileio.ch"
#include "hbclass.ch"
#include "common.ch"
#include "hbapi.ch"

Func    main()
        test0()
Return  NIL

// 測試呼叫 harbour 函數
func    test1()
return  NIL

// 基礎測試/base test
Func    test0()
Local   p
        p := DUK_INIT() // 初始化 Duktape
        If ValType(p) <> 'P'
           ? 'Create heap fail!'
           return
        EndIf
        ? 'Create heap success!!'
        
        // 執行運算 / run
        duk_eval(ctx, "1+2")
        c := sprintf("1+2=%d\n", duk_get_int(ctx, -1)) // 取得數值並輸出
        ? c
        
        // 執行 js, 將字串 'testString' 轉為大寫
        duk_eval(ctx, "'testString'.toUpperCase()")
        c := sprintf("result is: %s\n", duk_get_string(ctx, -1)) // 輸出
        ? c
        
        // 釋放資源
        p := NIL
        ? 'Release heap success!!'
Return  NIL

FUNCTION Main()
   LOCAL p, cJS, cResult, nObjIdx, nArrIdx

   // 初始化 Duktape
   p := DUK_INIT()
   IF ValType(p) <> 'P'
      ? "Failed to initialize Duktape"
      RETURN
   ENDIF

   // 測試 1: 基本 JavaScript 運算
   cJS := "2 + 2 * 3"
   cResult := DUK_EVAL(cJS)
   ? "Test 1 - Basic calculation:", cResult  // 應該輸出 8

   // 測試 2: 字符串操作
   cJS := "'Hello ' + 'World'"
   cResult := DUK_EVAL(cJS)
   ? "Test 2 - String concatenation:", cResult  // 應該輸出 Hello World

   // 測試 3: 創建和使用 JavaScript 對象
   nObjIdx := DUK_PUSH_OBJECT()
   DUK_PUT_PROP_STRING(nObjIdx, "name", "John")
   DUK_PUT_PROP_STRING(nObjIdx, "age", "30")
   
   cJS := "obj.name + ' is ' + obj.age + ' years old'"
   cResult := DUK_EVAL(cJS)
   ? "Test 3 - Object properties:", cResult  // 應該輸出 John is 30 years old

   // 測試 4: 創建和使用 JavaScript 數組
   nArrIdx := DUK_PUSH_ARRAY()
   DUK_PUT_ARRAY_ELEMENT(nArrIdx, 0, "Apple")
   DUK_PUT_ARRAY_ELEMENT(nArrIdx, 1, "Banana")
   DUK_PUT_ARRAY_ELEMENT(nArrIdx, 2, "Orange")
   
   cJS := "arr.join(', ')"
   cResult := DUK_EVAL(cJS)
   ? "Test 4 - Array join:", cResult  // 應該輸出 Apple, Banana, Orange

   // 測試 5: 使用 JavaScript 函數
   cJS := "function add(a, b) { return a + b; } add(5, 3)"
   cResult := DUK_EVAL(cJS)
   ? "Test 5 - Function call:", cResult  // 應該輸出 8

   // 測試 6: JSON 處理
   cJS := "JSON.stringify({ name: 'John', age: 30 })"
   cResult := DUK_EVAL(cJS)
   ? "Test 6 - JSON stringify:", cResult  // 應該輸出 {"name":"John","age":30}

   // 測試 7: 錯誤處理
   cJS := "try { throw new Error('Test error'); } catch(e) { e.message }"
   cResult := DUK_EVAL(cJS)
   ? "Test 7 - Error handling:", cResult  // 應該輸出 Test error

   // 測試 8: 使用 JavaScript 的內置對象
   cJS := "new Date().toLocaleDateString()"
   cResult := DUK_EVAL(cJS)
   ? "Test 8 - Date object:", cResult  // 應該輸出當前日期

   // 測試 9: 使用 JavaScript 的數學函數
   cJS := "Math.PI.toFixed(2)"
   cResult := DUK_EVAL(cJS)
   ? "Test 9 - Math functions:", cResult  // 應該輸出 3.14

   // 測試 10: 使用 JavaScript 的正則表達式
   cJS := "'Hello World'.replace(/World/, 'Harbour')"
   cResult := DUK_EVAL(cJS)
   ? "Test 10 - Regular expressions:", cResult  // 應該輸出 Hello Harbour

   // 釋放資源
   p := NIL

RETURN

// 輔助函數：執行 JavaScript 代碼並返回結果
FUNCTION DukEval(cJS)
   LOCAL cResult
   
   IF !DukEval(cJS)
      cResult := "Error executing JavaScript"
   ELSE
      cResult := DukGetVar("result")
   ENDIF

RETURN cResult

