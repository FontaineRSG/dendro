[main-pkg]

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

    [pre-build-commands]
        autogen
        some_command
    [/pre-build-commands

    [build-commands]
        ./configure
        make
        make install DESTDIR=&dendro_buildroot/install
        libtool --finish &dendro_buildroot/install
    [/build-commands]

    [post-build-commands]
        ls &dendro_buildroot/install
    [/post-build-commands

    [pre-install-script]
        echo "installing systemd"
    [/pre-install-script]
[/main-pkg]

[variant:systemd-sysvcompat]   <-variant
    [metadata]
    name: "systemd-sysvcompat"
    author: "redhat"
    arch: "x86_64"
    version: 237
    build_deps: {some,build,deps}
    description: "systemd init system libs"
    license: "LGPL"
    sources: {"tar+source1.org","git+github.com/systemd/systemd","local+/home/cv2/systemd"}

    # anemo metadata
    provides: {systemd.so=1,systemd.lib=257}
    conflicts: {openrc}
    replaces: {openrc}
    protected: true
    deps: {some,deps}
    [/metadata]

[/variant:systemd]

[variant:systemd-libs]  <-variant

[/variant:systemd-libs]
