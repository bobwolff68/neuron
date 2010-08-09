# Opensplice
# gcc *.c ../../DDS/*.c -g3 -I$OSPL_HOME/include/dcps/C/SAC -L$OSPL_HOME/lib -ldcpssac -o x264mux

# RTI Style
# generate DDS support via the IDL compiler (not necessary unless .idl is changed)

#cd ../../DDS;$NDDSHOME/scripts/rtiddsgen NeuronDDS.idl -replace -language C;cd ../Server/x264_stream_mux

# RTI 64-bit
#gcc *.c ../../DDS/*.c -g3 -DRTI_STYLE -DRTI_UNIX -DRTI_LINUX -DRTI_64BIT -m64 -I$NDDSHOME/include -I$NDDSHOME/include/ndds -L$NDDSHOME/lib/x64Linux2.6gcc4.1.1 -ldl -lnddscd -lnddscored -o x264mux

#RTI 32-bit
gcc *.c -g3 -L/usr/local/lib -lNeuronVideoPubSub -L$NDDSHOME/lib/i86Linux2.6gcc4.1.1 -ldl -lnddsc -lnddscpp -lnddscore -o x264mux
