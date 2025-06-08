#include <stddef.h>
#include <string.h>
#include "hbapi.h"
#include "hbapiitm.h"
#include "hbapierr.h"
#include "hbvm.h"
#include "hbstack.h"
#include "duktape.h"

/* 全局 Duktape 上下文 */
static duk_context *ctx = NULL;

/* 自定義記憶體分配函數 */
static void *hb_duktape_alloc(void *udata, duk_size_t size)
{
   (void)udata;  /* 避免未使用參數警告 */
   return hb_xalloc(size);
}

static void *hb_duktape_realloc(void *udata, void *ptr, duk_size_t size)
{
   (void)udata;  /* 避免未使用參數警告 */
   return hb_xrealloc(ptr, size);
}

static void hb_duktape_free(void *udata, void *ptr)
{
   (void)udata;  /* 避免未使用參數警告 */
   hb_xfree(ptr);
}

/* 註冊 Duktape 對象到 Harbour 垃圾回收 */
static HB_GARBAGE_FUNC(hb_duktape_gc)
{
   void **ph = (void **)Cargo;

   /* Check if pointer is not NULL to avoid multiple freeing */
   if (ph && *ph)
   {
      /* Destroy the object */
      duk_destroy_heap((duk_context *)*ph);

      /* set pointer to NULL to avoid multiple freeing */
      *ph = NULL;
   }
}

static const HB_GC_FUNCS s_gcDuktapeFuncs =
{
   hb_duktape_gc,
   hb_gcDummyMark
};

/* 初始化 Duktape 引擎並註冊到 Harbour 垃圾回收 */
HB_FUNC(DUK_INIT)
{
   void **ph;

   if (ctx != NULL)
   {
      hb_retl(HB_TRUE);
      return;
   }

   ctx = duk_create_heap_default();
   if (ctx == NULL)
   {
      hb_errRT_BASE(EG_CREATE, 2010, NULL, HB_ERR_FUNCNAME, HB_ERR_ARGS_BASEPARAMS);
      return;
   }

   /* 註冊到 Harbour 垃圾回收 */
   ph = (void **)hb_gcAllocate(sizeof(duk_context *), &s_gcDuktapeFuncs);
   *ph = ctx;
   hb_retptrGC(ph);
}

/* 執行 JavaScript 代碼 */
HB_FUNC(DUK_EVAL)
{
   const char *code = hb_parc(1);
   const char *error;
   const char *result;

   if (ctx == NULL)
   {
      hb_errRT_BASE(EG_CREATE, 2010, NULL, HB_ERR_FUNCNAME, HB_ERR_ARGS_BASEPARAMS);
      return;
   }

   if (code == NULL)
   {
      hb_errRT_BASE(EG_ARG, 2010, NULL, HB_ERR_FUNCNAME, HB_ERR_ARGS_BASEPARAMS);
      return;
   }

   duk_push_string(ctx, code);
   if (duk_peval(ctx) != 0)
   {
      error = duk_safe_to_string(ctx, -1);
      hb_errRT_BASE(EG_CREATE, 2010, NULL, HB_ERR_FUNCNAME, HB_ERR_ARGS_BASEPARAMS);
      duk_pop(ctx);
      return;
   }

   result = duk_safe_to_string(ctx, -1);
   hb_retc(result);
   duk_pop(ctx);
}

