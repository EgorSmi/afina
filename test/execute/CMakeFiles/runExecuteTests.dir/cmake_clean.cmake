file(REMOVE_RECURSE
  "runExecuteTests"
  "runExecuteTests.pdb"
)

# Per-language clean rules from dependency scanning.
foreach(lang CXX)
  include(CMakeFiles/runExecuteTests.dir/cmake_clean_${lang}.cmake OPTIONAL)
endforeach()
