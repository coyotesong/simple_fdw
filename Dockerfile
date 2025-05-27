FROM postgres:17.2-bookworm
LABEL authors="Bear Giles <bgiles@coyotesong.com>"

# can/should be set as build property...
ENV PG_VERS=17
ENV LIBDIR=/usr/lib/postgresql/${PG_VERS}/lib
ENV EXTDIR=/usr/share/postgresql/${PG_VERS}/extension

ENV FDW_NAME=simple_fdw
ENV FDW_VERS=0.0.3

RUN apt-get update && apt-get install -y postgresql-${PG_VERS}-pljava postgresql-${PG_VERS}-pljava-dbgsym

COPY ${FDW_NAME}.so ${LIBDIR}/${FDW_NAME}.so

COPY ${FDW_NAME}.control ${EXTDIR}/
COPY sql/${FDW_NAME}*.sql ${EXTDIR}/

# ENTRYPOINT ["top", "-b"]
