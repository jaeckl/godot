class_name MyCustomAnnotation
extends GDScriptAnnotation

# Attribute bag for @MyCustomAnnotation. Applying the annotation does not
# construct this class and does not call _init.
# `my_dict` defaults to an empty dictionary when a use does not pass it.
var my_dict: Dictionary = {}
