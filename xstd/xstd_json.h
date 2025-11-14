#pragma once

// Port of JSONitator (https://github.com/wAIfu-DEV/JSONitator)

#include "xstd_string.h"
#include "xstd_math.h"
#include "xstd_io.h"

typedef u8 JsonValueType;

#define JSON_VAL_OBJECT (JsonValueType)0
#define JSON_VAL_ARRAY (JsonValueType)1
#define JSON_VAL_NUMBER (JsonValueType)2
#define JSON_VAL_STRING (JsonValueType)3
#define JSON_VAL_BOOL (JsonValueType)4
#define JSON_VAL_NULL (JsonValueType)5

static ConstStr _JSON_VAL_TO_STR[6] = {
    [JSON_VAL_OBJECT] = "object",
    [JSON_VAL_ARRAY] = "array",
    [JSON_VAL_NUMBER] = "number",
    [JSON_VAL_STRING] = "string",
    [JSON_VAL_BOOL] = "bool",
    [JSON_VAL_NULL] = "null",
};

static ConstStr _JSON_LIST_SEQ = "\"\\\b\f\n\r\t";
static ConstStr _JSON_LIST_ESC = "\"\\/bfnrt";

static i8 _JSON_ESC_TO_SEQ[117] = {
    ['\"'] = '\"',
    ['\\'] = '\\',
    ['/'] = '/',
    ['b'] = '\b',
    ['f'] = '\f',
    ['n'] = '\n',
    ['r'] = '\r',
    ['t'] = '\t',
};

static ConstStr _JSON_SEQ_TO_ESC[93] = {
    ['\"'] = "\\\"",
    ['\\'] = "\\\\",
    ['\b'] = "\\b",
    ['\f'] = "\\f",
    ['\n'] = "\\n",
    ['\r'] = "\\r",
    ['\t'] = "\\t",
};

static const u64 _MAX_ITER = ((u64)0) - 1;

typedef struct _json
{
    void *value;
    JsonValueType type;
} Json;

typedef struct _json_obj
{
    String *fields;
    Json **values;
    u64 length;
} JsonObject;

typedef struct _json_array
{
    Json **elements;
    u64 length;
} JsonArray;

typedef f64 JsonNumber;

Error _json_parse_any_value(Allocator *a, Json *self, ConstStr s, u64 *i);
Error _json_parse_object(Allocator *a, Json *self, ConstStr s, u64 *i);
Error _json_parse_array(Allocator *a, Json *self, ConstStr s, u64 *i);

Error _json_init_object(Allocator *a, JsonObject *self)
{
    self->fields = (char**)a->alloc(a, sizeof(char *));
    if (!self->fields)
    {
        return X_ERR_EXT("json", "_json_init_object",
            ERR_OUT_OF_MEMORY, "alloc failure");
    }

    self->values = (Json**)a->alloc(a, sizeof(Json *));
    if (!self->values)
    {
        return X_ERR_EXT("json", "_json_init_object",
            ERR_OUT_OF_MEMORY, "alloc failure");
    }

    self->fields[0] = NULL;
    self->values[0] = NULL;
    self->length = 0;

    return X_ERR_OK;
}

int _json_str_comp(const char *x, const char *y)
{
    for (u64 i = 0; i < _MAX_ITER; ++i)
    {
        if (!x[i] || !y[i])
        {
            return (x[i] == y[i]);
        }

        if (x[i] != y[i])
            return 0;
    }
    return 0;
}

u64 _json_str_len(ConstStr s)
{
    if (!s)
        return 0;

    u64 i = 0;

    if (s[0] == '\0')
        return 0;

    while (s[i] != '\0')
    {
        if (i == _MAX_ITER)
            return 0;

        ++i;
    }
    return i;
}

u64 _json_number_len(ConstStr s)
{
    if (s == NULL)
        return 0;

    u64 i = 0;

    if (s[0] == '\0')
        return 0;

    while (s[i] != '\0' && (char_is_digit(s[i]) || s[i] == '.' || s[i] == '-'))
    {
        if (i == _MAX_ITER)
            return 0;

        ++i;
    }
    return i;
}

Error _json_str_copy_alloc(Allocator* a, ConstStr s, String *buff)
{
    if (!s)
        return X_ERR_EXT("json", "_json_str_copy_alloc",
            ERR_INVALID_PARAMETER, "null string");

    u64 len = _json_str_len(s);
    (*buff) = a->alloc(a, sizeof(char) * (len + 1));

    if (!(*buff))
        return X_ERR_EXT("json", "_json_str_copy_alloc",
            ERR_OUT_OF_MEMORY, "alloc failure");

    if (len == 0)
    {
        (*buff)[0] = '\0';
        return X_ERR_OK;
    }

    u64 i = 0;
    while (s[i] != 0)
    {
        if (i == _MAX_ITER)
        {
            (*buff)[0] = '\0';
            return X_ERR_EXT("json", "_json_str_copy_alloc",
                ERR_RANGE_ERROR, "max iter reached in string traversal");
        }

        (*buff)[i] = s[i];
        ++i;
    }

    (*buff)[i] = '\0';
    return X_ERR_OK;
}

Error _append_object_entry(Allocator *a, JsonObject *self, const char *field, Json *value)
{
    self->fields = (char**)a->realloc(a, self->fields, sizeof(char *) * (self->length + 2));

    if (!self->fields)
        return X_ERR_EXT("json", "_append_object_entry",
            ERR_INVALID_PARAMETER, "alloc failure");

    self->values = (Json**)a->realloc(a, self->values, sizeof(Json *) * (self->length + 2));

    if (!self->values)
        return X_ERR_EXT("json", "_append_object_entry",
            ERR_INVALID_PARAMETER, "alloc failure");

    Error err = _json_str_copy_alloc(a, field, &self->fields[self->length]);

    if (err.code != ERR_OK)
        return err;

    self->fields[self->length + 1] = NULL;

    self->values[self->length] = value;
    self->fields[self->length + 1] = NULL;

    self->length += 1;
    return X_ERR_OK;
}

