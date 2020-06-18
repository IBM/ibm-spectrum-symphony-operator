# Running IBM Spectrum Symphony CE cluster in docker.
Creates shared directory in work directory, as well as a separate docker network.
Configure your cluster using scripts in scripts directory.
Uses host user id to run Symphony.

Start Symphony cluster
```
make start
```

Kill Symphony cluster
```
make kill
```

Print conainer ids of the cluster
```
make info
```

Docker exec to the master container
```
make master_rsh
```

# Building Symphony app
To install packages must be root user.

## C++ sample build
Go to client container as root
```
make client_rsh_root
```

Install g++
```
microdnf install --nodocs gcc-c++ 
```

Build Sample application
```
cd /opt/ibm/spectrumcomputing/soam/7.3/samples/CPP/SampleApp
make
```

Deploy the application
```
egosh user logon -u Admin -x Admin
egosh consumer add /SampleAppCPP -g ComputeHosts,ManagementHosts -u Guest -e egoadmin
cd Output
tar czf SampleServiceCPP.tgz SampleServiceCPP
soamdeploy add SampleServiceCPP -p SampleServiceCPP.tgz -c /SampleAppCPP -f
soamreg ../SampleApp.xml -f
```

Submit workload
```
./AsyncClient 
```
