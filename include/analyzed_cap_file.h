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
 * \file analyzed_cap_file.h
 * \brief This header defines an analyzed CAP file structure. This structure
 * can be allocated and filled by a call to analyze_cap_file().
 */

#ifndef ANALYZED_CAP_FILE_H
#define ANALYZED_CAP_FILE_H
#include "exp_file.h"

/**
 * \brief Information contained in the manifest.
 * 
 * Those information could not be generated from other sources so we keep them.
 */
typedef struct {
    char *version;              /**< Version of the manifest. */
    char *created_by;           /**< Creator of the manifest. */

    char *name;                 /**< Name. */
    char *package_name;         /**< The package name. */

    char *converter_provider;   /**< The converter provider. */
    char *converter_version;    /**< The converter version. */

    char *creation_time;        /**< The creation time. */
} manifest_info;


/**
 * \brief Information about a custom component. 
 */
typedef struct {
    u1 tag;     /**< A custom component tag. */
    u2 size;    /**< The size in byte of info[] in a custom component. */
    u1 aid_length;  /**< The length of the AID. */
    u1* aid;    /**< The AID. */
} custom_component_info;

/**
 * \brief Information about the package that should not be changed.
 */
typedef struct {
    char *path;     /**< The path before each component in the CAP file. */
    char *manifest; /**< The raw manifest. */

    u1 javacard_minor_version;  /**< The used Java Card minor version. */
    u1 javacard_major_version;  /**< The used Java Card major version. */

    u1 package_minor_version;   /**< The package minor version. */
    u1 package_major_version;   /**< The package major version. */
    u1 package_aid_length;      /**< The length of the AID. */
    u1 *package_aid;            /**< The package AID. */
    char has_package_name;      /**< Does the package have a name? */
    char *package_name;         /**< The package name. */

    u1 custom_count;            /**< The number of custom components. */
    custom_component_info *custom_components;   /**< The custom components. */
} cap_file_constant_info;


/**
 * \brief Information about an imported package.
 *
 * If an imported package is not referenced then it will be removed from the
 * Import component at the generation stage.
 */
typedef struct {
    u1 my_index;        /**< Index within the imported package array. */
    u2 count;           /**< How many time it was referred to. */

    u1 minor_version;   /**< The minor version of the imported package. */
    u1 major_version;   /**< The major version of the imported package. */
    u1 aid_length;      /**< The length of the AID. */
    u1 *aid;            /**< The AID of the imported package. */

    export_file* ef;    /**< The parsed export file representing this package. */
} imported_package_info;


struct constant_pool_entry_info;


/**
 * \brief Describe a type or part of a signature.
 *
 * Only one of the fields defining a type should true (!=0) at a time. If
 * is_reference or is_array_reference is true then ref should be set to
 * something before generating a CAP file.
 */
typedef struct {
#define TYPE_DESCRIPTOR_VOID    0x00    /**< Cannot be an array. */
#define TYPE_DESCRIPTOR_BOOLEAN 0x01    /**< A boolean ; can be an array. */
#define TYPE_DESCRIPTOR_BYTE    0x02    /**< A byte ; can be an array. */
#define TYPE_DESCRIPTOR_SHORT   0x04    /**< A short ; can be an array. */
#define TYPE_DESCRIPTOR_INT     0x08    /**< An int ; can be an array. */
#define TYPE_DESCRIPTOR_REF     0x10    /**< A ref ; cannot be an array. FIXME */
#define TYPE_DESCRIPTOR_ARRAY   0x20    /**< An array ; need a basic type. */
    u1 type;                            /**< The type of this one type descriptor. */
    struct constant_pool_entry_info* ref;    /**< The class reference constant
                                                 pool entry of the type. */

    /* Used before the second pass for reference type constant pool entry
       linking. */
    u1 is_external; /**< If !=0 then offset is used else p1 & c1 */
    u1 p1;          /**< The external package token. */
    u1 c1;          /**< The external class token. */
    u2 offset;      /**< The internal class offset. */
} one_type_descriptor_info;


