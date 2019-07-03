# Maintainer: Sam Stuewe <halosghost at archlinux dot info>

pkgname=dstat-git
pkgver=0
pkgrel=1

pkgdesc="A small status program"
url='https://github.com/HalosGhost/dstat'
arch=('i686' 'x86_64')
license=('GPL2')

optdepends=('xorg-server')
makedepends=('git' 'tup' 'clang')

source=('git+https://github.com/HalosGhost/dstat.git')
sha256sums=('SKIP')

pkgver () {
    cd dstat
    printf '0.r%s.%s' "$(git rev-list --count HEAD)" "$(git log -1 --pretty=format:%h)"
}

prepare () {
    cd dstat
    ./configure --en-iface=enp0s25 --wl-iface=wlp3s0 --pa-sink=1
}

build () {
    cd dstat
    make
}

package () {
    cd dstat
    make DESTDIR="$pkgdir" PREFIX="/usr" install
}
