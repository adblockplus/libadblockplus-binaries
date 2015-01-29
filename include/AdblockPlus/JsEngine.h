/*
 * This file is part of Adblock Plus <https://adblockplus.org/>,
 * Copyright (C) 2006-2015 Eyeo GmbH
 *
 * Adblock Plus is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * Adblock Plus is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Adblock Plus.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ADBLOCK_PLUS_JS_ENGINE_H
#define ADBLOCK_PLUS_JS_ENGINE_H

#include <map>
#include <stdexcept>
#include <stdint.h>
#include <string>
#include <AdblockPlus/AppInfo.h>
#include <AdblockPlus/tr1_functional.h>
#include <AdblockPlus/LogSystem.h>
#include <AdblockPlus/FileSystem.h>
#include <AdblockPlus/JsValue.h>
#include <AdblockPlus/WebRequest.h>

#include "tr1_memory.h"
#include "V8ValueHolder.h"

namespace v8
{
  class Arguments;
  class Isolate;
  class Value;
  class Context;
  template<class T> class Handle;
  typedef Handle<Value>(*InvocationCallback)(const Arguments &args);
}

namespace AdblockPlus
{
  class JsEngine;

  /**
   * Shared smart pointer to a `JsEngine` instance.
   */
  typedef std::tr1::shared_ptr<JsEngine> JsEnginePtr;

  /**
   * JavaScript engine used by `FilterEngine`, wraps v8.
   */
  class JsEngine : public std::tr1::enable_shared_from_this<JsEngine>
  {
    friend class JsValue;
    friend class JsContext;

  public:
    /**
     * Event callback function.
     */
    typedef std::tr1::function<void(JsValueList& params)> EventCallback;

    /**
     * Maps events to callback functions.
     */
    typedef std::map<std::string, EventCallback> EventMap;

    /**
     * Creates a new JavaScript engine instance.
     * @param appInfo Information about the app.
     * @return New `JsEngine` instance.
     */
    static JsEnginePtr New(const AppInfo& appInfo = AppInfo());

    /**
     * Registers the callback function for an event.
     * @param eventName Event name. Note that this can be any string - it's a
     *        general purpose event handling mechanism.
     * @param callback Event callback function.
     */
    void SetEventCallback(const std::string& eventName, EventCallback callback);

    /**
     * Removes the callback function for an event.
     * @param eventName Event name.
     */
    void RemoveEventCallback(const std::string& eventName);

    /**
     * Triggers an event.
     * @param eventName Event name.
     * @param params Event parameters.
     */
    void TriggerEvent(const std::string& eventName, JsValueList& params);

    /**
     * Evaluates a JavaScript expression.
     * @param source JavaScript expression to evaluate.
     * @param filename Optional file name for the expression, used in error
     *        messages.
     * @return Result of the evaluated expression.
     */
    JsValuePtr Evaluate(const std::string& source,
        const std::string& filename = "");

    /**
     * Initiates a garbage collection.
     */
    void Gc();

    //@{
    /**
     * Creates a new JavaScript value.
     * @param val Value to convert.
     * @return New `JsValue` instance.
     */
    JsValuePtr NewValue(const std::string& val);
    JsValuePtr NewValue(int64_t val);
    JsValuePtr NewValue(bool val);
    inline JsValuePtr NewValue(const char* val)
    {
      return NewValue(std::string(val));
    }
    inline JsValuePtr NewValue(int val)
    {
      return NewValue(static_cast<int64_t>(val));
    }
#ifdef __APPLE__
    inline JsValuePtr NewValue(long val)
    {
      return NewValue(static_cast<int64_t>(val));
    }
#endif
    //@}

    /**
     * Creates a new JavaScript object.
     * @return New `JsValue` instance.
     */
    JsValuePtr NewObject();

    /**
     * Creates a JavaScript function that invokes a C++ callback.
     * @param callback C++ callback to invoke. The callback receives a
     *        `v8::Arguments` object and can use `FromArguments()` to retrieve
     *        the current `JsEngine`.
     * @return New `JsValue` instance.
     */
    JsValuePtr NewCallback(v8::InvocationCallback callback);

    /**
     * Returns a `JsEngine` instance contained in a `v8::Arguments` object.
     * Use this in callbacks created via `NewCallback()` to retrieve the current
     * `JsEngine`.
     * @param arguments `v8::Arguments` object containing the `JsEngine`
     *        instance.
     * @return `JsEngine` instance from `v8::Arguments`.
     */
    static JsEnginePtr FromArguments(const v8::Arguments& arguments);

    /**
     * Converts v8 arguments to `JsValue` objects.
     * @param arguments `v8::Arguments` object containing the arguments to
     *        convert.
     * @return List of arguments converted to `JsValue` objects.
     */
    JsValueList ConvertArguments(const v8::Arguments& arguments);

    /**
     * @see `SetFileSystem()`.
     */
    FileSystemPtr GetFileSystem();

    /**
     * Sets the `FileSystem` implementation used for all file I/O.
     * Setting this is optional, the engine will use a `DefaultFileSystem`
     * instance by default, which might be sufficient.
     * @param The `FileSystem` instance to use.
     */
    void SetFileSystem(FileSystemPtr val);

    /**
     * @see `SetWebRequest()`.
     */
    WebRequestPtr GetWebRequest();

    /**
     * Sets the `WebRequest` implementation used for XMLHttpRequests.
     * Setting this is optional, the engine will use a `DefaultWebRequest`
     * instance by default, which might be sufficient.
     * @param The `WebRequest` instance to use.
     */
    void SetWebRequest(WebRequestPtr val);

    /**
     * @see `SetLogSystem()`.
     */
    LogSystemPtr GetLogSystem();

    /**
     * Sets the `LogSystem` implementation used for logging (e.g. to handle
     * `console.log()` calls from JavaScript).
     * Setting this is optional, the engine will use a `DefaultLogSystem`
     * instance by default, which might be sufficient.
     * @param The `LogSystem` instance to use.
     */
    void SetLogSystem(LogSystemPtr val);

  private:
    JsEngine();

    FileSystemPtr fileSystem;
    WebRequestPtr webRequest;
    LogSystemPtr logSystem;
    v8::Isolate* isolate;
    V8ValueHolder<v8::Context> context;
    EventMap eventCallbacks;
  };
}

#endif
