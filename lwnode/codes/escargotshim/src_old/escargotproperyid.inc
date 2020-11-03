/*
 * Copyright (c) 2019-present Samsung Electronics Co., Ltd
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
 *  USA
 */

#ifndef DEF
#define DEF(x)
#endif

#ifndef DEFSYMBOL
#define DEFSYMBOL(x)
#endif

#ifndef DEF_IS_TYPE
#define DEF_IS_TYPE(x) DEF(x)
#endif

DEF(apply)
DEF(concat)
DEF(push)
DEF(console)
DEF(construct)
DEF(constructor)
DEF(defineProperty)
DEF(deleteProperty)
DEF(preventExtensions)
DEF(setPrototypeOf)
DEF(enumerate)
DEF(get)
DEF(has)
DEF(set)
DEF(ownKeys)
DEF(gc)
DEF(getOwnPropertyDescriptor)
DEF(getPrototypeOf)
DEF(hasOwnProperty)
DEF(isExtensible)
DEF(name)
DEF(length)
DEF(log)
DEF(writable)
DEF(configurable)
DEF(enumerable)
DEF(value)
DEF(function)
DEF(script)
DEF(source)
DEF(filename)
DEF(stack)
DEF(cloneObject)
DEF(getPropertyNames)
DEF(getEnumerableNamedProperties)
DEF(getEnumerableIndexedProperties)
DEF(createEnumerationIterator)
DEF(createPropertyDescriptor)
DEF(createPropertyDescriptorsEnumerationIterator)
DEF(getNamedOwnKeys)
DEF(getIndexedOwnKeys)
DEF(getStackTrace)
DEF(getSymbolKeyFor)
DEF(getSymbolFor)
DEF(ensureDebug)
DEF(enqueueMicrotask)
DEF(dequeueMicrotask)
DEF(saveInHandleScope)
DEF(getPropertyAttributes)
DEF(getOwnPropertyNames)
DEF(getFunctionName)
DEF(getFileName)
DEF(getColumnNumber)
DEF(getLineNumber)
DEF(message)
DEF(prototype)
DEF(toString)
DEF(valueOf)
DEF(jsonParse)
DEF(jsonStringify)

DEF(arguments)
DEF(breakpointId)
DEF(className)
DEF(column)
DEF(columnNumber)
DEF(debuggerOnlyProperties)
DEF(description)
DEF(display)
DEF(exception)
DEF(fileName)
DEF(functionCallsReturn)
DEF(functionHandle)
DEF(functionLocation)
DEF(functionName)
DEF(globals)
DEF(handle)
DEF(index)
DEF(line)
DEF(lineCount)
DEF(lineNumber)
DEF(locals)
DEF(location)
DEF(object)
DEF(objectId)
DEF(properties)
DEF(propertyAttributes)
DEF(returnValue)
DEF(scopeChain)
DEF(scopes)
DEF(scriptId)
DEF(scriptType)
DEF(subtype)
DEF(thisObject)
DEF(type)
DEF(uncaught)
DEF(url)

// DEFSYMBOL(self)
DEFSYMBOL(__external__)
DEFSYMBOL(__hiddenvalues__)
// DEFSYMBOL(__isexternal__)
DEFSYMBOL(__keepalive__)

DEF_IS_TYPE(isMapIterator)
DEF_IS_TYPE(isSetIterator)
DEF_IS_TYPE(isBooleanObject)
DEF_IS_TYPE(isDate)
DEF_IS_TYPE(isMap)
DEF_IS_TYPE(isNativeError)
DEF_IS_TYPE(isPromise)
DEF_IS_TYPE(isProxy)
DEF_IS_TYPE(isRegExp)
DEF_IS_TYPE(isAsyncFunction)
DEF_IS_TYPE(isSet)
DEF_IS_TYPE(isStringObject)
DEF_IS_TYPE(isNumberObject)
DEF_IS_TYPE(isArgumentsObject)
DEF_IS_TYPE(isGeneratorObject)
DEF_IS_TYPE(isWeakMap)
DEF_IS_TYPE(isWeakSet)
DEF_IS_TYPE(isSymbolObject)
DEF_IS_TYPE(isName)
DEF_IS_TYPE(isDataView)
DEF_IS_TYPE(isFunction)


#undef DEF
#undef DEFSYMBOL
#undef DEF_IS_TYPE
