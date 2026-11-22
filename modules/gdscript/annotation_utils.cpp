/**************************************************************************/
/*  annotation_utils.cpp                                                  */
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

#include "annotation_utils.h"

#include "core/object/class_db.h"
#include "gdscript.h"
#include "gdscript_cache.h"

struct AnnotationClass {
	Ref<GDScript> script;
	StringName class_name;
};

static Variant _mutable_annotation_container(const Variant &p_value) {
	if (p_value.get_type() == Variant::ARRAY) {
		Array copy = Array(p_value).duplicate(true);
		for (int i = 0; i < copy.size(); i++) {
			copy[i] = _mutable_annotation_container(copy[i]);
		}
		return copy;
	}
	if (p_value.get_type() == Variant::DICTIONARY) {
		Dictionary copy = Dictionary(p_value).duplicate(true);
		Array keys = copy.keys();
		for (int i = 0; i < keys.size(); i++) {
			const Variant key = keys[i];
			copy[key] = _mutable_annotation_container(copy[key]);
		}
		return copy;
	}
	return p_value;
}

static bool _extends_gdscript_annotation(const GDScript *p_script) {
	const StringName native = p_script->get_instance_base_type();
	return native == SNAME("GDScriptAnnotation") || ClassDB::is_parent_class(native, SNAME("GDScriptAnnotation"));
}

static bool _resolve_named_class(const StringName &p_class_name, AnnotationClass &r_out, String &r_error) {
	if (ClassDB::class_exists(p_class_name) && !ScriptServer::is_global_class(p_class_name)) {
		r_error = vformat(R"(Annotation class "%s" must be a GDScript class.)", p_class_name);
		return false;
	}
	if (!ScriptServer::is_global_class(p_class_name)) {
		r_error = vformat(R"(Unknown annotation class "%s".)", p_class_name);
		return false;
	}
	if (ScriptServer::get_global_class_language(p_class_name) != SNAME("GDScript")) {
		r_error = vformat(R"(Annotation class "%s" must be a GDScript class.)", p_class_name);
		return false;
	}

	Error load_error = OK;
	Ref<GDScript> script = GDScriptCache::get_full_script(ScriptServer::get_global_class_path(p_class_name), load_error);
	if (script.is_null() || load_error != OK || !script->is_script_valid()) {
		r_error = vformat(R"(Could not parse annotation class "%s".)", p_class_name);
		return false;
	}
	if (!_extends_gdscript_annotation(script.ptr())) {
		r_error = vformat(R"(Annotation class "%s" must extend "GDScriptAnnotation".)", p_class_name);
		return false;
	}

	r_out.script = script;
	r_out.class_name = p_class_name;
	return true;
}

static bool _resolve_annotation_class(const Variant &p_annotation_class, AnnotationClass &r_out, String &r_error) {
	if (p_annotation_class.get_type() == Variant::OBJECT) {
		GDScript *script = Object::cast_to<GDScript>(p_annotation_class.operator Object *());
		if (script == nullptr) {
			r_error = "Annotation class must be a GDScript or a global class name.";
			return false;
		}
		StringName class_name = script->get_global_name();
		if (class_name == StringName()) {
			r_error = "Annotation class must have a class_name.";
			return false;
		}
		if (!script->is_script_valid()) {
			r_error = vformat(R"(Could not parse annotation class "%s".)", class_name);
			return false;
		}
		if (!_extends_gdscript_annotation(script)) {
			r_error = vformat(R"(Annotation class "%s" must extend "GDScriptAnnotation".)", class_name);
			return false;
		}
		r_out.script = script;
		r_out.class_name = class_name;
		return true;
	}

	if (p_annotation_class.get_type() == Variant::STRING || p_annotation_class.get_type() == Variant::STRING_NAME) {
		return _resolve_named_class(StringName(p_annotation_class), r_out, r_error);
	}

	r_error = "Annotation class must be a GDScript or a global class name.";
	return false;
}

static bool _annotation_class_matches(const GDScript *p_entry_script, const GDScript *p_query) {
	const GDScript *current = p_entry_script;
	while (current != nullptr) {
		if (current == p_query) {
			return true;
		}
		Ref<GDScript> base = current->get_base();
		current = base.ptr();
	}
	return false;
}