/**
 * \brief Describe a type or a signature.
 * 
 * If type_count is equal to 1 then it can be type except if the type is void.
 */
typedef struct {
    u2 count;       /**< How many time it was referred to. */
    u2 offset;      /**< Offset within the signature pool. */

    u1 types_count;  /**< If equal to 1 it can be a type. */
    one_type_descriptor_info* types; /**< The type or signature. */
} type_descriptor_info;


/**
 * \brief Describe a field.
 * 
 * type field is redundant with the other fields typing the field if the field
 * type is boolean, byte, short or int.
 */
typedef struct {
    u1 token;   /**< The initial value is took from the original CAP file for
                     will be generated again anyway.*/
    u2 offset;  /**< Offset within the type_descriptor_info field (see page
                     6-56 of Virtual Machine Specification, Java Card Platform,
                     v2.2.2). */

    struct constant_pool_entry_info* this_field; /**< instance or static field
                                                     reference constant pool
                                                     entry from the analyzed
                                                     constant pool. */

#define FIELD_PUBLIC        0x01    /**< The field has public visibility. */
#define FIELD_PACKAGE       0x02    /**< The field has package visibility. */
#define FIELD_PROTECTED     0x04    /**< The field has protected visibility. */
#define FIELD_PRIVATE       0x08    /**< The field has private visibility. */
#define FIELD_STATIC        0x10    /**< The field is static. */
#define FIELD_FINAL         0x20    /**< The field is final. */
#define FIELD_HAS_VALUE     0x40    /**< The field has a value (static only). */
    u1 flags;                       /**< Describe a field properties. */

    type_descriptor_info* type; /**< The type is part of the analyzed signature
                                     pool. The type_count should be 1. */

    u2 value_size;  /**< The number of bytes used for the value. */
    u1* value;      /**< Used to generate the Static Field component. */
} field_info;


/* Forward declarations. */
struct class_info;
struct interface_info;
struct bytecode_info;
struct method_info;

/**
 * \brief Describe an analyzed constant pool entry.
 * 
 * The final constant pool will be created from bytecode references so more than
 * currently usefull constant pool entry can be added.
 */
typedef struct constant_pool_entry_info {
#define CONSTANT_POOL_CLASSREF          0x01
#define CONSTANT_POOL_INSTANCEFIELDREF  0x02
#define CONSTANT_POOL_VIRTUALMETHODREF  0x04
#define CONSTANT_POOL_SUPERMETHODREF    0x08
#define CONSTANT_POOL_STATICFIELDREF    0x10
#define CONSTANT_POOL_STATICMETHODREF   0x20
#define CONSTANT_POOL_IS_EXTERNAL       0x40
    u1 flags;     /**< The constant pool tag (should be between 1 and 6
                     included). */

    u2 my_index;  /**< Index within the constant pool array. */
    u2 count;     /**< How many time it was referred to. */

    type_descriptor_info* type; /**< type of the entry ; not applicable for
                                     classref. */

    imported_package_info* external_package;    /**< A link to the external
                                                     package if the entry is
                                                     external. */

    struct class_info* internal_class;          /**< A link to the interal
                                                     class refered to. If not
                                                     NULL, internal_interface
                                                     should be. */
    struct interface_info* internal_interface;  /**< A link to the internal
                                                     interface refered to. If
                                                     not NULL, internal_class
                                                     should be. */
    struct method_info* internal_method;        /**< A link to the internal
                                                     method. Not applicable for
                                                     classref and fieldrefs. */
    field_info* internal_field;                 /**< A link to the internal
                                                     field. Not applicable for
                                                     classref and methodrefs. */

    u1 external_class_token;    /**< A token refering to an external class. */
    u1 external_field_token;    /**< A token refering to an external field. */

    u1 method_token;            /**< A token refereing to a method. */
} constant_pool_entry_info;


/**
 * \brief Represent a case for an ilookupswitch bytecode.
 */
