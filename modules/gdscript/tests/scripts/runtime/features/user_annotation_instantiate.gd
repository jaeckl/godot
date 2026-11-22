@warning_ignore_start("unused_parameter", "unsafe_cast", "unsafe_method_access", "unsafe_property_access")
@UserAnnotation(label="script")
extends RefCounted

@UserAnnotation(my_dict={"asd": "field"}, label="field")
@UserAnnotationSibling(tag="nope")
var health := 1

@UserAnnotation(label="stacked")
@UserAnnotationAlso()
var stacked := 0

@UserAnnotationTracked(label="tracked")
var tracked := 0

@UserAnnotation(label="function")
func ping() -> void:
	pass

func greet(@UserAnnotation(label="parameter") who: String) -> void:
	pass

func _on_each(ann_name: StringName, values: Dictionary, context: Dictionary) -> void:
	print("each ", context["target"], "|", context["member"], "|", context["parameter"], "|", ann_name, "|", values.get("label", ""), "|", values.get("extra", ""))

func test() -> void:
	var script := get_script() as GDScript

	AnnotationUtils.for_each_annotation(UserAnnotation, script, _on_each)
	print("base query")
	AnnotationUtils.for_each_annotation(UserAnnotationBase, script, _on_each)

	var header: Dictionary = AnnotationUtils.get_annotation(script, UserAnnotationBase)
	var header_values: Dictionary = AnnotationUtils.get_annotation_values(script, UserAnnotationBase)
	print("header ", header["name"], " ", header_values["label"], " ", header_values.has("name"))
	print("has header ", AnnotationUtils.has_annotation(script, UserAnnotationBase))
	print("has missing ", AnnotationUtils.has_annotation(script, UserAnnotationBase, "missing"))
	print("has members only ", AnnotationUtils.has_annotation(UserAnnotationHost, UserAnnotationBase))
	print("has host member ", AnnotationUtils.has_annotation(UserAnnotationHost, UserAnnotationBase, "host_value"))

	var stacked_base: Dictionary = AnnotationUtils.get_annotation(script, UserAnnotationBase, "stacked")
	var stacked_exact: Dictionary = AnnotationUtils.get_annotation(script, UserAnnotation, "stacked")
	print("stacked base ", stacked_base["name"])
	print("stacked exact ", stacked_exact["name"])

	var found: Array = AnnotationUtils.find_annotated(script, UserAnnotationBase)
	for row in found:
		print("find ", row["target"], "|", row["member"], "|", row["parameter"])

	AnnotationUtils.for_each_annotation(UserAnnotation, UserAnnotationChild, _on_each)
	print("child each done")
	AnnotationUtils.for_each_annotation_in_bases(UserAnnotation, UserAnnotationChild, _on_each)
	AnnotationUtils.for_each_annotation(UserAnnotation, UserAnnotationHost, _on_each)
	print("host each done")

	var script_entry: Dictionary = header
	var field_entry: Dictionary = AnnotationUtils.get_annotation(script, UserAnnotation, "health")
	var function_entry: Dictionary = AnnotationUtils.get_annotation(script, UserAnnotation, "ping")
	var parameter_entry: Dictionary = AnnotationUtils.get_annotation(script, UserAnnotation, "greet", "who")
	_show_instance("script", script_entry)
	_show_instance("field", field_entry)
	_show_instance("function", function_entry)
	_show_instance("parameter", parameter_entry)

	var first: UserAnnotation = AnnotationUtils.instantiate_annotation(field_entry)
	var second: UserAnnotation = AnnotationUtils.instantiate_annotation(field_entry)
	print("same ", is_same(first, second))
	print("copy readonly ", first.my_dict.is_read_only(), " ", first.my_dict["asd"])
	first.my_dict["extra"] = 1
	var bag: Dictionary = AnnotationUtils.get_annotation_values(script, UserAnnotation, "health")
	var bag_dict: Dictionary = bag["my_dict"]
	print("bag ", bag_dict.is_read_only(), " ", bag_dict.has("extra"), " ", second.my_dict.has("extra"), " ", first.my_dict.has("extra"))

	var stored: Dictionary = script.get_annotations()["script"][0]
	print("stored ", typeof(stored) == TYPE_DICTIONARY, " ", typeof(stored["values"]) == TYPE_DICTIONARY)

	var tracked_entry: Dictionary = AnnotationUtils.get_annotation(script, UserAnnotationTracked, "tracked")
	var tracked_obj: UserAnnotationTracked = AnnotationUtils.instantiate_annotation(tracked_entry)
	print("tracked ", UserAnnotationTracked.init_calls, " ", UserAnnotationTracked.setter_calls, " ", tracked_obj.label)

func _show_instance(tag: String, entry: Dictionary) -> void:
	var created: UserAnnotation = AnnotationUtils.instantiate_annotation(entry)
	print("new ", tag, " ", created is UserAnnotation, " ", created is GDScriptAnnotation, " ", created.from_base, " ", created.my_dict.get("asd", ""))
