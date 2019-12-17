# Name

This repository demonstrates integration of IBM&reg; Spectrum Symphony Community Edition with RedHat Openshift.

IBM&reg; Spectrum Symphony (https://www.ibm.com/us-en/marketplace/analytics-workload-management) is an enterprise-class workload manager for compute- and data-intensive workload on a scalable, shared grid. It provides an efficient computing environment for dozens of distributed parallel applications to deliver faster results and better resource utilization.
The IBM&reg; Spectrum Symphony Community Edition provides the full functionality of IBM&reg; Spectrum Symphony for a cluster of up to 64 cores.

To deploy IBM&reg; Spectrum Symphony Community Edition cluster you must accept [licensing terms and conditions](https://www.ibm.com/developerworks/community/wikis/home?lang=en#!/wiki/W603320068720_4115_89b9_817244550810/page/Licenses). See ``Installing`` section for the details.

# Prerequisites
- OpenShift 4.2

# Symphony Application template
ibm-spectrum-symphony-app-template.yaml provides Openshift application template with Symphony application source code built from github repository.

samples/sampleapp_cpp directory has Symphony C++ sample application which uses docker build strategy to create and deploy new application version.

Symphony cluster has the following pods:
* Master pod - Symphony master node to manage Symphony cluster
* Compute pods - compute nodes, scalable with HorizontalPodAutoscaler
* Client pod - docker built from git sources to deploy and submit Symphony application workload
* Build pod - build sources for the client pod

Dynamic PersistentVolumeClaim is created to store Symphony cluster data. It requires ReadWriteMany access mode.

Route is created to access Symphony WEBGUI.

Symphony cluster is created with the default user names and passwords.

## Installing

* Login to your Openshift cluster
```bash
oc login
```
* Create application template for Symphony
```bash
oc create -f ibm-spectrum-symphony-app-template.yaml
```
* Add new "IBM&reg; Spectrum Symphony Application" (template instance) from the catalog
* You must enter "accept" value for the LICENSE parameter to accept terms and conditions of IBM&reg; Spectrum Symphony Community Edition.

## Uninstalling

* Delete the application template
```bash
oc delete template ibm-spectrum-symphony-app
```
* Find and delete all TemplateInstances of the Symphony applications created

