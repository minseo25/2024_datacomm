# CMake generated Testfile for 
# Source directory: /home/minseo/snu_project/ns-allinone-3.36.1/ns-3.36.1/src/fd-net-device
# Build directory: /home/minseo/snu_project/ns-allinone-3.36.1/ns-3.36.1/cmake-cache/src/fd-net-device
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(ctest-raw-sock-creator "ns3.36.1-raw-sock-creator-default")
set_tests_properties(ctest-raw-sock-creator PROPERTIES  WORKING_DIRECTORY "/home/minseo/snu_project/ns-allinone-3.36.1/ns-3.36.1/build/src/fd-net-device/" _BACKTRACE_TRIPLES "/home/minseo/snu_project/ns-allinone-3.36.1/ns-3.36.1/build-support/macros-and-definitions.cmake;1337;add_test;/home/minseo/snu_project/ns-allinone-3.36.1/ns-3.36.1/src/fd-net-device/CMakeLists.txt;164;set_runtime_outputdirectory;/home/minseo/snu_project/ns-allinone-3.36.1/ns-3.36.1/src/fd-net-device/CMakeLists.txt;0;")
add_test(ctest-tap-device-creator "ns3.36.1-tap-device-creator-default")
set_tests_properties(ctest-tap-device-creator PROPERTIES  WORKING_DIRECTORY "/home/minseo/snu_project/ns-allinone-3.36.1/ns-3.36.1/build/src/fd-net-device/" _BACKTRACE_TRIPLES "/home/minseo/snu_project/ns-allinone-3.36.1/ns-3.36.1/build-support/macros-and-definitions.cmake;1337;add_test;/home/minseo/snu_project/ns-allinone-3.36.1/ns-3.36.1/src/fd-net-device/CMakeLists.txt;199;set_runtime_outputdirectory;/home/minseo/snu_project/ns-allinone-3.36.1/ns-3.36.1/src/fd-net-device/CMakeLists.txt;0;")
subdirs("examples")