static bool _resolve_entry_class(const StringName &p_class_name, HashMap<StringName, AnnotationClass> &r_cache, AnnotationClass &r_out, String &r_error) {
	if (AnnotationClass *cached = r_cache.getptr(p_class_name)) {
		r_out = *cached;
		return true;
	}
	if (!_resolve_named_class(p_class_name, r_out, r_error)) {
		return false;
	}
	r_cache.insert(p_class_name, r_out);
	return true;
}

static Dictionary _context(const StringName &p_target, const StringName &p_member, const StringName &p_parameter) {
	Dictionary context;
	context["target"] = String(p_target);
	context["member"] = String(p_member);
	context["parameter"] = String(p_parameter);
	return context;
}

static bool _call_annotation_callback(const Callable &p_callback, const StringName &p_name, const Dictionary &p_values, const Dictionary &p_context, String &r_error) {
	const Variant name = p_name;
	const Variant values = p_values;
	const Variant context = p_context;
	const Variant *args[3] = { &name, &values, &context };
	Callable::CallError call_error;
	Variant return_value;
	p_callback.callp(args, 3, return_value, call_error);
	if (call_error.error != Callable::CallError::CALL_OK) {
		r_error = Variant::get_call_error_text("for_each_annotation", args, 3, call_error);
		return false;
	}
	return true;
}

static bool _call_initialized_callback(const Callable &p_callback, const Variant &p_object, const Dictionary &p_context, String &r_error) {
	const Variant context = p_context;
	const Variant *args[2] = { &p_object, &context };
	Callable::CallError call_error;
	Variant return_value;
	p_callback.callp(args, 2, return_value, call_error);
	if (call_error.error != Callable::CallError::CALL_OK) {
		r_error = Variant::get_call_error_text("for_each_annotation_initialized", args, 2, call_error);
		return false;
	}
	return true;
}

static bool _each_entries(const Array &p_entries, const StringName &p_target, const StringName &p_member, const StringName &p_parameter, const GDScript *p_query, const Callable *p_callback, Array *r_found, bool p_instantiate, HashMap<StringName, AnnotationClass> &r_cache, String &r_error) {
	for (int i = 0; i < p_entries.size(); i++) {
		Dictionary entry = p_entries[i];
		StringName entry_name = entry["name"];
		AnnotationClass entry_class;
		if (!_resolve_entry_class(entry_name, r_cache, entry_class, r_error)) {
			return false;
		}
		if (!_annotation_class_matches(entry_class.script.ptr(), p_query)) {
			continue;
		}
		Dictionary context = _context(p_target, p_member, p_parameter);
		Variant object;
		if (p_instantiate) {
			Dictionary entry_copy;
			entry_copy["name"] = entry_name;
			entry_copy["values"] = entry["values"];
			object = AnnotationUtils::instantiate_annotation(entry_copy);
			if (object.get_type() != Variant::OBJECT) {
				// instantiate_annotation already reported the failure.
				r_error = String();
				return false;
			}
		}
		if (r_found != nullptr) {
			r_found->push_back(p_instantiate ? object : Variant(context));
		}
		if (p_callback == nullptr) {
			continue;
		}
		if (p_instantiate) {
			if (!_call_initialized_callback(*p_callback, object, context, r_error)) {
				return false;
			}
		} else if (!_call_annotation_callback(*p_callback, entry_name, entry["values"], context, r_error)) {
			return false;
		}
	}
	return true;
}

static bool _as_array(const Variant &p_value, Array &r_array) {
	if (p_value.get_type() != Variant::ARRAY) {
		return false;
	}
	r_array = p_value;
	return true;
}

