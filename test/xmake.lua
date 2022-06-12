set_languages("c99 cxx11")

target("test_set")
    set_kind("binary")
    add_files("test_set.c")
    add_deps("c-avl")
target_end()

target("test_custom")
    set_kind("binary")
    add_files("test_custom.c")
    add_deps("c-avl")
target_end()