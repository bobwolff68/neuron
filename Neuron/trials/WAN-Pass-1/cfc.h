
/*
  WARNING: THIS FILE IS AUTO-GENERATED. DO NOT MODIFY.

  This file was generated from cfc.idl using "rtiddsgen".
  The rtiddsgen tool is part of the RTI Data Distribution Service distribution.
  For more information, type 'rtiddsgen -help' at a command shell
  or consult the RTI Data Distribution Service manual.
*/

#ifndef cfc_1284594982818_h
#define cfc_1284594982818_h

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


#ifdef __cplusplus
extern "C" {
#endif

        
extern const char *cfcTYPENAME;
        

#ifdef __cplusplus
}
#endif

typedef struct cfc
{
    DDS_Long  x;
    char*  str; /* maximum length = (1000) */
    char*  from; /* maximum length = (64) */
    char*  timestamp; /* maximum length = (64) */
     DDS_OctetSeq  payload;

} cfc;
    
                            
#if (defined(RTI_WIN32) || defined (RTI_WINCE)) && defined(NDDS_USER_DLL_EXPORT)
  /* If the code is building on Windows, start exporting symbols.
   */
  #undef NDDSUSERDllExport
  #define NDDSUSERDllExport __declspec(dllexport)
#endif

    
NDDSUSERDllExport DDS_TypeCode* cfc_get_typecode(void); /* Type code */
    

DDS_SEQUENCE(cfcSeq, cfc);
        
NDDSUSERDllExport
RTIBool cfc_initialize(
        cfc* self);
        
NDDSUSERDllExport
RTIBool cfc_initialize_ex(
        cfc* self,RTIBool allocatePointers);

NDDSUSERDllExport
void cfc_finalize(
        cfc* self);
                        
NDDSUSERDllExport
void cfc_finalize_ex(
        cfc* self,RTIBool deletePointers);
        
NDDSUSERDllExport
RTIBool cfc_copy(
        cfc* dst,
        const cfc* src);

#if (defined(RTI_WIN32) || defined (RTI_WINCE)) && defined(NDDS_USER_DLL_EXPORT)
  /* If the code is building on Windows, stop exporting symbols.
   */
  #undef NDDSUSERDllExport
  #define NDDSUSERDllExport
#endif



#endif /* cfc_1284594982818_h */
