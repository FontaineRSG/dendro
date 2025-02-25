[metadata]
name: "acl"
author: "noGNU savannah.org"
arch: "x86_64"
version: 2.3.2
build_deps: {attr}
description: "Access control list utilities, libraries and headers"
license: "LGPL"
source: "http://www.mirrorservice.org/sites/download.savannah.gnu.org/releases/acl/acl-2.3.2.tar.xz"

# anemo metadata
provides: {libacl.so=1-64, xfsacl}
conflicts: {xfsacl}
replaces: {xfsacl}
protected: true
deps: {glibc}
[/metadata]

[build-commands]
./configure
make
make install DESTDIR=&dendro_buildroot/install
libtool --finish &dendro_buildroot/install
[/build-commands]