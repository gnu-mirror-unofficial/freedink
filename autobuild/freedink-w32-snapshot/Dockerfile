FROM debian:8.7
MAINTAINER Sylvain Beucler <beuc@beuc.net>
COPY dependencies-mxe.sh /usr/src/dependencies-mxe.sh
RUN /usr/src/dependencies-mxe.sh
COPY dependencies.sh /usr/src/dependencies.sh
RUN /usr/src/dependencies.sh

RUN useradd repro -m -s /bin/bash
USER repro
RUN mkdir /home/repro/wd/
WORKDIR /home/repro/wd/

CMD /bin/bash
