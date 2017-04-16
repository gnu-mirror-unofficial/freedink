FROM debian:stretch
MAINTAINER Sylvain Beucler <beuc@beuc.net>

# Packaged dependencies
COPY dependencies.sh /usr/src/dependencies.sh
RUN /usr/src/dependencies.sh

# Unprivileged compilation user:
# - control installed dependencies (no sudo)
# - generate files with default host uid/gid in bind mounts
# - when debugging reproducibility issues, one can assume the build didn't do crazy kernel stuff
RUN groupadd repro -g 1000
RUN useradd repro -m -s /bin/bash -u 1000 -g 1000
USER repro
RUN mkdir /home/repro/wd/
WORKDIR /home/repro/wd/

# Build Stage 1 - rebuilt dependencies
USER root
RUN mkdir /opt/mxe
RUN chown repro: /opt/mxe
USER repro
COPY build-deps.sh /usr/src/build-deps.sh
RUN /usr/src/build-deps.sh

# Build Stage 2
CMD /bin/bash