static bool _walk_script(const GDScript *p_script, const GDScript *p_query, const Callable *p_callback, Array *r_found, bool p_instantiate, HashMap<StringName, AnnotationClass> &r_cache, String &r_error) {
	const Dictionary annotations = p_script->get_annotations();

	if (annotations.has("script")) {
		Array entries;
		if (_as_array(annotations["script"], entries)) {
			if (!_each_entries(entries, "script", StringName(), StringName(), p_query, p_callback, r_found, p_instantiate, r_cache, r_error)) {
				return false;
			}
		}
	}

	if (annotations.has("variables")) {
		Dictionary variables = annotations["variables"];
		Array members = variables.keys();
		for (int i = 0; i < members.size(); i++) {
			StringName member = members[i];
			Array entries;
			if (!_as_array(variables[member], entries)) {
				continue;
			}
			if (!_each_entries(entries, "variable", member, StringName(), p_query, p_callback, r_found, p_instantiate, r_cache, r_error)) {
				return false;
			}
		}
	}

	if (annotations.has("functions")) {
		Dictionary functions = annotations["functions"];
		Array members = functions.keys();
		for (int i = 0; i < members.size(); i++) {
			StringName member = members[i];
			Array entries;
			if (!_as_array(functions[member], entries)) {
				continue;
			}
			if (!_each_entries(entries, "function", member, StringName(), p_query, p_callback, r_found, p_instantiate, r_cache, r_error)) {
				return false;
			}
		}
	}

	if (annotations.has("parameters")) {
		Dictionary parameters = annotations["parameters"];
		Array functions = parameters.keys();
		for (int i = 0; i < functions.size(); i++) {
			StringName function_name = functions[i];
			Dictionary parameter_map = parameters[function_name];
			Array parameter_names = parameter_map.keys();
			for (int j = 0; j < parameter_names.size(); j++) {
				StringName parameter = parameter_names[j];
				Array entries;
				if (!_as_array(parameter_map[parameter], entries)) {
					continue;
				}
				if (!_each_entries(entries, "parameter", function_name, parameter, p_query, p_callback, r_found, p_instantiate, r_cache, r_error)) {
					return false;
				}
			}
		}
	}

	return true;
}

static bool _prepare_callback(const Variant &p_callback, Callable &r_callback, String &r_error) {
	if (p_callback.get_type() != Variant::CALLABLE) {
		r_error = "Annotation callback is not callable.";
		return false;
	}
	r_callback = p_callback;
	if (!r_callback.is_valid()) {
		r_error = "Annotation callback is not callable.";
		return false;
	}
	return true;
}

static bool _entries_for_target(const Dictionary &p_annotations, const StringName &p_member, const StringName &p_parameter, Array &r_entries, String &r_error) {
	if (p_parameter != StringName()) {
		if (p_member == StringName()) {
			r_error = "Annotation parameter lookup requires a member name.";
			return false;
		}
		if (!p_annotations.has("parameters")) {
			return true;
		}
		Dictionary parameters = p_annotations["parameters"];
		if (!parameters.has(p_member)) {
			return true;
		}
		Dictionary parameter_map = parameters[p_member];
		if (!parameter_map.has(p_parameter)) {
			return true;
		}
		_as_array(parameter_map[p_parameter], r_entries);
		return true;
	}

	if (p_member == StringName()) {
		if (p_annotations.has("script")) {
			_as_array(p_annotations["script"], r_entries);
		}
		return true;
	}

	if (p_annotations.has("variables")) {
		Dictionary variables = p_annotations["variables"];
		if (variables.has(p_member)) {
			_as_array(variables[p_member], r_entries);
			return true;
		}
	}
	if (p_annotations.has("functions")) {
		Dictionary functions = p_annotations["functions"];
		if (functions.has(p_member)) {
			_as_array(functions[p_member], r_entries);
		}
	}
	return true;
}

static Variant _first_match(const Array &p_entries, const GDScript *p_query, HashMap<StringName, AnnotationClass> &r_cache, String &r_error) {
	for (int i = 0; i < p_entries.size(); i++) {
		Dictionary entry = p_entries[i];
		StringName entry_name = entry["name"];
		AnnotationClass entry_class;
		if (!_resolve_entry_class(entry_name, r_cache, entry_class, r_error)) {
			return Variant();
		}
		if (!_annotation_class_matches(entry_class.script.ptr(), p_query)) {
			continue;
		}
		Dictionary copy;
		copy["name"] = entry_name;
		copy["values"] = entry["values"];
		return copy;
	}
	r_error = String();
	return Variant();
}