typedef struct {
    int32_t match;                  /**< The case value. */
    struct bytecode_info* branch;   /**< The analyzed bytecode should be part of
                                         the same method. */
} ilookupswitch_pair_info;


/**
 * \brief Represent an ilookupswitch bytecode.
 */
typedef struct {
    struct bytecode_info* default_branch;   /**< The analyzed bytecode should be
                                                 part of the same method. */
    u2 nb_cases;                            /**< The number of cases. */
    ilookupswitch_pair_info* cases;         /**< The cases. */
} ilookupswitch_info;


/**
 * \brief Represent a slookupswitch case.
 */
typedef struct {
    int16_t match;                  /**< The case value. */
    struct bytecode_info* branch;   /**< The analyzed bytecode should be
                                         part of the same method. */
} slookupswitch_pair_info;


/**
 * \brief Represent a slookupswitch bytecode.
 */
typedef struct {
    struct bytecode_info* default_branch;   /**< The analyzed bytecode should be
                                                 part of the same method. */
    u2 nb_cases;                            /**< The number of cases. */
    slookupswitch_pair_info* cases;         /**< The cases. */
} slookupswitch_info;


/**
 * \brief Represent an itableswitch bytecode.
 */
typedef struct {
    struct bytecode_info* default_branch;   /**< The analyzed bytecode should be
                                                 part of the same method. */
    u2 nb_cases;    /**< Should be high - low. Size of the branches array. */
    int32_t low;    /**< The low value of the cases. */
    int32_t high;   /**< The high value of the cases. */
    struct bytecode_info** branches;    /**< The analyzed bytecodes should be
                                             part of the same method. */
} itableswitch_info;


/**
 * \brief Represent a stableswitch bytecode.
 */
typedef struct {
    struct bytecode_info* default_branch;   /**< The analyzed bytecode should be
                                                 part of the same method. */
    u2 nb_cases;    /**< Should be high - low. Size of the branches array. */
    int16_t low;    /**< The low value of the cases. */
    int16_t high;   /**< The high value of the cases. */
    struct bytecode_info** branches;    /**< The analyzed bytecodes should be
                                             part of the same method. */
} stableswitch_info;


/**
 * \brief Represent an analyzed bytecode and its arguments.
 */
typedef struct bytecode_info {
    u1 opcode;          /**< The opcode of the bytecode (see page 7-1 of Virtual
                             Machine Specification, Java Card Platform,
                             v2.2.2). */
    u2 offset;          /**< Offset within the method. */
    u2 info_offset;     /**< Offset within info[] of the Method component. */

    u1 nb_args;         /**< Number of bytes representing the arguments for the
                             represented bytecode */

    u1 nb_byte_args;    /**< Number of byte arguments (do not include references
                             and branches) like pushed static final values and such. */
    u1 args[4];         /**< Value of the byte arguments. */

    u1 has_ref;         /**< Does the bytecode use a reference? */
    constant_pool_entry_info* ref;  /**< Should be set if has_ref is true (!=0). */

    u1 has_branch;      /**< Does the bytecode use a branch? */
    struct bytecode_info* branch;   /**< Should be set of has_branch is true
                                        (!=0). */

    ilookupswitch_info ilookupswitch;   /**< Should be set if opcode is equal to
                                             118. */
    slookupswitch_info slookupswitch;   /**< Should be set if opcode is equal to
                                             117. */
    itableswitch_info itableswitch;     /**< Should be set if opcode is equal to
                                             116. */
    stableswitch_info stableswitch;     /**< Should be set if opcode is equal to
                                             115. */
} bytecode_info;


/**
 * \brief Represent an exception handler.
 */
typedef struct {
    u1 stop_bit;                /**< Is it the last handler? */
    u1 my_index;                /**< Index within the exception handler array */
    struct method_info* try_in; /**< Method in which the exception try is done */
    bytecode_info* start;       /**< The analyzed bytecode should be in the
                                     try_in method. */
    bytecode_info* end;         /**< The analyzed bytecode should be in the
                                     try_in method. */
    bytecode_info* handler;     /**< The analyzed bytecode should be in the
                                     try_in method. */
    constant_pool_entry_info* catch_type;   /**< Should be an analyzed class
                                                 reference constant pool
                                                 entry. */
} exception_handler_info;


