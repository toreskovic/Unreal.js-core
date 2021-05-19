#include "JavascriptCompatibilityHelpers.h"
#include <V8\Private\Helpers.h>

v8::Local<v8::Value> FJavascriptCompatibilityHelpers::Undefined(v8::Isolate* isolate)
{
	// Hack for android since we're getting a random number from v8::Undefined there
#if PLATFORM_ANDROID
	auto context = isolate->GetCurrentContext();
	FIsolateHelper I(isolate);

	auto sourceUndefined = TEXT("undefined");
	auto scriptUndefined = v8::Script::Compile(context, I.String(sourceUndefined)).ToLocalChecked();
	auto resultUndefined = scriptUndefined->Run(context);

	return resultUndefined.ToLocalChecked();
#else
	return v8::Undefined(isolate);
#endif
}

v8::Local<v8::Value> FJavascriptCompatibilityHelpers::Null(v8::Isolate* isolate)
{
	// Hack for android since we're getting a random number from v8::Null there
#if PLATFORM_ANDROID
	auto context = isolate->GetCurrentContext();
	FIsolateHelper I(isolate);

	auto sourceUndefined = TEXT("null");
	auto scriptUndefined = v8::Script::Compile(context, I.String(sourceUndefined)).ToLocalChecked();
	auto resultUndefined = scriptUndefined->Run(context);

	return resultUndefined.ToLocalChecked();
#else
	return v8::Null(isolate);
#endif
}
