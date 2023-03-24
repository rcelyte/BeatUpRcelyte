FROM debian:bullseye-slim

RUN apt-get install -y libssl1.1

RUN useradd beatup

COPY --chown=beatup --chmod=777 beatupserver /bin

USER beatup

EXPOSE 2328
EXPOSE 5000

ENTRYPOINT "/bin/beatupserver"