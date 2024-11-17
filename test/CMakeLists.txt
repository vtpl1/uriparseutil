# *****************************************************
#    Copyright 2022 Videonetics Technology Pvt Ltd
# *****************************************************

find_package(Poco REQUIRED Foundation)

add_executable(tests
	src/test.cpp
)

target_link_libraries(tests
	PRIVATE uriparseutil
	PRIVATE Poco::Foundation
)
