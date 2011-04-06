#ifndef SAMPLEQUEUEOUTPUTOBJECT_H_
#define SAMPLEQUEUEOUTPUTOBJECT_H_

#include <iostream>
#include <sstream>
#include <queue>
#include "MediaOutputObject.h"

class SampleQueueOutputObject : public MediaOutputObject
{
    private:

        int             maxQueueLen;
        std::queue<int> SeqNumQueue;

        int NumSamplesLost(void)
        {
            // Assuming samples are received in order
            int nSamplesLost = 0;

            if(!SeqNumQueue.empty())
                nSamplesLost = SeqNumQueue.back()-SeqNumQueue.front()-SeqNumQueue.size()+1;

            return nSamplesLost;
        }

        double PercentSamplesLost(void)
        {
            double pcSamplesLost = 0.0;

            if(!SeqNumQueue.empty())
                pcSamplesLost = 100.0*((double)NumSamplesLost())/((double)(NumSamplesLost()+SeqNumQueue.size()));

            return pcSamplesLost;
        }

    public:

        SampleQueueOutputObject(int ownerIdP,int maxQueueLenP):MediaOutputObject(ownerIdP)
        {
            maxQueueLen = maxQueueLenP;
        }

        ~SampleQueueOutputObject()
        {
        }

        void Write(void)
        {
        }

        void Write(int seqNum)
        {
            if(SeqNumQueue.size()==maxQueueLen)
                SeqNumQueue.pop();
            SeqNumQueue.push(seqNum);

            if(!SeqNumQueue.empty())
            {
                std::cout.precision(2);
                std::cout << std::fixed << "Head: " << SeqNumQueue.back() << "  Tail: " << SeqNumQueue.front() << "  Lost: "
                          << NumSamplesLost() << " (" << PercentSamplesLost() << ")\r" << std::flush;
            }

            return;
        }

        void Clear()
        {
            while(!SeqNumQueue.empty())
                SeqNumQueue.pop();

            return;
        }
};


#endif // SAMPLEQUEUEOUTPUTOBJECT_H_
