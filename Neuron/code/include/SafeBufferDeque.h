//
//  SafeBufferDeque.h
//  
//
//  Created by Robert Wolff on 9/7/11.
//  Copyright 2011 XVD Technology LTD USA. All rights reserved.
//

#ifndef _SAFE_BUFFER_DEQUE_H_
#define _SAFE_BUFFER_DEQUE_H_

#include <iostream>
#include <sstream>
#include <deque>
#include <assert.h>
#include <errno.h>

typedef void (*DataSigFunc)(void*);

typedef struct strBufferItem {
    unsigned char* pData;
    int32_t length;
} BufferItem;

//
// By commenting out this #define, you will be asking the deque to literally 
// allocate and duplicate all inbound pointer-data. This could be costly for high volume/large data.
//
//#define NO_DUPLICATE_DATA

class SafeBufferDeque {
public:
    SafeBufferDeque(const int _maxItems): 
    bDefunct(false), maxItems(_maxItems), signalData(NULL), p_src(NULL)
    { dequeItems.clear();	pthread_mutex_init(&mutex, NULL);   };
    
    ~SafeBufferDeque() { clearAll();	pthread_mutex_destroy(&mutex); };
    
    void SetSigDataFunc(DataSigFunc _sigData, void* _psrc)
    {
        signalData = _sigData;
        p_src = _psrc;
    }
    
    bool RemoveItem(unsigned char** pReturnData, int32_t* pLength) 
    { 
        int rc;
        
        rc = pthread_mutex_lock(&mutex);
        if (rc)
        {
            std::cerr << "Lock failed with returncode = " << rc << std::endl;
            std::cerr << "errno=" << errno << " at this moment." << std::endl;
            std::cerr << "Equates to: " << strerror(errno) << std::endl;
            assert(false && "Lock Failed");
            return false;
        }
        
        BufferItem* pItem;
        
        // Get the item.
        pItem = dequeItems.front();
        
        // Remove the item at the front.
        dequeItems.pop_front();
        
        // Send back the data pointer and length.
        *pReturnData = pItem->pData;
        *pLength = pItem->length;
        
        delete pItem;
        
        rc = pthread_mutex_unlock(&mutex);
        if (rc)
        {
            std::cerr << "UnLock failed with returncode = " << rc << std::endl;
            assert(false && "Unlock Failed");
            
            return false;
        }
        
        //std::cout << "RemItem(): Queue size = " << dequeItems.size() << std::endl;
        return true;
    }

    bool AddItem(unsigned char* data, int32_t length) 
    { 
        int rc;
        
        rc = pthread_mutex_lock(&mutex);
        if (rc)
        {
            std::cerr << "Lock failed with returncode = " << rc << std::endl;
            std::cerr << "errno=" << errno << " at this moment." << std::endl;
            std::cerr << "Equates to: " << strerror(errno) << std::endl;
            assert(false && "Lock Failed");
            return false;
        }
        
        if(dequeItems.size() == maxItems)
        {
            delete (unsigned char*) dequeItems.front()->pData;
            delete dequeItems.front();
            dequeItems.pop_front();
        }
        
        BufferItem* pItem = new BufferItem;
        
        
#ifdef NO_DUPLICATE_DATA
        //Can't use this coz need to add start-code
        pItem->length = length;
        pItem->pData = 0;//data;
#else
#if 0
        pItem->length = length+4;
        pItem->pData = new unsigned char[length+4];
        if (!pItem->pData)
            return false;
        static char sc[4] = {0,0,0,1};
        memcpy(pItem->pData, sc, 4);
        memcpy(((unsigned char*)(pItem->pData)+4), (unsigned char*)data, length);
#else
        pItem->length = length;
        pItem->pData = new unsigned char[length];
        if (!pItem->pData)
            return false;
        memcpy((unsigned char*)(pItem->pData), (unsigned char*)data, length);        
#endif
#endif

        dequeItems.push_back(pItem);
        
        rc = pthread_mutex_unlock(&mutex);
        if (rc)
        {
            std::cerr << "UnLock failed with returncode = " << rc << std::endl;
            assert(false && "Unlock Failed");
            
            return false;
        }
        
        //Signal arrival of data
        if(signalData && p_src)
            signalData(p_src);
        
        return true;
    }
        
    void clearAll(void) 
    { 
        BufferItem* pItem;
        
        pthread_mutex_lock(&mutex);
        // Pop items off the deque until it is empty deleting each as we go.
        while(dequeItems.size())
        {
            pItem = dequeItems.front();
            // Remove the item at the front.
            dequeItems.pop_front();
            
            // Since we are clearing the queue early, we must delete the pointers as well.
            delete (unsigned char*)(pItem->pData);
            delete pItem;
        }
        
        pthread_mutex_unlock(&mutex);
    }
    
    int qsize(void) const
    {
        return dequeItems.size();
    }
    
    void SetDefunct(void)
    {
        bDefunct = false;
    }
    
    bool IsDefunct(void) const
    {
        return bDefunct;
    }
        
protected:
    const int               maxItems;
    std::deque<BufferItem*> dequeItems;
	pthread_mutex_t         mutex;
    bool                    bDefunct;
    DataSigFunc             signalData;
    void*                   p_src;
};

#endif