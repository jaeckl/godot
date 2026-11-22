# @DottedAnnotationHost.Mark names the inner class. Not called from _ready.
@DottedAnnotationHost.Mark(label="inner")
extends Node

func read_dotted() -> void:
	var script := get_script() as GDScript
	var entry: Dictionary = AnnotationUtils.get_annotation(script, "DottedAnnotationHost.Mark")
	print(entry["name"])
	print(AnnotationUtils.find_annotations(script, DottedAnnotationHost.Mark).size())
