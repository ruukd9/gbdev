FROM ubuntu:latest

RUN apt-get update && apt-get install -y \
    curl \
    vim

####### = START INSTALLS = #######

# https://github.com/scottyhardy/docker-wine/blob/master/Dockerfile
# https://stackoverflow.com/questions/61815364/how-can-i-get-my-win32-app-running-with-wine-in-docker

# Install prerequisites
RUN apt-get update \
    && DEBIAN_FRONTEND="noninteractive" apt-get install -y --no-install-recommends \
        apt-transport-https \
        ca-certificates \
        cabextract \
        git \
        gnupg \
        gosu \
        gpg-agent \
        locales \
        p7zip \
        pulseaudio \
        pulseaudio-utils \
        sudo \
        tzdata \
        unzip \
        wget \
        winbind \
        xvfb \
        zenity \
    && rm -rf /var/lib/apt/lists/*

# Install wine
ARG WINE_BRANCH="stable"
RUN wget -nv -O- https://dl.winehq.org/wine-builds/winehq.key | APT_KEY_DONT_WARN_ON_DANGEROUS_USAGE=1 apt-key add - \
    && echo "deb https://dl.winehq.org/wine-builds/ubuntu/ $(grep VERSION_CODENAME= /etc/os-release | cut -d= -f2) main" >> /etc/apt/sources.list \
    && dpkg --add-architecture i386 \
    && apt-get update \
    && DEBIAN_FRONTEND="noninteractive" apt-get install -y --install-recommends winehq-${WINE_BRANCH} \
    && rm -rf /var/lib/apt/lists/*

# Install winetricks
RUN wget -nv -O /usr/bin/winetricks https://raw.githubusercontent.com/Winetricks/winetricks/master/src/winetricks \
    && chmod +x /usr/bin/winetricks

# Configure locale for unicode
RUN locale-gen en_US.UTF-8
ENV LANG en_US.UTF-8

####### = END INSTALLS = #######

# ARG WWWUSER
# ARG WWWGROUP

# RUN groupadd --force -g $WWWGROUP user
# RUN useradd -ms /bin/bash --no-user-group -g $WWWGROUP -u $WWWUSER user

WORKDIR /usr/src

USER ubuntu

CMD ["tail", "-f", "/dev/null"]
ENTRYPOINT [ "./entrypoint.sh" ]