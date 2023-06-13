FROM gcc:latest

RUN apt-get update && apt-get install -y sudo build-essential git cmake curl unzip tar zip && rm -rf /var/lib/apt/lists/*
WORKDIR /home
RUN git clone https://github.com/Microsoft/vcpkg.git
WORKDIR /home/vcpkg
RUN ./bootstrap-vcpkg.sh
ENV VCPKG_ROOT="/home/vcpkg"

WORKDIR /app/src
COPY . /app/src
RUN mkdir build
WORKDIR /app/src/build
RUN cmake ..
RUN cmake --build . --config Release

CMD [ "ContinuousDeliveryAPI/Server" ]