FROM mcr.microsoft.com/devcontainers/cpp:debian-12

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
	&& rm -rf /var/lib/apt/lists/*

# Build googletest and kcov
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
	&& rm -rf kcov

# Setup codechecker
# RUN apt-get update && export DEBIAN_FRONTEND=noninteractive \
#	&& apt-get -y install --no-install-recommends \
#		python3-dev \
#		python3-pip \
#		python3-venv \
#	&& pip3 install --break-system-packages codechecker
