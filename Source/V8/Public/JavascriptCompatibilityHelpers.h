#pragma once

#include "V8PCH.h"

class FJavascriptCompatibilityHelpers
{
public:
	static v8::Local<v8::Value> Undefined(v8::Isolate* isolate);

	static v8::Local<v8::Value> Null(v8::Isolate* isolate);
};