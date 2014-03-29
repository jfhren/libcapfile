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
 * \file cap_file.h
 * \brief This header defines a straightforward representation of a CAP file.
 * This structure can be allocated and filled by a call to 
 * \link cap_file_reader.c read_cap_file \endlink function.
 */

#ifndef CAP_FILE_H
#define CAP_FILE_H
#include <stdint.h>

#define COMPONENT_HEADER 1
#define COMPONENT_DIRECTORY 2
#define COMPONENT_APPLET 3
#define COMPONENT_IMPORT 4
#define COMPONENT_CONSTANTPOOL 5
#define COMPONENT_CLASS 6
#define COMPONENT_METHOD 7
#define COMPONENT_STATICFIELD 8
#define COMPONENT_REFERENCELOCATION 9
#define COMPONENT_EXPORT 10
#define COMPONENT_DESCRIPTOR 11
#define COMPONENT_DEBUG 12                  /* only for M.m > 2.1 */

#define HEADER_ACC_INT 0x01
#define HEADER_ACC_EXPORT 0x02
#define HEADER_ACC_APPLET 0x04

#define CF_CONSTANT_CLASSREF 1
#define CF_CONSTANT_INSTANCEFIELDREF 2
#define CF_CONSTANT_VIRTUALMETHODREF 3
#define CF_CONSTANT_SUPERMETHODREF 4
#define CF_CONSTANT_STATICFIELDREF 5
#define CF_CONSTANT_STATICMETHODREF 6

#define CLASS_ACC_INTERFACE 0x8
#define CLASS_ACC_SHAREABLE 0x4
#define CLASS_ACC_REMOTE 0X2

#define NIBBLE_VOID 0x1
#define NIBBLE_BOOLEAN 0x2
#define NIBBLE_BYTE 0x3
#define NIBBLE_SHORT 0x4
#define NIBBLE_INT 0x5
#define NIBBLE_REFERENCE 0x6
#define NIBBLE_BOOLEAN_ARRAY 0xA
#define NIBBLE_BYTE_ARRAY 0xB
#define NIBBLE_SHORT_ARRAY 0xC
#define NIBBLE_INT_ARRAY 0xD
#define NIBBLE_REFERENCE_ARRAY 0xE

#define DESCRIPTOR_ACC_PUBLIC 0x01
#define DESCRIPTOR_ACC_PRIVATE 0x02
#define DESCRIPTOR_ACC_PROTECTED 0x04
#define DESCRIPTOR_ACC_STATIC 0x08
#define DESCRIPTOR_ACC_FINAL 0x10
#define DESCRIPTOR_ACC_INTERFACE 0x40
#define DESCRIPTOR_ACC_ABSTRACT1 0x80
#define DESCRIPTOR_ACC_ABSTRACT2 0x40
#define DESCRIPTOR_ACC_INIT 0x80

#define DEBUG_ACC_PUBLIC 0x0001
#define DEBUG_ACC_PRIVATE 0x0002
#define DEBUG_ACC_PROTECTED 0x0004
#define DEBUG_ACC_STATIC 0x0008
#define DEBUG_ACC_FINAL 0x0010
#define DEBUG_ACC_REMOTE 0x0020
#define DEBUG_ACC_NATIVE 0x0100
#define DEBUG_ACC_INTERFACE 0x0200
#define DEBUG_ACC_ABSTRACT 0x0400
#define DEBUG_ACC_SHAREABLE 0x0800

#define METHOD_ACC_EXTENDED 0x8
#define METHOD_ACC_ABSTRACT 0x4

#ifndef PRIMITIVE_TYPES
#define PRIMITIVE_TYPES
typedef uint8_t u1;
typedef uint16_t u2;
typedef uint32_t u4;
#endif

/**
 * \brief Information about a package.
 * 
 * This structure is defined page 6-7 of Virtual Machine Specification, Java
 * Card Platform, v2.2.2.
 */
typedef struct {
    u1 minor_version;   /**< The used Java Card minor version for the package. */
    u1 major_version;   /**< The used Java Card major version for the package. */
    u1 AID_length;      /**< The length of the AID. */
    u1* AID;            /**< The package AID. */
} cf_package_info;


/**
 * \brief The name of a package.
 *
 * This structure is defined page 6-8 of Virtual Machine Specification, Java
 * Card Platform, v2.2.2.
 */
typedef struct {
    u1 name_length; /**< The length of the name. */
    u1* name;       /**< The package name. */
} cf_package_name_info;


/**
 * \brief Straightforward representation of the Header component.
 *
 * This structure is defined page 6-6 of Virtual Machine Specification, Java
 * Card Platform, v2.2.2.
 */