u64 _json_unparsed_str_len(ConstStr s)
{
    int is_escaped = 0;

    u64 i = 0;

    while (!(s[i] == '"' && !is_escaped))
    {
        is_escaped = 0;

        if (i == _MAX_ITER)
            return 0;

        if (s[i] == '\0')
            return 0;

        if (s[i] == '\\')
            is_escaped = 1;

        ++i;
    }
    return i;
}

HeapStr _json_unescape_string(Allocator* a, const char *s)
{
    u64 len = _json_str_len(s);
    HeapStr parsed = a->alloc(a, sizeof(char) * (len + 1));

    u64 si = 0;
    u64 pi = 0;
    int is_escaped = 0;

    while (!(s[si] == '"' && !is_escaped))
    {
        if (si == _MAX_ITER)
            return NULL;

        if (is_escaped)
        {
            if (s[si] == '\0' || string_find_char(_JSON_LIST_ESC, s[si]) == -1)
            {
                //_printf("JSONparser: Found invalid escaped character '\\%c' inside string.", s[si] ? s[si] : '0');
                return NULL;
            }

            char seq = _JSON_ESC_TO_SEQ[s[si]];

            parsed[si - pi] = seq;
            ++si;

            is_escaped = 0;
            continue;
        }

        is_escaped = 0;

        if (s[si] == '\0')
        {
            //_print("Failed to unescape string: found NULL before end of string, how does that happen??");
            return NULL;
        }

        if (s[si] == '\\')
        {
            is_escaped = 1;
            ++si;
            ++pi;
            continue;
        }

        parsed[si - pi] = s[si];
        ++si;
    }
    parsed[si - pi] = '\0';
    return parsed;
}

char *_json_parse_string(Allocator *a, const char *s, u64 *i)
{
    (*i) += 1; // Add opening quote to i

    u64 len = _json_unparsed_str_len(&s[*i]);
    char *parsed = (char*)a->alloc(a, sizeof(char) * (len + 1));

    u64 si = *i;
    u64 pi = 0;
    int is_escaped = 0;

    while (!(s[si] == '"' && !is_escaped))
    {
        if (si == _MAX_ITER)
        {
            // TODO: Return ResultOwnedStr
            //return X_ERR_EXT("json", "_json_parse_string",
            //    ERR_RANGE_ERROR, "max iter while traversing string");
            return NULL;
        }

        if (is_escaped)
        {
            if (s[si] == '\0' || string_find_char(_JSON_LIST_ESC, s[si]) == -1)
            {
                //_printf("JSONparser: Found invalid escaped character '\\%c' inside string.", s[si] ? s[si] : '0');
                return NULL;
            }

            char seq = _JSON_ESC_TO_SEQ[s[si]];

            parsed[si - pi - (*i)] = seq;
            ++si;

            is_escaped = 0;
            continue;
        }

        is_escaped = 0;

        if (s[si] == '\0')
        {
            //_print("Failed to parse string: found EOF during parsing.");
            return NULL;
        }

        if (s[si] == '\\')
        {
            is_escaped = 1;
            ++si;
            ++pi;
            continue;
        }

        parsed[si - pi - (*i)] = s[si];
        ++si;
    }
    parsed[si - pi - (*i)] = '\0';

    (*i) += len + 1; // we add length of str + last closing quote
    return parsed;
}

Error _json_consume_colon(const char *s, u64 *i)
{
    while (s[*i] != '\0')
    {
        if ((*i) == _MAX_ITER)
        {
            return X_ERR_EXT("json", "_json_consume_colon",
                ERR_RANGE_ERROR, "max iter while traversing string seeking colon");
        }

        if (char_is_whitespace(s[*i]))
        {
            ++(*i);
            continue;
        }

        if (s[*i] == ':')
        {
            ++(*i);
            return X_ERR_OK;
        }

        return X_ERR_EXT("json", "_json_consume_colon",
            ERR_UNEXPECTED_BYTE, "unexpected byte while seeking colon");
    }

    return X_ERR_EXT("json", "_json_consume_colon",
        ERR_UNEXPECTED_BYTE, "EOF while seeking colon");
}

Error _json_parse_number(Allocator* a, Json *self, const char *s, u64 *i)
{
    Error ret = X_ERR_OK;

    if (s[*i] == '\0')
        return X_ERR_EXT("json", "_json_parse_number",
            ERR_UNEXPECTED_BYTE, "EOF while parsing number");

    u64 len = _json_number_len(&s[*i]);

    char *digits_buff = a->alloc(a, sizeof(char) * (len + 1));

    if (!digits_buff)
        return X_ERR_EXT("json", "_json_parse_number",
            ERR_OUT_OF_MEMORY, "alloc failure");

    for (u64 j = 0; j < len; ++j)
        digits_buff[j] = s[*i + j];

    digits_buff[len] = '\0';

    JsonNumber *num_ptr = a->alloc(a, sizeof(JsonNumber));
    if (num_ptr == NULL)
    {
        ret = X_ERR_EXT("json", "_json_parse_number",
            ERR_OUT_OF_MEMORY, "alloc failure");
        goto clean_digits;
    }

    ResultF64 parseRes = string_parse_float(digits_buff);
    if (parseRes.error.code)
    {
        a->free(a, num_ptr);
        num_ptr = NULL;

        ret = X_ERR_EXT("json", "_json_parse_number",
            ERR_PARSE_ERROR, "number parse failure");
        goto clean_digits;
    }
    *num_ptr = parseRes.value;

    self->type = JSON_VAL_NUMBER;
    self->value = num_ptr;

    (*i) += len;

clean_digits:
    a->free(a, digits_buff);
    digits_buff = NULL;
    return ret;
}

