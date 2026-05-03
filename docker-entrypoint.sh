#!/bin/bash
set -e
LISTEN_PORT="${PORT:-80}"
sed -ri "s/^Listen .*/Listen ${LISTEN_PORT}/" /etc/apache2/ports.conf
sed -ri "s/<VirtualHost \*:80>/<VirtualHost *:${LISTEN_PORT}>/" /etc/apache2/sites-available/000-default.conf
exec apache2-foreground