typedef struct {
    u1 tag;     /**< The component tag (should be 1). */
    u2 size;    /**< The size in byte of info[]. */
    u4 magic;   /**< The magic number of a CAP file (should be 0xDECAFFED). */
    u1 minor_version;   /**< The used Java Card minor version for this CAP
                             file. */
    u1 major_version;   /**< The used Java Card major version for this CAP
                             file (should be 2). */
    u1 flags;   /**< A combination of HEADER_ACC_INT, HEADER_ACC_EXPORT et
                     HEADER_ACC_APPLET. */
    cf_package_info package;            /**< Information about this package. */
    char has_package_name;              /**< Added for usability */
    cf_package_name_info package_name;  /**< only for M.m > 2.1 */
} cf_header_component;


/**
 * \brief Information about the Static Field component.
 *
 * This structure is defined page 6-10 of Virtual Machine Specification, Java
 * Card Platform, v2.2.2.
 */
typedef struct {
    u2 image_size;          /**< Should be equal to image_size in the Static
                                 Field component. */
    u2 array_init_count;    /**< Should be equal to array_init_count in the
                                 Static Field component. */
    u2 array_init_size;     /**< Should be equal to array_init_size in the
                                 Static Field component. */
} cf_static_field_size_info;


/**
 * \brief Information about a custom component.
 *
 * This structure is defined page 6-11 of Virtual Machine Specification, Java
 * Card Platform, v2.2.2.
 */
typedef struct {
    u1 component_tag;   /**< The tag of the custom component. */
    u2 size;            /**< The size in byte of info[] of  the custom
                             component. */
    u1 AID_length;      /**< The length of the AID. */
    u1* AID;            /**< The AID of the custom component. */
} cf_custom_component_info;


/**
 * \brief Straightforward representation of the Directory component.
 *
 * This structure is defined page 6-9 of Virtual Machine Specification, Java
 * Card Platform, v2.2.2.
 */
typedef struct {
    u1 tag;     /**< The component tag (should be 2). */
    u2 size;    /**< The size in byte of info[]. */
    char can_have_debug_component;          /**< added for usability */
    u2 component_sizes[12];                 /**< info[] sizes (11 when M.m < 2.2) */
    cf_static_field_size_info static_field_size;    /**< Information on the
                                                         Static Field component. */
    u1 import_count;    /**< Number of imported packages. */
    u1 applet_count;    /**< Number of defined applets. */
    u1 custom_count;    /**< Number of custom components. */
    cf_custom_component_info* custom_components;    /**< Information on custom
                                                         components. */
} cf_directory_component;


/**
 * \brief Information about an applet.
 *
 * This structure is defined page 6-12 of Virtual Machine Specification, Java
 * Card Platform, v2.2.2.
 */
typedef struct {
    u1 AID_length;              /**< The length of the AID. */
    u1* AID;                    /**< The AID of the applet. */
    u2 install_method_offset;   /**< The offset of the install method for the
                                     applet. */
} cf_applet_info;


/**
 * \brief Straightforward representation of the Applet component.
 *
 * This structure is defined page 6-12 of Virtual Machine Specification, Java
 * Card Platform, v2.2.2.
 */
typedef struct {
    u1 tag;     /**< The component tag (should be 3). */
    u2 size;    /**< The size in byte of info[]. */
    u1 count;   /**< The number of defined applets. */
    cf_applet_info* applets;    /**< Information of the applets. */
} cf_applet_component;


/**
 * \brief Straightforward representation of the Import component.
 *
 * This structure is defined page 6-13 of Virtual Machine Specification, Java
 * Card Platform, v2.2.2.
 */
typedef struct {
    u1 tag;     /**< The component tag (should be 4). */
    u2 size;    /**< The size in byte of info[]. */
    u1 count;   /**< The number of imported packages. */
    cf_package_info* packages;  /**< Information on imported packages. */
} cf_import_component;


/**
 * \brief Straightforward representation of a class reference.
 *
 * This structure is defined page 6-16 of Virtual Machine Specification, Java
 * Card Platform, v2.2.2.
 */
typedef struct {
    u1 isExternal;  /**< Added for usability. If 0 then it's an internal class
                         reference, if 1 then it's an external class
                         reference.*/
    union {
        u2 internal_class_ref;  /**< The offset of the class. */
        struct {
            u1 package_token;   /**< The package token of the class. */
            u1 class_token;     /**< The class token of the class. */
        } external_class_ref;   /**< The external class reference. */
    } ref;  /**< The class reference. */
} cf_class_ref_info;


/**
 * \brief Straightforward representation of a class reference constant pool
 *        entry.
 *
 * This structure is defined page 6-16 of Virtual Machine Specification, Java
 * Card Platform, v2.2.2.
 */