Error _json_parse_any_value(Allocator* a, Json *self, ConstStr s, u64 *i)
{
    if (s[*i] == '\0')
    {
        return X_ERR_EXT("json", "_json_parse_any_value",
            ERR_UNEXPECTED_BYTE, "EOF while traversing value");
    }

    while (s[*i] != '\0')
    {
        if ((*i) == _MAX_ITER)
            return X_ERR_EXT("json", "_json_parse_any_value",
                ERR_RANGE_ERROR, "max iter while traversing value");

        if (char_is_whitespace(s[*i]))
        {
            ++(*i);
            continue;
        }

        if (s[*i] == 'n')
        {
            if (s[(*i) + 1] == 'u' && s[(*i) + 2] == 'l' && s[(*i) + 3] == 'l')
            {
                self->type = JSON_VAL_NULL;
                self->value = NULL;
                (*i) += 4;
                return X_ERR_OK;
            }
            else
            {
                return X_ERR_EXT("json", "_json_parse_any_value",
                    ERR_UNEXPECTED_BYTE, "failed to parse null");
            }
        }

        if (s[*i] == 't')
        {
            if (s[(*i) + 1] == 'r' && s[(*i) + 2] == 'u' && s[(*i) + 3] == 'e')
            {
                ibool *boolean = a->alloc(a, sizeof(ibool));

                if (!boolean)
                    return X_ERR_EXT("json", "_json_parse_any_value",
                        ERR_OUT_OF_MEMORY, "alloc failure");

                (*boolean) = 1;

                self->type = JSON_VAL_BOOL;
                self->value = boolean;
                (*i) += 4;
                return X_ERR_OK;
            }
            else
            {
                return X_ERR_EXT("json", "_json_parse_any_value",
                    ERR_UNEXPECTED_BYTE, "failed to parse true");
            }
        }

        if (s[*i] == 'f')
        {
            if (s[(*i) + 1] == 'a' && s[(*i) + 2] == 'l' && s[(*i) + 3] == 's' && s[(*i) + 4] == 'e')
            {
                ibool *boolean = a->alloc(a, sizeof(ibool));

                if (!boolean)
                    return X_ERR_EXT("json", "_json_parse_any_value",
                        ERR_OUT_OF_MEMORY, "alloc failure");

                (*boolean) = 0;

                self->type = JSON_VAL_BOOL;
                self->value = boolean;
                (*i) += 5;
                return X_ERR_OK;
            }
            else
            {
                return X_ERR_EXT("json", "_json_parse_any_value",
                    ERR_UNEXPECTED_BYTE, "failed to parse false");
            }
        }

        if (s[*i] == '{')
        {
            self->type = JSON_VAL_OBJECT;
            return _json_parse_object(a, self, s, i);
        }

        if (s[*i] == '[')
        {
            self->type = JSON_VAL_ARRAY;
            return _json_parse_array(a, self, s, i);
        }

        if (s[*i] == '"')
        {
            char *str = _json_parse_string(a, s, i);

            if (str == NULL)
            {
                return X_ERR_EXT("json", "_json_parse_any_value",
                    ERR_UNEXPECTED_BYTE, "failed to parse string value");
            }

            self->type = JSON_VAL_STRING;
            self->value = str;
            return X_ERR_OK;
        }

        if (char_is_digit(s[*i]) || s[*i] == '.' || s[*i] == '-')
        {
            self->type = JSON_VAL_NUMBER;
            return _json_parse_number(a, self, s, i);
        }

        return X_ERR_EXT("json", "_json_parse_any_value",
            ERR_UNEXPECTED_BYTE, "unexpected byte while traversing value");
    }

    return X_ERR_OK;
}

Error _json_parse_object(Allocator *a, Json *self, ConstStr s, u64 *i)
{
    ++(*i);

    if (s[*i] == '\0')
        return X_ERR_EXT("json", "_json_parse_object",
            ERR_UNEXPECTED_BYTE, "EOF during object parsing");

    JsonObject *obj = a->alloc(a, sizeof(JsonObject));
    Error err = _json_init_object(a, obj);

    if (err.code != ERR_OK)
    {
        a->free(a, obj);
        obj = NULL;
        return X_ERR_EXT("json", "_json_parse_object",
            ERR_OUT_OF_MEMORY, "alloc failure");
    }

    self->type = JSON_VAL_OBJECT;
    self->value = obj;

    while (s[*i] != '\0')
    {
        if ((*i) == _MAX_ITER)
        {
            err = X_ERR_EXT("json", "_json_parse_object",
                ERR_RANGE_ERROR, "max iter while parsing object");
            goto clean_obj_entries;
        }

        if (char_is_whitespace(s[*i]))
        {
            ++(*i);
            continue;
        }

        if (s[*i] == '}')
        {
            ++(*i);
            return X_ERR_OK;
        }

        if (s[*i] == '"')
        {
            char *field = _json_parse_string(a, s, i);

            if (field == NULL)
            {
                err = X_ERR_EXT("json", "_json_parse_object",
                    ERR_RANGE_ERROR, "failed to parse field string");
                goto clean_obj_entries;
            }

            Error res = _json_consume_colon(s, i);
            if (res.code != ERR_OK)
            {
                err = res;
                goto clean_field;
            }

            Json *value = a->alloc(a, sizeof(Json));

            if (!value)
            {
                err = X_ERR_EXT("json", "_json_parse_object",
                    ERR_OUT_OF_MEMORY, "alloc failure");
                goto clean_field;
            }

            err = _json_parse_any_value(a, value, s, i);

            if (err.code != ERR_OK)
                goto clean_value;

            err = _append_object_entry(a, obj, field, value);

            if (err.code != ERR_OK)
                goto clean_value;

            while (char_is_whitespace(s[*i]))
            {
                ++(*i);
            }

            if (s[*i] == ',')
            {
                a->free(a, field);
                field = NULL;

                ++(*i);
                continue;
            }
            else if (s[*i] != '}')
            {
                err = X_ERR_EXT("json", "_json_parse_object",
                    ERR_UNEXPECTED_BYTE, "expected ',' or '}'");
                goto clean_value;
            }

            a->free(a, field);
            field = NULL;

            ++(*i);
            return X_ERR_OK;

        clean_value:
            a->free(a, value);
            value = NULL;
        clean_field:
            a->free(a, field);
            field = NULL;
        clean_obj_entries:
            a->free(a, obj->fields);
            obj->fields = NULL;
            a->free(a, obj->values);
            obj->values = NULL;
        clean_obj:
            a->free(a, obj);
            obj = NULL;
            return err;
        }

        err = X_ERR_EXT("json", "_json_parse_object",
            ERR_UNEXPECTED_BYTE, "unexpected byte");

        a->free(a, obj);
        obj = NULL;
        return err;
    }

    err = X_ERR_EXT("json", "_json_parse_object",
        ERR_UNEXPECTED_BYTE, "found EOF while parsing object");

    a->free(a, obj);
    obj = NULL;
    return err;
};

