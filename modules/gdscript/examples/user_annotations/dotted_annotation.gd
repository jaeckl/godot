class_name DottedAnnotationHost
extends RefCounted

# Inner annotation class. Use it as @DottedAnnotationHost.Mark.
# A global class_name annotation stays @Name.
class Mark:
	extends GDScriptAnnotation

	var label := ""