typedef struct {
    u1 tag;     /**< Constant pool tag (should be 1). */
    cf_class_ref_info class_ref;    /**< The class reference. */
    u1 padding; /**< Padding (should be 0). */
} cf_CONSTANT_Classref_info;


/**
 * \brief Straightforward representation of an instance field reference constant
 *        pool entry.
 *
 * This structure is defined page 6-18 of Virtual Machine Specification, Java
 * Card Platform, v2.2.2.
 */
typedef struct {
    u1 tag;     /**< Constant pool tag (should be 2). */
    cf_class_ref_info class;    /**< The class reference defining the instance
                                     field. */
    u1 token;   /**< The field token of the instance field. */
} cf_CONSTANT_InstanceFieldref_info;


/**
 * \brief Straightforward representation of a virtual method reference constant
 *        pool entry.
 *
 * This structure is defined page 6-18 of Virtual Machine Specification, Java
 * Card Platform, v2.2.2.
 */
typedef struct {
    u1 tag;     /**< Constant pool tag (should be 3). */
    cf_class_ref_info class;    /**< The Class reference defining the virtual
                                     method. */
    u1 token;   /**< The method token of the virtual method. */
} cf_CONSTANT_VirtualMethodref_info;


/**
 * \brief Straightforward representation of a super method reference constant
 *        pool entry.
 *
 * This structure is defined page 6-18 of Virtual Machine Specification, Java
 * Card Platform, v2.2.2.
 */
typedef struct {
    u1 tag;     /**< Constant pool tag (should be 4). */
    cf_class_ref_info class;    /**< The class reference defining the method
                                     with the supercall. */
    u1 token;   /**< The method token of the super method. */
} cf_CONSTANT_SuperMethodref_info;


/**
 * \brief Straightforward representation of a static reference.
 *
 * This structure is defined page 6-20 of Virtual Machine Specification, Java
 * Card Platform, v2.2.2.
 */
typedef struct {
    u1 isExternal;  /**< Added for usability. If 0 then it's an internal
                         reference, if 1 then it's an external reference.*/
    union {
        struct {
            u1 padding; /**< Padding (should be 0). */
            u2 offset;  /**< Offset of the reference (static field or static
                             method). */
        } internal_ref; /**< The internal static field or static method
                             reference. */
        struct {
            u1 package_token;   /**< The package token of the class. */
            u1 class_token;     /**< The class token of the class defining the
                                     static field or the static method. */
            u1 token;           /**< The token of the static field or of the
                                     static method. */
        } external_ref; /**< The external static field or static method
                             reference. */
    } ref;  /**< The static field or static method reference. */
} cf_static_ref_info;


/**
 * \brief Straightforward representation of a static field reference constant
 *        pool entry.
 *
 * This structure is defined page 6-19 of Virtual Machine Specification, Java
 * Card Platform, v2.2.2.
 */
typedef struct {
    u1 tag;     /**< Constant pool tag (should be 5). */
    cf_static_ref_info static_field_ref;    /**< The static field reference. */
} cf_CONSTANT_StaticFieldref_info;


/**
 * \brief Straightforward representation of a static method reference constant
 *        pool entry.
 *
 * This structure is defined page 6-19 of Virtual Machine Specification, Java
 * Card Platform, v2.2.2.
 */
typedef struct {
    u1 tag;     /**< Constant pool tag (should be 6). */
    cf_static_ref_info static_method_ref;   /**< The static method reference. */
} cf_CONSTANT_StaticMethodref_info;


/**
 * \brief Straightforward representation of a constant pool entry.
 *
 * This structure is defined page 6-15 of Virtual Machine Specification, Java
 * Card Platform, v2.2.2.
 */
typedef union {
    u1 tag;     /**< The constant pool tag (should be between 1 and 6
                     included). */
    cf_CONSTANT_Classref_info CONSTANT_Classref; /**< Defined if tag is equal to
                                                      1. */
    cf_CONSTANT_InstanceFieldref_info CONSTANT_InstanceFieldref; /**< Defined if
                                                                      tag is
                                                                      equal to
                                                                      2. */
    cf_CONSTANT_VirtualMethodref_info CONSTANT_VirtualMethodref; /**< Defined if
                                                                      tag is 
                                                                      equal to
                                                                      3. */
    cf_CONSTANT_SuperMethodref_info CONSTANT_SuperMethodref; /**< Defined if tag
                                                                  is equal to 4. */
    cf_CONSTANT_StaticFieldref_info CONSTANT_StaticFieldref; /**< Defined if tag
                                                                  is equal to 5. */
    cf_CONSTANT_StaticMethodref_info CONSTANT_StaticMethodref; /**< Defined if
                                                                    tag is equal
                                                                    to 6. */
} cf_cp_info;