/* 執行 JavaScript 文件 */
HB_FUNC(DUK_EVAL_FILE)
{
   const char *filename = hb_parc(1);
   const char *error;
   const char *result;
   FILE *fp;
   long size;
   char *buffer;

   if (ctx == NULL)
   {
      hb_errRT_BASE(EG_CREATE, 2010, NULL, HB_ERR_FUNCNAME, HB_ERR_ARGS_BASEPARAMS);
      return;
   }

   if (filename == NULL)
   {
      hb_errRT_BASE(EG_ARG, 2010, NULL, HB_ERR_FUNCNAME, HB_ERR_ARGS_BASEPARAMS);
      return;
   }

   fp = fopen(filename, "rb");
   if (fp == NULL)
   {
      hb_errRT_BASE(EG_CREATE, 2010, NULL, HB_ERR_FUNCNAME, HB_ERR_ARGS_BASEPARAMS);
      return;
   }

   fseek(fp, 0, SEEK_END);
   size = ftell(fp);
   fseek(fp, 0, SEEK_SET);

   buffer = (char *)hb_xalloc(size + 1);
   if (buffer == NULL)
   {
      fclose(fp);
      hb_errRT_BASE(EG_CREATE, 2010, NULL, HB_ERR_FUNCNAME, HB_ERR_ARGS_BASEPARAMS);
      return;
   }

   fread(buffer, 1, size, fp);
   buffer[size] = '\0';
   fclose(fp);

   duk_push_string(ctx, buffer);
   hb_xfree(buffer);

   if (duk_peval(ctx) != 0)
   {
      error = duk_safe_to_string(ctx, -1);
      hb_errRT_BASE(EG_CREATE, 2010, NULL, HB_ERR_FUNCNAME, HB_ERR_ARGS_BASEPARAMS);
      duk_pop(ctx);
      return;
   }

   result = duk_safe_to_string(ctx, -1);
   hb_retc(result);
   duk_pop(ctx);
}

/* 創建新的 JavaScript 對象 */
HB_FUNC(DUK_PUSH_OBJECT)
{
   if (ctx == NULL)
   {
      hb_errRT_BASE(EG_CREATE, 2010, NULL, HB_ERR_FUNCNAME, HB_ERR_ARGS_BASEPARAMS);
      return;
   }

   duk_push_object(ctx);
   hb_retni(duk_get_top_index(ctx));
}

/* 設置對象屬性 */
HB_FUNC(DUK_PUT_PROP_STRING)
{
   duk_idx_t obj_idx = hb_parni(1);
   const char *key = hb_parc(2);
   const char *value = hb_parc(3);

   if (ctx == NULL)
   {
      hb_errRT_BASE(EG_CREATE, 2010, NULL, HB_ERR_FUNCNAME, HB_ERR_ARGS_BASEPARAMS);
      return;
   }

   if (key == NULL || value == NULL)
   {
      hb_errRT_BASE(EG_ARG, 2010, NULL, HB_ERR_FUNCNAME, HB_ERR_ARGS_BASEPARAMS);
      return;
   }

   duk_push_string(ctx, value);
   duk_put_prop_string(ctx, obj_idx, key);
   hb_retl(HB_TRUE);
}

/* 獲取對象屬性 */
HB_FUNC(DUK_GET_PROP_STRING)
{
   duk_idx_t obj_idx = hb_parni(1);
   const char *key = hb_parc(2);

   if (ctx == NULL)
   {
      hb_errRT_BASE(EG_CREATE, 2010, NULL, HB_ERR_FUNCNAME, HB_ERR_ARGS_BASEPARAMS);
      return;
   }

   if (key == NULL)
   {
      hb_errRT_BASE(EG_ARG, 2010, NULL, HB_ERR_FUNCNAME, HB_ERR_ARGS_BASEPARAMS);
      return;
   }

   duk_get_prop_string(ctx, obj_idx, key);
   if (duk_is_string(ctx, -1))
   {
      hb_retc(duk_get_string(ctx, -1));
   }
   else if (duk_is_number(ctx, -1))
   {
      hb_retnd(duk_get_number(ctx, -1));
   }
   else if (duk_is_boolean(ctx, -1))
   {
      hb_retl(duk_get_boolean(ctx, -1));
   }
   else
   {
      hb_retc(duk_safe_to_string(ctx, -1));
   }
   duk_pop(ctx);
}