void AnnotationUtils::_bind_methods() {
	ClassDB::bind_static_method(get_class_static(), D_METHOD("instantiate_annotation", "entry"), &AnnotationUtils::instantiate_annotation);
	ClassDB::bind_static_method(get_class_static(), D_METHOD("for_each_annotation", "annotation_class", "script", "callback"), &AnnotationUtils::for_each_annotation);
	ClassDB::bind_static_method(get_class_static(), D_METHOD("for_each_annotation_in_bases", "annotation_class", "script", "callback"), &AnnotationUtils::for_each_annotation_in_bases);
	ClassDB::bind_static_method(get_class_static(), D_METHOD("get_annotation", "script", "annotation_class", "member", "parameter"), &AnnotationUtils::get_annotation, DEFVAL(StringName()), DEFVAL(StringName()));
	ClassDB::bind_static_method(get_class_static(), D_METHOD("has_annotation", "script", "annotation_class", "member", "parameter"), &AnnotationUtils::has_annotation, DEFVAL(StringName()), DEFVAL(StringName()));
	ClassDB::bind_static_method(get_class_static(), D_METHOD("get_annotation_values", "script", "annotation_class", "member", "parameter"), &AnnotationUtils::get_annotation_values, DEFVAL(StringName()), DEFVAL(StringName()));
	ClassDB::bind_static_method(get_class_static(), D_METHOD("find_annotated", "script", "annotation_class"), &AnnotationUtils::find_annotated);
	ClassDB::bind_static_method(get_class_static(), D_METHOD("find_annotated_initialized", "script", "annotation_class"), &AnnotationUtils::find_annotated_initialized);
	ClassDB::bind_static_method(get_class_static(), D_METHOD("for_each_annotation_in_bases_initialized", "annotation_class", "script", "callback"), &AnnotationUtils::for_each_annotation_in_bases_initialized);
	ClassDB::bind_static_method(get_class_static(), D_METHOD("for_each_annotation_initialized", "annotation_class", "script", "callback"), &AnnotationUtils::for_each_annotation_initialized);
	ClassDB::bind_static_method(get_class_static(), D_METHOD("get_annotation_initialized", "script", "annotation_class", "member", "parameter"), &AnnotationUtils::get_annotation_initialized, DEFVAL(StringName()), DEFVAL(StringName()));
}

Variant AnnotationUtils::instantiate_annotation(const Variant &p_entry) {
	if (p_entry.get_type() != Variant::DICTIONARY) {
		ERR_FAIL_V_MSG(Variant(), "Annotation entry must be a Dictionary with \"name\" and \"values\".");
	}
	const Dictionary entry = p_entry;
	if (!entry.has("name") || !entry.has("values")) {
		ERR_FAIL_V_MSG(Variant(), "Annotation entry must be a Dictionary with \"name\" and \"values\".");
	}
	const Variant name_value = entry["name"];
	const Variant values_value = entry["values"];
	if ((name_value.get_type() != Variant::STRING && name_value.get_type() != Variant::STRING_NAME) || values_value.get_type() != Variant::DICTIONARY) {
		ERR_FAIL_V_MSG(Variant(), "Annotation entry must be a Dictionary with \"name\" and \"values\".");
	}

	const StringName class_name = name_value;
	AnnotationClass annotation_class;
	String error;
	if (!_resolve_named_class(class_name, annotation_class, error)) {
		ERR_FAIL_V_MSG(Variant(), error);
	}
	if (annotation_class.script->is_abstract()) {
		ERR_FAIL_V_MSG(Variant(), vformat(R"(Annotation class "%s" is abstract.)", class_name));
	}

	Ref<RefCounted> owner;
	owner.instantiate();
	GDScript *script = annotation_class.script.ptr();
	GDScriptInstance *instance = script->_create_annotation_instance(owner.ptr());
	if (instance == nullptr) {
		ERR_FAIL_V_MSG(Variant(), vformat(R"(Could not parse annotation class "%s".)", class_name));
	}

	const Dictionary values = values_value;
	Array keys = values.keys();
	for (int i = 0; i < keys.size(); i++) {
		const Variant key = keys[i];
		if (key.get_type() != Variant::STRING && key.get_type() != Variant::STRING_NAME) {
			owner.unref();
			ERR_FAIL_V_MSG(Variant(), vformat(R"(Annotation "%s" has no attribute "%s".)", class_name, key));
		}
		const StringName attribute = key;
		if (!script->member_indices.has(attribute)) {
			owner.unref();
			ERR_FAIL_V_MSG(Variant(), vformat(R"(Annotation "%s" has no attribute "%s".)", class_name, attribute));
		}
		const GDScript::MemberInfo &member = script->member_indices[attribute];
		const Variant value = values[key];
		if (member.data_type.has_type() && !member.data_type.is_type(value)) {
			owner.unref();
			ERR_FAIL_V_MSG(Variant(), vformat(R"(Invalid type for annotation attribute "%s".)", attribute));
		}
		instance->write_member_raw(member.index, _mutable_annotation_container(value));
	}
	return owner;
}

