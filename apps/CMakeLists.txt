cmake_minimum_required (VERSION 2.8)

add_subdirectory("lgp-collision-avoidance")
#add_subdirectory("lgp-gjk")
add_subdirectory("DomainPlayer")

add_subdirectory("lgp-seq-boxes")
add_subdirectory("lgp-tree-boxes")

add_subdirectory("lgp-seq-blocks")
add_subdirectory("lgp-tree-blocks")

add_subdirectory("lgp-seq-po-blocks")
add_subdirectory("lgp-tree-po-blocks")
add_subdirectory("lgp-tree-po-blocks-baxter")
add_subdirectory("lgp-tree-po-blocks-explo-baxter")

add_subdirectory("lgp-tree-po-blocks-franka")
add_subdirectory("lgp-tree-po-blocks-explo-franka")

add_subdirectory("lgp-seq-self-driving")
add_subdirectory("lgp-tree-po-self-driving")
add_subdirectory("lgp-learn-self-driving")

add_subdirectory("lgp-real-time")

#add_subdirectory("tmkit-path-evaluator")