Error _init_array(Allocator *a, JsonArray *self)
{
    self->elements = a->alloc(a, sizeof(Json *));
    if (!self->elements)
        return X_ERR_EXT("json", "_init_array", ERR_OUT_OF_MEMORY, "alloc failure");

    self->length = 0;
    return X_ERR_OK;
}

Error _append_array_element(Allocator* a, JsonArray *array, Json *value)
{
    array->elements = a->realloc(a, array->elements, sizeof(Json *) * (array->length + 1));

    if (!array->elements)
        return X_ERR_EXT("json", "_append_array_element",
            ERR_OUT_OF_MEMORY, "alloc failure");

    array->elements[array->length] = value;
    array->length += 1;
    return X_ERR_OK;
}

Error _json_parse_array(Allocator* a, Json *self, ConstStr s, u64 *i)
{
    (*i)++;

    if (s[*i] == '\0')
        return X_ERR_EXT("json", "_json_parse_array",
            ERR_UNEXPECTED_BYTE, "EOF during array parsing");

    JsonArray *array = a->alloc(a, sizeof(JsonArray));

    if (!array)
        return X_ERR_EXT("json", "_json_parse_array",
            ERR_OUT_OF_MEMORY, "alloc failure");

    Error err = _init_array(a, array);
    if (err.code != ERR_OK)
    {
        a->free(a, array);
        array = NULL;
        return err;
    }

    self->type = JSON_VAL_ARRAY;
    self->value = array;

    while (s[*i] != '\0')
    {
        if ((*i) == _MAX_ITER)
        {
            err = X_ERR_EXT("json", "_json_parse_array",
                ERR_RANGE_ERROR, "max iter reached while parsing array");
            goto clean_arr_elems;
        }

        if (char_is_whitespace(s[*i]))
        {
            ++(*i);
            continue;
        }

        if (s[*i] == ']')
        {
            ++(*i);
            return X_ERR_OK;
        }

        Json *value = a->alloc(a, sizeof(Json));
        if (!value)
        {
            err = X_ERR_EXT("json", "_json_parse_array",
                ERR_OUT_OF_MEMORY, "alloc failure");
            goto clean_arr_elems;
        }

        err = _json_parse_any_value(a, value, s, i);
        if (err.code != ERR_OK)
        {
            goto clean_value;
        }

        err = _append_array_element(a, array, value);
        if (err.code != ERR_OK)
        {
            goto clean_value;
        }

        while (char_is_whitespace(s[*i]))
        {
            ++(*i);
        }

        if (s[*i] == ',')
        {
            ++(*i);
            continue;
        }

        if (s[*i] == ']')
        {
            ++(*i);
            return X_ERR_OK;
        }

        err = X_ERR_EXT("json", "_json_parse_array",
            ERR_UNEXPECTED_BYTE, "unexpected byte while parsing array");

    clean_value:
        free(value);
        value = NULL;
    clean_arr_elems:
        free(array->elements);
        array->elements = NULL;
    clean_arr:
        free(array);
        array = NULL;
        return err;
    }

    err = X_ERR_EXT("json", "_json_parse_array",
        ERR_UNEXPECTED_BYTE, "EOF while parsing array");

    a->free(a, array->elements);
    array->elements = NULL;
    a->free(a, array);
    array = NULL;
    return err;
}

/**
 * @brief Parses a Json string and returns a `Json` struct pointer
 *
 * @param s json string (s is not modified)
 * @return NULL | Json* (memory owned, you need to free it using `json_free`)
 */
Json *json_parse(Allocator* a, ConstStr s)
{
    if (!s)
        return NULL;

    u64 i = 0;

    while (1)
    {
        if (i == _MAX_ITER)
        {
            _print("_MAX_ITER reached in json_parse");
            return NULL;
        }

        if (s[i] == '\0')
        {
            _print("Found EOF in json_parse");
            return NULL;
        }

        if (s[i] == '{')
        {
            Json *obj = a->alloc(a, sizeof(Json));

            if (!obj)
            {
                _print("Failed to alloc object in json_parse");
                return NULL;
            }

            obj->type = JSON_VAL_OBJECT;

            Error err = _json_parse_object(a, obj, s, &i);

            if (err.code != ERR_OK)
            {
                _print("Failed to parse outer object.");
                goto clean_obj;
            }

            return obj;

        clean_obj:
            a->free(a, obj);
            obj = NULL;
            return NULL;
        }

        if (s[i] == '[')
        {
            Json *arr = a->alloc(a, sizeof(Json));
            if (!arr)
            {
                _print("Failed to alloc array in json_parse");
                return NULL;
            }

            arr->type = JSON_VAL_ARRAY;
            Error err = _json_parse_array(a, arr, s, &i);

            if (err.code != ERR_OK)
            {
                _print("Failed to parse outer array.");
                goto clean_arr;
            }

            return arr;

        clean_arr:
            a->free(a, arr);
            arr = NULL;
            return NULL;
        }

        if (char_is_whitespace(s[i]))
        {
            ++i;
            continue;
        }
        else
        {
            //_printf("JSONparser: Expected characters ['{','[',' ','\\n','\\r','\\t'] but got unexpected '%c' instead at position %lld\n", s[i], i);
            return NULL;
        }
    }
    //_print("Somehow you escaped an infinite loop, how did we get here??");
    return NULL;
}

