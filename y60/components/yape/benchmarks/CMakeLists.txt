# XXX need spidermonkey defs
#add_definitions( -DXP_UNIX )

y60_add_component(
    ape_bench_compile_time
    SOURCES n_functions.cpp
    ${Y60_APE_LINK_ARGS}
)

y60_add_component(
    ape_bench_ape_binding
    SOURCES ape_binding.cpp bench_functions.cpp
    ${Y60_APE_LINK_ARGS}
)


y60_add_component(
    ape_bench_classic_binding
    SOURCES classic_binding.cpp bench_functions.cpp
    DEPENDS y60jsbase y60jslib
    EXTERNS ${JSEXTERNS}
)