/**
 * \brief Straightforward representation of the Constant Pool component.
 *
 * This structure is defined page 6-14 of Virtual Machine Specification, Java
 * Card Platform, v2.2.2.
 */
typedef struct {
    u1 tag;     /**< The component tag (should be 5). */
    u2 size;    /**< The size in byte of info[]. */
    u2 count;   /**< The number of constant pool entries. */
    cf_cp_info* constant_pool;  /**< The constant pool. */
} cf_constant_pool_component;


/**
 * \brief Straightforward representation of a type descriptor.
 *
 * This structure is defined page 6-23 of Virtual Machine Specification, Java
 * Card Platform, v2.2.2.
 */
typedef struct {
    u2 offset;  /**< Added for usability and later linking. offset inside the
                     type_descriptor array for the class component or
                     type_descriptor_info for the descriptor component. */
    u1 nibble_count;    /**< The number of nibbles (half a byte). */
    u1* type;   /**< Array of nibbles. 2 nibbles per byte. */
} cf_type_descriptor;


/**
 * \brief Straightforward representation of an interface name.
 *
 * This structure is defined page 6-28 of Virtual Machine Specification, Java
 * Card Platform, v2.2.2.
 */
typedef struct {
    u1 interface_name_length;   /**< The length of the interface name. */
    u1* interface_name;         /**< The interface name. */
} cf_interface_name_info;


/**
 * \brief Straightforward representation of an interface.
 *
 * This structure is defined page 6-26 of Virtual Machine Specification, Java
 * Card Platform, v2.2.2.
 */
typedef struct {
    u1 flags;   /**< A combination of CLASS_ACC_INTERFACE and
                     CLASS_ACC_SHAREABLE and/or CLASS_ACC_REMOTE. Lower 4 bits
                     only. */
    u1 interface_count;                     /**< Lower 4 bits only */
    cf_class_ref_info* superinterfaces;     /**< Class references of the
                                                 superinterfaces. */
    u2 offset;                              /**< Added for usability and later
                                                 linking. */
    char has_interface_name;                /**< Added for usability */
    cf_interface_name_info interface_name;  /**< Only when M.m > 2.1 */
} cf_interface_info;


/**
 * \brief Straightforward representation of an implemented interface.
 *
 * This structure is defined page 6-32 of Virtual Machine Specification, Java
 * Card Platform, v2.2.2.
 */
typedef struct {
    cf_class_ref_info interface;    /**< A class reference to the implemented
                                         reference. */
    u1 count;                       /**< The number of methods defined by the
                                         interface. */
    u1* index;                      /**< Map interface methods token with
                                         implementing virtual method tokens. */
} cf_implemented_interface_info;


/**
 * \brief Straightforward representation of a remote method.
 *
 * This structure is defined page 6-33 of Virtual Machine Specification, Java
 * Card Platform, v2.2.2. Only when M.m > 2.1.
 */
typedef struct {
    u2 remote_method_hash;  /**< Remote method hash. */
    u2 signature_offset;    /**< Offset of the signature. */
    u1 virtual_method_token;    /**< Virtual method token. */
} cf_remote_method_info;


/**
 * \brief Straightforward representation of a remote interface.
 *
 * This structure is defined page 6-32 of Virtual Machine Specification, Java
 * Card Platform, v2.2.2. Only when M.m > 2.1.
 */
typedef struct {
    u1 remote_methods_count;    /**< Remote methods count. */
    cf_remote_method_info* remote_methods;  /**< Remote methods. */
    u1 hash_modifier_length;    /**< Hash modifier length. */
    u1* hash_modifier;      /**< Hash modifier. */
    u1 class_name_length;   /**< Class name length. */
    u1* class_name;     /**< Class name. */
    u1 remote_interfaces_count; /**< Remote interface count. */
    cf_class_ref_info* remote_interfaces;   /**< Remote interfaces. */
} cf_remote_interface_info;


/**
 * \brief Straightforward representation of a class.
 *
 * This structure is defined page 6-26 of Virtual Machine Specification, Java
 * Card Platform, v2.2.2.
 */
