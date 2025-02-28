prepare() {
  echo "no patches found"
}

build() {
  cd "$dendro_buildroot"
  ./autogen.sh
  ./configure --prefix=/usr --with-crypto=libgcrypt
  make
}

package() {
  cd "${pkgname}"
  make DESTDIR="$dendro_installloc" install install_python

  # install bash completion (FS#44618)
  /usr/bin/install -Dm644 btrfs-completion "${dendro_installloc}/usr/share/bash-completion/completions/btrfs"

  # install mkinitcpio hooks
  cd "${srcdir}"
  /usr/bin/install -Dm644 initcpio-install-btrfs "${dendro_installloc}/usr/lib/initcpio/install/btrfs"
  install -Dm644 initcpio-hook-btrfs "${dendro_installloc}/usr/lib/initcpio/hooks/btrfs"

  # install scrub service/timer
  /usr/bin/install -Dm644 btrfs-scrub@.service "${dendro_installloc}/usr/lib/systemd/system/btrfs-scrub@.service"
  /usr/bin/install -Dm644 btrfs-scrub@.timer "${dendro_installloc}/usr/lib/systemd/system/btrfs-scrub@.timer"
}

