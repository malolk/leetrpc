#!/bin/sh

CURRENT_DIR=`pwd`
cd ${CURRENT_DIR}/src/code_generator/
make; make clean; mv conv ${CURRENT_DIR}