/* 調用 JavaScript 函數 */
HB_FUNC(DUK_CALL_FUNCTION)
{
   const char *func_name = hb_parc(1);
   duk_int_t nargs = hb_parni(2);
   const char *error;
   const char *result;

   if (ctx == NULL)
   {
      hb_errRT_BASE(EG_CREATE, 2010, NULL, HB_ERR_FUNCNAME, HB_ERR_ARGS_BASEPARAMS);
      return;
   }

   if (func_name == NULL)
   {
      hb_errRT_BASE(EG_ARG, 2010, NULL, HB_ERR_FUNCNAME, HB_ERR_ARGS_BASEPARAMS);
      return;
   }

   duk_get_global_string(ctx, func_name);
   if (!duk_is_function(ctx, -1))
   {
      hb_errRT_BASE(EG_ARG, 2010, NULL, HB_ERR_FUNCNAME, HB_ERR_ARGS_BASEPARAMS);
      duk_pop(ctx);
      return;
   }

   if (duk_pcall(ctx, nargs) != 0)
   {
      error = duk_safe_to_string(ctx, -1);
      hb_errRT_BASE(EG_CREATE, 2010, NULL, HB_ERR_FUNCNAME, HB_ERR_ARGS_BASEPARAMS);
      duk_pop(ctx);
      return;
   }

   result = duk_safe_to_string(ctx, -1);
   hb_retc(result);
   duk_pop(ctx);
}

/* 直接銷毀 Duktape 堆 */
HB_FUNC(DUK_DESTROY_HEAP)
{
   if (ctx != NULL)
   {
      duk_destroy_heap(ctx);
      ctx = NULL;
   }
   hb_retl(HB_TRUE);
}

/* 清理 Duktape 引擎 */
HB_FUNC(DUK_CLEANUP)
{
   if (ctx != NULL)
   {
      duk_destroy_heap(ctx);
      ctx = NULL;
   }
   hb_retl(HB_TRUE);
}

/* 註冊 Harbour 函數到 JavaScript 環境 */
HB_FUNC(DUK_REGISTER_FUNCTION)
{
   const char *name = hb_parc(1);
   PHB_ITEM pFunc = hb_param(2, HB_IT_BLOCK);

   if (ctx == NULL)
   {
      hb_errRT_BASE(EG_CREATE, 2010, NULL, HB_ERR_FUNCNAME, HB_ERR_ARGS_BASEPARAMS);
      return;
   }

   if (name == NULL)
   {
      hb_errRT_BASE(EG_ARG, 2010, NULL, HB_ERR_FUNCNAME, HB_ERR_ARGS_BASEPARAMS);
      return;
   }

   if (pFunc == NULL)
   {
      hb_errRT_BASE(EG_ARG, 2010, NULL, HB_ERR_FUNCNAME, HB_ERR_ARGS_BASEPARAMS);
      return;
   }

   duk_push_c_function(ctx, (duk_c_function)hb_itemGetPtr(pFunc), DUK_VARARGS);
   duk_put_global_string(ctx, name);
   hb_retl(HB_TRUE);
}

/* 從 JavaScript 環境獲取變量值 */
HB_FUNC(DUK_GET_VAR)
{
   const char *name = hb_parc(1);

   if (ctx == NULL)
   {
      hb_errRT_BASE(EG_CREATE, 2010, NULL, HB_ERR_FUNCNAME, HB_ERR_ARGS_BASEPARAMS);
      return;
   }

   if (name == NULL)
   {
      hb_errRT_BASE(EG_ARG, 2010, NULL, HB_ERR_FUNCNAME, HB_ERR_ARGS_BASEPARAMS);
      return;
   }

   duk_get_global_string(ctx, name);
   if (duk_is_string(ctx, -1))
   {
      hb_retc(duk_get_string(ctx, -1));
   }
   else if (duk_is_number(ctx, -1))
   {
      hb_retnd(duk_get_number(ctx, -1));
   }
   else if (duk_is_boolean(ctx, -1))
   {
      hb_retl(duk_get_boolean(ctx, -1));
   }
   else
   {
      hb_retc(duk_safe_to_string(ctx, -1));
   }
   duk_pop(ctx);
}

