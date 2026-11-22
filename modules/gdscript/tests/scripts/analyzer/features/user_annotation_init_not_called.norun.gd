class_name UserAnnotationWithInit
extends GDScriptAnnotation

var tagged := 0:
	set(value):
		tagged = value
	get:
		return tagged

func _init() -> void:
	pass