typedef struct {
    u1 flags;/**< A combination of CLASS_ACC_SHAREABLE and CLASS_ACC_REMOTE.
                  Lower 4 bits only. */
    u1 interface_count; /**< The interface count. Lower 4 bits only. */
    u1 has_superclass;  /**< Added for usability. */
    cf_class_ref_info super_class_ref;  /**< A class reference to the
                                             superclass. */
    u2 offset;  /**< Added for usability and later linking. */
    u1 declared_instance_size;  /**< Number of bytes / 2 for instance fields. */
    u1 first_reference_token;   /**< Token of the first reference type instance
                                     field. */
    u1 reference_count; /**< Number of reference type instance fields. */
    u1 public_method_table_base;    /**< See page 6-29. */
    u1 public_method_table_count;   /**< See page 6-29. */
    u1 package_method_table_base;   /**< See page 6-30. */
    u1 package_method_table_count;  /**< See page 6-30. */
    u2* public_virtual_method_table;    /**< See page 6-30. */
    u2* package_virtual_method_table;   /**< See page 6-31. */
    cf_implemented_interface_info* interfaces;  /**< Information on implemented
                                                    interfaces. */
    char has_remote_interfaces; /**< Added for usability. */
    cf_remote_interface_info remote_interfaces; /**< Information on remote
                                                     interfaces. Only when M.m
                                                     > 2.1. */
} cf_class_info;


/**
 * \brief Straightforward representation of the Class component.
 *
 * This structure is defined page 6-21 of Virtual Machine Specification, Java
 * Card Platform, v2.2.2.
 */
typedef struct {
    u1 tag;     /**< The component tag (should be 6). */
    u2 size;    /**< The size in byte of info[]. */
    char can_have_signature_pool;       /**< Added for usability */
    u2 signature_pool_count;            /**< Added for usability. Only when M.m
                                             > 2.1 */
    u2 signature_pool_length;           /**< Only when M.m > 2.1 */
    cf_type_descriptor* signature_pool; /**< Only when M.m > 2.1 */
    u2 interfaces_count;                /**< Added for usability */
    cf_interface_info* interfaces;      /**< Interfaces defined in the package. */
    u2 classes_count;                   /**< Added for usability */
    cf_class_info* classes;             /**< Classes defined in the package. */
} cf_class_component;


/**
 * \brief Straightforward representation of an exception handler.
 *
 * This structure is defined page 6-37 of Virtual Machine Specification, Java
 * Card Platform, v2.2.2.
 */
typedef struct {
    u2 start_offset;        /**< Offset of the try block.*/
    u1 stop_bit;            /**< Indicate the last catch block. Only the lower
                                 bit is meaningful. */
    u2 active_length;       /**< Length in byte of the try block. The higher
                                 bit should be ignored. */
    u2 handler_offset;      /**< Offset of the catch block.*/
    u2 catch_type_index;    /**< Index of the class reference constant pool
                                 entry of the catched exception. If equal to 0,
                                 it is a finally block. */
} cf_exception_handler_info;


/**
 * \brief Straightforward representation of a standard method header.
 *
 * This structure is defined page 6-39 of Virtual Machine Specification, Java
 * Card Platform, v2.2.2.
 */
typedef struct {
    u1 flags;       /**< 0 or METHOD_ACC_ABSTRACT */
    u1 max_stack;   /**< The number of 16bits cells for the stack. */
    u1 nargs;       /**< The number of 16bits cells for parameters. */
    u1 max_locals;  /**< The number of 16bits cells for locals. */
} cf_standard_method_header_info;


/**
 * \brief Straightforward representation of an extended method header.
 *
 * This structure is defined page 6-39 of Virtual Machine Specification, Java
 * Card Platform, v2.2.2.
 */
typedef struct {
    u1 flags;       /**< METHOD_ACC_EXTENDED eventually combined with
                         METHOD_ACC_ABSTRACT. */
    u1 padding;     /**< Padding (should be 0). */
    u1 max_stack;   /**< The number of 16bits cells for the stack. */
    u1 nargs;       /**< The number of 16bits cells for parameters. */
    u1 max_locals;  /**< The number of 16bits cells for locals. */
} cf_extended_method_header_info;


/**
 * \brief Straightforward representation of a method header.
 *
 * This structure is defined page 6-39 of Virtual Machine Specification, Java
 * Card Platform, v2.2.2.
 */
typedef union {
    u1 flags;   /**< A combinaison of METHOD_ACC_EXTENDED and
                     METHOD_ACC_ABSTRACT. */
    cf_standard_method_header_info standard_method_header;  /**< Defined if
                                                                 flags does not
                                                                 contain
                                                                 METHOD_ACC_EXTENDED. */
    cf_extended_method_header_info extended_method_header;  /**< Defined if
                                                                 flags contains
                                                                 METHOD_ACC_EXTENDED. */
} cf_method_header_info;


/**
 * \brief Straightforward representation of a method.
 *
 * This structure is defined page 6-38 of Virtual Machine Specification, Java
 * Card Platform, v2.2.2.
 */
typedef struct {
    u2 offset;          /**< Added for usability and later linking. It's the
                           offset of this structure inside info[] of method
                           component. */
    cf_method_header_info method_header;    /**< Information on the method. */
    u2 bytecode_count;  /**< Added for usability. Should come from the
                             descriptor component. */
    u1* bytecodes;  /**< The bytecodes. */
} cf_method_info;


