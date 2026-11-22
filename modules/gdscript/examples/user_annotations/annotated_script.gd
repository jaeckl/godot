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
@MyCustomAnnotation(my_dict={"asd": "asda"})
extends Node

@MyCustomAnnotation(my_dict={"asd": "field"})
var health := 1

@MyCustomAnnotation(my_dict={"asd": "function"})
func ping() -> void:
	pass

func greet(@MyCustomAnnotation(my_dict={"asd": "parameter"}) who: String) -> void:
	pass