/**
 * @brief Get a value in object using field name
 *
 * @param self Json struct of type JSON_VAL_OBJECT
 * @param field name of field
 * @return NULL | Json* (do not free, if you wish to free the memory, free the highest parent instead using json_free)
 */
Json *json_object_get(Json *self, const char *field)
{
    if (self->type != JSON_VAL_OBJECT)
    {
        //_print("Cannot use json_object_get on object that is not of type JSON_VAL_OBJECT.");
        return NULL;
    }

    JsonObject *obj = self->value;

    for (u64 i = 0; i < obj->length; ++i)
    {
        if (_json_str_comp(field, obj->fields[i]))
        {
            return obj->values[i];
        }
    }
    //_printf("JSONparser: json_object_get failed to find field \"%s\" in object.\n", field);
    return NULL;
}

/**
 * @brief Get the item at index in array
 *
 * @param self Json struct of type JSON_VAL_ARRAY
 * @param index
 * @return NULL | Json* (do not free, if you wish to free the memory, free the highest parent instead using json_free)
 */
Json *json_array_get(Json *self, u64 index)
{
    if (self->type != JSON_VAL_ARRAY)
    {
        //_print("Cannot use json_array_get on object that is not of type JSON_VAL_ARRAY.");
        return NULL;
    }

    JsonArray *arr = self->value;

    if (index >= arr->length)
    {
        //_printf("JSONparser: in json_get_array, tried to access index %llu, which is out of bounds of array.", index);
        return NULL;
    }

    return arr->elements[index];
}

Json *json_get_deep(Json *self, u64 fields_amount, const char *fields[fields_amount])
{
    if (fields_amount == 0)
    {
        //_print("json_get_deep called with 0 fields to access.");
        return NULL;
    }

    const char *access = fields[0];

    if (access == NULL)
    {
        //_print("argument fields[0] of json_get_deep is NULL.");
        return NULL;
    }

    if (self->type == JSON_VAL_OBJECT)
    {
        Json *val = json_object_get(self, access);

        if (val == NULL)
        {
            //_print("Failed to get value from object access.");
            return NULL;
        }

        if (fields_amount == 1)
        {
            return val;
        }
        else
        {
            return json_get_deep(val, fields_amount - 1, &fields[1]);
        }
    }
    else if (self->type == JSON_VAL_ARRAY)
    {
        u64 index = strtoull(access, NULL, 10);
        Json *val = json_array_get(self, index);

        if (val == NULL)
        {
            //_print("Failed to get value from array access.");
            return NULL;
        }

        if (fields_amount == 1)
        {
            return val;
        }
        else
        {
            return json_get_deep(val, fields_amount - 1, &fields[1]);
        }
    }
    else
    {
        //_print("Type of Json value cannot be accessed.");
        return NULL;
    }
}

/**
 * @brief Returns a string representation of a Json value type.
 *
 * @param type Obtained from json->type
 * @return NULL | const char* (read-only memory, do not free)
 */
const char *json_type_to_str(JsonValueType type)
{
    if (type > JSON_VAL_NULL)
    {
        return NULL;
    }
    return _JSON_VAL_TO_STR[type];
}

char *_json_escape_string(Allocator *a, const char *value)
{
    u64 len = _json_str_len(value);
    u64 capacity = len + 1;
    char *escaped = a->alloc(a, sizeof(char) * capacity);

    if (!escaped)
    {
        //_print("Memory allocation failed in _json_escape_string");
        return NULL;
    }

    u64 index = 0;
    for (u64 i = 0; i < len; ++i)
    {
        char c = value[i];

        if (c != '\0' && string_find_char(_JSON_LIST_SEQ, c) != -1)
        {
            const char *esc_seq = _JSON_SEQ_TO_ESC[(unsigned char)c];
            u64 esc_len = 2;

            if (index + esc_len >= capacity)
            {
                capacity *= 2;
                char *temp = a->realloc(a, escaped, sizeof(char) * capacity);
                if (!temp)
                {
                    //_print("Memory reallocation failed in _json_escape_string");
                    a->free(a, escaped);
                    escaped = NULL;
                    return NULL;
                }
                escaped = temp;
            }

            escaped[index] = esc_seq[0];
            escaped[index + 1] = esc_seq[1];
            index += esc_len;
        }
        else
        {
            if (index + 1 >= capacity)
            {
                capacity *= 2;
                char *temp = a->realloc(a, escaped, sizeof(char) * capacity);
                if (!temp)
                {
                    //_print("Memory reallocation failed in _json_escape_string");
                    a->free(a, escaped);
                    escaped = NULL;
                    return NULL;
                }
                escaped = temp;
            }
            escaped[index++] = c;
        }
    }

    escaped[index] = '\0';
    return escaped;
}

/**
 * @brief Stringifies a Json struct as well as all its descendants.
 *
 * @param json Json struct (obtained from json_parse)
 * @return NULL | char* (memory owned, you need to free it)
 */
