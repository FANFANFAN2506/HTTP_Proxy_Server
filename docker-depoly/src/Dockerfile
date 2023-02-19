FROM gcc:latest

RUN mkdir /var/log/erss
COPY ../log /var/log/erss
WORKDIR /var/log/erss
RUN chmod o+x ./run.sh
RUN ./run.sh