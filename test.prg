#include "fileio.ch"
#include "hbclass.ch"
#include "common.ch"
#include "hbapi.ch"
#include "hbapiitm.h"
#include "hbapierr.h"
#include "hbvm.h"
#include "hbstack.h"
#include "fivewin.ch"

FUNCTION Main()
   LOCAL p, cJS, cResult, nObjIdx, nArrIdx

   // 初始化 Duktape
   p := DUK_INIT()
   IF ValType(p) <> 'P'
      msginfo("Failed to initialize Duktape")
      RETURN
   ENDIF

   // 測試 1: 基本 JavaScript 運算
   cJS := "2 + 2 * 3"
   cResult := DUK_EVAL(cJS)
   msginfo("Test 1 - Basic calculation: " + cResult)  // 應該輸出 8

   // 測試 2: 字符串操作
   cJS := "'Hello ' + 'World'"
   cResult := DUK_EVAL(cJS)
   msginfo("Test 2 - String concatenation: " + cResult)  // 應該輸出 Hello World

   // 測試 3: 創建和使用 JavaScript 對象
   nObjIdx := DUK_PUSH_OBJECT()
   DUK_PUT_PROP_STRING(nObjIdx, "name", "John")
   DUK_PUT_PROP_STRING(nObjIdx, "age", "30")
   
   // 將對象賦值給全局變量 obj
   cJS := "var obj = " + DUK_JSON_STRINGIFY(nObjIdx) + ";"
   DUK_EVAL(cJS)
   
   // 現在可以使用 obj 變量了
   cJS := "obj.name + ' is ' + obj.age + ' years old'"
   cResult := DUK_EVAL(cJS)
   msginfo("Test 3 - Object properties: " + cResult)  // 應該輸出 John is 30 years old

   // 測試 4: 創建和使用 JavaScript 數組
   nArrIdx := DUK_PUSH_ARRAY()
   DUK_PUT_ARRAY_ELEMENT(nArrIdx, 0, "Apple")
   DUK_PUT_ARRAY_ELEMENT(nArrIdx, 1, "Banana")
   DUK_PUT_ARRAY_ELEMENT(nArrIdx, 2, "Orange")
   
   // 將數組賦值給全局變量 arr
   cJS := "var arr = " + DUK_JSON_STRINGIFY(nArrIdx) + ";"
   DUK_EVAL(cJS)
   
   // 現在可以使用 arr 變量了
   cJS := "arr.join(', ')"
   cResult := DUK_EVAL(cJS)
   msginfo("Test 4 - Array join: " + cResult)  // 應該輸出 Apple, Banana, Orange

   // 測試 5: 使用 JavaScript 函數
   cJS := "function add(a, b) { return a + b; } add(5, 3)"
   cResult := DUK_EVAL(cJS)
   msginfo("Test 5 - Function call: " + cResult)  // 應該輸出 8

   // 測試 6: JSON 處理
   cJS := "JSON.stringify({ name: 'John', age: 30 })"
   cResult := DUK_EVAL(cJS)
   msginfo("Test 6 - JSON stringify: " + cResult)  // 應該輸出 {"name":"John","age":30}

   // 測試 7: 錯誤處理
   cJS := "try { throw new Error('Test error'); } catch(e) { e.message }"
   cResult := DUK_EVAL(cJS)
   msginfo("Test 7 - Error handling: " + cResult)  // 應該輸出 Test error

   // 測試 8: 使用 JavaScript 的內置對象
   cJS := "new Date().toLocaleDateString()"
   cResult := DUK_EVAL(cJS)
   msginfo("Test 8 - Date object: " + cResult)  // 應該輸出當前日期

   // 測試 9: 使用 JavaScript 的數學函數
   cJS := "Math.PI.toFixed(2)"
   cResult := DUK_EVAL(cJS)
   msginfo("Test 9 - Math functions: " + cResult)  // 應該輸出 3.14

   // 測試 10: 使用 JavaScript 的正則表達式
   cJS := "'Hello World'.replace(/World/, 'Harbour')"
   cResult := DUK_EVAL(cJS)
   msginfo("Test 10 - Regular expressions: " + cResult)  // 應該輸出 Hello Harbour

   // 釋放資源
   p := NIL

RETURN

PROCEDURE Main()
   LOCAL p, aMemInfo, aGCStats, aAllocStats
   LOCAL cJS := ""

   // 初始化 Duktape
   p := DUK_INIT()
   IF ValType(p) <> 'P'
      msginfo("Failed to initialize Duktape")
      RETURN
   ENDIF

   // 設置記憶體分配回調
   cJS := "function allocCallback(size) {" + ;
          "  print('Allocating ' + size + ' bytes');" + ;
          "  return true;" + ;
          "}"
   DUK_SET_ALLOC_CALLBACK(cJS)

   // 設置記憶體釋放回調
   cJS := "function freeCallback(ptr) {" + ;
          "  print('Freeing memory at ' + ptr);" + ;
          "  return true;" + ;
          "}"
   DUK_SET_FREE_CALLBACK(cJS)

   // 設置垃圾回收回調
   cJS := "function gcCallback() {" + ;
          "  print('Garbage collection triggered');" + ;
          "  return true;" + ;
          "}"
   DUK_SET_GC_CALLBACK(cJS)

   // 設置記憶體限制 (10MB)
   DUK_SET_MEMORY_LIMIT(10 * 1024 * 1024)

   // 創建一個大對象來測試記憶體管理
   cJS := "var bigArray = [];" + ;
          "for(var i = 0; i < 1000000; i++) {" + ;
          "  bigArray.push('test' + i);" + ;
          "}"
   DUK_EVAL(cJS)

   // 獲取記憶體使用情況
   aMemInfo := DUK_GET_MEMORY_INFO()
   msginfo("Memory Info:")
   msginfo("Alloc Function: " + aMemInfo[1])
   msginfo("Realloc Function: " + aMemInfo[2])
   msginfo("Free Function: " + aMemInfo[3])
   msginfo("User Data: " + aMemInfo[4])

   // 強制執行垃圾回收
   DUK_GC()

   // 獲取垃圾回收統計信息
   aGCStats := DUK_GET_GC_STATS()
   msginfo("GC Stats:")
   msginfo("Alloc Function: " + aGCStats[1])
   msginfo("Realloc Function: " + aGCStats[2])
   msginfo("Free Function: " + aGCStats[3])

   // 獲取記憶體分配統計信息
   aAllocStats := DUK_GET_ALLOC_STATS()
   msginfo("Alloc Stats:")
   msginfo("Alloc Function: " + aAllocStats[1])
   msginfo("Free Function: " + aAllocStats[2])

   // 釋放資源
   p := NIL

RETURN 