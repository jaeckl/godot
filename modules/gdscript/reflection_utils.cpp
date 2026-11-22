/**************************************************************************/
/*  reflection_utils.cpp                                                  */
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

#include "reflection_utils.h"

#include "core/object/class_db.h"

void ScriptInfo::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_gdscript"), &ScriptInfo::get_gdscript);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "gdscript", PROPERTY_HINT_RESOURCE_TYPE, "GDScript"), "", "get_gdscript");
}

void VariableInfo::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_gdscript"), &VariableInfo::get_gdscript);
	ClassDB::bind_method(D_METHOD("get_name"), &VariableInfo::get_name);
	ClassDB::bind_method(D_METHOD("get_type"), &VariableInfo::get_type);
	ClassDB::bind_method(D_METHOD("get_type_class"), &VariableInfo::get_type_class);
	ClassDB::bind_method(D_METHOD("is_exported"), &VariableInfo::is_exported);
	ClassDB::bind_method(D_METHOD("get_is_static"), &VariableInfo::get_is_static);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "gdscript", PROPERTY_HINT_RESOURCE_TYPE, "GDScript"), "", "get_gdscript");
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "name"), "", "get_name");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "type"), "", "get_type");
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "type_class"), "", "get_type_class");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "exported"), "", "is_exported");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "is_static"), "", "get_is_static");
}

void FunctionInfo::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_gdscript"), &FunctionInfo::get_gdscript);
	ClassDB::bind_method(D_METHOD("get_name"), &FunctionInfo::get_name);
	ClassDB::bind_method(D_METHOD("get_arguments"), &FunctionInfo::get_arguments);
	ClassDB::bind_method(D_METHOD("get_argument_count"), &FunctionInfo::get_argument_count);
	ClassDB::bind_method(D_METHOD("get_is_static"), &FunctionInfo::get_is_static);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "gdscript", PROPERTY_HINT_RESOURCE_TYPE, "GDScript"), "", "get_gdscript");
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "name"), "", "get_name");
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_STRING_ARRAY, "arguments"), "", "get_arguments");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "argument_count"), "", "get_argument_count");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "is_static"), "", "get_is_static");
}

void ParameterInfo::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_gdscript"), &ParameterInfo::get_gdscript);
	ClassDB::bind_method(D_METHOD("get_function"), &ParameterInfo::get_function);
	ClassDB::bind_method(D_METHOD("get_name"), &ParameterInfo::get_name);
	ClassDB::bind_method(D_METHOD("get_type"), &ParameterInfo::get_type);
	ClassDB::bind_method(D_METHOD("get_type_class"), &ParameterInfo::get_type_class);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "gdscript", PROPERTY_HINT_RESOURCE_TYPE, "GDScript"), "", "get_gdscript");
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "function"), "", "get_function");
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "name"), "", "get_name");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "type"), "", "get_type");
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "type_class"), "", "get_type_class");
}

void ReflectionUtils::_bind_methods() {
	ClassDB::bind_static_method(get_class_static(), D_METHOD("get_function_info", "script", "name"), &ReflectionUtils::get_function_info);
	ClassDB::bind_static_method(get_class_static(), D_METHOD("get_function_list", "script"), &ReflectionUtils::get_function_list);
	ClassDB::bind_static_method(get_class_static(), D_METHOD("get_parameter_info", "script", "function", "parameter"), &ReflectionUtils::get_parameter_info);
	ClassDB::bind_static_method(get_class_static(), D_METHOD("get_parameter_list", "script", "function"), &ReflectionUtils::get_parameter_list);
	ClassDB::bind_static_method(get_class_static(), D_METHOD("get_script_info", "script"), &ReflectionUtils::get_script_info);
	ClassDB::bind_static_method(get_class_static(), D_METHOD("get_variable_info", "script", "name"), &ReflectionUtils::get_variable_info);
	ClassDB::bind_static_method(get_class_static(), D_METHOD("get_variable_list", "script"), &ReflectionUtils::get_variable_list);
}

static bool _is_script_category(const PropertyInfo &p_info) {
	return (p_info.usage & PROPERTY_USAGE_CATEGORY) != 0;
}

Ref<VariableInfo> ReflectionUtils::_make_variable(const Ref<GDScript> &p_script, const PropertyInfo &p_info, bool p_static) {
	Ref<VariableInfo> info;
	info.instantiate();
	info->script = p_script;
	info->name = p_info.name;
	info->type = p_info.type;
	info->type_class = p_info.class_name;
	info->exported = (p_info.usage & (PROPERTY_USAGE_STORAGE | PROPERTY_USAGE_EDITOR)) != 0;
	info->is_static = p_static;
	return info;
}

Ref<FunctionInfo> ReflectionUtils::_make_function(const Ref<GDScript> &p_script, const MethodInfo &p_info) {
	Ref<FunctionInfo> info;
	info.instantiate();
	info->script = p_script;
	info->name = p_info.name;
	info->is_static = (p_info.flags & METHOD_FLAG_STATIC) != 0;
	for (const PropertyInfo &argument : p_info.arguments) {
		info->arguments.push_back(argument.name);
	}
	return info;
}

