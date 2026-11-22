# User annotations on the script, a field, a function, and a parameter.
# Dictionary attributes are real dictionaries, initialized in place.
#
# (get_script() as GDScript).get_annotations() returns:
# {
#   "script": [{
#     "name": &"MyCustomAnnotation",
#     "values": { "my_dict": { "asd": "asda" } },
#   }],
#   "variables": {
#     "health": [{
#       "name": &"MyCustomAnnotation",
#       "values": { "my_dict": { "asd": "field" } },
#     }],
#   },
#   "functions": {
#     "ping": [{
#       "name": &"MyCustomAnnotation",
#       "values": { "my_dict": { "asd": "function" } },
#     }],
#   },
#   "parameters": {
#     "greet": {
#       "who": [{
#         "name": &"MyCustomAnnotation",
#         "values": { "my_dict": { "asd": "parameter" } },
#       }],
#     },
#   },
# }
#
# for_each_annotation view rows for this script:
#   ScriptInfo
#   VariableInfo name "health"
#   FunctionInfo name "ping"
#   ParameterInfo function "greet" name "who"
#
# read_annotations() walks those rows, reads the health entry, lists every
# match as a ReflectionUtils view, and instantiates the script-header entry.
# The _initialized methods return annotation objects. The callback is
# (annotation, target). find_annotations returns objects, not views.
# None of this runs from _ready.
@MyCustomAnnotation(my_dict={"asd": "asda"})
extends Node

@MyCustomAnnotation(my_dict={"asd": "field"})
var health := 1

@MyCustomAnnotation(my_dict={"asd": "function"})
func ping() -> void:
	pass

func greet(@MyCustomAnnotation(my_dict={"asd": "parameter"}) who: String) -> void:
	pass

func read_annotations() -> void:
	var script := get_script() as GDScript
	AnnotationUtils.for_each_annotation(MyCustomAnnotation, script, func(ann_name: StringName, values: Dictionary, target: Object) -> void:
		if target is VariableInfo:
			var info := target as VariableInfo
			print(ann_name, info.name, info.type, values["my_dict"])
		elif target is FunctionInfo:
			var info := target as FunctionInfo
			print(ann_name, info.name, info.argument_count, values["my_dict"])
		elif target is ParameterInfo:
			var info := target as ParameterInfo
			print(ann_name, info.function, info.name, info.type, values["my_dict"])
		elif target is ScriptInfo:
			print(ann_name, values["my_dict"])
	)
	var field: Dictionary = AnnotationUtils.get_annotation(script, MyCustomAnnotation, &"health")
	print(field)
	var found: Array = AnnotationUtils.find_annotated_elements(script, MyCustomAnnotation)
	for row in found:
		if row is VariableInfo:
			print((row as VariableInfo).name)
	var header: Dictionary = AnnotationUtils.get_annotation(script, MyCustomAnnotation)
	var ann: MyCustomAnnotation = AnnotationUtils.instantiate_annotation(header)
	print(ann.my_dict)
	var created: MyCustomAnnotation = AnnotationUtils.get_annotation_initialized(script, MyCustomAnnotation, &"health")
	print(created.my_dict)
	AnnotationUtils.for_each_annotation_initialized(MyCustomAnnotation, script, func(annotation: MyCustomAnnotation, target: Object) -> void:
		if target is VariableInfo:
			print((target as VariableInfo).name, annotation.my_dict)
	)
	print(AnnotationUtils.find_annotations(script, MyCustomAnnotation).size())

