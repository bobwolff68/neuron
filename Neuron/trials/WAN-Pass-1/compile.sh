NDDSCORE=lnddscore
NDDSC=lnddsc
NDDSCPP=lnddscpp
RTIMONITORING=lrtimonitoring
DEBUGFLAG='-DNDEBUG'
BINDIR=bin_release
if [ $1 = DEBUG ]; then
	NDDSCORE=lnddscored
	NDDSC=lnddscd
	NDDSCPP=lnddscppd
	RTIMONITORING=lrtimonitoringd
	DEBUGFLAG=
	BINDIR=bin_debug
fi

if [ -d $BINDIR ]; then
	mkdir $BINDIR
fi

echo "----------------------------------------------------"
echo "Libraries: $NDDSCORE,$NDDSC,$NDDSCPP,$RTIMONITORING"
echo "Debug: $DEBUGFLAG"
echo "Binary Directory: $BINDIR"
echo "----------------------------------------------------"

g++ -g3 -DRTI_UNIX -DRTI_LINUX $DEBUGFLAG -I$NDDSHOME/include -I$NDDSHOME/include/ndds -L$NDDSHOME/lib/i86Linux2.6gcc4.1.1 -ldl -lnsl -lm -lncurses -lpthread -lrt -$NDDSCORE -$NDDSC -$NDDSCPP -$RTIMONITORING -o $BINDIR/cfc_pub cfc_publisher.cxx cfcSupport.cxx cfcPlugin.cxx cfc.cxx parsecmd.cpp anyoption/anyoption.cpp cfc_transport_config.cpp NeuronBasics/ThreadMultiple.cpp choices.cpp

g++ -g3 -DRTI_UNIX -DRTI_LINUX $DEBUGFLAG -I$NDDSHOME/include -I$NDDSHOME/include/ndds -L$NDDSHOME/lib/i86Linux2.6gcc4.1.1 -ldl -lnsl -lm -lncurses -lpthread -lrt -$NDDSCORE -$NDDSC -$NDDSCPP -$RTIMONITORING -o $BINDIR/cfc_sub cfc_subscriber.cxx cfcSupport.cxx cfcPlugin.cxx cfc.cxx parsecmd.cpp anyoption/anyoption.cpp cfc_transport_config.cpp NeuronBasics/ThreadMultiple.cpp choices.cpp

#g++ -g3 -DRTI_UNIX -DRTI_LINUX -DNDEBUG -I$NDDSHOME/include -I$NDDSHOME/include/ndds -L$NDDSHOME/lib/i86Linux2.6gcc4.1.1 -ldl -lnddscore -lnddsc -lnddscpp -lrtimonitoring -o cfc_pub cfc_publisher.cxx cfcSupport.cxx cfcPlugin.cxx cfc.cxx parsecmd.cpp anyoption/anyoption.cpp cfc_transport_config.cpp

#g++ -g3 -DRTI_UNIX -DRTI_LINUX -DNDEBUG -I$NDDSHOME/include -I$NDDSHOME/include/ndds -L$NDDSHOME/lib/i86Linux2.6gcc4.1.1 -ldl -lnddscore -lnddsc -lnddscpp -lrtimonitoring -o cfc_sub cfc_subscriber.cxx cfcSupport.cxx cfcPlugin.cxx cfc.cxx parsecmd.cpp anyoption/anyoption.cpp cfc_transport_config.cpp