void AnnotationUtils::for_each_annotation(const Variant &p_annotation_class, const Ref<GDScript> &p_script, const Variant &p_callback) {
	ERR_FAIL_COND_MSG(p_script.is_null(), "Annotation script must be a GDScript.");
	AnnotationClass query;
	String error;
	if (!_resolve_annotation_class(p_annotation_class, query, error)) {
		ERR_FAIL_MSG(error);
	}
	Callable callback;
	if (!_prepare_callback(p_callback, callback, error)) {
		ERR_FAIL_MSG(error);
	}
	HashMap<StringName, AnnotationClass> cache;
	if (!_walk_script(p_script.ptr(), query.script.ptr(), &callback, nullptr, false, cache, error)) {
		ERR_FAIL_MSG(error);
	}
}

void AnnotationUtils::for_each_annotation_in_bases(const Variant &p_annotation_class, const Ref<GDScript> &p_script, const Variant &p_callback) {
	ERR_FAIL_COND_MSG(p_script.is_null(), "Annotation script must be a GDScript.");
	AnnotationClass query;
	String error;
	if (!_resolve_annotation_class(p_annotation_class, query, error)) {
		ERR_FAIL_MSG(error);
	}
	Callable callback;
	if (!_prepare_callback(p_callback, callback, error)) {
		ERR_FAIL_MSG(error);
	}

	HashMap<StringName, AnnotationClass> cache;
	Ref<GDScript> base = p_script->get_base();
	while (base.is_valid()) {
		if (!_walk_script(base.ptr(), query.script.ptr(), &callback, nullptr, false, cache, error)) {
			ERR_FAIL_MSG(error);
		}
		base = base->get_base();
	}
}

Variant AnnotationUtils::get_annotation(const Ref<GDScript> &p_script, const Variant &p_annotation_class, const StringName &p_member, const StringName &p_parameter) {
	ERR_FAIL_COND_V_MSG(p_script.is_null(), Variant(), "Annotation script must be a GDScript.");
	AnnotationClass query;
	String error;
	if (!_resolve_annotation_class(p_annotation_class, query, error)) {
		ERR_FAIL_V_MSG(Variant(), error);
	}
	Array entries;
	if (!_entries_for_target(p_script->get_annotations(), p_member, p_parameter, entries, error)) {
		ERR_FAIL_V_MSG(Variant(), error);
	}
	HashMap<StringName, AnnotationClass> cache;
	Variant match = _first_match(entries, query.script.ptr(), cache, error);
	if (!error.is_empty()) {
		ERR_FAIL_V_MSG(Variant(), error);
	}
	return match;
}

bool AnnotationUtils::has_annotation(const Ref<GDScript> &p_script, const Variant &p_annotation_class, const StringName &p_member, const StringName &p_parameter) {
	Variant match = get_annotation(p_script, p_annotation_class, p_member, p_parameter);
	return match.get_type() == Variant::DICTIONARY;
}

