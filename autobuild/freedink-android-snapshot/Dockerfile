FROM debian:8.7
MAINTAINER Sylvain Beucler <beuc@beuc.net>
COPY dependencies-dl-android.sh /usr/src/dependencies-dl-android.sh
RUN /usr/src/dependencies-dl-android.sh
COPY dependencies-dl-sdl.sh /usr/src/dependencies-dl-sdl.sh
RUN /usr/src/dependencies-dl-sdl.sh
COPY dependencies-sdl.sh /usr/src/dependencies-sdl.sh
RUN /usr/src/dependencies-sdl.sh

RUN useradd repro -m -s /bin/bash
USER repro
RUN mkdir /home/repro/wd/
WORKDIR /home/repro/wd/

CMD /bin/bash
