#
# This exposed source code is the confidential and proprietary property of
# IBM Corporation. Your right to use is strictly limited by the terms of the
# license agreement entered into with IBM Corporation. 
#
all:
	cd Common; $(MAKE) all
	cd Service; $(MAKE) all
	cd AsyncClient; $(MAKE) all
	cd Output; tar czf SampleServiceCPP.tgz SampleServiceCPP; rm SampleServiceCPP *.a *.o

clean:
	cd Common; $(MAKE) clean
	cd Service; $(MAKE) clean
	cd AsyncClient; $(MAKE) clean
