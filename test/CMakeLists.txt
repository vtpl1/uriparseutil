# *****************************************************
#    Copyright 2022 Videonetics Technology Pvt Ltd
# *****************************************************

add_executable(tests_uriparseutil
	# src/test.cpp
	src/main1.cpp
)

target_link_libraries(tests_uriparseutil
	PRIVATE uriparseutil
)
