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

   // 測試 1: 簡單對象
   cJS := "var person = {" + ;
          "  name: 'John'," + ;
          "  age: 30," + ;
          "  address: {" + ;
          "    city: 'New York'," + ;
          "    country: 'USA'" + ;
          "  }" + ;
          "};" + ;
          "person.name + ' lives in ' + person.address.city;"
   cResult := DUK_EVAL(cJS)
   msginfo("Test 1 - Simple object: " + cResult)  // 應該輸出 John lives in New York

   // 測試 2: 簡單數組
   cJS := "var fruits = ['apple', 'banana', 'orange'];" + ;
          "var result = '';" + ;
          "for(var i = 0; i < fruits.length; i++) {" + ;
          "  result += fruits[i] + ' ';" + ;
          "}" + ;
          "result;"
   cResult := DUK_EVAL(cJS)
   msginfo("Test 2 - Simple array: " + cResult)  // 應該輸出 apple banana orange

   // 測試 3: 對象數組
   cJS := "var students = [" + ;
          "  { name: 'John', grade: 'A' }," + ;
          "  { name: 'Jane', grade: 'B' }," + ;
          "  { name: 'Bob', grade: 'A' }" + ;
          "];" + ;
          "var result = '';" + ;
          "for(var i = 0; i < students.length; i++) {" + ;
          "  result += students[i].name + ' got ' + students[i].grade + ' ';" + ;
          "}" + ;
          "result;"
   cResult := DUK_EVAL(cJS)
   msginfo("Test 3 - Array of objects: " + cResult)  // 應該輸出 John got A Jane got B Bob got A

   // 測試 4: 簡單計算
   cJS := "var numbers = [10, 20, 30, 40, 50];" + ;
          "var sum = 0;" + ;
          "for(var i = 0; i < numbers.length; i++) {" + ;
          "  sum += numbers[i];" + ;
          "}" + ;
          "sum / numbers.length;"
   cResult := DUK_EVAL(cJS)
   msginfo("Test 4 - Simple calculation: " + cResult)  // 應該輸出 30

   // 測試 5: 字符串處理
   cJS := "var text = 'hello world';" + ;
          "var words = text.split(' ');" + ;
          "var result = '';" + ;
          "for(var i = 0; i < words.length; i++) {" + ;
          "  result += words[i].toUpperCase() + ' ';" + ;
          "}" + ;
          "result;"
   cResult := DUK_EVAL(cJS)
   msginfo("Test 5 - String processing: " + cResult)  // 應該輸出 HELLO WORLD

   // 釋放資源
   p := NIL

RETURN 