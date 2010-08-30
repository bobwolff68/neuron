gcc -L$NDDSHOME/lib/i86Linux2.6gcc4.1.1  -L/usr/local/lib HelloWorldPub.c -ldl -lnddscore -lnddsc -lnddscpp -lHelloWorldPartPubSub -o HelloWorldPub 
gcc -L$NDDSHOME/lib/i86Linux2.6gcc4.1.1  -L/usr/local/lib HelloWorldSub.c -ldl -lnddscore -lnddsc -lnddscpp -lHelloWorldPartPubSub -o HelloWorldSub 

