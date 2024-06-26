FROM mcr.microsoft.com/devcontainers/cpp:1-debian-12

# Upgrade the image
RUN apt-get update && export DEBIAN_FRONTEND=noninteractive \
    && apt-get -y upgrade

# Install dependencies for static analysis
RUN apt-get update && export DEBIAN_FRONTEND=noninteractive \
	&& apt-get -y install --no-install-recommends \
		clang-format \
		clang-tidy \
		cppcheck \
		doxygen \
		libgtest-dev \
		clangd \
		bear

# Setup googletest
RUN mkdir -p $HOME/build \
	&& cd $HOME/build \
	&& cmake /usr/src/googletest/googletest \
	&& make \
	&& cp lib/libgtest* /usr/lib/ \
	&& cd .. \
	&& sudo rm -rf build

# Setup codechecker
# RUN apt-get update && export DEBIAN_FRONTEND=noninteractive \
#	&& apt-get -y install --no-install-recommends \
#		python3-dev \
#		python3-pip \
#		python3-venv \
#	&& pip3 install --break-system-packages codechecker
