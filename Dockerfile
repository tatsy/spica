#
## Load docker image
#
FROM tatsy/ubuntu-cxx:default

#
## Environment variables
#
ENV TERM xterm
ENV BRANCH_NAME @BRANCH_NAME@
ENV PULL_REQUEST @PULL_REQUEST@
ENV CC @C_COMPILER@
ENV CXX @CXX_COMPILER@

#
## apt-get update/upgrade
#
RUN apt-get update -qq

# Install Google Test
RUN git clone --depth=1 -b release-1.7.0 https://github.com/google/googletest.git /usr/src/gtest
RUN \
  cd /usr/src/gtest && \
  mkdir build && \
  cd build && \
  cmake .. && \
  make -j4 && \
  mkdir -p /usr/local/lib && \
  mkdir -p /usr/local/include && \
  mv libg* /usr/local/lib && \
  mv ../include/* /usr/local/include && \
  cd /
ENV GTEST_LIBRARY /usr/local/lib/libgtest.a
ENV GTEST_MAIN_LIBRARY /usr/local/lib/libgtest_main.a
ENV GTEST_INCLUDE_DIRS /usr/include/local

#
## Install Gcovr
#
RUN apt-get -qq install python-pip
RUN pip install gcovr

#
## Install cppcheck, cccc, and doxygen
#
RUN apt-get -qq install cppcheck cccc doxygen

#
## Build spica
#
RUN git clone --depth 10 -b $BRANCH_NAME https://github.com/tatsy/spica.git
RUN \
  if [ $PULL_REQUEST != "false" ]; then \
    cd spica && \
    git fetch origin +refs/pull/$PULL_REQUEST/merge && \
    git checkout --quiet --force FETCH_HEAD; \
  fi

RUN \
  cd spica && \
  git submodule update --init --recursive && \
  cmake -D QT5_ROOT=/opt/Qt5 -D SPICA_BUILD_MAIN=ON -D SPICA_BUILD_TESTS=ON . && \
  cmake --build .

#
## # of threads used by OpenMP
#
ENV OMP_NUM_THREADS 4

#
## Define working direcotry
#
WORKDIR /root/spica

#
## Run unit tests
#
RUN lcov --directory . --zerocounters
