FROM php:8.2-apache

RUN docker-php-ext-install mysqli

# Apache must load exactly one MPM; php:apache uses mod_php with prefork.
RUN a2dismod mpm_event mpm_worker 2>/dev/null || true \
 && a2enmod mpm_prefork

WORKDIR /var/www/html

COPY docker-entrypoint.sh /usr/local/bin/docker-entrypoint.sh
RUN chmod +x /usr/local/bin/docker-entrypoint.sh

COPY . /var/www/html

RUN a2enmod rewrite

EXPOSE 80

ENTRYPOINT ["docker-entrypoint.sh"]