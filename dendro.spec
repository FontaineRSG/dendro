[metadata]
name: "package"
author: "elemOS team"
arch: "x86_64"
version: 1.0
build_deps: {glibc<=4.2,test-lib=0.1,test-test>=14.2}
description: "test-package"
license: "GPLv3"

# anemo metadata
provides: {pkg.so=1,pkg-test.so.1}
conflicts: {package-old,test-old}
replaces: {package-old}
protected: true
deps: {yaml-cpp=0.8.0,sqlite3>=3.4.0,glibc>=14.0}
[/metadata]


[pre-build-commands]
make clean
make -j16
make install
[/pre-build-commands]

[post-build-commands]
echo "good!"
chown /test/test.txt
[/post-build-commands]

[pre-install-script]
echo "preparing for install!"
[/pre-install-script]

[post-install-script]
echo "package installed!"
[/post-install-script]

[pre-remove-script]
echo "preparing for removal!"
[/pre-remove-script]

[post-remove-script]
echo "package removed!"
[/post-remove-script]