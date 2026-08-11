#!/usr/bin/env bash
docker build -t ssh-forum-v2 . || exit 1
docker run -d -p 6666:2222 \
  --name "analogcity-2026" \
  -v "$(pwd)/boards:/usr/src/app/boards:rw" \
  -v "$(pwd)/boards:/usr/src/app/archive:rw" \
  ssh-forum-v2
