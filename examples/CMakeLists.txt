add_executable(physics physics.cpp)
target_link_libraries(physics PRIVATE ecs)
setWarnings(physics)
setStandard(physics)
# Profiling
#target_compile_options(physics PRIVATE -pg)
#target_link_libraries(physics PRIVATE -pg)