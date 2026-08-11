FROM --platform=linux/amd64 golang:alpine AS server-builder
WORKDIR /aqui

COPY go.mod go.sum server.go ./
RUN go mod download && go mod verify
RUN go build -v -o ssh-server server.go

FROM --platform=linux/amd64 gcc:13.4.0 AS forum-builder
WORKDIR /aqui

RUN git clone https://github.com/jfs-jfs/cursed-tea lib
RUN cd lib && make && make install

COPY . .
RUN bash compile.sh

FROM --platform=linux/amd64 debian:bookworm-slim AS release

RUN apt-get update && apt-get install -y --no-install-recommends \
    bash \
    ncurses-bin \
    ncurses-term \
    locales \
    && rm -rf /var/lib/apt/lists/*

RUN sed -i 's/^# *en_US.UTF-8 UTF-8/en_US.UTF-8 UTF-8/' /etc/locale.gen \
    && locale-gen en_US.UTF-8

WORKDIR /usr/src/app

COPY --from=server-builder /aqui/ssh-server ./
COPY --from=forum-builder /aqui/forum ./
COPY . .

RUN chmod +x ssh-server forum

ENV TERM=xterm-256color
ENV LANG=en_US.UTF-8
ENV LC_ALL=en_US.UTF-8
ENV LC_CTYPE=en_US.UTF-8

CMD ["./ssh-server"]
