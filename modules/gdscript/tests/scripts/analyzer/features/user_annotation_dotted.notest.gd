class_name UserAnnotationDotted
extends RefCounted

class Inner:
	extends GDScriptAnnotation

	var label := ""

class Mid:
	class Deeper:
		extends GDScriptAnnotation

		var label := ""