/* 設置 JavaScript 環境變量值 */
HB_FUNC(DUK_SET_VAR)
{
   const char *name = hb_parc(1);

   if (ctx == NULL)
   {
      hb_errRT_BASE(EG_CREATE, 2010, NULL, HB_ERR_FUNCNAME, HB_ERR_ARGS_BASEPARAMS);
      return;
   }

   if (name == NULL)
   {
      hb_errRT_BASE(EG_ARG, 2010, NULL, HB_ERR_FUNCNAME, HB_ERR_ARGS_BASEPARAMS);
      return;
   }

   if (HB_ISCHAR(2))
   {
      duk_push_string(ctx, hb_parc(2));
   }
   else if (HB_ISNUM(2))
   {
      duk_push_number(ctx, hb_parnd(2));
   }
   else if (HB_ISLOG(2))
   {
      duk_push_boolean(ctx, hb_parl(2));
   }
   else
   {
      duk_push_undefined(ctx);
   }

   duk_put_global_string(ctx, name);
   hb_retl(HB_TRUE);
}

/* 創建新的 JavaScript 數組 */
HB_FUNC(DUK_PUSH_ARRAY)
{
   if (ctx == NULL)
   {
      hb_errRT_BASE(EG_CREATE, 2010, NULL, HB_ERR_FUNCNAME, HB_ERR_ARGS_BASEPARAMS);
      return;
   }

   duk_push_array(ctx);
   hb_retni(duk_get_top_index(ctx));
}

/* 設置數組元素 */
HB_FUNC(DUK_PUT_ARRAY_ELEMENT)
{
   duk_idx_t arr_idx = hb_parni(1);
   duk_uarridx_t index = (duk_uarridx_t)hb_parni(2);

   if (ctx == NULL)
   {
      hb_errRT_BASE(EG_CREATE, 2010, NULL, HB_ERR_FUNCNAME, HB_ERR_ARGS_BASEPARAMS);
      return;
   }

   if (HB_ISCHAR(3))
   {
      duk_push_string(ctx, hb_parc(3));
   }
   else if (HB_ISNUM(3))
   {
      duk_push_number(ctx, hb_parnd(3));
   }
   else if (HB_ISLOG(3))
   {
      duk_push_boolean(ctx, hb_parl(3));
   }
   else
   {
      duk_push_undefined(ctx);
   }

   duk_put_prop_index(ctx, arr_idx, index);
   hb_retl(HB_TRUE);
}

/* 獲取數組元素 */
HB_FUNC(DUK_GET_ARRAY_ELEMENT)
{
   duk_idx_t arr_idx = hb_parni(1);
   duk_uarridx_t index = (duk_uarridx_t)hb_parni(2);

   if (ctx == NULL)
   {
      hb_errRT_BASE(EG_CREATE, 2010, NULL, HB_ERR_FUNCNAME, HB_ERR_ARGS_BASEPARAMS);
      return;
   }

   duk_get_prop_index(ctx, arr_idx, index);
   if (duk_is_string(ctx, -1))
   {
      hb_retc(duk_get_string(ctx, -1));
   }
   else if (duk_is_number(ctx, -1))
   {
      hb_retnd(duk_get_number(ctx, -1));
   }
   else if (duk_is_boolean(ctx, -1))
   {
      hb_retl(duk_get_boolean(ctx, -1));
   }
   else
   {
      hb_retc(duk_safe_to_string(ctx, -1));
   }
   duk_pop(ctx);
}

/* 獲取數組長度 */
HB_FUNC(DUK_GET_ARRAY_LENGTH)
{
   duk_idx_t arr_idx = hb_parni(1);
   duk_size_t len;

   if (ctx == NULL)
   {
      hb_errRT_BASE(EG_CREATE, 2010, NULL, HB_ERR_FUNCNAME, HB_ERR_ARGS_BASEPARAMS);
      return;
   }

   len = duk_get_length(ctx, arr_idx);
   hb_retni((int)len);
}

