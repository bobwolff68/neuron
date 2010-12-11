#ifndef H264DECODEROUTPUTOBJECT_H_
#define H264DECODEROUTPUTOBJECT_H_

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include "MediaOutputObject.h"

class H264DecoderOutputObject : public MediaOutputObject
{
    private:

        int decInFD;


    public:

        H264DecoderOutputObject(int ownerIdP,const char *decInFifoName):MediaOutputObject(ownerIdP)
        {
            //If decoder input fifo doesn't exist, create it
            //TODO: should fifo be created by the decoder thread?
            if(access(decInFifoName,F_OK)<0)
                mkfifo(decInFifoName,S_IRWXU|S_IRWXG);

            //Open the fifo and get a file descriptor
            if((decInFD=open(decInFifoName,O_WRONLY))<0)
            {
                perror("H264DecoderOutObj(open)");
                exit(0);
            }

        }

        ~H264DecoderOutputObject()
        {
            if(close(decInFD)<0)
            {
                perror("H264DecoderOutObj(close)");
                exit(0);
            }

            //TODO: rm fifo
        }

        void Write(void)
        {
        }

        void Write(unsigned char *pData,int size)
        {
            int sizeWritten;

            if((sizeWritten=write(decInFD,pData,size))<0)
                perror("H264DecoderOutObj(write)");

            else if(sizeWritten!=size)
                std::cout << "Expected: " << size << " bytes, Written: " << sizeWritten << " bytes" << std::endl;

            return;
        }
};

#endif // H264DECODEROUTPUTOBJECT_H_
