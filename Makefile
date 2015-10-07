CC	   = clang
CXX    = clang++

all: info build run test coverage cppcheck cccc

info:
	docker info

build:
	docker pull tatsy/ubuntu-cxx; \
	sed -e "s/@TRAVIS_C_COMPILER@/$(CC)/;s/@TRAVIS_CXX_COMPILER@/$(CXX)/" Dockerfile.in > Dockerfile; \
	docker build --tag=spica-env .

rebuild:
	docker pull tatsy/ubuntu-cxx; \
	sed -e "s/@TRAVIS_C_COMPILER@/$(CC)/;s/@TRAVIS_CXX_COMPILER@/$(CXX)/" Dockerfile.in > Dockerfile; \
	docker build --no-cache --tag=spica-env .

run:
	docker run --name=spica-env --env="CTEST_OUTPUT_ON_FAILURE=TRUE" -itd spica-env

test:
	docker exec spica-env make check

coverage:
	docker exec spica-env gcovr --xml --output="gcovr.xml" --gcov-executable=llvm-cov -r "./src/"; \
	docker exec spica-env cat gcovr.xml > gcovr.xml

cppcheck:
	docker exec spica-env cppcheck --enable=all --xml . 2> cppcheck.xml; \
	docker exec spica-env cat cppcheck.xml > cppcheck.xml

cccc:
	docker exec spica-env cccc src/**/*.cc src/**/*.h; \
	docker exec spica-env cat .cccc/cccc.xml > cccc.xml
