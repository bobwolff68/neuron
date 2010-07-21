cmdLineArgs=("$@")

if [ "${cmdLineArgs[1]}" == "ddsgen" ]; then
    # Autogenerate DDS headers and supporting cpp files
    rtiddsgen -language C++ -replace DDSChat.idl
    echo "DDS support files generated..." 
else
    echo "Skipping rtiddsgen"
fi

if [ "${cmdLineArgs[0]}" == "32bit" ]; then 
    # Compile command for 32-bit machine
    g++ main.cpp DDSChatModule.cpp DDSChat.cxx DDSChatSupport.cxx DDSChatPlugin.cxx -g3 -Wall -DRTI_UNIX -I$NDDSHOME/include -I$NDDSHOME/include/ndds -L$NDDSHOME/lib/i86Linux2.6gcc4.1.1 -ldl -lnddscd -lnddscppd -lnddscored -o DDSChat
    echo -e "Compilation Complete...\nUsage: ./DDSChat <Name> <Id>" 
else
    if [ "${cmdLineArgs[0]}" == "64bit" ]; then
        # Compile command for 64-bit machine
        g++ main.cpp DDSChatModule.cpp DDSChat.cxx DDSChatSupport.cxx DDSChatPlugin.cxx -g3 -Wall -DRTI_UNIX -DRTI_64BIT -m64 -I$NDDSHOME/include -I$NDDSHOME/include/ndds -L$NDDSHOME/lib/i86Linux2.6gcc4.1.1 -ldl -lnddscd -lnddscppd -lnddscored -o DDSChat
         echo "Compilation Complete. Usage: ./DDSChat <Name> <Id>" 
    else
        echo "Usage ./compile.sh (32|64)bit [ddsgen]"
    fi
fi