char *json_stringify(Allocator *a, Json *json)
{
    ResultStrBuilder strBldRes = strbuilder_init(a);
    if (strBldRes.error.code)
    {
        // TODO: return ResultOwnedStr
        return NULL;
    }

    StringBuilder builder = strBldRes.value;

    switch (json->type)
    {
    case JSON_VAL_OBJECT:
    {
        strbuilder_push_copy(&builder, "{");

        JsonObject *obj = json->value;

        for (u64 i = 0; i < obj->length; ++i)
        {
            // create dud STRING value object to parse as json string
            Json *field_obj = a->alloc(a, sizeof(Json));
            field_obj->type = JSON_VAL_STRING;
            field_obj->value = obj->fields[i];

            char *string_field = json_stringify(a, field_obj);
            if (string_field == NULL)
            {
                //_print("Failed to stringify field in object entry.");

                strbuilder_deinit(&builder);
                return NULL;
            }

            a->free(a, field_obj);
            field_obj = NULL;

            strbuilder_push_owned(&builder, string_field);
            strbuilder_push_copy(&builder, ":");

            char *string_val = json_stringify(a, obj->values[i]);
            if (string_val == NULL)
            {
                //_print("Failed to stringify value in object entry.");

                strbuilder_deinit(&builder);
                return NULL;
            }

            strbuilder_push_owned(&builder, string_val);

            if (i != obj->length - 1)
            {
                strbuilder_push_copy(&builder, ",");
            }
        }
        strbuilder_push_copy(&builder, "}");

        ResultOwnedStr builtRes = strbuilder_get_string(&builder);
        if (builtRes.error.code)
        {
            strbuilder_deinit(&builder);
            return NULL;
        }

        char *result = builtRes.value;

        strbuilder_deinit(&builder);
        return result;
    }
    case JSON_VAL_ARRAY:
    {
        strbuilder_push_copy(&builder, "[");

        JsonArray *arr = json->value;

        for (u64 i = 0; i < arr->length; ++i)
        {
            char *string_val = json_stringify(a, arr->elements[i]);
            if (string_val == NULL)
            {
                //_print("Failed to stringify value in array.");

                strbuilder_deinit(&builder);
                return NULL;
            }

            strbuilder_push_owned(&builder, string_val);

            if (i != arr->length - 1)
            {
                strbuilder_push_copy(&builder, ",");
            }
        }
        strbuilder_push_copy(&builder, "]");

        ResultOwnedStr builtRes = strbuilder_get_string(&builder);
        if (builtRes.error.code)
        {
            strbuilder_deinit(&builder);
            return NULL;
        }

        char *result = builtRes.value;

        strbuilder_deinit(&builder);
        return result;
    }
    case JSON_VAL_BOOL:
    {
        int *boolean = json->value;
        strbuilder_push_copy(&builder, (*boolean) ? "true" : "false");

        ResultOwnedStr builtRes = strbuilder_get_string(&builder);
        if (builtRes.error.code)
        {
            strbuilder_deinit(&builder);
            return NULL;
        }

        char *result = builtRes.value;

        strbuilder_deinit(&builder);
        return result;
    }
    case JSON_VAL_NUMBER:
    {
        // TODO: replace all stdlib calls

        JsonNumber *number = json->value;
        JsonNumber num_val = *number;

        f64 diff = math_f64_abs(math_f64_round(num_val) - num_val);

        if (diff < 0.00001)
        {
            ResultOwnedStr parseRes = string_from_int(a, (i64)num_val);
            if (parseRes.error.code)
            {
                // TODO: return error
                return NULL;
            }
            strbuilder_push_owned(&builder, parseRes.value);
        }
        else
        {
            ResultOwnedStr parseRes = string_from_float(a, num_val, 12);
            if (parseRes.error.code)
            {
                // TODO: return error
                return NULL;
            }
            strbuilder_push_owned(&builder, parseRes.value);
        }

        ResultOwnedStr builtRes = strbuilder_get_string(&builder);
        if (builtRes.error.code)
        {
            strbuilder_deinit(&builder);
            return NULL;
        }
        char *result = builtRes.value;

        strbuilder_deinit(&builder);
        return result;
    }
    case JSON_VAL_STRING:
    {
        strbuilder_push_copy(&builder, "\"");

        char *escaped_string = _json_escape_string(a, json->value);
        if (escaped_string == NULL)
        {
            //_print("Failed to escape string.");
            return NULL;
        }
        strbuilder_push_owned(&builder, escaped_string);
        strbuilder_push_copy(&builder, "\"");

        ResultOwnedStr builtRes = strbuilder_get_string(&builder);
        if (builtRes.error.code)
        {
            strbuilder_deinit(&builder);
            return NULL;
        }

        char *result = builtRes.value;

        strbuilder_deinit(&builder);
        return result;
    }
    case JSON_VAL_NULL:
    {
        strbuilder_push_copy(&builder, "null");

        ResultOwnedStr builtRes = strbuilder_get_string(&builder);
        if (builtRes.error.code)
        {
            strbuilder_deinit(&builder);
            return NULL;
        }

        char *result = builtRes.value;

        strbuilder_deinit(&builder);
        return result;
    }
    default:
        return NULL;
    }
    return NULL;
}

/**
 * @brief Prints the Json struct + a newline characters. If you do not want the
 * newline character, use json_stringify
 *
 * @param json Json struct (obtained from json_parse)
 */
void json_print(Allocator *a, Json *json)
{
    char *s = json_stringify(a, json);

    if (s == NULL)
    {
        io_println("<error>");
        return;
    }

    io_println(s);
    a->free(a, s);
}

/**
 * @brief Frees the Json struct as well as all its descendants.
 * @param json Json struct (obtained from json_parse)
 */
void json_free(Allocator *a, Json *json)
{
    switch (json->type)
    {
    case JSON_VAL_OBJECT:
    {
        JsonObject *obj = json->value;

        for (u64 i = 0; i < obj->length; ++i)
        {
            a->free(a, obj->fields[i]);
            obj->fields[i] = NULL;
            json_free(a, obj->values[i]);
            obj->values[i] = NULL;
        }
        a->free(a, obj->fields);
        obj->fields = NULL;
        a->free(a, obj->values);
        obj->values = NULL;
        a->free(a, obj);
        obj = NULL;
        break;
    }
    case JSON_VAL_ARRAY:
    {
        JsonArray *arr = json->value;

        for (u64 i = 0; i < arr->length; ++i)
        {
            json_free(a, arr->elements[i]);
            arr->elements[i] = NULL;
        }
        a->free(a, arr->elements);
        arr->elements = NULL;
        a->free(a, arr);
        arr = NULL;
        break;
    }
    case JSON_VAL_BOOL:
    case JSON_VAL_NUMBER:
    case JSON_VAL_STRING:
    {
        a->free(a, json->value);
        json->value = NULL;
        break;
    }
    case JSON_VAL_NULL:
    {
        break;
    }
    default:
        break;
    }

    a->free(a, json);
    return;
}

