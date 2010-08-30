g++ -DRTI_UNIX -DRTI_LINUX -I$NDDSHOME/include -I$NDDSHOME/include/ndds -L$NDDSHOME/lib/i86Linux2.6gcc4.1.1  HelloWorld_publisher.cxx HelloWorld_subscriber.cxx HelloWorld.cxx HelloWorldSupport.cxx HelloWorldPlugin.cxx -ldl -lnddscore -lnddsc -lnddscpp -shared -o"libHelloWorldPartPubSub.so"

#g++ -DRTI_UNIX -DRTI_LINUX -I$NDDSHOME/include -I$NDDSHOME/include/ndds -L$NDDSHOME/lib/i86Linux2.6gcc4.1.1  HelloWorld_subscriber.cxx HelloWorld.cxx HelloWorldSupport.cxx HelloWorldPlugin.cxx -ldl -lnddscore -lnddsc -lnddscpp -o shared-lib-output/HelloWorld_subscriber