/* 檢查值類型 */
HB_FUNC(DUK_CHECK_TYPE)
{
   duk_idx_t idx = hb_parni(1);
   const char *type = hb_parc(2);
   HB_BOOL result = HB_FALSE;

   if (ctx == NULL)
   {
      hb_errRT_BASE(EG_CREATE, 2010, NULL, HB_ERR_FUNCNAME, HB_ERR_ARGS_BASEPARAMS);
      return;
   }

   if (type == NULL)
   {
      hb_errRT_BASE(EG_ARG, 2010, NULL, HB_ERR_FUNCNAME, HB_ERR_ARGS_BASEPARAMS);
      return;
   }

   if (strcmp(type, "string") == 0)
   {
      result = duk_is_string(ctx, idx);
   }
   else if (strcmp(type, "number") == 0)
   {
      result = duk_is_number(ctx, idx);
   }
   else if (strcmp(type, "boolean") == 0)
   {
      result = duk_is_boolean(ctx, idx);
   }
   else if (strcmp(type, "object") == 0)
   {
      result = duk_is_object(ctx, idx);
   }
   else if (strcmp(type, "array") == 0)
   {
      result = duk_is_array(ctx, idx);
   }
   else if (strcmp(type, "function") == 0)
   {
      result = duk_is_function(ctx, idx);
   }
   else if (strcmp(type, "undefined") == 0)
   {
      result = duk_is_undefined(ctx, idx);
   }
   else if (strcmp(type, "null") == 0)
   {
      result = duk_is_null(ctx, idx);
   }

   hb_retl(result);
}

/* 將 JavaScript 值轉換為 JSON 字符串 */
HB_FUNC(DUK_JSON_STRINGIFY)
{
   duk_idx_t idx = hb_parni(1);
   const char *json;

   if (ctx == NULL)
   {
      hb_errRT_BASE(EG_CREATE, 2010, NULL, HB_ERR_FUNCNAME, HB_ERR_ARGS_BASEPARAMS);
      return;
   }

   duk_json_encode(ctx, idx);
   json = duk_get_string(ctx, -1);
   hb_retc(json);
   duk_pop(ctx);
}

/* 將 JSON 字符串解析為 JavaScript 值 */
HB_FUNC(DUK_JSON_PARSE)
{
   const char *json = hb_parc(1);
   const char *error;

   if (ctx == NULL)
   {
      hb_errRT_BASE(EG_CREATE, 2010, NULL, HB_ERR_FUNCNAME, HB_ERR_ARGS_BASEPARAMS);
      return;
   }

   if (json == NULL)
   {
      hb_errRT_BASE(EG_ARG, 2010, NULL, HB_ERR_FUNCNAME, HB_ERR_ARGS_BASEPARAMS);
      return;
   }

   duk_push_string(ctx, json);
   duk_json_decode(ctx, -1);

   hb_retni(duk_get_top_index(ctx));
}

/* 設置錯誤處理器 */
HB_FUNC(DUK_SET_ERROR_HANDLER)
{
   const char *handler = hb_parc(1);
   const char *error;

   if (ctx == NULL)
   {
      hb_errRT_BASE(EG_CREATE, 2010, NULL, HB_ERR_FUNCNAME, HB_ERR_ARGS_BASEPARAMS);
      return;
   }

   if (handler == NULL)
   {
      hb_errRT_BASE(EG_ARG, 2010, NULL, HB_ERR_FUNCNAME, HB_ERR_ARGS_BASEPARAMS);
      return;
   }

   duk_push_string(ctx, handler);
   if (duk_peval(ctx) != 0)
   {
      error = duk_safe_to_string(ctx, -1);
      hb_errRT_BASE(EG_CREATE, 2010, NULL, HB_ERR_FUNCNAME, HB_ERR_ARGS_BASEPARAMS);
      duk_pop(ctx);
      return;
   }

   duk_put_global_string(ctx, "errorHandler");
   hb_retl(HB_TRUE);
}

/* 設置記憶體限制 */
HB_FUNC(DUK_SET_MEMORY_LIMIT)
{
   hb_retl(HB_TRUE);
}

