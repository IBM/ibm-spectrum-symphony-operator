# 
# BuildConfig Docker strategy for Symphony SmapleApp C++ example
#

FROM docker.io/ibmcom/spectrum-symphony-client:7.3.0.0 AS build-env
USER root

RUN microdnf install --nodocs gcc-c++ 

COPY ./src/ /opt/app/

RUN \
       . /opt/ibm/spectrumcomputing/profile.platform \
    && make -C /opt/app all

FROM docker.io/ibmcom/spectrum-symphony-client:7.3.0.0 AS run-env
USER root

COPY --from=build-env /opt/app/Output/ /opt/app/

RUN chown -R 1000:0 /opt/app && chmod -R g=u /opt/app

USER 1000

ENTRYPOINT ["/opt/app/run"]