/**
 * \brief Straightforward representation of the Method component.
 *
 * This structure is defined page 6-35 of Virtual Machine Specification, Java
 * Card Platform, v2.2.2.
 */
typedef struct {
    u1 tag;     /**< The component tag (should be 7). */
    u2 size;    /**< The size in byte of info[]. */
    u1 handler_count;   /**< The number of exception handlers. */
    cf_exception_handler_info* exception_handlers;  /**< The exception
                                                         handlers. */
    u2 method_count;    /**< Added for usability. Should come from the
                             descriptor component. */
    cf_method_info* methods;    /**< The methods for each classes defined in the
                                     package. */
} cf_method_component;


/**
 * \brief Straightforward representation of the initial values of a static
 *        array.
 *
 * This structure is defined page 6-43 of Virtual Machine Specification, Java
 * Card Platform, v2.2.2.
 */
typedef struct {
    u1 type;    /**< Should be 2, 3, 4 or 5. */
    u2 count;   /**< The number of values. */
    u1* values; /**< The initial values. */
} cf_array_init_info;


/**
 * \brief Straightforward representation of the Static Field component.
 *
 * This structure is defined page 6-41 of Virtual Machine Specification, Java
 * Card Platform, v2.2.2.
 */
typedef struct {
    u1 tag;     /**< The component tag (should be 8). */
    u2 size;    /**< The size in byte of info[]. */
    u2 image_size;  /**<  See page 6-42. */
    u2 reference_count; /**< Number of reference type static fields. */
    u2 array_init_count;    /**< Number of array_init entries. */
    cf_array_init_info* array_init; /**< Information on static arrays initial
                                        values. */
    u2 default_value_count;     /**< See page 6-44. */
    u2 non_default_value_count; /**< See page 6-44. */
    u1* non_default_values;     /**< See page 6-44. */
} cf_static_field_component;


/**
 * \brief Straightforward representation of the Reference Location component.
 *
 * This structure is defined page 6-44 of Virtual Machine Specification, Java
 * Card Platform, v2.2.2.
 */
typedef struct {
    u1 tag;     /**< The component tag (should be 9). */
    u2 size;    /**< The size in byte of info[]. */
    u2 byte_index_count;    /**< The number of entries in
                                 offset_to_byte_indices. */
    u1* offset_to_byte_indices; /**< See page 6-45. */
    u2 byte2_index_count;   /**< The number of entries in
                                 offset_to_byte2_indices. */
    u1* offset_to_byte2_indices;    /**< See page 6-46. */
} cf_reference_location_component;


/**
 * \brief Straightforward representation of an exported class.
 *
 * This structure is defined page 6-47 of Virtual Machine Specification, Java
 * Card Platform, v2.2.2.
 */
typedef struct {
    u2 class_offset;            /**< The offset of the exported class. */
    u1 static_field_count;      /**< The number of exported static fields in the
                                     exported class. */
    u1 static_method_count;     /**< The number of exported static methods in
                                     the exported class. */
    u2* static_field_offsets;   /**< The offset of the exported static fields. */
    u2* static_method_offsets;  /**< The offset of the exported static methods. */
} cf_class_export_info;


/**
 * \brief Straightforward representation of the Export component.
 *
 * This structure is defined page 6-47 of Virtual Machine Specification, Java
 * Card Platform, v2.2.2.
 */
typedef struct {
    u1 tag;     /**< The component tag (should be 10) */
    u2 size;    /**< The size in byte of info[]. */
    u1 class_count; /**< The number of exported classes. */
    cf_class_export_info* class_exports;    /**< Information on exported
                                                 classes. */
} cf_export_component;


/**
 * \brief Straightforward representation of a field reference.
 *
 * This structure is defined page 6-52 of Virtual Machine Specification, Java
 * Card Platform, v2.2.2.
 */
typedef union {
    cf_static_ref_info static_field;    /**< Static field reference. */
    struct {
        cf_class_ref_info class_ref;    /**< Class reference. */
        u1 token;   /**< Field token of the instance field. */
    } instance_field;   /**< Instance field reference. */
} cf_field_ref_info;


/**
 * \brief Straightforward representation of a field type.
 *
 * This structure is defined page 6-52 of Virtual Machine Specification, Java
 * Card Platform, v2.2.2.
 */
typedef union {
    u2 primitive_type;  /**< Should be 0x0002, 0x0003, 0x0004 or 0x0005. */
    u2 reference_type;  /**< Offset to a signature pool entry. */
} cf_type_info;


