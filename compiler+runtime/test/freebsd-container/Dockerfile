# FreeBSD can't run in a Linux container, since containers share the host kernel. This
# image instead boots the official FreeBSD cloud image under QEMU and drives it over SSH.
# Use compiler+runtime/bin/freebsd-container-test rather than running this directly.
FROM debian:trixie-slim

RUN apt-get update \
  && apt-get install -y --no-install-recommends \
       ca-certificates curl xz-utils qemu-system-x86 qemu-utils genisoimage \
       openssh-client tar \
  && rm -rf /var/lib/apt/lists/*

COPY entrypoint.sh /usr/local/bin/freebsd-vm-test
ENTRYPOINT ["/usr/local/bin/freebsd-vm-test"]
