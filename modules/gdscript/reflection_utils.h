/**************************************************************************/
/*  reflection_utils.h                                                    */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#pragma once

#include "core/object/object.h"
#include "core/variant/variant.h"
#include "gdscript.h"

class ScriptInfo : public RefCounted {
	GDCLASS(ScriptInfo, RefCounted);
	friend class ReflectionUtils;

	Ref<GDScript> script;

protected:
	static void _bind_methods();

public:
	Ref<GDScript> get_gdscript() const { return script; }
};

class VariableInfo : public RefCounted {
	GDCLASS(VariableInfo, RefCounted);
	friend class ReflectionUtils;

	Ref<GDScript> script;
	StringName name;
	Variant::Type type = Variant::NIL;
	StringName type_class;
	bool exported = false;
	bool is_static = false;

protected:
	static void _bind_methods();

public:
	Ref<GDScript> get_gdscript() const { return script; }
	StringName get_name() const { return name; }
	int get_type() const { return type; }
	StringName get_type_class() const { return type_class; }
	bool is_exported() const { return exported; }
	bool get_is_static() const { return is_static; }
};

class FunctionInfo : public RefCounted {
	GDCLASS(FunctionInfo, RefCounted);
	friend class ReflectionUtils;

	Ref<GDScript> script;
	StringName name;
	PackedStringArray arguments;
	bool is_static = false;

protected:
	static void _bind_methods();

public:
	Ref<GDScript> get_gdscript() const { return script; }
	StringName get_name() const { return name; }
	PackedStringArray get_arguments() const { return arguments; }
	int get_argument_count() const { return arguments.size(); }
	bool get_is_static() const { return is_static; }
};

class ParameterInfo : public RefCounted {
	GDCLASS(ParameterInfo, RefCounted);
	friend class ReflectionUtils;

	Ref<GDScript> script;
	StringName function;
	StringName name;
	Variant::Type type = Variant::NIL;
	StringName type_class;

protected:
	static void _bind_methods();

public:
	Ref<GDScript> get_gdscript() const { return script; }
	StringName get_function() const { return function; }
	StringName get_name() const { return name; }
	int get_type() const { return type; }
	StringName get_type_class() const { return type_class; }
};

// Maps GDScript MethodInfo and PropertyInfo into read-only views. Does not run at script load.
class ReflectionUtils : public Object {
	GDCLASS(ReflectionUtils, Object);

	static Ref<VariableInfo> _make_variable(const Ref<GDScript> &p_script, const PropertyInfo &p_info, bool p_static);
	static Ref<FunctionInfo> _make_function(const Ref<GDScript> &p_script, const MethodInfo &p_info);
	static Ref<ParameterInfo> _make_parameter(const Ref<GDScript> &p_script, const StringName &p_function, const PropertyInfo &p_info);
	static bool _find_method(const Ref<GDScript> &p_script, const StringName &p_name, Ref<GDScript> &r_owner, MethodInfo &r_info);

protected:
	static void _bind_methods();

public:
	static Ref<ScriptInfo> get_script_info(const Ref<GDScript> &p_script);
	static Ref<VariableInfo> get_variable_info(const Ref<GDScript> &p_script, const StringName &p_name);
	static Ref<FunctionInfo> get_function_info(const Ref<GDScript> &p_script, const StringName &p_name);
	static Ref<ParameterInfo> get_parameter_info(const Ref<GDScript> &p_script, const StringName &p_function, const StringName &p_parameter);
	static Array get_variable_list(const Ref<GDScript> &p_script);
	static Array get_function_list(const Ref<GDScript> &p_script);
	static Array get_parameter_list(const Ref<GDScript> &p_script, const StringName &p_function);
};
