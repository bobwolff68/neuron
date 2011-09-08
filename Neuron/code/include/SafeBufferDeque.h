//
//  SafeBufferDeque.h
//  
//
//  Created by Robert Wolff on 9/7/11.
//  Copyright 2011 XVD Technology LTD USA. All rights reserved.
//

#include <iostream>
#include <sstream>
#include <deque>
#include <assert.h>
#include <errno.h>

typedef struct strBufferItem {
    void* pData;
    int32_t length;
} BufferItem;

//
// By commenting out this #define, you will be asking the deque to literally 
// allocate and duplicate all inbound pointer-data. This could be costly for high volume/large data.
//
#define NO_DUPLICATE_DATA

class SafeBufferDeque {
public:
    SafeBufferDeque() { dequeItems.clear();	pthread_mutex_init(&mutex, NULL);   };
    ~SafeBufferDeque() { clearAll();	pthread_mutex_destroy(&mutex); };
    
    bool AddItem(void* data, int32_t length) 
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
        
        BufferItem* pItem = new BufferItem;
        
        pItem->length = length;
#ifdef NO_DUPLICATE_DATA
        pItem->pData = data;
#else
        pItem.pData = new unsigned char[length];
        if (!pItem.pData)
            return false;
        memcpy((unsigned char*)(pItem.pData), (unsigned char*)data, length);
#endif
        
        dequeItems.push_back(pItem);
        
        rc = pthread_mutex_unlock(&mutex);
        if (rc)
        {
            std::cerr << "UnLock failed with returncode = " << rc << std::endl;
            assert(false && "Unlock Failed");
            
            return false;
        }
        
        return true;
    };
    
    bool RemoveItem(void** pReturnData, int32_t* pLength) 
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
        
        return true;
    };
    
protected:
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
    };
    
    std::deque<BufferItem*> dequeItems;
	pthread_mutex_t         mutex;
}