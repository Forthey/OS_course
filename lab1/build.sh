#!/usr/bin/env bash
set -e

CONF_FILE="/etc/rsyslog.d/90-disk_monitor.conf"
LOG_FILE="/var/log/disk_monitor.log"

if [ "$EUID" -ne 0 ]; then
  echo "Please run script with root privileges"
  exit 1
fi

echo "Creating $CONF_FILE ..."
cat > "$CONF_FILE" <<EOF
# Logging messages with facility LOG_LOCAL0 in separate file
if \$syslogfacility-text == 'local0' then $LOG_FILE
& stop
EOF

echo "Creating $LOG_FILE ..."
touch "$LOG_FILE"
chown syslog:adm "$LOG_FILE"
chmod 640 "$LOG_FILE"

echo "Restarting rsyslog ..."
systemctl daemon-reload
systemctl restart rsyslog

echo "Creating build directory ..."
mkdir -p build
cd build

echo "Building project ..."
cmake ../ && make -j "$(nproc)"

echo "Preparing executable ..."
cd ..
mkdir -p bin
mv build/bin/disk_monitor bin/
cp config/* bin/

echo "Removing build files ..."
rm -rf build

echo "Fixing permissions ..."
OWNER="$SUDO_USER"
GROUP=$(id -gn "$SUDO_USER")

chown -R "$OWNER:$GROUP" bin
chmod 700 bin
chmod u+x bin/disk_monitor
chmod 600 bin/config.yaml