/**
 * \brief Straightforward representation of a field descriptor.
 *
 * This structure is defined page 6-52 of Virtual Machine Specification, Java
 * Card Platform, v2.2.2.
 */
typedef struct {
    u1 token;   /**< Token of the field. */
    u1 access_flags;    /**< See page 6-53. */
    cf_field_ref_info field_ref;    /**< Field reference to this field. */
    cf_type_info type;  /**< Type of the field. */
} cf_field_descriptor_info;


/**
 * \brief Straightforward representation of a method descriptor.
 *
 * This structure is defined page 6-54 of Virtual Machine Specification, Java
 * Card Platform, v2.2.2.
 */
typedef struct {
    u1 token;           /**< Token of the method. */
    u1 access_flags;    /**< See page 6-55. */
    u2 method_offset;   /**< Offset of the method in the Method component. */
    u2 type_offset;     /**< Offset to a signature pool entry. */
    u2 bytecode_count;  /**< Number of bytecodes. */
    u2 exception_handler_count; /**< Number of exception handlers in the method. */
    u2 exception_handler_index; /**< Index of the first exception handlers. */
} cf_method_descriptor_info;


/**
 * \brief Straightforward representation of a class descriptor.
 *
 * This structure is defined page 6-50 of Virtual Machine Specification, Java
 * Card Platform, v2.2.2.
 */
typedef struct {
    u1 token;           /**< Token of the class. */
    u1 access_flags;    /**< See page 6-51. */
    cf_class_ref_info this_class_ref;   /**< Class reference to this class. */
    u1 interface_count; /**< Number of implemented methods (should be 0 for an
                             interface). */
    u2 field_count;     /**< Number of fields defined (should be 0 for an
                             interface). */
    u2 method_count;    /**< Number of methods defined. */
    cf_class_ref_info* interfaces;  /**< Implemented interfaces. */
    cf_field_descriptor_info* fields;   /**< Defined fields. */
    cf_method_descriptor_info* methods; /**< Defined methods. */
} cf_class_descriptor_info;


/**
 * \brief Straightforward representation of the type descriptors.
 *
 * This structure is defined page 6-56 of Virtual Machine Specification, Java
 * Card Platform, v2.2.2.
 */
typedef struct {
    u2 constant_pool_count;     /**< Number of constant pool entries. */
    u2* constant_pool_types;    /**< Offset to a signature pool entry. */
    u2 type_desc_count; /**< Number of signature pool entries. Added for
                             usability. */
    cf_type_descriptor* type_desc;  /**< The signature pool. */
} cf_type_descriptor_info;


/**
 * \brief Straightforward representation of the Descriptor component.
 *
 * This structure is defined page 6-49 of Virtual Machine Specification, Java
 * Card Platform, v2.2.2.
 */
typedef struct {
    u1 tag;     /**< The component tag (should be 11). */
    u2 size;    /**< The size in byte of info[]. */
    u1 class_count; /**< The number of classes and interfaces. */
    cf_class_descriptor_info* classes;  /**< The defined classes and interfaces. */
    cf_type_descriptor_info types;  /**< The signature pool. */
} cf_descriptor_component;


/**
 * \brief Straightforward representation of an UTF8 string.
 *
 * This structure is defined page 6-58 of Virtual Machine Specification, Java
 * Card Platform, v2.2.2.
 */
typedef struct {
    u2 length;  /**< See page 6-58. */
    u1* bytes;  /**< See page 6-58. */
} cf_utf8_info;


/**
 * \brief Straightforward representation of an instance field token.
 *
 * This structure is defined page 6-61 of Virtual Machine Specification, Java
 * Card Platform, v2.2.2.
 */
typedef struct {
    u1 pad1;    /**< Padding (should be 0). */
    u1 pad2;    /**< Padding (should be 0). */
    u1 pad3;    /**< Padding (should be 0). */
    u1 token;   /**< See page 6-62. */
} cf_token_var_info;


/**
 * \brief Straightforward representation of a method offset.
 *
 * This structure is defined page 6-61 of Virtual Machine Specification, Java
 * Card Platform, v2.2.2.
 */
typedef struct {
    u2 pad;         /**< Padding (should be 0). */
    u2 location;    /**< See page 6-62. */
} cf_location_var_info;


/**
 * \brief Straightforward representation of a field.
 *
 * This structure is defined page 6-61 of Virtual Machine Specification, Java
 * Card Platform, v2.2.2.
 */
typedef union {
    cf_token_var_info token_var;        /**< See page 6-62. */
    cf_location_var_info location_var;  /**< See page 6-62. */
    u4 const_value;                     /**< See page 6-62. */
} cf_contents_info;


