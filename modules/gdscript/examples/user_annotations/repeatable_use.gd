# Two uses of RepeatableAnnotation, source order. get_annotation returns the
# first. for_each_annotation and find_annotations see both. Not called from _ready.
@RepeatableAnnotation(label="one")
@RepeatableAnnotation(label="two")
extends Node

func read_repeated() -> void:
	var script := get_script() as GDScript
	var first: Dictionary = AnnotationUtils.get_annotation(script, RepeatableAnnotation)
	print(first["values"]["label"])
	print(AnnotationUtils.find_annotated_elements(script, RepeatableAnnotation).size())
	print(AnnotationUtils.find_annotations(script, RepeatableAnnotation).size())
