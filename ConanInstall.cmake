# This file lists and installs the Conan packages needed
# Conan will be run again each time this file changes (based on SHA256 hash)

message(STATUS "openstudio: RUNNING CONAN")

conan_add_remote(NAME bincrafters
  URL https://api.bintray.com/conan/bincrafters/public-conan)

set(CONAN_OPTIONS "")
set(CONAN_BUILD "")
if (MSVC)
  set(CONAN_OPENSSL "")
  set(CONAN_BOOST_ASIO "")
  set(CONAN_WEBSOCKETPP "")
  list(APPEND CONAN_OPTIONS "zlib:minizip=True")
  list(APPEND CONAN_BUILD "missing")
elseif (APPLE)
  set(CONAN_OPENSSL "")
  set(CONAN_BOOST_ASIO "")
  set(CONAN_WEBSOCKETPP "")
  list(APPEND CONAN_OPTIONS "zlib:minizip=True")
  list(APPEND CONAN_BUILD "missing")
else()
  set(CONAN_OPENSSL "OpenSSL/1.1.0g@conan/stable")
  set(CONAN_BOOST_ASIO "boost_asio/1.69.0@bincrafters/stable")
  set(CONAN_WEBSOCKETPP "websocketpp/0.8.1@bincrafters/stable")
  list(APPEND CONAN_OPTIONS "zlib:minizip=True")
  list(APPEND CONAN_OPTIONS "jsoncpp:use_pic=True")
  list(APPEND CONAN_BUILD "missing")
  list(APPEND CONAN_BUILD "boost_asio")
  list(APPEND CONAN_BUILD "websocketpp")
endif()

if (BUILD_TESTING)
  set(CONAN_GTEST "gtest/1.8.1@bincrafters/stable")
else()
  set(CONAN_GTEST "")
endif()

# DLM: add option for shared libs if we are building shared?

# This will create the conanbuildinfo.cmake in the current binary dir, not the cmake_binary_dir
conan_cmake_run(REQUIRES
  ${CONAN_OPENSSL}
  ${CONAN_BOOST_ASIO}
  boost_program_options/1.69.0@bincrafters/stable
  boost_regex/1.69.0@bincrafters/stable
  boost_filesystem/1.69.0@bincrafters/stable
  boost_crc/1.69.0@bincrafters/stable
  boost_algorithm/1.69.0@bincrafters/stable
  boost_uuid/1.69.0@bincrafters/stable
  boost_log/1.69.0@bincrafters/stable
  boost_numeric_ublas/1.69.0@bincrafters/stable
  boost_functional/1.69.0@bincrafters/stable
  boost_geometry/1.69.0@bincrafters/stable
  pugixml/1.9@bincrafters/stable
  jsoncpp/1.8.4@theirix/stable
  zlib/1.2.11@conan/stable
  fmt/5.2.1@bincrafters/stable
  sqlite3/3.27.2@bincrafters/stable
  cpprestsdk/2.10.10@bincrafters/stable
  ${CONAN_WEBSOCKETPP}
  geographiclib/1.49@bincrafters/stable
  ${CONAN_GTEST}
  BASIC_SETUP CMAKE_TARGETS NO_OUTPUT_DIRS
  OPTIONS ${CONAN_OPTIONS}
  BUILD ${CONAN_BUILD}
)

#message("CONAN_TARGETS = ${CONAN_TARGETS}")
message(STATUS "openstudio: DONE RUNNING CONAN")