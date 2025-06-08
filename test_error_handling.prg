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

   // 測試 1: 基本錯誤處理
   cJS := "try {" + ;
          "  var x = undefined;" + ;
          "  x.someMethod();" + ;
          "} catch(e) {" + ;
          "  'Error: ' + e.message;" + ;
          "}"
   cResult := DUK_EVAL(cJS)
   msginfo("Test 1 - Basic error handling: " + cResult)  // 應該輸出錯誤信息

   // 測試 2: 自定義錯誤
   cJS := "try {" + ;
          "  var age = -1;" + ;
          "  if(age < 0) {" + ;
          "    throw new Error('Age cannot be negative');" + ;
          "  }" + ;
          "} catch(e) {" + ;
          "  'Error: ' + e.message;" + ;
          "}"
   cResult := DUK_EVAL(cJS)
   msginfo("Test 2 - Custom error: " + cResult)  // 應該輸出 Age cannot be negative

   // 測試 3: 錯誤堆棧
   cJS := "function level1() {" + ;
          "  level2();" + ;
          "}" + ;
          "function level2() {" + ;
          "  throw new Error('Error in level2');" + ;
          "}" + ;
          "try {" + ;
          "  level1();" + ;
          "} catch(e) {" + ;
          "  'Error: ' + e.message;" + ;
          "}"
   cResult := DUK_EVAL(cJS)
   msginfo("Test 3 - Error stack: " + cResult)  // 應該輸出錯誤信息

   // 測試 4: 資源清理
   cJS := "var result = '';" + ;
          "try {" + ;
          "  result += 'Starting... ';" + ;
          "  throw new Error('Test error');" + ;
          "} catch(e) {" + ;
          "  result += 'Error caught: ' + e.message + ' ';" + ;
          "} finally {" + ;
          "  result += 'Cleanup done';" + ;
          "}" + ;
          "result;"
   cResult := DUK_EVAL(cJS)
   msginfo("Test 4 - Resource cleanup: " + cResult)  // 應該輸出完整的錯誤處理過程

   // 測試 5: 錯誤恢復
   cJS := "var result = '';" + ;
          "function safeOperation() {" + ;
          "  try {" + ;
          "    result += 'Operation started ';" + ;
          "    throw new Error('Operation failed');" + ;
          "  } catch(e) {" + ;
          "    result += 'Error: ' + e.message + ' ';" + ;
          "    return false;" + ;
          "  }" + ;
          "  return true;" + ;
          "}" + ;
          "if(!safeOperation()) {" + ;
          "  result += 'Recovered from error';" + ;
          "}" + ;
          "result;"
   cResult := DUK_EVAL(cJS)
   msginfo("Test 5 - Error recovery: " + cResult)  // 應該輸出錯誤處理和恢復過程

   // 清理 Duktape
   DUK_CLEANUP()

RETURN 