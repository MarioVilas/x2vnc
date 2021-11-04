Name:          x2vnc
Version:       1.7.3.1
Release:       1%{?dist}
Summary:       Dual screen hack for VNC

Group:         User Interface/X
License:       GPLv2+
URL:           http://fredrik.hubbe.net/x2vnc.html
Source0:       https://github.com/sergiomb2/x2vnc/archive/v%{version}/x2vnc-%{version}.tar.gz
BuildRequires: gcc
BuildRequires: libXxf86dga-devel
BuildRequires: libXrandr-devel
BuildRequires: libXinerama-devel
BuildRequires: libXScrnSaver-devel
BuildRequires: autoconf
BuildRequires: automake

%description
This program will let you use two screens on two different computers
as if they were connected to the same computer even if
one computer runs Windows.

%prep
%autosetup -p1
autoreconf


%build
%configure --without-xf86dga
%make_build


%install
%make_install


%files
%doc README ChangeLog
%license COPYING
%{_bindir}/*
%{_mandir}/man1/*


%changelog
* Thu Nov 04 2021 Sérgio Basto <sergio@serjux.com> - 1.7.3.1-1
- Update x2vnc to 1.7.3.1

* Wed May 27 2020 Sérgio Basto <sergio@serjux.com> - 1.7.3-2
- Fix buid on epel7

* Mon Jul 27 2009 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 1.7.2-11
- Rebuilt for https://fedoraproject.org/wiki/Fedora_12_Mass_Rebuild

* Wed Feb 25 2009 Fedora Release Engineering <rel-eng@lists.fedoraproject.org> - 1.7.2-10
- Rebuilt for https://fedoraproject.org/wiki/Fedora_11_Mass_Rebuild

* Tue Aug 12 2008 Jason L Tibbitts III <tibbs@math.uh.edu> - 1.7.2-9
- Fix %%patch without corresponding "Patch:" tag error which broke the build.

* Tue Aug 12 2008 Jason L Tibbitts III <tibbs@math.uh.edu> - 1.7.2-8
- Fix license tag.

* Tue Feb 19 2008 Fedora Release Engineering <rel-eng@fedoraproject.org> - 1.7.2-7
- Autorebuild for GCC 4.3

* Sun Mar 04 2007 Michael Stahnke <mastahnke@gmail.com> - 1.7.2-6
- Fixed a bug in spec

* Sun Mar 04 2007 Michael Stahnke <mastahnke@gmail.com> - 1.7.2-5
- Cleaned up spec file 

* Sat Mar 03 2007 Michael Stahnke <mastahnke@gmail.com> - 1.7.2-4
- Removed ls and pwd from spec (was used for debugging).

* Sat Feb 24 2007 Michael Stahnke <mastahnke@gmail.com> - 1.7.2-3
- Fixing a few more items presented in Bug #228434.

* Tue Feb 20 2007 Michael Stahnke <mastahnke@gmail.com> - 1.7.2-2
- Fixing Items presented in Bug #228434.

* Tue Feb 12 2007 Michael Stahnke <mastahnke@gmail.com> - 1.7.2-1
- Initial packaging.
