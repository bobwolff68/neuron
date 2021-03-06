
/*
  WARNING: THIS FILE IS AUTO-GENERATED. DO NOT MODIFY.

  This file was generated from HelloWorld.idl using "rtiddsgen".
  The rtiddsgen tool is part of the RTI Data Distribution Service distribution.
  For more information, type 'rtiddsgen -help' at a command shell
  or consult the RTI Data Distribution Service manual.
*/

#ifndef HelloWorld_1282774486641_h
#define HelloWorld_1282774486641_h

#ifndef NDDS_STANDALONE_TYPE
    #ifdef __cplusplus
        #ifndef ndds_cpp_h
            #include "ndds/ndds_cpp.h"
        #endif
    #else
        #ifndef ndds_c_h
            #include "ndds/ndds_c.h"
        #endif
    #endif
#else
    #include "ndds_standalone_type.h"
#endif

             
static const DDS_Long HELLODDS_MAX_PAYLOAD_SIZE = 8192;             
static const DDS_Long HELLODDS_MAX_STRING_SIZE = 64;
#ifdef __cplusplus
extern "C" {
#endif

        
extern const char *HelloWorldTYPENAME;
        

#ifdef __cplusplus
}
#endif

typedef struct HelloWorld
{
    char*  prefix; /* maximum length = ((HELLODDS_MAX_STRING_SIZE)) */
    DDS_Long  sampleId;
     DDS_OctetSeq  payload;

} HelloWorld;
    
                            
#if (defined(RTI_WIN32) || defined (RTI_WINCE)) && defined(NDDS_USER_DLL_EXPORT)
  /* If the code is building on Windows, start exporting symbols.
   */
  #undef NDDSUSERDllExport
  #define NDDSUSERDllExport __declspec(dllexport)
#endif

    
NDDSUSERDllExport DDS_TypeCode* HelloWorld_get_typecode(void); /* Type code */
    

DDS_SEQUENCE(HelloWorldSeq, HelloWorld);
        
NDDSUSERDllExport
RTIBool HelloWorld_initialize(
        HelloWorld* self);
        
NDDSUSERDllExport
RTIBool HelloWorld_initialize_ex(
        HelloWorld* self,RTIBool allocatePointers);

NDDSUSERDllExport
void HelloWorld_finalize(
        HelloWorld* self);
                        
NDDSUSERDllExport
void HelloWorld_finalize_ex(
        HelloWorld* self,RTIBool deletePointers);
        
NDDSUSERDllExport
RTIBool HelloWorld_copy(
        HelloWorld* dst,
        const HelloWorld* src);

#if (defined(RTI_WIN32) || defined (RTI_WINCE)) && defined(NDDS_USER_DLL_EXPORT)
  /* If the code is building on Windows, stop exporting symbols.
   */
  #undef NDDSUSERDllExport
  #define NDDSUSERDllExport
#endif



#endif /* HelloWorld_1282774486641_h */
