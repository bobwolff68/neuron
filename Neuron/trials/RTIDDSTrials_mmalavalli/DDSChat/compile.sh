cmdLineArgs=("$@")

if [ "${cmdLineArgs[0]}" == "ddsgen" ]; then
    # Autogenerate DDS headers and supporting cpp files
    rtiddsgen -language C++ -replace DDSChat.idl
    echo "DDS support files generated..." 
else
    echo "Skipping rtiddsgen"
fi

if [ `uname -m` == "i686" ]; then 
    # Compile command for 32-bit machine
    g++ main.cpp DDSChatModule.cpp DDSChat.cxx DDSChatSupport.cxx DDSChatPlugin.cxx -g3 -Wall -DRTI_UNIX -I$NDDSHOME/include -I$NDDSHOME/include/ndds -L$NDDSHOME/lib/i86Linux2.6gcc4.1.1 -ldl -lnddscd -lnddscppd -lnddscored -o DDSChat
    echo -e "Compilation Complete...\nUsage: ./DDSChat <Name> <Id>" 
else
    if [ `uname -m` == "x86_64" ]; then
        # Compile command for 64-bit machine
        g++ main.cpp DDSChatModule.cpp DDSChat.cxx DDSChatSupport.cxx DDSChatPlugin.cxx -g3 -Wall -DRTI_UNIX -DRTI_64BIT -m64 -I$NDDSHOME/include -I$NDDSHOME/include/ndds -L$NDDSHOME/lib/x64Linux2.6gcc4.1.1 -ldl -lnddscd -lnddscppd -lnddscored -o DDSChat
         echo "Compilation Complete. Usage: ./DDSChat <Name> <Id>" 
    fi
fi

