@warning_ignore_start("unsafe_cast")
extends RefCounted

func _noop(_ann_name: StringName, _values: Dictionary, _target: Object) -> void:
	pass

func test() -> void:
	var script := get_script() as GDScript
	print("not a dictionary")
	AnnotationUtils.instantiate_annotation(0)
	print("unknown")
	AnnotationUtils.instantiate_annotation({"name": "NoSuchAnnotation", "values": {}})
	print("native")
	AnnotationUtils.instantiate_annotation({"name": "Node", "values": {}})
	print("not annotation")
	AnnotationUtils.instantiate_annotation({"name": "UserAnnotationPlain", "values": {}})
	print("bad attribute")
	AnnotationUtils.instantiate_annotation({"name": "UserAnnotation", "values": {"not_an_attribute": 1}})
	print("bad type")
	AnnotationUtils.instantiate_annotation({"name": "UserAnnotation", "values": {"label": "ok", "from_base": "nope"}})
	print("bad class arg")
	AnnotationUtils.for_each_annotation(1, script, _noop)
	print("bad callback")
	AnnotationUtils.for_each_annotation(UserAnnotation, script, 1)
	print("bad parameter")
	AnnotationUtils.get_annotation(script, UserAnnotation, "", "who")
