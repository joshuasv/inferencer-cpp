# Use an official Ubuntu as a parent image
FROM ubuntu:jammy

# Define build time variables
ARG LANG
ARG TZ
ARG DEJAVU_FONTS_VERSION
ARG DEJAVU_FONTS_VERSION_STR
ARG CMAKE_VERSION
ARG QT_MAJOR
ARG QT_VERSION
ARG HIREDIS_VERSION
ARG REDISPLUSPLUS_VERSION
ARG ONNXRUNTIME_VERSION
ARG QT_CMAKE_PATH
ARG QT_PLUGIN_PATH

# Set locale
RUN apt-get update && apt-get install locales && locale-gen en_US ${LANG} \
    && update-locale LC_ALL=${LANG} LANG=${LANG} 
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

# Install any needed packages specified
RUN apt-get update && apt-get install -y \
    git \
    wget \
    build-essential \
    python3 \
    python3-pip \
    libgl1-mesa-dev \
    lsb-release \
    curl \
    gpg \
    libopencv-dev \
    ninja-build \
    libwayland-dev \
    libxkbcommon-dev \
    libxkbcommon-x11-dev

# Add a font
RUN mkdir -p /usr/local/lib/fonts
RUN wget https://github.com/dejavu-fonts/dejavu-fonts/releases/download/version_${DEJAVU_FONTS_VERSION_STR}/dejavu-fonts-ttf-${DEJAVU_FONTS_VERSION}.tar.bz2 \
    && tar xvf dejavu-fonts-ttf-${DEJAVU_FONTS_VERSION}.tar.bz2 \
    && cp -r dejavu-fonts-ttf-${DEJAVU_FONTS_VERSION}/ttf/*.ttf /usr/local/lib/fonts \
    && fc-cache -fv

# Download, extract, and install the specific version of CMake
RUN wget https://github.com/Kitware/CMake/releases/download/v${CMAKE_VERSION}/cmake-${CMAKE_VERSION}-Linux-x86_64.sh \
    && chmod +x cmake-${CMAKE_VERSION}-Linux-x86_64.sh \
    && ./cmake-${CMAKE_VERSION}-Linux-x86_64.sh --skip-license --prefix=/usr/local

# Download, extract and install Qt
RUN wget https://download.qt.io/archive/qt/${QT_MAJOR}/${QT_VERSION}/single/qt-everywhere-src-${QT_VERSION}.tar.xz \
   && tar xf qt-everywhere-src-${QT_VERSION}.tar.xz \
   && cd ./qt-everywhere-src-${QT_VERSION} \
   && ./configure -release -opensource -nomake examples -nomake tests -confirm-license -skip webengine -skip qtdoc -skip qtgraphicaleffects -skip qtqa -skip qttranslations -skip qtvirtualkeyboard -skip qtquicktimeline -skip qtquick3d -skip qt3d -skip qtrepotools -skip qttools -skip qtimageformats -skip qtnetworkauth -skip qtwebsockets -skip qtactiveqt -skip qtmacextras -skip winextras -skip qtmultimedia -skip qtgamepad -skip qtserialbus -skip qtspeech -skip qtsensors -skip qtlocation -no-ssl -skip qtserialport -skip qtcharts -system-freetype -skip qtgraphs -skip qtquick3dphysics \
   && ninja qtbase/all qtwayland/all \
   && ninja install

# Install Redis, hiredis, and redis-plus-plus
RUN curl -fsSL https://packages.redis.io/gpg | gpg --dearmor -o /usr/share/keyrings/redis-archive-keyring.gpg \
    && echo "deb [signed-by=/usr/share/keyrings/redis-archive-keyring.gpg] https://packages.redis.io/deb $(lsb_release -cs) main" | tee /etc/apt/sources.list.d/redis.list \
    && apt-get update && apt-get install -y redis
RUN wget https://github.com/redis/hiredis/archive/refs/tags/v${HIREDIS_VERSION}.tar.gz \
    && tar xvf v${HIREDIS_VERSION}.tar.gz \
    && cd hiredis-${HIREDIS_VERSION} \
    && make \
    && make install
RUN wget https://github.com/sewenew/redis-plus-plus/archive/refs/tags/${REDISPLUSPLUS_VERSION}.tar.gz \
    && tar xvf ${REDISPLUSPLUS_VERSION}.tar.gz \
    && cd redis-plus-plus-${REDISPLUSPLUS_VERSION} \
    && mkdir build && cd build \
    && cmake .. && make && make install
    
# Download, extract and install onnxruntime
RUN wget https://github.com/microsoft/onnxruntime/releases/download/v${ONNXRUNTIME_VERSION}/onnxruntime-linux-x64-${ONNXRUNTIME_VERSION}.tgz \
    && tar xvf onnxruntime-linux-x64-${ONNXRUNTIME_VERSION}.tgz \
    && mv onnxruntime-linux-x64-${ONNXRUNTIME_VERSION}/lib/* /usr/local/lib \
    && mkdir /usr/local/include/onnxruntime \
    && cp -r onnxruntime-linux-x64-${ONNXRUNTIME_VERSION}/include/* /usr/local/include/onnxruntime

# Cleanup
RUN rm cmake-${CMAKE_VERSION}-Linux-x86_64.sh \
   && rm qt-everywhere-src-${QT_VERSION}.tar.xz \
   && rm -rf ./qt-everywhere-src-${QT_VERSION} \
   && rm v${HIREDIS_VERSION}.tar.gz \
   && rm -rf hiredis-${HIREDIS_VERSION} \
   && rm ${REDISPLUSPLUS_VERSION}.tar.gz \
   && rm -rf redis-plus-plus-${REDISPLUSPLUS_VERSION} \
   && rm onnxruntime-linux-x64-${ONNXRUNTIME_VERSION}.tgz \
   && rm -rf onnxruntime-linux-x64-${ONNXRUNTIME_VERSION} \
   && rm dejavu-fonts-ttf-${DEJAVU_FONTS_VERSION}.tar.bz2 \
   && rm -rf dejavu-fonts-ttf-${DEJAVU_FONTS_VERSION}

WORKDIR /app

COPY . /app

RUN ./scripts/configure.sh && ./scripts/build.sh && ./scripts/install.sh