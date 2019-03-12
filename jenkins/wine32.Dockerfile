# Requires a decent modern Docker version (v1.10.x at least ideally)

# Use semi-official Arch Linux image with fixed versioning
FROM archlinux/base

# Environment variables
ENV WINEPREFIX /wine32
ENV WINEARCH win32
ENV WINEDEBUG -all

# Install Wine (32-bit)
RUN \
        echo -e "#!/bin/sh\nwine \$@\nretval=\$?\nwineserver -w\nexit \$retval" > /usr/local/bin/wine-wrapper &&\
        chmod +x /usr/local/bin/wine-wrapper &&\
\
        (\
                echo '' &&\
                echo '[multilib]' &&\
                echo 'Include = /etc/pacman.d/mirrorlist'\
        ) >> /etc/pacman.conf &&\
        pacman -Sy --noconfirm \
                awk \
                lib32-gnutls \
                wine \
                wget \
                xorg-server-xvfb \
                pacman-contrib \
                awk \
                &&\
\
        wine-wrapper wineboot.exe -i &&\
        wget -Ovcredist_x86.exe https://download.microsoft.com/download/d/d/9/dd9a82d0-52ef-40db-8dab-795376989c03/vcredist_x86.exe &&\
        WINEDEBUG=+all-trace xvfb-run sh -c 'wine-wrapper vcredist_x86.exe /q' &&\
        rm vcredist_x86.exe &&\
\
        pacman -Rs --noconfirm \
                xorg-server-xvfb \
                wget \
                &&\
\
        find /. -name "*~" -type f -delete &&\
        rm -rf /tmp/* /var/tmp/* /usr/share/man/* /usr/share/info/* /usr/share/doc/* &&\
        pacman -Scc --noconfirm &&\
        rm -rf /var/lib/pacman/sync/*

USER 0