/**
 * \brief Straightforward representation of a field debug information.
 *
 * This structure is defined page 6-61 of Virtual Machine Specification, Java
 * Card Platform, v2.2.2.
 */
typedef struct {
    u2 name_index;              /**< See page 6-61. */
    u2 descriptor_index;        /**< See page 6-61. */
    u2 access_flags;            /**< See page 6-61. */
    cf_contents_info contents;  /**< See page 6-62. */
} cf_field_debug_info;


/**
 * \brief Straightforward representation of a method variables.
 *
 * This structure is defined page 6-64 of Virtual Machine Specification, Java
 * Card Platform, v2.2.2.
 */
typedef struct {
    u1 index;               /**< See page 6-64. */
    u2 name_index;          /**< See page 6-64. */
    u2 descriptor_index;    /**< See page 6-64. */
    u2 start_pc;            /**< See page 6-65. */
    u2 length;              /**< See page 6-65. */
} cf_variable_info;


/**
 * \brief Straightforward representation of the mapping between bytecode and
 *        lines of code.
 *
 * This structure is defined page 6-65 of Virtual Machine Specification, Java
 * Card Platform, v2.2.2.
 */
typedef struct {
    u2 start_pc;    /**< See page 6-65. */
    u2 end_pc;      /**< See page 6-65. */
    u2 source_line; /**< See page 6-65. */
} cf_line_info;


/**
 * \brief Straightforward representation of a method debug information.
 *
 * This structure is defined page 6-62 of Virtual Machine Specification, Java
 * Card Platform, v2.2.2.
 */
typedef struct {
    u2 name_index;          /**< See page 6-63. */
    u2 descriptor_index;    /**< See page 6-63. */
    u2 access_flags;        /**< See page 6-63. */
    u2 location;            /**< See page 6-63. */
    u1 header_size;         /**< See page 6-64. */
    u2 body_size;           /**< See page 6-64. */
    u2 variable_count;      /**< See page 6-64. */
    u2 line_count;          /**< See page 6-64. */
    cf_variable_info* variable_table;   /**< See page 6-64. */
    cf_line_info* line_table;           /**< See page 6-65. */
} cf_method_debug_info;


/**
 * \brief Straightforward representation of a class debug information.
 *
 * This structure is defined page 6-58 of Virtual Machine Specification, Java
 * Card Platform, v2.2.2.
 */
typedef struct {
    u2 name_index;                  /**< See page 6-59. */
    u2 access_flags;                /**< See page 6-59. */
    u2 location;                    /**< See page 6-59. */
    u2 superclass_name_index;       /**< See page 6-59. */
    u2 source_file_index;           /**< See page 6-60. */
    u1 interface_count;             /**< See page 6-60. */
    u2 field_count;                 /**< See page 6-60. */
    u2 method_count;                /**< See page 6-60. */
    u2* interface_names_indexes;    /**< See page 6-60. */
    cf_field_debug_info* fields;    /**< See page 6-60. */
    cf_method_debug_info* methods;  /**< See page 6-60. */
} cf_class_debug_info;


/**
 * \brief Straightforward representation of the Debug component.
 *
 * This structure is defined page 6-57 of Virtual Machine Specification, Java
 * Card Platform, v2.2.2.
 */
typedef struct {
    u1 tag;     /**< The component tag (should be 12). */
    u2 size;    /**< The size in byte of info[]. */
    u2 string_count;                /**< See page 6-60. */
    cf_utf8_info* strings_table;    /**< See page 6-58. */
    u2 package_name_index;          /**< See page 6-58. */
    u2 class_count;                 /**< See page 6-58. */
    cf_class_debug_info* classes;   /**< See page 6-58. */
} cf_debug_component;


/**
 * \brief Straightforward representation of a CAP file.
 *
 * This structure is defined page 6-1 of Virtual Machine Specification, Java
 * Card Platform, v2.2.2.
 */
typedef struct {
    char* path;     /**< The path before each component in the CAP file. */
    char* manifest; /**< The raw manifest as a null terminated string. */
    cf_header_component header; /**< The Header component. */
    cf_directory_component directory;   /**< The Directory component. */
    cf_applet_component applet; /**< The Applet component. */
    cf_import_component import; /**< The Import component. */
    cf_constant_pool_component constant_pool;   /**< The Constant Pool component. */
    cf_class_component class;   /**< The Class component. */
    cf_method_component method; /**< The Method component. */
    cf_static_field_component static_field; /**< The Static Field component. */
    cf_reference_location_component reference_location; /**< The Reference
                                                             Location component. */
    cf_export_component export; /**< The Export component. */
    cf_descriptor_component descriptor; /**< The Descriptor component. */
    cf_debug_component debug;   /**< The Debug component. Only when M.m > 2.1. */
} cap_file;

#endif
