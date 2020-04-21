# IBM Spectrum Symphony OpenShift Operator

IBM Spectrum Symphony OpenShift operator allows you to deploy IBM Spectrum Symphony (https://www.ibm.com/ca-en/marketplace/analytics-workload-management) cluster with either a fixed number of compute nodes or number of relicas controlled by a Horisontal Pod Autoscaler with a CPU threshold. Provided SymphonyCluster API allows to create client pods to submit Symphony workload either using pre-built docker images or build images using OpenShift BuildConfig Dockerfile build from GitHub sources.

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

## Operator architecture

When SymphonyCluster is created the operator creates and monitors children objects. If some objects were deleted, operator will recreate them. Note spec parameters changing (patch) is not supported, create a new SymphonyCluster object if necessary.

All spec parameters except `licenceAccepted` (must be `true`) are optional. Default values will be used for some parameters (see description below).

In addition to single master pod and compute pods, user can define list of client applications (pods) to submit IBM Spectrum Symphony workload.

Here is IBM Spectrum Symphony C++ sample application source code, which is built by the operator to a client image to submit the workload: https://github.com/IBM/ibm-spectrum-symphony-operator/tree/master/samples/sampleapp_cpp

Here is the list of SymphonyCluster children objects created by operator:

- ServiceAccount for running master and compute pods. 
Optional, user can use existing ServiceAccount with additional permissions and secrets.

- Volume claim to store cluster configuration for HA.
Optional, user can use existing (recommended) volume claim.

- Master node pod.
IBM Spectrum Symphony management node acts as the master.

- ConfigMap with IP address of master pod.
This ConfigMap in format of OS hosts file is used to resolve the master node.
Compute and client containers entrypoint scripts must override EGO hosts file periodically with it.

- Compute nodes Deployment.
Created after master pod IP address is known

- Horizontal Pod Autoscaler for compute nodes.
Optional (recommended), allows to automatically scale compute pods in the Symphony cluster depending on workload.

- Service for master pod services.
Optional, created if access to WEB GUI, Symphony and REST APIs are enabled.

- Routes for master por service.
Optional, created for enabled services.

- BuildConfig for client application.
Optional, if client docker image is built from GitHub sources.

- DeploymentConfig for client application.
Optional, created after master pod IP address is known.

- ImageStream for client application.
Optional, if BuildConfig is used to build client application image.

## Images

The operator image is stored in two repositories:

- docker.io/ibmcom/spectrum-symphony:v1

- registry.connect.redhat.com/ibm/spectrum-symphony-operator:v1

Operator definition (ClusterServiceVersion) uses three images for master, compute nodes and client (actually build image). Each image contains two architectures: amd64 and ppc64le and based on RedHat UBI 7 minimal image.

- docker.io/ibmcom/spectrum-symphony:7.3.0.0

- docker.io/ibmcom/spectrum-symphony-comp:7.3.0.0

- docker.io/ibmcom/spectrum-symphony-client:7.3.0.0


## Docker image liveness and readiness probes
Master and compute pods have liviness and rediness probes to check LIM (EGO component) process IP port.

Bash built-in virtual file system `/dev/PROTOCOL/HOST/PORT` is used to poll the port.
The following command returns 0 (no error) if local TCP port 17869 is in listening state.

```bash
$bash </dev/tcp/localhost/17869
```

## SymphonyCluster API

IBM Spectrum Symphony OpenShift operator provides and manages SymphonyCluster object:

```yaml
apiVersion: symphony.spectrumcomputing.ibm.com/v1
kind: SymphonyCluster
```

All parameters in the ``spec`` are optional except ``licenceAccepted`` which must be set to ``true`` to indicate the user accepted terms and conditions of the IBM Spectrum Symphony.

The following example contains all parameters with their default values to be applied if missing:

```yaml
spec:
  licenceAccepted: true
  serviceAccountName: ""
  cluster:
    clusterName: ""
    usersPasswordsSecretName: ""
    entitlementSecretName: ""
    scriptsSecretName: ""
    enableSharedSubdir: false
    logsOnShared: true
    productid: "762afa9e64da4fec89452dd822e63370"
    productname: "IBM Spectrum Symphony Community Edition"
    productversion: "7.3.0.0"
    productmetric: "VIRTUAL_PROCESSOR_CORE"
    productchargedcontainers: ""
    storage:
      pvcName: ""
      pvcSize: "1Gi"
      storageClassName: ""
      selector:
        label: ""
        value: ""
  master:
    image: docker.io/ibmcom/spectrum-symphony:7.3.0.0
    imagePullPolicy: Always
    uiEnabled: true
    egoRestEnabled: false
    symRestEnabled: false
    resources:
      requests:
        cpu: "1000m"
        memory: "4Gi"
      limits:
        cpu: "1000m"
        memory: "4Gi"
    env:
    - name: APP_NAME
      valueFrom:
        fieldRef:
          apiVersion: v1
          fieldPath: metadata.name
    volumes:
    - name: pvc1
      pvcName: sympvc
      mount: "/symPVC"
      readOnly: true
  compute:
    image: docker.io/ibmcom/spectrum-symphony-comp:7.3.0.0
    imagePullPolicy: Always
    replicaCount: 1
    usePodAutoscaler: true
    minReplicas: 1
    maxReplicas: 64
    targetCPUUtilizationPercentage: 70
    resources:
      requests:
        cpu: "250m"
        memory: "1Gi"
      limits:
        cpu: "250m"
        memory: "1Gi"
    env:
    - name: APP_NAME
      valueFrom:
        fieldRef:
          fieldPath: metadata.name
    volumes:
    - name: pvc1
      pvcName: sympvc
      mount: "/symPVC"
      readOnly: true
  client:
    - name: SampleAppCPP1
      serviceAccountName: ""
      image: ""
      imagePullPolicy: Always
      build:
        git:
          repository: https://github.com/IBM/ibm-spectrum-symphony-operator.git
          branch: master
          path: samples/sampleapp_cpp
        image: docker.io/ibmcom/spectrum-symphony-client:7.3.0.0
        serviceAccountName: ""
        resources:
          requests:
            cpu: "500m"
            memory: "2Gi"
          limits:
            cpu: "500m"
            memory: "2Gi"
      resources:
        requests:
          cpu: "250m"
          memory: "1Gi"
        limits:
          cpu: "250m"
          memory: "1Gi"
      env:
      - name: APP_NAME
        valueFrom:
          fieldRef:
            fieldPath: metadata.name
      volumes:
      - name: pvc1
        pvcName: sympvc
        mount: "/symPVC"
        readOnly: true
```

``serviceAccountName`` parameter allows you to use existing ServiceAccount with extra Roles and/or Secrets.

## Cluster
Cluster parameters define IBM Spectrum Symphony cluster specific configuration

```yaml
cluster:
  clusterName: ""
  usersPasswordsSecretName: ""
  entitlementSecretName: ""
  scriptsSecretName: ""
  enableSharedSubdir: false
  logsOnShared: true
  productid: "762afa9e64da4fec89452dd822e63370"
  productname: "IBM Spectrum Symphony Community Edition"
  productversion: "7.3.0.0"
  productmetric: "VIRTUAL_PROCESSOR_CORE"
  productchargedcontainers: ""
  storage:
    pvcName: ""
    pvcSize: "1Gi"
    storageClassName: ""
    selector:
      label: ""
      value: ""
```

### Product annotations
Below are product annotation fields for OpenShift metric system. If `cluster.entitlementSecretName` is set, the defaults will be changed to Advanced Edition values. Those fields are not presented in the example and hidden from CSV because software charge is calculated using those fields, still they are configurable for flexibility.

```yaml
cluster:
  productid: "762afa9e64da4fec89452dd822e63370"
  productname: "IBM Spectrum Symphony Community Edition"
  productversion: "7.3.0.0"
  productmetric: "VIRTUAL_PROCESSOR_CORE"
  productchargedcontainers: ""
```

## Environment variables

- Common (all pods)

```bash
LICENSE=accept
```

Must be set to accept terms and conditions, otherwise the bootstrap script will exit.

```bash
SHARED_TOP_SUBDIR=""
```

If `cluster.logsOnShared=true` this env is set to release name and creates subdirectory in the /shared.

```bash
LOGS_ON_SHARED="Y"
```

Asks master and compute to change local configuration files to save logs on the shared directory /shared/${SHARED_TOP_SUBDIR}/logs

- Master

```bash
HOST_ROLE=MANAGEMENT
```

```bash
GENERATE_SSL_CERT="Y"
```

Regenerate built-in certificate for SSL communication. It's a self-signed certificate.

```bash
CLUSTER_NAME=""
```

Comes from `cluster.clusterName` or release name. Sets Symphony internal cluster name.

- Compute

```bash
HOST_ROLE=COMPUTE
```

- Client

```bash
HOST_ROLE=CLIENT
```

### Users Passwords
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

## Volumes
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

## Master

## Compute

## Client

### Considerations for client script

Client script on the image must take care of resolving master host and deploy the application.
Master host IP address is mounted to ``/opt/ibm/spectrumcomputing/scripts/configmap/hosts`` and could be updated if changed.
Make sure the scrip waits until the IP address is known and copies it to EGO hosts file:

```bash
$cp /opt/ibm/spectrumcomputing/scripts/configmap/hosts /opt/ibm/spectrumcomputing/kernel/conf/hosts 
```

### Using prebuild client image to submit workload

Client image could be built outside of OCP and provided to client.Image parameter.
In this case no build objects will be created, only DeploimentConfig to submit workload
Here is an example how to build the sample:

- Get sources: https://github.com/IBM/ibm-spectrum-symphony-operator/tree/master/samples/sampleapp_cpp

- Create your public repository: quay.io/ibm-spectrum-symphony/sampleapp_cpp

- Build and push image

```bash
$cd sym-ocp/samples/sampleapp_cpp
$docker build -t quay.io/ibm-spectrum-symphony/sampleapp_cpp:latest .
$docker push quay.io/ibm-spectrum-symphony/sampleapp_cpp:latest
```

- Use the new image for your client:

```yaml
  client:
    - name: SampleAppCPP1
      image: quay.io/ibm-spectrum-symphony/sampleapp_cpp:latest
```

