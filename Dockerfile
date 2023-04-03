# We are going to use the Latest version of Centos
# FROM  ubuntu:latest
FROM ubuntu:22.04

ENV TZ=Europe/Helsinki
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

# install the programs
RUN apt-get -y update
RUN apt-get install -y liblog4cxx-dev
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y apt-utils
RUN apt-get install -y software-properties-common

RUN DEBIAN_FRONTEND=noninteractive apt-get install -y apt-utils
RUN apt-get install -y pip

RUN apt-get install --reinstall ca-certificates
RUN add-apt-repository ppa:ettusresearch/uhd
RUN apt update
RUN apt-get install -y libuhd-dev uhd-host
# CMD /lib/uhd/utils/uhd_images_downloader.py
RUN /usr/lib/uhd/utils/uhd_images_downloader.py

RUN apt-get install -y iputils-ping

EXPOSE 8090/udp

RUN mkdir -p /CPP
COPY ./C/ /CPP/

RUN cd /CPP/Nodes
#CMD g++ -O3 -std=c++17 testStateMachine.cpp -lfftw3f -lfftw3 -luhd -lboost_system -o rxSTM
RUN g++ -O3 -std=c++17 /CPP/Nodes/testStateMachine.cpp -lfftw3f -lfftw3 -luhd -lboost_system -o /rxSTM
#RUN cp /usr/src/CPP/Nodes/rxSTM /rxSTM

RUN pip install imageio
RUN pip install matplotlib
RUN pip install numpy
RUN pip install scipy
RUN pip install tk
RUN apt-get install -y python3-tk

COPY ./Py/ /Py/



