
/*
  WARNING: THIS FILE IS AUTO-GENERATED. DO NOT MODIFY.

  This file was generated from cfc.idl using "rtiddsgen".
  The rtiddsgen tool is part of the RTI Data Distribution Service distribution.
  For more information, type 'rtiddsgen -help' at a command shell
  or consult the RTI Data Distribution Service manual.
*/


#ifndef NDDS_STANDALONE_TYPE
    #ifdef __cplusplus
        #ifndef ndds_cpp_h
            #include "ndds/ndds_cpp.h"
        #endif
        #ifndef dds_c_log_impl_h              
            #include "dds_c/dds_c_log_impl.h"                                
        #endif        
    #else
        #ifndef ndds_c_h
            #include "ndds/ndds_c.h"
        #endif
    #endif
    
    #ifndef cdr_type_h
        #include "cdr/cdr_type.h"
    #endif    

    #ifndef osapi_heap_h
        #include "osapi/osapi_heap.h" 
    #endif
#else
    #include "ndds_standalone_type.h"
#endif



#include "cfc.h"

/* ========================================================================= */
const char *cfcTYPENAME = "cfc";

DDS_TypeCode* cfc_get_typecode()
{
    static RTIBool is_initialized = RTI_FALSE;

    static DDS_TypeCode cfc_g_tc_str_string = DDS_INITIALIZE_STRING_TYPECODE(1000);
    static DDS_TypeCode cfc_g_tc_from_string = DDS_INITIALIZE_STRING_TYPECODE(64);
    static DDS_TypeCode cfc_g_tc_payload_sequence = DDS_INITIALIZE_SEQUENCE_TYPECODE(400000,NULL);

    static DDS_TypeCode_Member cfc_g_tc_members[4]=
    {
        {
            (char *)"x",/* Member name */
            {
                0,/* Representation ID */
                DDS_BOOLEAN_FALSE,/* Is a pointer? */
                -1, /* Bitfield bits */
                NULL/* Member type code is assigned later */
            },
            0, /* Ignored */
            0, /* Ignored */
            0, /* Ignored */
            NULL, /* Ignored */
            DDS_BOOLEAN_FALSE, /* Is a key? */
            DDS_PRIVATE_MEMBER,/* Ignored */
            0,/* Ignored */
            NULL/* Ignored */
        },
        {
            (char *)"str",/* Member name */
            {
                0,/* Representation ID */
                DDS_BOOLEAN_FALSE,/* Is a pointer? */
                -1, /* Bitfield bits */
                NULL/* Member type code is assigned later */
            },
            0, /* Ignored */
            0, /* Ignored */
            0, /* Ignored */
            NULL, /* Ignored */
            DDS_BOOLEAN_FALSE, /* Is a key? */
            DDS_PRIVATE_MEMBER,/* Ignored */
            0,/* Ignored */
            NULL/* Ignored */
        },
        {
            (char *)"from",/* Member name */
            {
                0,/* Representation ID */
                DDS_BOOLEAN_FALSE,/* Is a pointer? */
                -1, /* Bitfield bits */
                NULL/* Member type code is assigned later */
            },
            0, /* Ignored */
            0, /* Ignored */
            0, /* Ignored */
            NULL, /* Ignored */
            DDS_BOOLEAN_FALSE, /* Is a key? */
            DDS_PRIVATE_MEMBER,/* Ignored */
            0,/* Ignored */
            NULL/* Ignored */
        },
        {
            (char *)"payload",/* Member name */
            {
                0,/* Representation ID */
                DDS_BOOLEAN_FALSE,/* Is a pointer? */
                -1, /* Bitfield bits */
                NULL/* Member type code is assigned later */
            },
            0, /* Ignored */
            0, /* Ignored */
            0, /* Ignored */
            NULL, /* Ignored */
            DDS_BOOLEAN_FALSE, /* Is a key? */
            DDS_PRIVATE_MEMBER,/* Ignored */
            0,/* Ignored */
            NULL/* Ignored */
        }
    };

    static DDS_TypeCode cfc_g_tc =
    {{
        DDS_TK_STRUCT,/* Kind */
        DDS_BOOLEAN_FALSE, /* Ignored */
        -1,/* Ignored */
        (char *)"cfc", /* Name */
        NULL, /* Ignored */
        0, /* Ignored */
        0, /* Ignored */
        NULL, /* Ignored */
        4, /* Number of members */
        cfc_g_tc_members, /* Members */
        DDS_VM_NONE /* Ignored */
    }}; /* Type code for cfc*/

    if (is_initialized) {
        return &cfc_g_tc;
    }

    cfc_g_tc_payload_sequence._data._typeCode = (RTICdrTypeCode *)&DDS_g_tc_octet;

    cfc_g_tc_members[0]._representation._typeCode = (RTICdrTypeCode *)&DDS_g_tc_long;
    cfc_g_tc_members[1]._representation._typeCode = (RTICdrTypeCode *)&cfc_g_tc_str_string;
    cfc_g_tc_members[2]._representation._typeCode = (RTICdrTypeCode *)&cfc_g_tc_from_string;
    cfc_g_tc_members[3]._representation._typeCode = (RTICdrTypeCode *)&cfc_g_tc_payload_sequence;

    is_initialized = RTI_TRUE;

    return &cfc_g_tc;
}