Variant AnnotationUtils::get_annotation_values(const Ref<GDScript> &p_script, const Variant &p_annotation_class, const StringName &p_member, const StringName &p_parameter) {
	Variant match = get_annotation(p_script, p_annotation_class, p_member, p_parameter);
	if (match.get_type() != Variant::DICTIONARY) {
		return Variant();
	}
	return Dictionary(match)["values"];
}

Array AnnotationUtils::find_annotated(const Ref<GDScript> &p_script, const Variant &p_annotation_class) {
	ERR_FAIL_COND_V_MSG(p_script.is_null(), Array(), "Annotation script must be a GDScript.");
	AnnotationClass query;
	String error;
	if (!_resolve_annotation_class(p_annotation_class, query, error)) {
		ERR_FAIL_V_MSG(Array(), error);
	}

	Array found;
	HashMap<StringName, AnnotationClass> cache;
	if (!_walk_script(p_script.ptr(), query.script.ptr(), nullptr, &found, false, cache, error)) {
		ERR_FAIL_V_MSG(Array(), error);
	}
	return found;
}

static bool _finish_initialized_walk(bool p_ok, const String &p_error) {
	if (p_ok) {
		return true;
	}
	if (!p_error.is_empty()) {
		ERR_FAIL_V_MSG(false, p_error);
	}
	return false;
}

void AnnotationUtils::for_each_annotation_initialized(const Variant &p_annotation_class, const Ref<GDScript> &p_script, const Variant &p_callback) {
	ERR_FAIL_COND_MSG(p_script.is_null(), "Annotation script must be a GDScript.");
	AnnotationClass query;
	String error;
	if (!_resolve_annotation_class(p_annotation_class, query, error)) {
		ERR_FAIL_MSG(error);
	}
	Callable callback;
	if (!_prepare_callback(p_callback, callback, error)) {
		ERR_FAIL_MSG(error);
	}
	HashMap<StringName, AnnotationClass> cache;
	if (!_finish_initialized_walk(_walk_script(p_script.ptr(), query.script.ptr(), &callback, nullptr, true, cache, error), error)) {
		return;
	}
}

void AnnotationUtils::for_each_annotation_in_bases_initialized(const Variant &p_annotation_class, const Ref<GDScript> &p_script, const Variant &p_callback) {
	ERR_FAIL_COND_MSG(p_script.is_null(), "Annotation script must be a GDScript.");
	AnnotationClass query;
	String error;
	if (!_resolve_annotation_class(p_annotation_class, query, error)) {
		ERR_FAIL_MSG(error);
	}
	Callable callback;
	if (!_prepare_callback(p_callback, callback, error)) {
		ERR_FAIL_MSG(error);
	}

	HashMap<StringName, AnnotationClass> cache;
	Ref<GDScript> base = p_script->get_base();
	while (base.is_valid()) {
		if (!_finish_initialized_walk(_walk_script(base.ptr(), query.script.ptr(), &callback, nullptr, true, cache, error), error)) {
			return;
		}
		base = base->get_base();
	}
}

Variant AnnotationUtils::get_annotation_initialized(const Ref<GDScript> &p_script, const Variant &p_annotation_class, const StringName &p_member, const StringName &p_parameter) {
	Variant entry = get_annotation(p_script, p_annotation_class, p_member, p_parameter);
	if (entry.get_type() != Variant::DICTIONARY) {
		return Variant();
	}
	return instantiate_annotation(entry);
}

Array AnnotationUtils::find_annotated_initialized(const Ref<GDScript> &p_script, const Variant &p_annotation_class) {
	ERR_FAIL_COND_V_MSG(p_script.is_null(), Array(), "Annotation script must be a GDScript.");
	AnnotationClass query;
	String error;
	if (!_resolve_annotation_class(p_annotation_class, query, error)) {
		ERR_FAIL_V_MSG(Array(), error);
	}

	Array found;
	HashMap<StringName, AnnotationClass> cache;
	if (!_walk_script(p_script.ptr(), query.script.ptr(), nullptr, &found, true, cache, error)) {
		if (!error.is_empty()) {
			ERR_FAIL_V_MSG(Array(), error);
		}
		return Array();
	}
	return found;
}
