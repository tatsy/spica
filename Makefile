CC	   = clang
CXX    = clang++

all: build coverage cppcheck cccc

build:
	docker pull tatsy/ubuntu-cxx; \
	sed -e "s/@TRAVIS_C_COMPILER@/$(CC)/;s/@TRAVIS_CXX_COMPILER@/$(CXX)/" Dockerfile.in > Dockerfile; \
	docker build --tag=spica-env .

rebuild:
	docker pull tatsy/ubuntu-cxx; \
	sed -e "s/@TRAVIS_C_COMPILER@/$(CC)/;s/@TRAVIS_CXX_COMPILER@/$(CXX)/" Dockerfile.in > Dockerfile; \
	docker build --no-cache --tag=spica-env .

coverage:
	docker run -t spica-env /bin/bash -c 'gcovr --xml --output="gcovr.xml" --gcov-executable=llvm-cov -r "./src/" && cat gcovr.xml' > gcovr.xml; \

cppcheck:
	docker run -t spica-env /bin/bash -c 'cppcheck --enable=all --xml . 2> cppcheck.xml && cat cppcheck.xml' > cppcheck.xml; \

cccc:
	docker run -t spica-env /bin/bash -c 'cccc src/**/*.cc src/**/*.h && cat .cccc/cccc.xml' > cccc.xml