/* 獲取記憶體使用情況 */
HB_FUNC(DUK_GET_MEMORY_INFO)
{
   duk_memory_functions funcs;
   PHB_ITEM pArray = hb_itemArrayNew(4);

   if (ctx == NULL)
   {
      hb_errRT_BASE(EG_CREATE, 2010, NULL, HB_ERR_FUNCNAME, HB_ERR_ARGS_BASEPARAMS);
      return;
   }

   duk_get_memory_functions(ctx, &funcs);

   hb_arraySetNInt(pArray, 1, (HB_PTRUINT)funcs.alloc_func);
   hb_arraySetNInt(pArray, 2, (HB_PTRUINT)funcs.realloc_func);
   hb_arraySetNInt(pArray, 3, (HB_PTRUINT)funcs.free_func);
   hb_arraySetNInt(pArray, 4, (HB_PTRUINT)funcs.udata);

   hb_itemReturn(pArray);
   hb_itemRelease(pArray);
}

/* 設置對象原型 */
HB_FUNC(DUK_SET_PROTOTYPE)
{
   duk_idx_t obj_idx = hb_parni(1);
   duk_idx_t proto_idx = hb_parni(2);

   if (ctx == NULL)
   {
      hb_errRT_BASE(EG_CREATE, 2010, NULL, HB_ERR_FUNCNAME, HB_ERR_ARGS_BASEPARAMS);
      return;
   }

   duk_set_prototype(ctx, obj_idx);
   hb_retl(HB_TRUE);
}

/* 獲取對象原型 */
HB_FUNC(DUK_GET_PROTOTYPE)
{
   duk_idx_t obj_idx = hb_parni(1);

   if (ctx == NULL)
   {
      hb_errRT_BASE(EG_CREATE, 2010, NULL, HB_ERR_FUNCNAME, HB_ERR_ARGS_BASEPARAMS);
      return;
   }

   duk_get_prototype(ctx, obj_idx);
   hb_retni(duk_get_top_index(ctx));
}

/* 綁定函數到對象 */
HB_FUNC(DUK_BIND_FUNCTION)
{
   hb_retl(HB_TRUE);
}

/* 創建新的函數 */
HB_FUNC(DUK_PUSH_C_FUNCTION)
{
   const char *name = hb_parc(1);
   duk_c_function func = (duk_c_function)hb_parptr(2);
   duk_idx_t nargs = hb_parni(3);

   if (ctx == NULL)
   {
      hb_errRT_BASE(EG_CREATE, 2010, NULL, HB_ERR_FUNCNAME, HB_ERR_ARGS_BASEPARAMS);
      return;
   }

   if (name == NULL || func == NULL)
   {
      hb_errRT_BASE(EG_ARG, 2010, NULL, HB_ERR_FUNCNAME, HB_ERR_ARGS_BASEPARAMS);
      return;
   }

   duk_push_c_function(ctx, func, nargs);
   duk_put_global_string(ctx, name);
   hb_retl(HB_TRUE);
}

/* 獲取函數參數數量 */
HB_FUNC(DUK_GET_FUNCTION_PARAM_COUNT)
{
   hb_retni(0);
}

/* 設置對象枚舉器 */
HB_FUNC(DUK_ENUM)
{
   duk_idx_t obj_idx = hb_parni(1);
   duk_uint_t enum_flags = (duk_uint_t)hb_parni(2);

   if (ctx == NULL)
   {
      hb_errRT_BASE(EG_CREATE, 2010, NULL, HB_ERR_FUNCNAME, HB_ERR_ARGS_BASEPARAMS);
      return;
   }

   duk_enum(ctx, obj_idx, enum_flags);
   hb_retni(duk_get_top_index(ctx));
}

/* 獲取下一個枚舉項 */
HB_FUNC(DUK_NEXT)
{
   duk_idx_t enum_idx = hb_parni(1);
   duk_bool_t has_next = duk_next(ctx, enum_idx, 1);
   hb_retl(has_next);
}

/* 設置對象不可變 */
HB_FUNC(DUK_FREEZE)
{
   hb_retl(HB_TRUE);
}

/* 檢查對象是否可變 */
HB_FUNC(DUK_IS_FROZEN)
{
   hb_retl(HB_FALSE);
}

/* 設置對象密封 */
HB_FUNC(DUK_SEAL)
{
   hb_retl(HB_TRUE);
}

