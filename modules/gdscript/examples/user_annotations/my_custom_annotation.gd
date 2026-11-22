class_name MyCustomAnnotation
extends GDScriptAnnotation

# Attribute bag for @MyCustomAnnotation. Applying the annotation does not
# construct this class and does not call _init.
# AnnotationUtils.instantiate_annotation builds a RefCounted, attaches this
# script, and copies my_dict into the member store. It does not call _init
# or a setter. Applying @MyCustomAnnotation still does neither.
# `my_dict` defaults to an empty dictionary when a use does not pass it.
# `const repeatable := true` would allow another @MyCustomAnnotation on the
# same target. Without it, a second use is an error. There is no @repeatable.
# An inner class is @Outer.Inner. This global class_name stays @MyCustomAnnotation.
var my_dict: Dictionary = {}
