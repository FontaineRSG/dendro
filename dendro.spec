name: "inetutils"
author: "arojas"
arch: "x86_64"
version: 2.5
build_deps: [help2man]
description: "A collection of common network programs"
license: "GPL-3.0-or-later"
sources: ["tar+https://ftp.gnu.org/gnu/inetutils/inetutils-2.5.tar.xz","git+https://gitlab.archlinux.org/archlinux/packaging/packages/inetutils"]
build_script: ""
anemo_script: ""

# anemo metadata
provides: []
conflicts: []
replaces: []
protected: true
deps: ["glibc", "libcap", "libcrypt.so=2-64", "libncursesw.so=6-64", "libpam.so=0-64", "libreadline.so=8-64", "libxcrypt", "ncurses", "pam", "readline"]