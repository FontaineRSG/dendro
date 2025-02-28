prepare() {
  cd $dendro_buildroot
  # fix issue with /etc/profile.d/debuginfod.sh for zsh:
  # https://gitlab.archlinux.org/archlinux/packaging/packages/elfutils/-/issues/2
  git cherry-pick -n 00cb3efe36337f27925dbff9b2e7d97c7df95bf8

  # remove failing test due to missing glibc debug package during test: https://bugs.archlinux.org/task/74875
  sed -e 's/run-backtrace-native.sh//g' -i tests/Makefile.am

  autoreconf -fiv
}

build() {
  local configure_options=(
    --prefix=/usr
    --sysconfdir=/etc
    --program-prefix="eu-"
    --enable-deterministic-archives
    --enable-maintainer-mode
  )

  # fat-lto-objects is required for non-mangled .a files in libelf
  CFLAGS+=" -ffat-lto-objects"

  # debugging information is required for test-suite
  CFLAGS+=" -g"

  cd $dendro_buildroot
  ./configure "${configure_options[@]}"
  make
}

check() {
  make -C $dendro_buildroot check
}

_pick() {
  local p="$1" f d; shift
  for f; do
    d="$dendro_buildroot/$p/${f#$dendro_installloc/}"
    mkdir -p "$(dirname "$d")"
    mv "$f" "$d"
    rmdir -p --ignore-fail-on-non-empty "$(dirname "$f")"
  done
}

package_elfutils() {

  make DESTDIR="$dendro_installloc" install -C $dendro_buildroot

  # set the default DEBUGINFOD_URLS environment variable to the distribution's debuginfod URL
  echo "https://debuginfod.archlinux.org" > "$dendro_installloc/etc/debuginfod/archlinux.urls"

  (
    cd "$dendro_installloc"

    _pick libelf etc/debuginfod
    _pick libelf etc/profile.d/*
    _pick libelf usr/{include,lib}
    _pick libelf usr/share/man/*/{,lib}elf*
    _pick debuginfod usr/bin/debuginfod*
    _pick debuginfod usr/share/fish/vendor_conf.d/debuginfod.fish
    _pick debuginfod usr/share/man/*/debuginfod*
  )

  install -vDm 644 $dendro_buildroot/{AUTHORS,ChangeLog,NEWS,NOTES,README} -t "$dendro_installloc/usr/share/doc/elfutils/"
}

package_libelf() {
  mv -v libelf/* "$dendro_installloc"
  install -vDm 644 $dendro_buildroot/{AUTHORS,ChangeLog,NEWS,NOTES,README} -t "$dendro_installloc/usr/share/doc/libelf/"
}

package_debuginfod() {
  mv -v debuginfod/* "$dendro_installloc"
  install -vDm 644 $dendro_buildroot/{AUTHORS,ChangeLog,NEWS,NOTES,README} -t "$dendro_installloc/usr/share/doc/debuginfod/"
}