/**
 * \brief Represent a class or interface method.
 */
typedef struct method_info {
    u1 token;   /**< The method token. */
    u2 size;    /**< Used when generating a CAP file. */
    u2 offset;  /**< Used for linking and generated again later. */

    constant_pool_entry_info* this_method;  /**< Only for class method. Should
                                                 be analyzed internal static or
                                                 virtual method reference
                                                 constant pool entry. */
    u1 is_overriding;   /**< Is the method overriding a supermethod? */
    struct method_info* internal_overrided_method;  /**< If overriding an
                                                         external method or if
                                                         the token is already
                                                         fixed, this should be
                                                         NULL. */

#define METHOD_PUBLIC       0x0001  /**< The method has public visibility. */
#define METHOD_PACKAGE      0x0002  /**< The method has package visibility. */
#define METHOD_PROTECTED    0x0004  /**< The method has protected visibility. */
#define METHOD_PRIVATE      0x0008  /**< The method has private visibility. */
#define METHOD_STATIC       0x0010  /**< The method is static. */
#define METHOD_FINAL        0x0020  /**< The method is final. */
#define METHOD_ABSTRACT     0x0040  /**< The method is abstract. */
#define METHOD_INIT         0x0080  /**< The method is a class init. */
#define METHOD_EXTENDED     0x0100  /**< The method header is extended. */
    u2 flags;                       /**< Describe the method properties. */
    u1 max_stack;           /**< Is not generated but should. FIXME */
    u1 nargs;               /**< Is not generated but should. FIXME */
    u1 max_locals;          /**< Is not generated but should. FIXME */

    type_descriptor_info* signature;    /**< The signature is part of the
                                             analyzed signature pool. */ 

    u2 bytecodes_count; /**< The number of bytecodes (bytecodes args are not
                             counted). The size of the bytecodes array. */
    u2 bytecodes_size;  /**< The size of the bytecodes and their args in byte. */
    bytecode_info** bytecodes;  /**< The bytecodes of the method. */

    u1 exception_handlers_count;    /**< The number of exception handlers. */
    exception_handler_info** exception_handlers; /**< Exception handlers with a
                                                      try in this method. */
} method_info;


/**
 * \brief Describe an interface.
 */
typedef struct interface_info {
    u1 token;   /**< The interface token. */
    u2 size;    /**< Used when generating a CAP file. */
    u2 offset;  /**< Used for linking and generated again later. */

    constant_pool_entry_info* this_interface;   /**< Should be an analyzed
                                                     internal class reference
                                                     constant pool entry. */

#define INTERFACE_SHAREABLE 0x01    /**< Should be generated but is not. FIXME */
#define INTERFACE_REMOTE    0x02    /**< Java RMI is not really supported. FIXME */
#define INTERFACE_PUBLIC    0x04    /**< The interface has public visibilty. */
#define INTERFACE_PACKAGE   0x08    /**< The interface has package visibility. */
#define INTERFACE_ABSTRACT  0x10    /**< The interface is abstract (always). */
    u1 flags;                       /**< Describe the interface properties. */

    u1 superinterfaces_count;   /**< The number of superinterfaces. */
    constant_pool_entry_info** superinterfaces; /**< Should be analyzed class
                                                     reference constant pool
                                                     entry(ies). */

    u2 methods_count;       /**< The number of defined methods. */
    method_info** methods;  /**< The defined public abstract methods. */
} interface_info;


/**
 * \brief Describe an implemented method.
 */
typedef struct {
    method_info* declaration;       /**< Might be NULL if the interface is
                                         external. */
    u1 method_token;                /**< Usefull if the implementing method is
                                         external. */
    method_info* implementation;    /**< Might be NULL if the implementing
                                         method is external. */
} implemented_method_info;


