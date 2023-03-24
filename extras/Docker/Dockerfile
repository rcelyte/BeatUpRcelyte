FROM debian:bullseye-slim

RUN useradd beatup

COPY --chown=beatup --chmod=777 beatupserver /usr/bin

USER beatup

EXPOSE 2328
EXPOSE 5000

ENTRYPOINT "/usr/bin/beatupserver"