Name:           zlog
Version:        1.2.8
Release:        1
Summary:        zlog logger framework

License:        LGPL
URL:            http://hardysimpson.github.io/zlog/

BuildRequires:  gcc make

%define _builddir %(echo $PWD)

%description
AIRTAME Web Service is powering the setup page.

%build

make clean all 

%install
install -d '%{buildroot}/usr/include'
install -d '%{buildroot}/usr/lib'
cp src/zlog.h '%{buildroot}/usr/include/zlog.h'
cp src/libzlog.a '%{buildroot}/usr/lib/libzlog.a'
cp src/libzlog.so.1.2 '%{buildroot}/usr/lib/libzlog.so.1.2'
ln -sf /usr/lib/libzlog.so.1.2 '%{buildroot}/usr/lib/libzlog.so.1'
ln -sf /usr/lib/libzlog.so.1.2 '%{buildroot}/usr/lib/libzlog.so'

%clean

%files
/usr/include/zlog.h
/usr/lib/libzlog.a
/usr/lib/libzlog.so
/usr/lib/libzlog.so.1
/usr/lib/libzlog.so.1.2


