#!/bin/sh

# For backtrace on error
set -x   

PROJECT_DIR=`pwd`
BUILD_TYPE=${BUILD_TYPE:-release}
BUILD_DIR=${PROJECT_DIR}/build/${BUILD_TYPE}
BUILD_NO_EXAMPLES=${BUILD_NO_EXAMPLES:-0}
BUILD_NO_TEST=${BUILD_NO_TEST:-0}
BUILD_INSTALL=${BUILD_INSTALL:-1}
INSTALL_DIR=${INSTALL_DIR:-${PROJECT_DIR}/build}

mkdir -p ${INSTALL_DIR} 

mkdir -p ${BUILD_DIR} \
	&& cd ${BUILD_DIR} \
	&& cmake \
			-DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
			-DCMAKE_BUILD_NO_EXAMPLES=${BUILD_NO_EXAMPLES} \
			-DCMAKE_BUILD_NO_TEST=${BUILD_NO_TEST} \
			-DCMAKE_BUILD_INSTALL=${BUILD_INSTALL} \
			-DCMAKE_INSTALL_PREFIX=${INSTALL_DIR} \
			${PROJECT_DIR} \
	&& make $* && make install	