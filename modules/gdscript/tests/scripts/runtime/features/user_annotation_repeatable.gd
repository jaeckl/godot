@warning_ignore_start("unused_parameter", "unsafe_cast", "unsafe_method_access", "unsafe_property_access")
@UserAnnotationRepeatable(label="one")
@UserAnnotationRepeatable(label="two")
extends RefCounted

func _on_each(_ann_name: StringName, values: Dictionary, _target: Object) -> void:
	print(values["label"])

func test() -> void:
	var script := get_script() as GDScript
	var first: Dictionary = AnnotationUtils.get_annotation(script, UserAnnotationRepeatable)
	var first_values: Dictionary = AnnotationUtils.get_annotation_values(script, UserAnnotationRepeatable)
	print("first ", first["name"], " ", first_values["label"])
	AnnotationUtils.for_each_annotation(UserAnnotationRepeatable, script, _on_each)
	print("elements ", AnnotationUtils.find_annotated_elements(script, UserAnnotationRepeatable).size())
	print("objects ", AnnotationUtils.find_annotations(script, UserAnnotationRepeatable).size())
	var created: UserAnnotationRepeatable = AnnotationUtils.get_annotation_initialized(script, UserAnnotationRepeatable)
	print("init ", created.label, " ", created is GDScriptAnnotation)
