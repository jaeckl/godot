@UserAnnotation(label="script")
extends RefCounted

@export
@UserAnnotation(my_dict={"asd": "asda"}, label="field")
var health := 1

@UserAnnotation(label="static")
static var marker := 0

@UserAnnotation(9, {"z": "q"}, "positional")
var positional := 0

@rpc
@UserAnnotation(label="function")
func ping() -> void:
	pass

@warning_ignore("unused_parameter")
func greet(@UserAnnotation(label="parameter") who: String) -> void:
	pass

@UserAnnotation(label="inner")
class Inner:
	var value := 1

@warning_ignore_start("unsafe_cast", "unsafe_method_access")
func test():
	var script := get_script() as GDScript
	var ann: Dictionary = script.get_annotations()

	var script_values: Dictionary = ann["script"][0]["values"]
	print("script ", ann["script"][0]["name"], " ", script_values["label"], " ", script_values["from_base"], " ", script_values["my_dict"].size())

	var field_values: Dictionary = ann["variables"]["health"][0]["values"]
	print("field ", field_values["label"], " ", field_values["from_base"], " ", field_values["my_dict"]["asd"], " ", typeof(field_values["my_dict"]) == TYPE_DICTIONARY)

	var static_values: Dictionary = ann["variables"]["marker"][0]["values"]
	print("static ", static_values["label"])

	var positional_values: Dictionary = ann["variables"]["positional"][0]["values"]
	print("positional ", positional_values["from_base"], " ", positional_values["label"], " ", positional_values["my_dict"]["z"])

	var function_values: Dictionary = ann["functions"]["ping"][0]["values"]
	print("function ", function_values["label"], " ", script.get_rpc_config().has("ping"))

	var parameter_values: Dictionary = ann["parameters"]["greet"]["who"][0]["values"]
	print("parameter ", parameter_values["label"])

	var inner_script: Variant = Inner
	var inner_values: Dictionary = (inner_script as GDScript).get_annotations()["script"][0]["values"]
	print("inner ", inner_values["label"])

	var exported := false
	for property in get_property_list():
		if property.name == "health" and property.usage & PROPERTY_USAGE_EDITOR:
			exported = true
	print("exported ", exported)
@warning_ignore_restore("unsafe_cast", "unsafe_method_access")
