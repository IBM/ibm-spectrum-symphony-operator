SYM_IMAGE=docker.io/ibmcom/spectrum-symphony:7.3.0.0

CLUSTER_NAME=$(shell id -un)-local
CLUSTER_LABEL=cluster=$(CLUSTER_NAME)
CLUSTER_SHARED_DIR=$$PWD/shared

# configure your cluster automatically with scripts
SCRIPTS=scripts.tar.gz

# cluster must be in a separate network
DOCKER_ARGS_NETWORK=--network $(CLUSTER_NAME)
#DOCKER_ARGS_CAP=--cap-drop=ALL
DOCKER_ENV_PARAMS=-e LICENSE=accept -e LOGS_ON_SHARED=Y -e SHARED_TOP_SUBDIR=$(CLUSTER_NAME)
DOCKER_ARGS_COMMON=${DOCKER_ARGS_CAP} -u $(shell id -u):0 ${DOCKER_ENV_PARAMS}
#DOCKER_ARGS_CPU=--cpu-period=100000 --cpu-quota=100000
DOCKER_ARGS_SCRIPTS=-v $$PWD/${SCRIPTS}:/opt/ibm/spectrumcomputing/scripts/config/scripts.tar.gz
DOCKER_ARGS=${DOCKER_ARGS_COMMON} ${DOCKER_ARGS_CPU} ${DOCKER_ARGS_SCRIPTS} ${DOCKER_ARGS_NETWORK} -l ${CLUSTER_LABEL} -v ${CLUSTER_SHARED_DIR}:/shared 


.PHONY: start kill info prepare start_node

all: start


# start a local cluster
start: prepare scripts
	@$(MAKE) ROLE=MANAGEMENT HOSTN="-h master" start_node 
	@$(MAKE) ROLE=COMPUTE start_node 
	@$(MAKE) ROLE=CLIENT HOSTN="-h client" start_node 

# kill the local cluster
kill:
	@echo "Killing $(CLUSTER_NAME) cluster"
	@docker ps -q -f label=${CLUSTER_LABEL} | xargs docker rm -f
	@echo Delete network $(CLUSTER_NAME)
	@docker network rm $(CLUSTER_NAME) | true
	@echo Remove directory ${CLUSTER_SHARED_DIR}
	@rm -rf ${CLUSTER_SHARED_DIR} | true

# get container ids
info:
	@echo master: `docker ps -q -f label=${CLUSTER_LABEL} -f label=role=MANAGEMENT`
	@echo compute: `docker ps -q -f label=${CLUSTER_LABEL} -f label=role=COMPUTE`
	@echo client: `docker ps -q -f label=${CLUSTER_LABEL} -f label=role=CLIENT`

prepare: kill
	@echo Recreating the shared dir ${CLUSTER_SHARED_DIR}
	@mkdir -p ${CLUSTER_SHARED_DIR}
	@chmod o+rwx ${CLUSTER_SHARED_DIR}
	@echo Create network $(CLUSTER_NAME)
	@docker network create $(CLUSTER_NAME) | true

start_node:
	@echo Starting ${ROLE}
	@docker run -P -d ${DOCKER_ARGS} ${HOSTN} -e HOST_ROLE=${ROLE} -l role=${ROLE} ${SYM_IMAGE}

scripts:
	@cd scripts; tar czf ../${SCRIPTS} *

get_master_id:
	@docker ps -q -f label=${CLUSTER_LABEL} -f label=role=MANAGEMENT

get_client_id:
	@docker ps -q -f label=${CLUSTER_LABEL} -f label=role=CLIENT

master_rsh:
	@docker exec -ti `make get_master_id` bash

client_rsh_root:
	@docker exec -ti -u 0:0 `make get_client_id` bash
