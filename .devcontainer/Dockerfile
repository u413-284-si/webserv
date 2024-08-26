FROM mcr.microsoft.com/devcontainers/cpp:debian-12

# Set default compiler to clang++
RUN update-alternatives --install /usr/bin/c++ c++ /usr/bin/clang++ 100

# Install tools
RUN apt-get update && export DEBIAN_FRONTEND=noninteractive \
	&& apt-get -y install --no-install-recommends \
	# Code formatting
		clang-format \
	# Code analysis
		clang-tidy \
		cppcheck \
	# Commenting
		doxygen \
		graphviz \
	# Google Test
		libgtest-dev \
		libgmock-dev \
	# Language Server
		clangd \
	# Create compile_commands.json
		bear \
	# Networking tools
		netcat-openbsd \
		telnet \
		siege \
	# Dependencies for kcov
		binutils-dev \
		libssl-dev \
		libcurl4-openssl-dev \
		libelf-dev \
		libstdc++-12-dev \
		zlib1g-dev \
		libdw-dev \
		libiberty-dev \
	# Remove valgrind because we build newer version
	&& apt-get remove -y \
		valgrind \
	&& rm -rf /var/lib/apt/lists/*

# Build googletest, kcov, and valgrind
RUN mkdir build \
	&& cd build \
	&& cmake /usr/src/googletest \
	&& make \
	&& cp lib/libg* /usr/lib/ \
	&& cd .. \
	&& sudo rm -rf build \
	&& git clone https://github.com/SimonKagstrom/kcov \
	&& cd kcov \
	&& cmake . \
	&& make \
	&& make install \
	&& cd .. \
	&& rm -rf kcov \
	&& wget https://sourceware.org/pub/valgrind/valgrind-3.23.0.tar.bz2 \
	&& tar -xvf valgrind-3.23.0.tar.bz2 \
	&& cd valgrind-3.23.0 \
	&& ./configure --prefix=/usr/ \
	&& make \
	&& make install \
	&& cd .. \
	&& rm -rf valgrind-3.23.0 \
	&& rm valgrind-3.23.0.tar.bz2

# Setup codechecker
# RUN apt-get update && export DEBIAN_FRONTEND=noninteractive \
#	&& apt-get -y install --no-install-recommends \
#		python3-dev \
#		python3-pip \
#		python3-venv \
#	&& pip3 install --break-system-packages codechecker
