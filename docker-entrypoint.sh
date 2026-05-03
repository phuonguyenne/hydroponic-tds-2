#!/bin/bash
set -e
LISTEN_PORT="${PORT:-80}"

# Enforce a single MPM at container start (belt-and-suspenders vs. bad layers/cache).
rm -f /etc/apache2/mods-enabled/mpm_event.load \
      /etc/apache2/mods-enabled/mpm_event.conf \
      /etc/apache2/mods-enabled/mpm_worker.load \
      /etc/apache2/mods-enabled/mpm_worker.conf 2>/dev/null || true
a2dismod -f mpm_event 2>/dev/null || true
a2dismod -f mpm_worker 2>/dev/null || true
a2enmod mpm_prefork 2>/dev/null || true

# Only retarget the main HTTP listener — do not rewrite every "Listen" line
# (ports.conf often also declares 443; rewriting all would duplicate the app port).
if [ -f /etc/apache2/ports.conf ]; then
  sed -ri \
    -e "s/^Listen 80\>/Listen ${LISTEN_PORT}/" \
    -e "s/^Listen 0\\.0\\.0\\.0:80\>/Listen ${LISTEN_PORT}/" \
    -e "s/^Listen \\[::\\]:80\>/Listen [::]:${LISTEN_PORT}/" \
    /etc/apache2/ports.conf
fi

if [ -f /etc/apache2/sites-available/000-default.conf ]; then
  sed -ri \
    -e "s/<VirtualHost \\*:80>/<VirtualHost *:${LISTEN_PORT}>/" \
    -e "s/<VirtualHost \\*:0\\.0\\.0\\.0:80>/<VirtualHost *:${LISTEN_PORT}>/" \
    /etc/apache2/sites-available/000-default.conf
fi

exec apache2-foreground
