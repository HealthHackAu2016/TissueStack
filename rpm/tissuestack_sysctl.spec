%global _enable_debug_package 0
%global debug_package %{nil}
%global __os_install_post /usr/lib/rpm/brp-compress %{nil}
Name:		tissuestack
Version:	2.3
Release:	0%{?dist}
Summary:	The TissueStack Application

Group:		Graphics
License:	GPLv3+
URL:		https://github.com/NIF-au/TissueStack
Source:		%{name}-%{version}.tar.gz

BuildRequires:	minc GraphicsMagick-devel
Requires:	minc nifticlib GraphicsMagick dcmtk postgresql-server httpd

%description
Tissue Stack is an open source web based image viewer which allows the user to view 3D data as 2D cross sections. While at its core it's modelled after GIS web mapping applications, it's intended use is for neuro-imaging. Tissue Stack aims at serving its data as image tiles which can be both pre-tiled in advance or created on the fly by extracting from the original source file. The used file formats will be anything that is supported by the MINC Tool Kit (http://www.bic.mni.mcgill.ca/ServicesSoftware/ServicesSoftwareMincToolKit) as well as the nifti format(http://nifti.nimh.nih.gov/).

%prep
tar xvzf /tmp/%{name}_build/%{name}-%{version}.tar.gz

%build
mkdir -p %{buildroot}
cp -r * %{buildroot}

%files
/etc/profile.d/*
/usr/local/%{name}/%{version}/*
/opt/%{name}/*

#%doc

%clean
rm -rf /tmp/%{name}_build

%pre
rm -f /tmp/pre-install.log
touch /tmp/pre-install.log
chmod 666 /tmp/pre-install.log
systemctl stop tissuestack &>> /tmp/pre-install.log
systemctl stop httpd  &>> /tmp/uninstall.log
#clean directories that don't contain user data
rm -rf /opt/tissuestack/web/* &>> /tmp/pre-install.log
rm -rf /opt/tissuestack/sql &>> /tmp/pre-install.log
rm -rf /opt/tissuestack/conf &>> /tmp/pre-install.log
exit 0

%preun
rm -f /tmp/uninstall.log
touch /tmp/uninstall.log
chmod 666 /tmp/uninstall.log
if [ $1 -ne 1 ]; then
	systemctl stop tissuestack &>> /tmp/uninstall.log
fi
exit 0

%postun
if [ $1 -ne 1 ]; then
	APACHE_PORT=80
	systemctl disable tissuestack &>> /tmp/uninstall.log
	rm -rf /etc/systemd/system/tissuestack.service &>> /tmp/uninstall.log
	rm -rf /etc/httpd/conf.d/tissuestack.conf &>> /tmp/uninstall.log
	mv /etc/httpd/conf.d/welcome.conf.disabled /etc/httpd/conf.d/welcome.conf &>> /tmp/uninstall.log
	if [ $APACHE_PORT -ne 80 ]; then sed -i "/^Listen[ ]*[0123456789]*/c\Listen 80" /etc/httpd/conf/httpd.conf; fi &>> /tmp/uninstall.log
	rm -rf /tmp/tissue_stack_communication &>> /tmp/uninstall.log
	systemctl restart httpd &>> /tmp/uninstall.log
fi
/sbin/ldconfig
exit 0

%post
rm -f /tmp/post-install.log
touch /tmp/post-install.log
chmod 666 /tmp/post-install.log
useradd -c "tissuestack" -m -d /opt/tissuestack -s /bin/bash -U tissuestack &>> /tmp/post-install.log
chown tissuestack:tissuestack /opt/tissuestack &>> /tmp/post-install.log
for dirs in `find /opt/tissuestack/* -prune -type d`;do
	if  [ $dirs = "/opt/tissuestack/tiles" ]; then
		chown tissuestack:tissuestack $dirs &>> /tmp/post-install.log
	else
		chown -R tissuestack:tissuestack $dirs &>> /tmp/post-install.log
		chown -R -H -h tissuestack:tissuestack $dirs &>> /tmp/post-install.log
	fi
done
su -c "su - postgres <<EOF
initdb &>> /tmp/post-install.log
EOF
"
systemctl enable postgresql &>> /tmp/post-install.log
systemctl start postgresql &>> /tmp/post-install.log
sleep 5s
su -c "su - postgres <<EOF
psql -f /opt/tissuestack/sql/create_tissuestack_db.sql &>> /tmp/post-install.log
psql -f /opt/tissuestack/sql/create_tissuestack_tables.sql tissuestack &>> /tmp/post-install.log
psql -f /opt/tissuestack/sql/patches.sql tissuestack &>> /tmp/post-install.log
psql -f /opt/tissuestack/sql/create_tissuestack_config.sql tissuestack &>> /tmp/post-install.log
psql -f /opt/tissuestack/sql/update_tissuestack_config.sql tissuestack &>> /tmp/post-install.log
EOF
"
APACHE_PORT=80
systemctl enable httpd &>> /tmp/post-install.log
echo "/opt/tissuestack" > /tmp/escaped.string
sed -i 's/\//\\\//g' /tmp/escaped.string &>> /tmp/post-install.log
ESCAPED_STRING=`cat /tmp/escaped.string` &>> /tmp/post-install.log
cp -f /opt/tissuestack/conf/tissuestack.conf /etc/httpd/conf.d/tissuestack.conf &>> /tmp/post-install.log
sed -i "s/##DOC_ROOT##/$ESCAPED_STRING\/web/g" /etc/httpd/conf.d/tissuestack.conf &>> /tmp/post-install.log
sed -i 's/##ERROR_LOG##/\/var\/log\/httpd\/tissuestack-error.log/g' /etc/httpd/conf.d/tissuestack.conf &>> /tmp/post-install.log
if [ $APACHE_PORT -ne 80 ]; then sed -i "/^<VirtualHost[ ]*\*[ ]*:[ 0123456789]*/c\<VirtualHost *:$APACHE_PORT>" /etc/httpd/conf.d/tissuestack.conf; fi &>> /tmp/post-install.log
HTTP_VERSION=`httpd -v | grep "Apache/" | cut -f2 -d "/" | cut -f1 -d " " | cut -f1,2 -d "." | sed 's/\.//g'` &>> /tmp/post-install.log
if [ $HTTP_VERSION -gt 23 ]; then sed -i 's/#Require all granted/Require all granted/g' /etc/httpd/conf.d/tissuestack.conf; fi &>> /tmp/post-install.log
mv /etc/httpd/conf.d/welcome.conf /etc/httpd/conf.d/welcome.conf.disabled &>> /tmp/post-install.log
if [ $APACHE_PORT -ne 80 ]; then sed -i "/^Listen[ ]*[0123456789]*/c\Listen $APACHE_PORT" /etc/httpd/conf/httpd.conf; fi &>> /tmp/post-install.log
sed -i "s/##DOC_ROOT##/$ESCAPED_STRING\/web/g" /etc/httpd/conf.d/tissuestack.conf &>> /tmp/post-install.log
setsebool -P httpd_can_network_connect 1 &>> /tmp/post-install.log
setsebool -P httpd_enable_homedirs 1 &>> /tmp/post-install.log
systemctl restart httpd &>> /tmp/post-install.log
firewall-cmd --zone=public --add-port=$APACHE_PORT/tcp --permanent &>> /tmp/post-install.log
firewall-cmd --reload &>> /tmp/post-install.log
cp -f /opt/tissuestack/conf/tissuestack.service /etc/systemd/system/tissuestack.service &>> /tmp/post-install.log
chmod 664 /etc/systemd/system/tissuestack.service &>> /tmp/post-install.log
echo "/usr/local/tissuestack/%{version}/bin" > /tmp/escaped.string
sed -i 's/\//\\\//g' /tmp/escaped.string &>> /tmp/post-install.log
ESCAPED_STRING=`cat /tmp/escaped.string` &>> /tmp/post-install.log
sed -i "s/##TS_BIN_DIR##/$ESCAPED_STRING/g" /etc/systemd/system/tissuestack.service &>> /tmp/post-install.log
systemctl enable tissuestack &>> /tmp/post-install.log
systemctl start tissuestack &>> /tmp/post-install.log
/sbin/ldconfig
exit 0
