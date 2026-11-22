@warning_ignore_start("unused_parameter", "unsafe_cast", "unsafe_method_access", "unsafe_property_access")
@UserAnnotationDotted.Inner(label="inner")
extends RefCounted

@UserAnnotationDotted.Mid.Deeper(label="deep")
var marked := 1

func test() -> void:
	var script := get_script() as GDScript
	var by_name: Dictionary = AnnotationUtils.get_annotation(script, "UserAnnotationDotted.Inner")
	print("name ", by_name["name"], " ", by_name["values"]["label"])
	var by_script: Dictionary = AnnotationUtils.get_annotation(script, UserAnnotationDotted.Inner)
	print("script ", by_script["name"])
	var created: UserAnnotationDotted.Inner = AnnotationUtils.get_annotation_initialized(script, "UserAnnotationDotted.Inner")
	print("init ", created.label, " ", created is GDScriptAnnotation)
	print("find ", AnnotationUtils.find_annotations(script, UserAnnotationDotted.Inner).size())
	var deep: Dictionary = AnnotationUtils.get_annotation(script, "UserAnnotationDotted.Mid.Deeper", "marked")
	print("deep ", deep["name"], " ", deep["values"]["label"])
	var values: Dictionary = AnnotationUtils.get_annotation_values(script, UserAnnotationDotted.Mid.Deeper, "marked")
	print("values ", values["label"])