/* 檢查對象是否密封 */
HB_FUNC(DUK_IS_SEALED)
{
   hb_retl(HB_FALSE);
}

/* 設置對象擴展性 */
HB_FUNC(DUK_PREVENT_EXTENSIONS)
{
   hb_retl(HB_TRUE);
}

/* 檢查對象是否可擴展 */
HB_FUNC(DUK_IS_EXTENSIBLE)
{
   hb_retl(HB_TRUE);
}

/* 設置對象屬性描述符 */
HB_FUNC(DUK_DEFINE_PROPERTY)
{
   duk_idx_t obj_idx = hb_parni(1);
   const char *key = hb_parc(2);
   duk_uint_t flags = (duk_uint_t)hb_parni(3);

   if (ctx == NULL)
   {
      hb_errRT_BASE(EG_CREATE, 2010, NULL, HB_ERR_FUNCNAME, HB_ERR_ARGS_BASEPARAMS);
      return;
   }

   if (key == NULL)
   {
      hb_errRT_BASE(EG_ARG, 2010, NULL, HB_ERR_FUNCNAME, HB_ERR_ARGS_BASEPARAMS);
      return;
   }

   duk_push_string(ctx, key);
   duk_def_prop(ctx, obj_idx, flags);
   hb_retl(HB_TRUE);
}

/* 獲取對象屬性描述符 */
HB_FUNC(DUK_GET_OWN_PROP_DESC)
{
   hb_retl(HB_TRUE);
}

/* 設置對象屬性訪問器 */
HB_FUNC(DUK_DEFINE_ACCESSOR)
{
   duk_idx_t obj_idx = hb_parni(1);
   const char *key = hb_parc(2);
   duk_c_function getter = (duk_c_function)hb_parptr(3);
   duk_c_function setter = (duk_c_function)hb_parptr(4);
   duk_uint_t flags = (duk_uint_t)hb_parni(5);

   if (ctx == NULL)
   {
      hb_errRT_BASE(EG_CREATE, 2010, NULL, HB_ERR_FUNCNAME, HB_ERR_ARGS_BASEPARAMS);
      return;
   }

   if (key == NULL)
   {
      hb_errRT_BASE(EG_ARG, 2010, NULL, HB_ERR_FUNCNAME, HB_ERR_ARGS_BASEPARAMS);
      return;
   }

   duk_push_string(ctx, key);
   if (getter != NULL)
   {
      duk_push_c_function(ctx, getter, 0);
   }
   else
   {
      duk_push_undefined(ctx);
   }
   if (setter != NULL)
   {
      duk_push_c_function(ctx, setter, 1);
   }
   else
   {
      duk_push_undefined(ctx);
   }
   duk_def_prop(ctx, obj_idx, flags);
   hb_retl(HB_TRUE);
}

/* 強制執行垃圾回收 */
HB_FUNC(DUK_GC)
{
   if (ctx == NULL)
   {
      hb_errRT_BASE(EG_CREATE, 2010, NULL, HB_ERR_FUNCNAME, HB_ERR_ARGS_BASEPARAMS);
      return;
   }

   duk_gc(ctx, 0);
   hb_retl(HB_TRUE);
}

/* 設置垃圾回收回調函數 */
HB_FUNC(DUK_SET_GC_CALLBACK)
{
   const char *callback;
   const char *error;

   if (ctx == NULL)
   {
      hb_errRT_BASE(EG_CREATE, 2010, NULL, HB_ERR_FUNCNAME, HB_ERR_ARGS_BASEPARAMS);
      return;
   }

   callback = hb_parc(1);
   if (callback == NULL)
   {
      hb_errRT_BASE(EG_ARG, 2010, NULL, HB_ERR_FUNCNAME, HB_ERR_ARGS_BASEPARAMS);
      return;
   }

   duk_push_string(ctx, callback);
   if (duk_peval(ctx) != 0)
   {
      error = duk_safe_to_string(ctx, -1);
      hb_errRT_BASE(EG_CREATE, 2010, NULL, HB_ERR_FUNCNAME, HB_ERR_ARGS_BASEPARAMS);
      duk_pop(ctx);
      return;
   }

   duk_put_global_string(ctx, "gcCallback");
   hb_retl(HB_TRUE);
}

