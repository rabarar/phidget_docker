FROM python:3.8-buster

WORKDIR /app

COPY entrypoint.sh /app/
COPY libphidgets22 /app/libphidgets22
COPY Phidget22Python /app/Phidget22Python
COPY libphidget22extra /app/libphidget22extra
COPY phidget22admin /app/phidget22admin
COPY *.py /app/


RUN set -ex \
 && chown 1001 /app \
 && apt-get update && apt-get install -y --no-install-recommends avahi-daemon libnss-mdns libusb-1.0-0-dev gcc make vim \
 # allow hostnames with more labels to be resolved. so that we can
 # resolve node1.mycluster.local.
 # (https://github.com/lathiat/nss-mdns#etcmdnsallow)
 && echo '*' > /etc/mdns.allow \
 # Configure NSSwitch to use the mdns4 plugin so mdns.allow is respected
 && sed -i "s/hosts:.*/hosts:          files mdns4 dns/g" /etc/nsswitch.conf \
 && printf "[server]\nenable-dbus=no\n" >> /etc/avahi/avahi-daemon.conf \
 && chmod 777 /etc/avahi/avahi-daemon.conf \
 && mkdir -p /var/run/avahi-daemon \
 && chown avahi:avahi /var/run/avahi-daemon \
 && chmod 777 /var/run/avahi-daemon \
 && pip3 install influxdb_client

WORKDIR /app/libphidgets22 
RUN sh configure \
 && make \
 && make install  \
 && ldconfig

WORKDIR /app/libphidget22extra
RUN sh configure \
 && make \
 && make install  \
 && ldconfig

WORKDIR /app/phidget22admin
RUN sh configure \
 && make \
 && make install  \
 && ldconfig

WORKDIR  /app/Phidget22Python 
 RUN python3 setup.py install 

USER 1001

WORKDIR /app
ENTRYPOINT ["bash", "./entrypoint.sh"]

CMD ["python3"]