/**
 * \brief Describe an implemented interface.
 */
typedef struct {
    constant_pool_entry_info* ref;  /**< Should be an analyzed class reference
                                         constant pool entry. */
    u1 count;                       /**< The number of implemented methods. */
    implemented_method_info* index; /**< The implemented methods. */
} implemented_interface_info;


/**
 * \brief Describe a class.
 */
typedef struct class_info {
    u1 token;   /**< The class token. */
    u2 size;    /**< Used when generating a CAP file. */
    u2 offset;  /**< Used for linking and genereted again later. */

    constant_pool_entry_info* this_class;   /**< Should be an analyzed internal
        class reference constant pool entry. */

#define CLASS_PUBLIC    0x01    /**< The class has public visibility. */
#define CLASS_PACKAGE   0x02    /**< The class has package visibility. */
#define CLASS_FINAL     0x04    /**< The class is final. */
#define CLASS_ABSTRACT  0x08    /**< The class is abstract. */
#define CLASS_SHAREABLE 0x10    /**< The class is shareable. FIXME not generated */
#define CLASS_REMOTE    0x20    /**< The class is remote. FIXME not supported */
#define CLASS_APPLET    0x40    /**< The class is an applet. */
    u1 flags;                   /**< Describe the class properties. */

    /* FIXME support for more than one applet per class. */
    char *name; /**< Name of the applet in the manifest. */
    u1 aid_length;  /**< The length of the AID. */
    u1 *aid;        /**< AID of the class (since it should be an applet). */
    method_info* install_method;    /**< This class implements an install
        method (and thus is considered as an applet). */

    constant_pool_entry_info* superclass;   /**< An analyzed class reference
        constant pool entry. NULL if the class has no superclass. */

    u1 interfaces_count;    /**< The number of implemented interfaces. */
    implemented_interface_info* interfaces; /**< The interfaces implemented by
        the class. */

    u2 fields_count;    /**< The number of fields defined in the class. Static
        final fields are not included. */
    field_info** fields;    /**< The fields defined in the class. Static final
        fields are not included. */

    u2 methods_count;   /**< The number of methods defined/implemented in the
        class. Defined abstract methods are included. Non-overriden methods are
        not included. */
    method_info** methods;  /**< The methods defined/implemented in the class.
        Defined abstract methods are included. Non-overriden methods are not
        included. */

    u1 has_largest_public_method_token; /**< Used for public method overriding */
    u1 largest_public_method_token; /**< The largest public method token for
        this class. */

    u1 has_largest_package_method_token;    /**< Used for package method
        overriding */
    u1 largest_package_method_token;    /**< The largest package method token
        for this class. */

    void *tweak;   /**< Added for usability when tweaking. */
} class_info;

/**
 * \brief The analyzed CAP file. 
 */
typedef struct {
    manifest_info manifest; /**< Parsed manifest of this CAP file */
    cap_file_constant_info info;    /**< Information about the CAP file. */

    u1 imported_packages_count; /**< Number of analyzed imported packages. */
    imported_package_info** imported_packages;  /**< Analyzed imported packages. */

    u2 interfaces_count;    /**< The number of analyzed interfaces defined in
        the CAP file. */
    interface_info** interfaces;    /**< The interfaces defined in the CAP
        file. */

    u2 classes_count;   /**< The number if analyzed classes defined in the CAP
        file. */
    class_info** classes;   /**< The classes defined in the CAP file. */

    u2 constant_pool_count; /**< The number of analyzed constant pool entries. */
    constant_pool_entry_info** constant_pool;   /**< The analyzed constant pool. */

    u2 signature_pool_count;    /**< The number of analyzed signature pool
        entries. */
    type_descriptor_info** signature_pool;  /**< The analyzed signature pool. */

    u1 exception_handlers_count;    /**< The number of analyzed exception
        handlers. */
    exception_handler_info** exception_handlers;    /**< The analyzed exception
        handlers. */
} analyzed_cap_file;

#endif
