Name:           gummi
Version:        0.8.1
Release:        1%{?dist}
Summary:        A simple LaTeX editor

License:        MIT
URL:            https://github.com/alexandervdm/gummi
Source0:        https://github.com/alexandervdm/%{name}/releases/download/%{version}/%{name}-%{version}.tar.gz

BuildRequires:  gtk3-devel
BuildRequires:  gtksourceview3-devel
BuildRequires:  poppler-glib-devel
BuildRequires:  gtkspell3-devel
BuildRequires:  intltool
BuildRequires:  desktop-file-utils
BuildRequires:  texlive-lib-devel

Requires:       texlive-latex

%description
Gummi is a LaTeX editor written in the C programming language using the
GTK+ interface toolkit. It was designed with simplicity and the novice
user in mind, but also offers features that speak to the more advanced user.

%prep
%setup -q

%build
%configure
make %{?_smp_mflags}

%install
make install DESTDIR=%{buildroot} INSTALL="install -p"
%find_lang %{name}
desktop-file-install                                 \
    --remove-key="Version"                           \
    --add-category="Publishing;"                     \
    --dir=%{buildroot}%{_datadir}/applications       \
    %{buildroot}%{_datadir}/applications/%{name}.desktop

%files -f %{name}.lang
%doc AUTHORS ChangeLog COPYING NEWS
%{_mandir}/man*/*.1*
%{_bindir}/%{name}
%{_datadir}/applications/%{name}.desktop
%{_datadir}/pixmaps/%{name}.png
%{_datadir}/%{name}/
%{_libdir}/%{name}/

%changelog
* Tue Mar 3 2020 Adam Boutcher <adam.j.boutcher@durham.ac.uk> - 0.8.1
- Build for CentOS8 - Please see GitHub for true Changelog