/**
 * @brief Get the char* value out of a Json struct of type JSON_VAL_STRING
 *
 * @param json Json struct (obtained from json_parse)
 * @return NULL | char* (memory not owned, do not free)
 */
char *json_value_string(Json *json)
{
    if (json->type != JSON_VAL_STRING)
    {
        //_print("Tried to get string value out of Json struct not of type JSON_VAL_STRING");
        return NULL;
    }

    return (char *)json->value;
}

/**
 * @brief Get a pointer to bool(int) value out of a Json struct of type JSON_VAL_BOOL
 *
 * @param json Json struct (obtained from json_parse)
 * @return NULL | int* (memory not owned, do not free)
 */
int *json_value_bool(Json *json)
{
    if (json->type != JSON_VAL_BOOL)
    {
        //_print("Tried to get bool value out of Json struct not of type JSON_VAL_BOOL");
        return NULL;
    }

    return (int *)json->value;
}

/**
 * @brief Get a pointer to double value out of a Json struct of type JSON_VAL_NUMBER
 *
 * @param json Json struct (obtained from json_parse)
 * @return NULL | double* (memory not owned, do not free)
 */
double *json_value_number(Json *json)
{
    if (json->type != JSON_VAL_NUMBER)
    {
        //_print("Tried to get number value out of Json struct not of type JSON_VAL_NUMBER");
        return NULL;
    }

    return (double *)json->value;
}

/**
 * @brief No real need for this one, just use NULL instead lol
 * @param json Json struct (obtained from json_parse)
 * @return NULL
 */
void *json_value_null(Json *json)
{
    return NULL;
}

/**
 * @brief Check if Json struct is of type JSON_VAL_NULL
 *
 * @param json Json struct (obtained from json_parse)
 * @return bool(int)
 */
int json_is_null(Json *json)
{
    return json->type == JSON_VAL_NULL;
}

/**
 * @brief Get a pointer to the JsonObject value out of a Json struct of type JSON_VAL_OBJECT
 *
 * @param json Json struct (obtained from json_parse)
 * @return NULL | JsonObject* (memory not owned, do not free)
 */
JsonObject *json_value_object(Json *json)
{
    if (json->type != JSON_VAL_OBJECT)
    {
        //_print("Tried to get object value out of Json struct not of type JSON_VAL_OBJECT");
        return NULL;
    }

    return (JsonObject *)json->value;
}

/**
 * @brief Get a pointer to the JsonArray value out of a Json struct of type JSON_VAL_ARRAY
 *
 * @param json Json struct (obtained from json_parse)
 * @return NULL | JsonArray* (memory not owned, do not free)
 */
JsonArray *json_value_array(Json *json)
{
    if (json->type != JSON_VAL_ARRAY)
    {
        //_print("Tried to get array value out of Json struct not of type JSON_VAL_ARRAY");
        return NULL;
    }

    return (JsonArray *)json->value;
}

/**
 * @brief Creates a Json struct of type JSON_VAL_STRING. Creates a copy of the string.
 *
 * @param s
 * @return NULL | Json* (memory is owned, use json_free to release if standalone)
 */
Json *json_make_string(Allocator* alloc, ConstStr s)
{
    if (!s)
        return NULL;

    Json *j = alloc->alloc(alloc, sizeof(Json));
    j->type = JSON_VAL_STRING;

    char *buff;
    Error err = _json_str_copy_alloc(alloc, s, &buff);

    if (err.code != ERR_OK)
        return NULL;

    j->value = buff;
    return j;
}

/**
 * @brief Creates a Json struct of type JSON_VAL_NUMBER.
 *
 * @param d
 * @return NULL | Json* (memory is owned, use json_free to release if standalone)
 */
Json *json_make_number(Allocator* alloc, f64 num)
{
    f64 neg_nan = (float)(((float)(1e+300 * 1e+300)) * 0.0F);
    f64 pos_nan = (-(float)(((float)(1e+300 * 1e+300)) * 0.0F));

    if (num == neg_nan || num == pos_nan)
        return NULL;

    Json *j = alloc->alloc(alloc, sizeof(Json));
    if (!j)
        return NULL;

    f64 *num_ptr = alloc->alloc(alloc, sizeof(f64));
    if (!num_ptr)
        return NULL;

    (*num_ptr) = num;

    j->type = JSON_VAL_NUMBER;
    j->value = num_ptr;
    return j; // TODO return ResultT
}

/**
 * @brief Creates a Json struct of type JSON_VAL_BOOL.
 *
 * @param b bool (0|1). if != 0, will be considered true.
 * @return NULL | Json* (memory is owned, use json_free to release if standalone)
 */
Json *json_make_bool(Allocator* alloc, ibool b)
{
    Json *j = alloc->alloc(alloc, sizeof(Json));
    if (!j)
        return NULL;

    ibool *bool_pt = alloc->alloc(alloc, sizeof(ibool));
    if (!bool_pt)
        return NULL;

    (*bool_pt) = b ? 1 : 0;

    j->type = JSON_VAL_BOOL;
    j->value = bool_pt;
    return j;
}

/**
 * @brief Creates a Json struct of type JSON_VAL_NULL.
 *
 * @return NULL | Json* (memory is owned, use json_free to release if standalone)
 */
Json *json_make_null(Allocator *alloc)
{
    Json *j = alloc->alloc(alloc, sizeof(Json));
    if (!j)
        return NULL;

    j->type = JSON_VAL_NULL;
    j->value = NULL;
    return j;
}

/**
 * @brief Creates a Json struct of type JSON_VAL_OBJECT. Field strings are copied.
 * Make sure to not free the Json structs or let them fall out of scope.
 *
 * @param len
 * @param fields
 * @param values
 * @return NULL | Json* (memory is owned, use json_free to release if standalone)
 */
