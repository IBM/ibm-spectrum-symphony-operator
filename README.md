# IBM Spectrum Symphony OpenShift Operator

## Table of Contents
1. [Symphony OpenShift Operator](#operator)
2. [Changelog](#changelog)
3. [Operator architecture](#architecture)
    - [SymphonyCluster children objects](#children)
    - [Containers environment variables](#env)
4. [SymphonyCluster API](#api)
    - [Product annotations](#product)
    - [Symphony Entitlement](#entitlement)
    - [Users Passwords](#passwords)
    - [External scripts](#scripts)
    - [Volumes](#volumes)
    - [Considerations for client script](#clnscripts)
    - [Using prebuild client image to submit workload](#clnprebuild)
5. [Miscellaneous](#miscellaneous)
    - [Using ImageStreams](#imagestreams)

## Symphony OpenShift Operator <a name="operator"></a>

IBM Spectrum Symphony OpenShift operator allows you to deploy IBM Spectrum Symphony <https://www.ibm.com/ca-en/marketplace/analytics-workload-management> cluster with either a fixed number of compute nodes or number of relicas controlled by a Horisontal Pod Autoscaler with a CPU threshold. Provided SymphonyCluster API allows to create client pods to submit Symphony workload either using pre-built docker images or build images using OpenShift BuildConfig Dockerfile build from GitHub sources.

The current operator version has the following limitations:

- It's not recommended to use OpenShift Form UI but use YAML editing instead

- The operator does not support patch, do not change SymphonyCluster parameters after the creation

Symphony cluster is running in a simplified WEM mode not allowing users impersonation. There are no root processes in the pods, all container capabilities are dropped for the maximum security. Each pod has resource requirements and limits for beter resource utilization. Exteranl communication uses HTTPS protocol for security.

There is a single Symphony management node acted as Symphony master. Cluster is configured for high availability (HA) and master recovery is done through Kubernetes pod recovery.

The Symphony master host has a hard coded hostname `master`. The operator creates Kubernetes ConfigMap with IP address of the master host in format of OS hosts file and pods scripts use it to resolve the master.

IBM Spectrum Symphony image comes with pre-installed Community Edition entitlement which allowes to use most of IBM Spectrum Symphony features with some limitations, for example, usage of the maximum of 64 cores. SymphonyCluster API allows to install different entitlement during installation.

To create SymphonyCluster object you must set spec parameter `licenceAccepted` to `true` to indicate you accept terms and conditions of IBM Spectrum Symphony.

Using Kubernetes Service and OpenShift Router master pod provides access to IBM Spectrum Symphony Management console (WEB GUI) as well as Symphony and EGO REST APIs.

Compute and client pods could mount existing Kubernetes volume claims to exchange workload information.

All created Kubernetes objects are properly labeled and annotated to use for monitoring, metering and audit.

## Changelog <a name="changelog"></a>

- Version 1.1.0 (November 2020) has the following improvements and bugfixes:
  - IBM Spectrum Symphony version 7.3.1
  - Added Deploment for extra management hosts (default is zero pods)
  - Smaller image for compute hosts
  - No samples on the compute image, use the namagement image for the build
  - Operator sets environment variables to containers to help with scripting
  - Renaming objects, replacing master with primary
  - The primary namagement hostname changed from master to primary
  - New option (cluster.cacheImages) to use local repository for management and compute pods
  - Bugfix: operator missed primary management pod monitoring, fixes the cluster recovery if pod is killed
  - Bugfix: build used client's service account
  - Bugfix: client used array of environment variables from master parameter

## Operator architecture <a name="architecture"></a>

When SymphonyCluster is created the operator creates and monitors children objects. If some objects were deleted, operator will recreate them. Note spec parameters changing (patch) is not supported, create a new SymphonyCluster object if necessary.

All spec parameters except `licenceAccepted` (must be `true`) are optional. Default values will be used for some parameters (see description below).

### SymphonyCluster children objects <a name="children"></a>

Here is the list of SymphonyCluster children objects created by operator. When created, uses release name of SymphonyCluster, adds `-master`, `-compute` or other suffixes for bette naming.

- `ServiceAccount` for running master and compute pods.
Optional, user can use existing ServiceAccount with additional permissions and secrets.

- `PersistentVolumeClaim` to store cluster configuration for HA.
Optional, user can use existing (recommended) volume claim. This volume is used to store cluster configuration and required for the Symphony cluster recovery. Also logs from master and comute nodes could be stored there if `cluster.logsOnShared=true`. If `cluster.enableSharedSubdir=true` then the release name subdirectory is created and used as the top directory for files.

- Master node `Pod`.
IBM Spectrum Symphony management node acts as the master.

- `ConfigMap` with IP address of master pod.
This ConfigMap in format of OS hosts file is used to resolve the master node.
Compute and client containers entrypoint scripts must override EGO hosts file periodically with it.

- Compute nodes `Deployment`.
Created after master pod IP address is known and the ConfigMap is created. This Deployemnt controls pod nodes.

- `HorizontalPodAutoscaler` for compute nodes.
Optional (recommended), allows to automatically scale compute pods in the Symphony cluster depending on workload.

- `Service` for master pod services.
Optional, created if access to WEB GUI, Symphony and REST APIs are enabled.

- `Routes` for master por service.
Optional, created for enabled services.

- `BuildConfig` for client application.
Optional, if client docker image is built from GitHub sources.

- `DeploymentConfig` for client application.
Optional, created after master pod IP address is known.

- `ImageStream` for client application.
Optional, if BuildConfig is used to build client application image.

In addition to single master pod and compute depolyment (which controls compute pods), user can define list of client applications (pods) to Symphony workload. Here is IBM Spectrum Symphony C++ sample application source code, which is built by the operator to a client image to submit the workload: <https://github.com/IBM/ibm-spectrum-symphony-operator/tree/master/samples/sampleapp_cpp>.

SymphonyCluster `status` consist of the master host IP address. When it's changed (master pod was restarted) the ConfigMap is updated and all pods will get the new IP in the mounted file. Compute nodes entrypoint script (bootstrap.sh) monitors the ConfigMap file changes and update EGO hosts file, that make IBM Spectrum Symphony software to be able to resolve master host. Client images must implement the same functionality, here is an implementation example: <https://github.com/IBM/ibm-spectrum-symphony-operator/blob/master/samples/sampleapp_cpp/src/Output/run>.

### Containers environment variables <a name="env"></a>

Starting version 1.1.0 Symphony OpenShift operator sets the following environment variables to containers:

Information about the Symphony Cluster Kubernetes object:
```
SOAM_OPENSHIFT_RELEASE_APIVERSION=symphony.spectrumcomputing.ibm.com/v1
SOAM_OPENSHIFT_RELEASE_KIND=SymphonyCluster
SOAM_OPENSHIFT_RELEASE_NAME=symcluster
SOAM_OPENSHIFT_NAMESPACE=default
```

Client image:
```
SOAM_OPENSHIFT_COMPUTE_IMAGE=docker.io/ibmcom/spectrum-symphony:7.3.0.0
```

Information used in the metering system (set as annotation):
```
SOAM_OPENSHIFT_PRODUCT_CHARGE=All
SOAM_OPENSHIFT_PRODUCT_ID=28826cfd6dcd4beebca2cb2d9ef0ffe4
SOAM_OPENSHIFT_PRODUCT_METRIC=VIRTUAL_PROCESSOR_CORE
SOAM_OPENSHIFT_PRODUCT_NAME=IBM Spectrum Symphony
SOAM_OPENSHIFT_PRODUCT_VERSION=7.3.0.0
```

Name of the PersistentVolumeClaim used for the shared directory and ServiceAccount used for this pod:
```
SOAM_OPENSHIFT_PVC=pvctest
SOAM_OPENSHIFT_SERVICEACCOUNT=default
```

### Images <a name="images"></a>

The operator image is stored in two repositories:

- docker.io/ibmcom/spectrum-symphony:1.0.0

- registry.connect.redhat.com/ibm/spectrum-symphony-operator:1.0.0

IBM Spectrum Symphony docker image (docker.io/ibmcom/spectrum-symphony:7.3.0.0) could be used for master, compute and client (actually build) containers. The image contains two architectures: amd64 and ppc64le and based on RedHat UBI 7 minimal image. Symphony image entrypoint script (bootstrap.sh) takes care to create Symphony cluster and monitor it. This image also contains IBM Spectrum Symphony SDK to build Symphony application.

Master and compute pods have liviness and rediness probes to check LIM (EGO component) process IP port.

Bash built-in virtual file system `/dev/PROTOCOL/HOST/PORT` is used to poll the port.
The following command returns 0 (no error) if local TCP port 17869 is in listening state.

```bash
$ bash </dev/tcp/localhost/17869
```

## SymphonyCluster API <a name="api"></a>

IBM Spectrum Symphony OpenShift operator provides and manages SymphonyCluster object:

```yaml
apiVersion: symphony.spectrumcomputing.ibm.com/v1
kind: SymphonyCluster
```

All parameters in the ``spec`` are optional except ``licenceAccepted`` which must be set to ``true`` to indicate the user accepted terms and conditions of the IBM Spectrum Symphony.

The following example contains all parameters with their default values to be applied if missing:

```yaml
spec:
  licenceAccepted: true                                         # Indicates user accepted term and conditions IBM Spectrum Symphony license
  serviceAccountName: ""                                        # To run master and compute pods, if empty creates ServiceAccount with release name
  cluster:
    clusterName: ""                                             # If empty release name is used as internal Symphony cluster name
    usersPasswordsSecretName: ""                                # Secret with list of Symphonhy users to create or update and their passwords
    entitlementSecretName: ""                                   # Secret with Symphony entitlement file to replace Symphony Community Edition
    scriptsSecretName: ""                                       # Secret with archive with scripts and binaries to update Symphony cluster configuration
    enableSharedSubdir: false                                   # If true creates release name top directory on the shared directory
    logsOnShared: true                                          # If true  Symphony components store logs on the shared directory
    productid: "762afa9e64da4fec89452dd822e63370"               # Annotation for metric system, changed if custome entitlement is set
    productname: "IBM Spectrum Symphony Community Edition"      # Annotation for metric system, changed if custome entitlement is set
    productversion: "7.3.0.0"                                   # Annotation for metric system, changed if custome entitlement is set
    productmetric: "VIRTUAL_PROCESSOR_CORE"                     # Annotation for metric system, changed if custome entitlement is set
    productchargedcontainers: ""                                # Annotation for metric system, changed if custome entitlement is set
    cacheImages: false                                          # Creates local ImageStrems to use for management and compute pods
    storage:
      pvcName: ""                                               # Use custom volume claim name, if empty creates one
      pvcSize: "1Gi"                                            # Size of volume claim to create
      storageClassName: ""                                      # Optional storage class for existing PVC
      selector:                                                 # Optional selector to exisitng PVC
        label: ""
        value: ""
  master:
    image: docker.io/ibmcom/spectrum-symphony:7.3.0.0           # Docker image for master host
    imagePullPolicy: Always                                     # PullPolicy for the master image
    uiEnabled: true                                             # If enabled, adds WEBGUI port to Service and creates Route
    egoRestEnabled: false                                       # If enabled, adds EGO REST API port to Service and creates Route
    symRestEnabled: false                                       # If enabled, adds SYM REST API port to Service and creates Route
    replicaCount: 0                                             # v1.1.0: Initial number of extra management pods replicas
    resources:                                                  # Resources allocated for master pod, note 4G is minimum for Symphony management host
      requests:
        cpu: "1000m"
        memory: "4Gi"
      limits:
        cpu: "1000m"
        memory: "4Gi"
    env:                                                        # Array of additional environment variables for master host
    - name: APP_NAME
      valueFrom:
        fieldRef:
          apiVersion: v1
          fieldPath: metadata.name
    volumes:                                                    # Array of additioanl PVC for master host
    - name: pvc1
      pvcName: sympvc
      mount: "/symPVC"
      readOnly: true
  compute:
    image: docker.io/ibmcom/spectrum-symphony-comp:7.3.0.0      # Docker image for compute hosts
    imagePullPolicy: Always                                     # PullPolicy for the compute image
    replicaCount: 1                                             # Initial number of compute pods replicas
    usePodAutoscaler: true                                      # Creates HorisontalPodAutoscaler for compute Deployment
    minReplicas: 1                                              # Min replicas for the autoscaler
    maxReplicas: 64                                             # Max replicas for the autoscaler
    targetCPUUtilizationPercentage: 70                          # CPU threshold for the autoscaler
    resources:                                                  # Resources allocated for compute pods
      requests:
        cpu: "250m"
        memory: "1Gi"
      limits:
        cpu: "250m"
        memory: "1Gi"
    env:                                                        # Array of additional environment variables for compute hosts
    - name: APP_NAME
      valueFrom:
        fieldRef:
          fieldPath: metadata.name
    volumes:                                                    # Array of additioanl PVC for compute hosts
    - name: pvc1
      pvcName: sympvc
      mount: "/symPVC"
      readOnly: true
  client:                                                       # Array of client applications configuration
    - name: SampleAppCPP1                                       # Name of application used
      serviceAccountName: ""
      image: ""                                                 # If not empty uses prebuilt client image
      imagePullPolicy: Always
      build:                                                    # Configuration for BuildConfig
        git:
          repository: https://github.com/IBM/ibm-spectrum-symphony-operator.git         # GitHub repository with app sources
          branch: master                                                                # GitHub branch
          path: samples/sampleapp_cpp                                                   # Path to sources in the repository
        image: docker.io/ibmcom/spectrum-symphony:7.3.0.0                               # Image to replace during the build
        serviceAccountName: ""                                                          # Account for build pod
        resources:                                              # Resources allocated for build pod
          requests:
            cpu: "500m"
            memory: "2Gi"
          limits:
            cpu: "500m"
            memory: "2Gi"
      resources:                                                # Resources allocated for client pod
        requests:
          cpu: "250m"
          memory: "1Gi"
        limits:
          cpu: "250m"
          memory: "1Gi"
      env:                                                      # Array of additional environment variables for client host
      - name: APP_NAME
        valueFrom:
          fieldRef:
            fieldPath: metadata.name
      volumes:                                                  # Array of additioanl PVC for client host
      - name: pvc1
        pvcName: sympvc
        mount: "/symPVC"
        readOnly: true
```

### Product annotations <a name="product"></a>

Below are product annotation fields for OpenShift metric system. If `cluster.entitlementSecretName` is set, the defaults will be changed to Advanced Edition values. Those fields are not presented in the example and hidden from CSV because software charge is calculated using those fields, still they are configurable for flexibility.

```yaml
cluster:
  productid: "762afa9e64da4fec89452dd822e63370"
  productname: "IBM Spectrum Symphony Community Edition"
  productversion: "7.3.0.0"
  productmetric: "VIRTUAL_PROCESSOR_CORE"
  productchargedcontainers: ""
```

### Symphony Entitlement <a name="entitlement"></a>

To replace built-in IBM Spectrum Symphony Community Edition entitlement with a commercial one create a secret with entitlement file content and set `cluster.entitlementSecretName=mysym-entitlement` parameter. Note accepting terms and conditions of the IBM Spectrum Symphony will be applied to the new commercial licence. You can find more inforamtion about IBM Spectrum Symphony licences from the [Licence Information](https://www-03.ibm.com/software/sla/sladb.nsf/searchlis/?searchview&searchorder=4&searchmax=0&query=(Spectrum+Symphony+7.3)) link in the operator description.

Here is an example how to create the entitlement secret:

```bash
$ cp sym_adv_entitlement.dat entitlement
$ oc create secret generic mysym-entitlement --from-file=entitlement
secret/mysym-entitlement created
$ rm entitlement
```

### Users Passwords <a name="passwords"></a>

You could change IBM Spectrum Symphony user`s passwords by providing a Kubernetes secret as cluster.usersPasswordsSecretName parameter. The secret will be mounted on master and client hosts at /opt/ibm/spectrumcomputing/scripts/users directory, each user as a separate filename. Your script could decode the password and use it to submit the workload.

Example:

- Encode passwords

```bash
$ echo AdminNewPass | base64
QWRtaW5OZXdQYXNzCg==
$ echo User1NewPass | base64
VXNlcjFOZXdQYXNzCg==
```

- Create secret in OpenShift UI with encoded values (here is the values with salt)

```yaml
kind: Secret
apiVersion: v1
metadata:
  name: symuserspasswords
data:
  Admin: UVdSdGFXNU9aWGRRWVhOekNnPT0=
  User1: VlhObGNqRk9aWGRRWVhOekNnPT0=
type: Opaque
```

- Use secret name in SymphonyCluster creation:

```yaml
cluster:
  usersPasswordsSecretName: "symuserspasswords"
```

- The files are mounted and could be decoded:

```bash
$ ls -l /opt/ibm/spectrumcomputing/scripts/users/
total 0
lrwxrwxrwx. 1 root root 12 Mar 30 21:06 Admin -> ..data/Admin
lrwxrwxrwx. 1 root root 12 Mar 30 21:06 User1 -> ..data/User1
$ cat /opt/ibm/spectrumcomputing/scripts/users/Admin | base64 -d
AdminNewPass
```

### External scripts <a name="scripts"></a>

External scripts feature allows to reconfigure the Symphony cluster and or replace binaries. It's necessary to prepare bash scripts with certain names. Scripts with MANAGEMENT name will be executed on the master host, scripts with name COMPUTE will be executed on computed hosts. Scripts with name `pre` will be executed before Symphony cluster starts and `post` will be executed after the Symphony cluster was started. The scripts archive will be mounted, extracted and executed from /tmp/scripts directory.

Here are scripts names must be used, make sure they have execute permision:

```bash
$ ls -l
total 32
-rwxr-xr--  1 egoadmin  root  32 21 Apr 10:32 COMPUTE_post_start.sh
-rwxr-xr--  1 egoadmin  root  32 21 Apr 10:32 COMPUTE_pre_start.sh
-rwxr-xr--  1 egoadmin  root  32 21 Apr 10:32 MANAGEMENT_post_start.sh
-rwxr-xr--  1 egoadmin  root  32 21 Apr 10:31 MANAGEMENT_pre_start.sh
```

Prepare archive `scripts.tar.gz` (must use this name) with the scripts and any other files:

```bash
$ tar czf scripts.tar.gz *
```

Create Kubernetes secret from the archive:

```bash
$ oc create secret generic my-scripts --from-file=scripts.tar.gz
secret/my-scripts created
```

Use the secret when you create SymphonyCluster: `cluster.scriptsSecretName='my-scripts'`. You will see in container logs the scripts were executed.

### Volumes <a name="volumes"></a>

Master, compute and client pods could mount existing PVC to exchange data.
Note you can mount the same PVC in the container only once.
Volumes are removed from CSV example.

```yaml
    volumes:
    - name: pvc1
      pvcName: sympvc1
      mount: "/symPVC1"
      readOnly: true
    - name: pvc1
      pvcName: sympvc2
      mount: "/symPVC2"
      readOnly: false
```

### Considerations for client script <a name="clnscripts"></a>

Client script on the image must take care of resolving master host and deploy the application.
Master host IP address is mounted to ``/opt/ibm/spectrumcomputing/scripts/configmap/hosts`` and could be updated if changed.
Make sure the script waits until the IP address is known and copies it to EGO hosts file:

```bash
$ cp /opt/ibm/spectrumcomputing/scripts/configmap/hosts /opt/ibm/spectrumcomputing/kernel/conf/hosts
```

### Using prebuild client image to submit workload <a name="clnprebuild"></a>

Client image could be built outside of OCP and provided to client.Image parameter. In this case no build objects will be created, only DeploimentConfig to submit workload.

Here is an example how to pre-build IBM Spectrum Symphony C++ sample application image:

- Get sources: <https://github.com/IBM/ibm-spectrum-symphony-operator/tree/master/samples/sampleapp_cpp>

- Create your public repository: quay.io/ibm-spectrum-symphony/sampleapp_cpp

- Build and push image

```bash
$ cd sym-ocp/samples/sampleapp_cpp
$ docker build -t quay.io/ibm-spectrum-symphony/sampleapp_cpp:latest .
$ docker push quay.io/ibm-spectrum-symphony/sampleapp_cpp:latest
```

- Use the new image for your client:

```yaml
  client:
    - name: SampleAppCPP1
      image: quay.io/ibm-spectrum-symphony/sampleapp_cpp:latest
```

## Miscellaneous <a name="miscellaneous"></a>

Here is just some miscellaneous and best practicies information.

### Using ImageStreams <a name="imagestreams"></a>

(v1.0.0) If pulling symphony images from external repository is slow you can setup internal ImageStream object to reference to the external repository, but it will be cached inside your OpenShift cluster. Then you can use ImageStream name duering your cluster creation.

For example instead of using 
```master.image=very-slow-repository.com/spectrum-symphony:7.3.1.0```, create ImageStream ```spectrum-symphony``` (see the yaml below) and use it: ```master.image=spectrum-symphony:7.3.1.0```

```yaml
kind: ImageStream
apiVersion: image.openshift.io/v1
metadata:
  name: spectrum-symphony
spec:
  lookupPolicy:
    local: true
  tags:
    - name: 7.3.1.0
      from:
        kind: DockerImage
        name: >-
          very-slow-repository.com/spectrum-symphony:7.3.1.0
      importPolicy:
        scheduled: true
      referencePolicy:
        type: Local
```

Starting version 1.1.0 there is a new option cluster.cacheImages=true (deafult is false) which will automatically create ImageStreams for management and compute nodes.