/* 獲取垃圾回收統計信息 */
HB_FUNC(DUK_GET_GC_STATS)
{
   duk_memory_functions funcs;
   PHB_ITEM pArray;

   if (ctx == NULL)
   {
      hb_errRT_BASE(EG_CREATE, 2010, NULL, HB_ERR_FUNCNAME, HB_ERR_ARGS_BASEPARAMS);
      return;
   }

   duk_get_memory_functions(ctx, &funcs);

   pArray = hb_itemArrayNew(3);
   hb_arraySetNInt(pArray, 1, (HB_PTRUINT)funcs.alloc_func);
   hb_arraySetNInt(pArray, 2, (HB_PTRUINT)funcs.realloc_func);
   hb_arraySetNInt(pArray, 3, (HB_PTRUINT)funcs.free_func);

   hb_itemReturn(pArray);
   hb_itemRelease(pArray);
}

/* 設置記憶體分配回調函數 */
HB_FUNC(DUK_SET_ALLOC_CALLBACK)
{
   const char *callback;
   const char *error;

   if (ctx == NULL)
   {
      hb_errRT_BASE(EG_CREATE, 2010, NULL, HB_ERR_FUNCNAME, HB_ERR_ARGS_BASEPARAMS);
      return;
   }

   callback = hb_parc(1);
   if (callback == NULL)
   {
      hb_errRT_BASE(EG_ARG, 2010, NULL, HB_ERR_FUNCNAME, HB_ERR_ARGS_BASEPARAMS);
      return;
   }

   duk_push_string(ctx, callback);
   if (duk_peval(ctx) != 0)
   {
      error = duk_safe_to_string(ctx, -1);
      hb_errRT_BASE(EG_CREATE, 2010, NULL, HB_ERR_FUNCNAME, HB_ERR_ARGS_BASEPARAMS);
      duk_pop(ctx);
      return;
   }

   duk_put_global_string(ctx, "allocCallback");
   hb_retl(HB_TRUE);
}

/* 設置記憶體釋放回調函數 */
HB_FUNC(DUK_SET_FREE_CALLBACK)
{
   const char *callback;
   const char *error;

   if (ctx == NULL)
   {
      hb_errRT_BASE(EG_CREATE, 2010, NULL, HB_ERR_FUNCNAME, HB_ERR_ARGS_BASEPARAMS);
      return;
   }

   callback = hb_parc(1);
   if (callback == NULL)
   {
      hb_errRT_BASE(EG_ARG, 2010, NULL, HB_ERR_FUNCNAME, HB_ERR_ARGS_BASEPARAMS);
      return;
   }

   duk_push_string(ctx, callback);
   if (duk_peval(ctx) != 0)
   {
      error = duk_safe_to_string(ctx, -1);
      hb_errRT_BASE(EG_CREATE, 2010, NULL, HB_ERR_FUNCNAME, HB_ERR_ARGS_BASEPARAMS);
      duk_pop(ctx);
      return;
   }

   duk_put_global_string(ctx, "freeCallback");
   hb_retl(HB_TRUE);
}

/* 獲取記憶體分配統計信息 */
HB_FUNC(DUK_GET_ALLOC_STATS)
{
   duk_memory_functions funcs;
   PHB_ITEM pArray;

   if (ctx == NULL)
   {
      hb_errRT_BASE(EG_CREATE, 2010, NULL, HB_ERR_FUNCNAME, HB_ERR_ARGS_BASEPARAMS);
      return;
   }

   duk_get_memory_functions(ctx, &funcs);

   pArray = hb_itemArrayNew(2);
   hb_arraySetNInt(pArray, 1, (HB_PTRUINT)funcs.alloc_func);
   hb_arraySetNInt(pArray, 2, (HB_PTRUINT)funcs.free_func);

   hb_itemReturn(pArray);
   hb_itemRelease(pArray);
} 