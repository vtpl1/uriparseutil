# *****************************************************
#    Copyright 2022 Videonetics Technology Pvt Ltd
# *****************************************************

add_executable(tests
	src/test.cpp
)

target_link_libraries(tests
	PRIVATE uriparseutil
)