RTIBool cfc_initialize(
    cfc* sample) {
  return cfc_initialize_ex(sample,RTI_TRUE);
}
        
RTIBool cfc_initialize_ex(
    cfc* sample,RTIBool allocatePointers)
{

    void* buffer;                
    buffer = NULL;        

    if (!RTICdrType_initLong(&sample->x)) {
        return RTI_FALSE;
    }                
            
    sample->str = DDS_String_alloc((1000));
    if (sample->str == NULL) {
        return RTI_FALSE;
    }
            
    sample->from = DDS_String_alloc((64));
    if (sample->from == NULL) {
        return RTI_FALSE;
    }
            
    DDS_OctetSeq_initialize(&sample->payload);
                
    if (!DDS_OctetSeq_set_maximum(&sample->payload,
            (400000))) {
        return RTI_FALSE;
    }
            

    return RTI_TRUE;
}

void cfc_finalize(
    cfc* sample)
{
    cfc_finalize_ex(sample,RTI_TRUE);
}
        
void cfc_finalize_ex(
    cfc* sample,RTIBool deletePointers)
{        

    DDS_String_free(sample->str);                
            
    DDS_String_free(sample->from);                
            
    DDS_OctetSeq_finalize(&sample->payload);
            
}

RTIBool cfc_copy(
    cfc* dst,
    const cfc* src)
{        

    if (!RTICdrType_copyLong(
        &dst->x, &src->x)) {
        return RTI_FALSE;
    }
            
    if (!RTICdrType_copyString(
        dst->str, src->str, (1000) + 1)) {
        return RTI_FALSE;
    }
            
    if (!RTICdrType_copyString(
        dst->from, src->from, (64) + 1)) {
        return RTI_FALSE;
    }
            
    if (!DDS_OctetSeq_copy_no_alloc(&dst->payload,
                                          &src->payload)) {
        return RTI_FALSE;
    }
            

    return RTI_TRUE;
}


/**
 * <<IMPLEMENTATION>>
 *
 * Defines:  TSeq, T
 *
 * Configure and implement 'cfc' sequence class.
 */
#define T cfc
#define TSeq cfcSeq
#define T_initialize_ex cfc_initialize_ex
#define T_finalize_ex   cfc_finalize_ex
#define T_copy       cfc_copy

#ifndef NDDS_STANDALONE_TYPE
#include "dds_c/generic/dds_c_sequence_TSeq.gen"
#ifdef __cplusplus
#include "dds_cpp/generic/dds_cpp_sequence_TSeq.gen"
#endif
#else
#include "dds_c_sequence_TSeq.gen"
#ifdef __cplusplus
#include "dds_cpp_sequence_TSeq.gen"
#endif
#endif

#undef T_copy
#undef T_finalize_ex
#undef T_initialize_ex
#undef TSeq
#undef T

