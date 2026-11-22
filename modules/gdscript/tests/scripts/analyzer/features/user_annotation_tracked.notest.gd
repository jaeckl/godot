class_name UserAnnotationTracked
extends GDScriptAnnotation

static var init_calls := 0
static var setter_calls := 0

var label := "":
	set(value):
		setter_calls += 1
		label = value

func _init() -> void:
	init_calls += 1
