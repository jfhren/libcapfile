/*
 * Copyright Inria:
 * Jean-François Hren
 * 
 * jfhren[at]gmail[dot]com
 * michael[dot]hauspie[at]lifl[dot]com
 * 
 * This software is a computer program whose purpose is to read, analyze,
 * modify, generate and write Java Card 2 CAP file.
 * 
 * This software is governed by the CeCILL-B license under French
 * law and
 * abiding by the rules of distribution of free software.  You can  use, 
 * modify and/ or redistribute the software under the terms of the
 * CeCILL-B
 * license as circulated by CEA, CNRS and INRIA at the following URL
 * "http://www.cecill.info". 
 * 
 * As a counterpart to the access to the source code and  rights to copy,
 * modify and redistribute granted by the license, users are provided only
 * with a limited warranty  and the software's author,  the holder of the
 * economic rights,  and the successive licensors  have only  limited
 * liability. 
 * 
 * In this respect, the user's attention is drawn to the risks associated
 * with loading,  using,  modifying and/or developing or reproducing the
 * software by the user in light of its specific status of free software,
 * that may mean  that it is complicated to manipulate,  and  that  also
 * therefore means  that it is reserved for developers  and  experienced
 * professionals having in-depth computer knowledge. Users are therefore
 * encouraged to load and test the software's suitability as regards their
 * requirements in conditions enabling the security of their systems and/or 
 * data to be ensured and,  more generally, to use and operate it in the 
 * same conditions as regards security. 
 * 
 * The fact that you are presently reading this means that you have had
 * knowledge of the CeCILL-B license and that you accept its
 * terms.
 */

/**
 * \file exp_file.h
 * \brief A straightforward representation of an export file. 
 */

#ifndef EXP_FILE_H
#define EXP_FILE_H
#include <stdint.h>

#define EF_CONSTANT_PACKAGE 13
#define EF_CONSTANT_CLASSREF 7
#define EF_CONSTANT_INTEGER 3
#define EF_CONSTANT_UTF8 1

#define EF_ACC_LIBRARY 0x01
#define EF_ACC_PUBLIC 0x0001
#define EF_ACC_PROTECTED 0x0004
#define EF_ACC_STATIC 0x0008
#define EF_ACC_FINAL 0x0010
#define EF_ACC_INTERFACE 0x0200
#define EF_ACC_ABSTRACT 0x0400
#define EF_ACC_SHAREABLE 0x0800
#define EF_ACC_REMOTE 0x1000

#ifndef PRIMITIVE_TYPES
#define PRIMITIVE_TYPES
typedef uint8_t u1;
typedef uint16_t u2;
typedef uint32_t u4;
#endif


typedef struct {
    u1 tag;
    u1 flags;
    u2 name_index;
    u1 minor_version;
    u1 major_version;
    u1 aid_length;
    u1* aid;
} ef_CONSTANT_Package_info;


typedef struct {
    u1 tag;
    u2 name_index;
} ef_CONSTANT_Classref_info;


typedef struct {
    u1 tag;
    u4 bytes;
} ef_CONSTANT_Integer_info;


typedef struct {
    u1 tag;
    u2 length;
    u1* bytes;
} ef_CONSTANT_Utf8_info;


typedef union {
    u1 tag;
    ef_CONSTANT_Package_info CONSTANT_Package;
    ef_CONSTANT_Classref_info CONSTANT_Classref;
    ef_CONSTANT_Integer_info CONSTANT_Integer;
    ef_CONSTANT_Utf8_info CONSTANT_Utf8;
} ef_cp_info;


typedef struct {
    u2 attribute_name_index;
    u4 attribute_length;
    u2 constantvalue_index;
} ef_ConstantValue_attribute;


typedef ef_ConstantValue_attribute ef_attribute_info;


typedef struct {
    u1 token;
    u2 access_flags;
    u2 name_index;
    u2 descriptor_index;
    u2 attributes_count;
    ef_attribute_info* attributes;
} ef_field_info;


typedef struct {
    u1 token;
    u2 access_flags;
    u2 name_index;
    u2 descriptor_index;
} ef_method_info;


typedef struct {
    u1 token;
    u2 access_flags;
    u2 name_index;
    u2 export_supers_count;
    u2* supers;
    u1 export_interfaces_count;
    u2* interfaces;
    u2 export_fields_count;
    ef_field_info* fields;
    u2 export_methods_count;
    ef_method_info* methods;
} ef_class_info;


typedef struct {
    u4 magic;
    u1 minor_version;
    u1 major_version;
    u2 constant_pool_count;
    ef_cp_info* constant_pool;
    u2 this_package;
    u1 export_class_count;
    ef_class_info* classes;
} export_file;

#endif