Json *json_make_object(Allocator* alloc, u64 len, char *fields[len], Json *values[len])
{
    JsonObject *obj = alloc->alloc(alloc, sizeof(JsonObject));
    if (!obj)
        return NULL;

    Error err = _json_init_object(alloc, obj);

    if (err.code != ERR_OK)
        return NULL;

    for (u64 i = 0; i < len; ++i)
    {
        Error err = _append_object_entry(alloc, obj, fields[i], values[i]);

        if (err.code != ERR_OK)
            return NULL;
    }

    Json *j = alloc->alloc(alloc, sizeof(Json));
    if (!j)
        return NULL;

    j->type = JSON_VAL_OBJECT;
    j->value = obj;
    return j;
}

/**
 * @brief Creates a Json struct of type JSON_VAL_ARRAY.
 * Make sure to not free the Json structs or let them fall out of scope.
 *
 * @param len
 * @param values
 * @return NULL | Json* (memory is owned, use json_free to release if standalone)
 */
Json *json_make_array(Allocator* alloc, u64 len, Json *values[len])
{
    JsonArray *arr = alloc->alloc(alloc, sizeof(JsonArray));
    if (!arr)
        return NULL;

    Error err = _init_array(alloc, arr);

    if (err.code != ERR_OK)
        return NULL;

    for (u64 i = 0; i < len; ++i)
    {
        Error err = _append_array_element(alloc, arr, values[i]);

        if (err.code != ERR_OK)
            return NULL;
    }

    Json *j = alloc->alloc(alloc, sizeof(Json));
    if (!j)
        return NULL;

    j->type = JSON_VAL_ARRAY;
    j->value = arr;
    return j;
}

Error json_object_append(Allocator *alloc, Json *json, const char *field, Json *value)
{
    if (!json || !field || !value)
        return X_ERR_EXT("json", "json_object_append",
            ERR_INVALID_PARAMETER, "null arg");

    if (json->type != JSON_VAL_OBJECT)
        return X_ERR_EXT("json", "json_object_append",
            ERR_INVALID_PARAMETER, "passed json is not object");

    return _append_object_entry(alloc, json->value, field, value);
}

Error json_object_delete(Allocator* alloc, Json *json, ConstStr field)
{
    if (!json || !field)
        return X_ERR_EXT("json", "json_object_delete",
            ERR_INVALID_PARAMETER, "null arg");

    if (json->type != JSON_VAL_OBJECT)
        return X_ERR_EXT("json", "json_object_delete",
            ERR_INVALID_PARAMETER, "passed json not object");

    JsonObject *obj = json->value;

    if (obj->length == 0)
        return X_ERR_EXT("json", "json_object_delete",
            ERR_RANGE_ERROR, "empty object");

    JsonObject *new_obj = alloc->alloc(alloc, sizeof(JsonObject));

    u64 new_length = obj->length - 1;

    if (new_length == 0)
    {
        new_obj->fields = NULL;
        new_obj->values = NULL;
        new_obj->length = 0;
    }
    else
    {
        new_obj->fields = alloc->alloc(alloc, sizeof(char *) * new_length);

        if (!new_obj->fields)
            return X_ERR_EXT("json", "json_object_delete",
                ERR_OUT_OF_MEMORY, "alloc failure");

        new_obj->values = alloc->alloc(alloc, sizeof(Json *) * new_length);

        if (!new_obj->values)
            return X_ERR_EXT("json", "json_object_delete",
                ERR_OUT_OF_MEMORY, "alloc failure");

        new_obj->length = new_length;
    }

    u64 k = 0;
    for (u64 i = 0; i < obj->length; ++i)
    {
        if (_json_str_comp(field, obj->fields[i]))
        {
            continue;
        }
        new_obj->fields[k] = obj->fields[i];
        new_obj->values[k] = obj->values[i];
        ++k;
    }

    alloc->free(alloc, obj->fields);
    obj->fields = NULL;
    alloc->free(alloc, obj->values);
    obj->values = NULL;
    alloc->free(alloc, obj);

    json->value = new_obj;
    return X_ERR_OK;
}

Error json_array_append(Allocator* alloc, Json *json, Json *value)
{
    if (!json || !value)
        return X_ERR_EXT("json", "json_array_append",
            ERR_INVALID_PARAMETER, "null arg");

    if (json->type != JSON_VAL_ARRAY)
        return X_ERR_EXT("json", "json_array_append",
                    ERR_INVALID_PARAMETER, "passed json is not array");

    return _append_array_element(alloc, json->value, value);
}

Error json_array_delete(Allocator* alloc, Json *json, u64 index)
{
    if (!json)
        return X_ERR_EXT("json", "json_array_delete",
                        ERR_INVALID_PARAMETER, "null arg");

    if (json->type != JSON_VAL_ARRAY)
            return X_ERR_EXT("json", "json_array_delete",
                        ERR_INVALID_PARAMETER, "passed json is not array");

    JsonArray *arr = json->value;

    if (arr->length == 0)
        return X_ERR_EXT("json", "json_array_delete",
                ERR_RANGE_ERROR, "empty array");

    JsonArray *new_arr = alloc->alloc(alloc, sizeof(JsonArray));
    if (!new_arr)
        return X_ERR_EXT("json", "json_array_delete",
            ERR_OUT_OF_MEMORY, "alloc failure");

    u64 new_length = arr->length - 1;

    if (new_length == 0)
    {
        new_arr->elements = NULL;
        new_arr->length = 0;
    }
    else
    {
        new_arr->elements = alloc->alloc(alloc, sizeof(Json *) * new_length);

        if (!new_arr->elements)
            return X_ERR_EXT("json", "json_array_delete",
                ERR_OUT_OF_MEMORY, "alloc failure");

        new_arr->length = new_length;
    }

    u64 k = 0;
    for (u64 i = 0; i < arr->length; ++i)
    {
        if (i == index)
            continue;

        new_arr->elements[k] = arr->elements[i];
        ++k;
    }

    alloc->free(alloc, arr->elements);
    arr->elements = NULL;
    alloc->free(alloc, arr);

    json->value = new_arr;
    return X_ERR_OK;
}
