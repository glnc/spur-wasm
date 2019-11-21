#!/bin/sh
PROGRAM="gmp"
VERSION="6.1.2"
PROGRAM_VERSION="${PROGRAM}-${VERSION}"
ARCHIVE="${PROGRAM_VERSION}.tar.bz2"
TMP_FOLDER="tmp"
ROOT_DIR=$PWD

echo "\n### Cleaning ${PROGRAM}..."

# remove gmp sources
rm -rf "${TMP_FOLDER}"

# remove compiled library
rm -rf include
rm -rf lib
rm -rf share

# download
mkdir -p "${TMP_FOLDER}"
cd "${TMP_FOLDER}"
echo "\n### Downloading ${ARCHIVE}..."
wget https://gmplib.org/download/gmp/${ARCHIVE}

# unpack
echo "\n### Unpacking ${ARCHIVE}..."
tar -xjf "${ARCHIVE}"
rm -f "${ARCHIVE}"

# configure
echo "\n### Configuring ${PROGRAM_VERSION} ..."
cd ${PROGRAM_VERSION}
C_FOR_BUILD=/usr/bin/gcc ABI=standard emconfigure ./configure \
    --build i686-pc-linux-gnu --host none --disable-assembly --enable-cxx \
    --prefix=${ROOT_DIR}

# build
echo "\n### Building ${PROGRAM_VERSION}..."
emmake make
emmake make install

# remove source folder
echo "\n### Cleanup ${PROGRAM_VERSION} sources..."
cd ${ROOT_DIR}
rm -rf ${TMP_FOLDER}
