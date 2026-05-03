FROM --platform=linux/amd64 php:8.2-apache

RUN docker-php-ext-install mysqli

# Apache must load exactly one MPM (mod_php requires prefork).
# Remove conflicting symlinks + force-disable, then enable only prefork.
RUN set -eux; \
    rm -f /etc/apache2/mods-enabled/mpm_event.load \
          /etc/apache2/mods-enabled/mpm_event.conf \
          /etc/apache2/mods-enabled/mpm_worker.load \
          /etc/apache2/mods-enabled/mpm_worker.conf; \
    a2dismod -f mpm_event 2>/dev/null || true; \
    a2dismod -f mpm_worker 2>/dev/null || true; \
    a2dismod -f mpm_prefork 2>/dev/null || true; \
    a2enmod mpm_prefork

# Avoid AH00558 "Could not reliably determine the server's FQDN" on some hosts.
RUN printf '\nServerName localhost\n' >> /etc/apache2/apache2.conf

WORKDIR /var/www/html

COPY docker-entrypoint.sh /usr/local/bin/docker-entrypoint.sh
RUN chmod +x /usr/local/bin/docker-entrypoint.sh \
 && sed -i 's/\r$//' /usr/local/bin/docker-entrypoint.sh

COPY . /var/www/html

RUN a2enmod rewrite \
 && apache2ctl configtest

EXPOSE 80

ENTRYPOINT ["docker-entrypoint.sh"]
