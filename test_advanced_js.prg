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

   // 測試 1: 基本對象操作
   cJS := "var person = {" + ;
          "  name: 'John'," + ;
          "  age: 30," + ;
          "  sayHello: function() {" + ;
          "    return 'Hello, my name is ' + this.name;" + ;
          "  }" + ;
          "};" + ;
          "person.sayHello();"
   cResult := DUK_EVAL(cJS)
   msginfo("Test 1 - Basic object: " + cResult)  // 應該輸出 Hello, my name is John

   // 測試 2: 數組操作
   cJS := "var numbers = [1, 2, 3, 4, 5];" + ;
          "var sum = 0;" + ;
          "for(var i = 0; i < numbers.length; i++) {" + ;
          "  sum += numbers[i];" + ;
          "}" + ;
          "sum;"
   cResult := DUK_EVAL(cJS)
   msginfo("Test 2 - Array operations: " + cResult)  // 應該輸出 15

   // 測試 3: 函數定義和調用
   cJS := "function add(a, b) {" + ;
          "  return a + b;" + ;
          "}" + ;
          "add(5, 3);"
   cResult := DUK_EVAL(cJS)
   msginfo("Test 3 - Function call: " + cResult)  // 應該輸出 8

   // 測試 4: 字符串操作
   cJS := "var str = 'hello world';" + ;
          "str.toUpperCase();"
   cResult := DUK_EVAL(cJS)
   msginfo("Test 4 - String operations: " + cResult)  // 應該輸出 HELLO WORLD

   // 測試 5: 條件和循環
   cJS := "var result = '';" + ;
          "for(var i = 1; i <= 5; i++) {" + ;
          "  if(i % 2 === 0) {" + ;
          "    result += 'even ';" + ;
          "  } else {" + ;
          "    result += 'odd ';" + ;
          "  }" + ;
          "}" + ;
          "result;"
   cResult := DUK_EVAL(cJS)
   msginfo("Test 5 - Conditions and loops: " + cResult)  // 應該輸出 odd even odd even odd

   // 釋放資源
   p := NIL

RETURN 