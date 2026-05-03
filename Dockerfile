FROM --platform=linux/amd64 php:8.2-apache

RUN docker-php-ext-install mysqli

# Apache must load exactly one MPM; php:apache uses mod_php with prefork.
RUN a2dismod mpm_event mpm_worker 2>/dev/null || true \
 && a2enmod mpm_prefork

# Avoid AH00558 "Could not reliably determine the server's FQDN" on some hosts.
RUN printf '\nServerName localhost\n' >> /etc/apache2/apache2.conf

WORKDIR /var/www/html

COPY docker-entrypoint.sh /usr/local/bin/docker-entrypoint.sh
RUN chmod +x /usr/local/bin/docker-entrypoint.sh \
 && sed -i 's/\r$//' /usr/local/bin/docker-entrypoint.sh

COPY . /var/www/html

RUN a2enmod rewrite

EXPOSE 80

ENTRYPOINT ["docker-entrypoint.sh"]