Ref<ParameterInfo> ReflectionUtils::_make_parameter(const Ref<GDScript> &p_script, const StringName &p_function, const PropertyInfo &p_info) {
	Ref<ParameterInfo> info;
	info.instantiate();
	info->script = p_script;
	info->function = p_function;
	info->name = p_info.name;
	info->type = p_info.type;
	info->type_class = p_info.class_name;
	return info;
}

bool ReflectionUtils::_find_method(const Ref<GDScript> &p_script, const StringName &p_name, Ref<GDScript> &r_owner, MethodInfo &r_info) {
	Ref<GDScript> current = p_script;
	while (current.is_valid()) {
		MethodInfo info = current->get_method_info(p_name);
		if (info.name == String(p_name)) {
			r_owner = current;
			r_info = info;
			return true;
		}
		current = current->get_base();
	}
	return false;
}

Ref<ScriptInfo> ReflectionUtils::get_script_info(const Ref<GDScript> &p_script) {
	ERR_FAIL_COND_V_MSG(p_script.is_null(), Ref<ScriptInfo>(), "Reflection script must be a GDScript.");
	Ref<ScriptInfo> info;
	info.instantiate();
	info->script = p_script;
	return info;
}

Ref<VariableInfo> ReflectionUtils::get_variable_info(const Ref<GDScript> &p_script, const StringName &p_name) {
	ERR_FAIL_COND_V_MSG(p_script.is_null(), Ref<VariableInfo>(), "Reflection script must be a GDScript.");
	Ref<GDScript> current = p_script;
	while (current.is_valid()) {
		List<PropertyInfo> properties;
		current->_get_script_property_list(&properties, false);
		for (const PropertyInfo &property : properties) {
			if (_is_script_category(property)) {
				continue;
			}
			if (property.name == String(p_name)) {
				return _make_variable(current, property, false);
			}
		}
		if (current->static_variables_indices.has(p_name)) {
			return _make_variable(current, current->static_variables_indices[p_name].property_info, true);
		}
		current = current->get_base();
	}
	return Ref<VariableInfo>();
}

Ref<FunctionInfo> ReflectionUtils::get_function_info(const Ref<GDScript> &p_script, const StringName &p_name) {
	ERR_FAIL_COND_V_MSG(p_script.is_null(), Ref<FunctionInfo>(), "Reflection script must be a GDScript.");
	Ref<GDScript> owner;
	MethodInfo info;
	if (!_find_method(p_script, p_name, owner, info)) {
		return Ref<FunctionInfo>();
	}
	return _make_function(owner, info);
}

Ref<ParameterInfo> ReflectionUtils::get_parameter_info(const Ref<GDScript> &p_script, const StringName &p_function, const StringName &p_parameter) {
	ERR_FAIL_COND_V_MSG(p_script.is_null(), Ref<ParameterInfo>(), "Reflection script must be a GDScript.");
	Ref<GDScript> owner;
	MethodInfo info;
	if (!_find_method(p_script, p_function, owner, info)) {
		return Ref<ParameterInfo>();
	}
	for (const PropertyInfo &argument : info.arguments) {
		if (argument.name == String(p_parameter)) {
			return _make_parameter(owner, p_function, argument);
		}
	}
	return Ref<ParameterInfo>();
}

Array ReflectionUtils::get_variable_list(const Ref<GDScript> &p_script) {
	ERR_FAIL_COND_V_MSG(p_script.is_null(), Array(), "Reflection script must be a GDScript.");
	Array found;
	Ref<GDScript> current = p_script;
	while (current.is_valid()) {
		List<PropertyInfo> properties;
		current->_get_script_property_list(&properties, false);
		for (const PropertyInfo &property : properties) {
			if (_is_script_category(property)) {
				continue;
			}
			found.push_back(_make_variable(current, property, false));
		}
		for (const KeyValue<StringName, GDScript::MemberInfo> &entry : current->static_variables_indices) {
			found.push_back(_make_variable(current, entry.value.property_info, true));
		}
		current = current->get_base();
	}
	return found;
}

Array ReflectionUtils::get_function_list(const Ref<GDScript> &p_script) {
	ERR_FAIL_COND_V_MSG(p_script.is_null(), Array(), "Reflection script must be a GDScript.");
	Array found;
	Ref<GDScript> current = p_script;
	while (current.is_valid()) {
		List<MethodInfo> methods;
		current->_get_script_method_list(&methods, false);
		for (const MethodInfo &method : methods) {
			found.push_back(_make_function(current, method));
		}
		current = current->get_base();
	}
	return found;
}

Array ReflectionUtils::get_parameter_list(const Ref<GDScript> &p_script, const StringName &p_function) {
	ERR_FAIL_COND_V_MSG(p_script.is_null(), Array(), "Reflection script must be a GDScript.");
	Ref<GDScript> owner;
	MethodInfo info;
	if (!_find_method(p_script, p_function, owner, info)) {
		return Array();
	}
	Array found;
	for (const PropertyInfo &argument : info.arguments) {
		found.push_back(_make_parameter(owner, p_function, argument));
	}
	return found;
}
