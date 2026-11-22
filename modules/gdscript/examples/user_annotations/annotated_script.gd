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
# for_each_annotation context rows for this script:
#   target "script", member "", parameter ""
#   target "variable", member "health", parameter ""
#   target "function", member "ping", parameter ""
#   target "parameter", member "greet", parameter "who"
#
# read_annotations() walks those rows, reads the health entry, lists every
# match, and instantiates the script-header entry. The _initialized methods
# return objects for those same matches. The callback is (annotation, context).
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
	AnnotationUtils.for_each_annotation(MyCustomAnnotation, script, func(ann_name: StringName, values: Dictionary, context: Dictionary) -> void:
		print(ann_name, context["target"], context["member"], context["parameter"], values["my_dict"])
	)
	var field: Dictionary = AnnotationUtils.get_annotation(script, MyCustomAnnotation, &"health")
	print(field)
	print(AnnotationUtils.find_annotated(script, MyCustomAnnotation))
	var header: Dictionary = AnnotationUtils.get_annotation(script, MyCustomAnnotation)
	var ann: MyCustomAnnotation = AnnotationUtils.instantiate_annotation(header)
	print(ann.my_dict)
	var created: MyCustomAnnotation = AnnotationUtils.get_annotation_initialized(script, MyCustomAnnotation, &"health")
	print(created.my_dict)
	AnnotationUtils.for_each_annotation_initialized(MyCustomAnnotation, script, func(annotation: MyCustomAnnotation, context: Dictionary) -> void:
		print(context["target"], annotation.my_dict)
	)
	print(AnnotationUtils.find_annotated_initialized(script, MyCustomAnnotation).size())

