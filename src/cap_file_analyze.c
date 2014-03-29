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
 * \file cap_file_analyze.c
 * \brief Convert a straightforward representation of CAP file into an analyzed one.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <linux/limits.h>
#include <sys/stat.h>
#include <unistd.h>

#include "cap_file.h"
#include "analyzed_cap_file.h"
#include "exp_file_reader.h"

 
/**
 * \brief Convert the straightforward representation of a CAP file type into a
 * human readable one.
 *
 * More precisely, convert a cf_type_descriptor structure into a
 * type_descriptor_info structure.  Nibbles are defined in 6.8.1 type_descriptor
 * of Virtual Machine Specification, Java Card Platform, v2.2.2. Class
 * references are linked later, raw values being kept.
 *
 * \param nibbles The straightforward representation of a signature/type by
 *                nibbles.
 * \param type    The human readable representation of a signature/type.
 *
 * \return Return -1 if an error occured, 0 else.
 */
static int analyze_nibbles(cf_type_descriptor* nibbles, type_descriptor_info* type) {

    u1 i = 0;
    type->types = NULL;
    type->types_count = 0;

    while(i < nibbles->nibble_count) {
        /* Each nibble is 4 bits long so we alternatively take the lower or higher part of each byte. */
        u1 crt_nibble = (i % 2) ? (nibbles->type[i / 2] & 0x0F) : (nibbles->type[i / 2] >> 4 );
        one_type_descriptor_info* tmp = (one_type_descriptor_info*)realloc(type->types, sizeof(one_type_descriptor_info) * (type->types_count + 1));
        if(tmp == NULL) {
            perror("analyze_nibbles");
            return -1;
        }
        type->types = tmp;

        memset(type->types + type->types_count, 0, sizeof(one_type_descriptor_info));

        if((crt_nibble != NIBBLE_REFERENCE) && (crt_nibble != NIBBLE_REFERENCE_ARRAY)) {
            switch(crt_nibble) {
                case NIBBLE_VOID:
                    type->types[type->types_count].type = TYPE_DESCRIPTOR_VOID;
                    break;

                case NIBBLE_BOOLEAN:
                    type->types[type->types_count].type = TYPE_DESCRIPTOR_BOOLEAN;
                    break;

                case NIBBLE_BYTE:
                    type->types[type->types_count].type = TYPE_DESCRIPTOR_BYTE;
                    break;

                case NIBBLE_SHORT:
                    type->types[type->types_count].type = TYPE_DESCRIPTOR_SHORT;
                    break;

                case NIBBLE_INT:
                    type->types[type->types_count].type = TYPE_DESCRIPTOR_INT;
                    break;

                case NIBBLE_BOOLEAN_ARRAY:
                    type->types[type->types_count].type = TYPE_DESCRIPTOR_BOOLEAN | TYPE_DESCRIPTOR_ARRAY;
                    break;

                case NIBBLE_BYTE_ARRAY:
                    type->types[type->types_count].type = TYPE_DESCRIPTOR_BYTE | TYPE_DESCRIPTOR_ARRAY;
                    break;

                case NIBBLE_SHORT_ARRAY:
                    type->types[type->types_count].type = TYPE_DESCRIPTOR_SHORT | TYPE_DESCRIPTOR_ARRAY;
                    break;

                case NIBBLE_INT_ARRAY:
                    type->types[type->types_count].type = TYPE_DESCRIPTOR_INT | TYPE_DESCRIPTOR_ARRAY;
                    break;

                default:
                    fprintf(stderr, "Nibble value not supported: 0x%X\n", crt_nibble);
                    return -1;
            }

        } else {
            if((i + 4) >= nibbles->nibble_count) {
                fprintf(stderr, "Missing nibble for a reference\n");
                return -1;
            }

            /* Linking will be done later so we keep the information about the class reference. */

            if(crt_nibble == NIBBLE_REFERENCE)
                type->types[type->types_count].type = TYPE_DESCRIPTOR_REF;
            else 
                type->types[type->types_count].type = TYPE_DESCRIPTOR_REF | TYPE_DESCRIPTOR_ARRAY;

            /* Taking next nibble */
            ++i;
            crt_nibble = (i % 2) ? (nibbles->type[i / 2] & 0x0F) : (nibbles->type[i / 2] >> 4 );

            type->types[type->types_count].is_external = crt_nibble >> 3;

            if(type->types[type->types_count].is_external) {
                /* High part of the package token is the current nibble. */
                type->types[type->types_count].p1 = (crt_nibble & 0x7) << 4;
                /* We fetch the lower part of the package and then the class token. */
                ++i;
                if(i % 2) {
                    type->types[type->types_count].p1 |= nibbles->type[i / 2] & 0x0F;
                    ++i;
                    type->types[type->types_count].c1 = nibbles->type[i / 2] & 0xF0;
                    ++i;
                    type->types[type->types_count].c1 |= nibbles->type[i / 2] & 0x0F;
                } else {
                    type->types[type->types_count].p1 |= nibbles->type[i / 2] >> 4;
                    ++i;
                    type->types[type->types_count].c1 = nibbles->type[i / 2] << 4;
                    ++i;
                    type->types[type->types_count].c1 |= nibbles->type[i / 2] >> 4;
                }
            } else {
                /* High part of the high byte of the internal class offset is the current nibble. */
                type->types[type->types_count].offset = ((u2)crt_nibble) << 12;
                /* We fetch the remaining internal class offset nibbles. */
                ++i;
                if(i % 2) {
                    type->types[type->types_count].offset |= ((u2)(nibbles->type[i / 2] & 0x0F)) << 8;
                    ++i;
                    type->types[type->types_count].offset |= nibbles->type[i / 2] & 0xF0;
                    ++i;
                    type->types[type->types_count].offset |= nibbles->type[i / 2] & 0x0F;
                } else {
                    type->types[type->types_count].offset |= ((u2)(nibbles->type[i / 2] & 0xF0)) << 4;
                    ++i;
                    type->types[type->types_count].offset |= (nibbles->type[i / 2] & 0x0F) << 4;
                    ++i;
                    type->types[type->types_count].offset |= (nibbles->type[i / 2] & 0xF0) >> 4;
                }
            }
        }

        ++type->types_count;
        ++i;
    }

    return 0;

}


/**
 * \brief Add an external class reference constant pool entry given its imported
 *        package and class token.
 *
 * No duplicate check is done before adding the class reference to the constant
 * pool.
 *
 * \param acf              The analyzed CAP file to which constant pool the new
 *                         entry is added.
 * \param imported_package The imported package of the class reference to add.
 *                         The imported package comes from the analyzed import
 *                         component.
 * \param class_token      The token of the external class of the class
 *                         reference to add.
 *
 * \return Return the newly added class reference constant pool entry or NULL if
 *         an error occurred.
 */
static constant_pool_entry_info* add_new_external_class_ref_to_constant_pool(analyzed_cap_file* acf, imported_package_info* imported_package, u1 class_token) {

    constant_pool_entry_info** tmp = (constant_pool_entry_info**)realloc(acf->constant_pool, sizeof(constant_pool_entry_info*) * (acf->constant_pool_count + 1));
    if(tmp == NULL) {
        perror("add_new_external_class_ref_to_constant_pool");
        return NULL;
    }
    acf->constant_pool = tmp;

    acf->constant_pool[acf->constant_pool_count] = (constant_pool_entry_info*)malloc(sizeof(constant_pool_entry_info));
    if(acf->constant_pool[acf->constant_pool_count] == NULL) {
        perror("add_new_external_class_ref_to_constant_pool");
        return NULL;
    }

    acf->constant_pool[acf->constant_pool_count]->flags = CONSTANT_POOL_CLASSREF | CONSTANT_POOL_IS_EXTERNAL;
    acf->constant_pool[acf->constant_pool_count]->my_index = acf->constant_pool_count;
    acf->constant_pool[acf->constant_pool_count]->count = 0;
    acf->constant_pool[acf->constant_pool_count]->external_package = imported_package;
    acf->constant_pool[acf->constant_pool_count]->external_class_token = class_token;
    acf->constant_pool[acf->constant_pool_count]->internal_class = NULL;
    acf->constant_pool[acf->constant_pool_count]->internal_interface = NULL;

    return acf->constant_pool[acf->constant_pool_count++];

}


/**
 * \brief Add internal class reference constant pool entry given the analyzed
 * class or interface.
 *
 * No duplicate check is done before adding the class reference to the constant
 * pool.
 *
 * \param acf                The analyzed CAP file to which constant pool the
 *                           new entry is added.
 * \param internal_class     The analyzed internal class for which a class
 *                           reference is created (if different from NULL).
 * \param internal_interface The analyzed internal interface for a class
 *                           reference is created (if different from NULL).
 *
 * \return Return the newly added class reference constant pool entry or NULL if
 *         an error occurred.
 */
static constant_pool_entry_info* add_new_internal_class_ref_to_constant_pool(analyzed_cap_file* acf, class_info* internal_class, interface_info* internal_interface) {

    constant_pool_entry_info** tmp = (constant_pool_entry_info**)realloc(acf->constant_pool, sizeof(constant_pool_entry_info*) * (acf->constant_pool_count + 1));
    if(tmp == NULL) {
        perror("add_new_internal_class_ref_to_constant_pool");
        return NULL;
    }
    acf->constant_pool = tmp;

    acf->constant_pool[acf->constant_pool_count] = (constant_pool_entry_info*)malloc(sizeof(constant_pool_entry_info));
    if(acf->constant_pool[acf->constant_pool_count] == NULL) {
        perror("add_new_internal_class_ref_to_constant_pool");
        return NULL;
    }

    acf->constant_pool[acf->constant_pool_count]->flags = CONSTANT_POOL_CLASSREF;
    acf->constant_pool[acf->constant_pool_count]->my_index = acf->constant_pool_count;
    acf->constant_pool[acf->constant_pool_count]->count = 0;
    acf->constant_pool[acf->constant_pool_count]->external_package = NULL;
    acf->constant_pool[acf->constant_pool_count]->external_class_token = 0xFF;
    acf->constant_pool[acf->constant_pool_count]->internal_class = internal_class;
    acf->constant_pool[acf->constant_pool_count]->internal_interface = internal_interface;

    return acf->constant_pool[acf->constant_pool_count++];

}


/**
 * \brief Analyze the constant information like package AID or custom
 * components.
 *
 * These informations should not change while tweaking the CAP file.
 *
 * \param acf The analyzed CAP file to which the constant informations will be
 *            added.
 * \param cf  The straightforward CAP file from which the constant informations
 *            will be fetched.
 * 
 * \return Return -1 if an error occurred, 0 else.
 */
static int analyze_constant_info(analyzed_cap_file* acf, cap_file* cf) {

    u1 u1Index = 0;

    acf->info.path = (char*)malloc(strlen(cf->path) + 1);
    if(acf->info.path == NULL) {
        perror("analyze_constant_info");
        return -1;
    }
    strcpy(acf->info.path, cf->path);

    acf->info.javacard_minor_version = cf->header.minor_version;
    acf->info.javacard_major_version = cf->header.major_version;

    acf->info.package_minor_version = cf->header.package.minor_version;
    acf->info.package_major_version = cf->header.package.major_version;
    acf->info.package_aid_length = cf->header.package.AID_length;
    acf->info.package_aid = (u1*)malloc(sizeof(u1) * acf->info.package_aid_length);
    if(acf->info.package_aid == NULL) {
        perror("analyze_constant_info");
        return -1;
    }
    memcpy(acf->info.package_aid, cf->header.package.AID, sizeof(u1) * acf->info.package_aid_length);

    if(cf->header.has_package_name) {
        acf->info.has_package_name = 1;
        acf->info.package_name = (char*)malloc(sizeof(char) * (cf->header.package_name.name_length + 1));
        if(acf->info.package_name == NULL) {
            perror("analyze_constant_info");
            return -1;
        }
        memcpy(acf->info.package_name, cf->header.package_name.name, sizeof(char) * cf->header.package_name.name_length);
        acf->info.package_name[cf->header.package_name.name_length] = '\0';
    } else {
        acf->info.has_package_name = 0;
        acf->info.package_name = NULL;
    }

    /* We read and now keep the custom components information but in fact we did not read the custom components... */
    acf->info.custom_count = cf->directory.custom_count;
    acf->info.custom_components = (custom_component_info*)malloc(sizeof(custom_component_info) * acf->info.custom_count);
    if(acf->info.custom_components == NULL) {
        perror("analyze_constant_info");
        return -1;
    }

    for(u1Index = 0; u1Index < acf->info.custom_count; ++u1Index) {
        acf->info.custom_components[u1Index].tag = cf->directory.custom_components[u1Index].component_tag;
        acf->info.custom_components[u1Index].size = cf->directory.custom_components[u1Index].size;
        acf->info.custom_components[u1Index].aid_length = cf->directory.custom_components[u1Index].AID_length;
        acf->info.custom_components[u1Index].aid = (u1*)malloc(sizeof(u1) * acf->info.custom_components[u1Index].aid_length);
        if(acf->info.custom_components[u1Index].aid == NULL) {
            perror("analyze_constant_info");
            return -1;
        }
        memcpy(acf->info.custom_components[u1Index].aid, cf->directory.custom_components[u1Index].AID, acf->info.custom_components[u1Index].aid_length);
    }

    return 0;

}


/**
 * \brief Analyze the imported packages.
 *
 * Export files linking is done later.
 *
 * \param acf The analyzed CAP file to which the imported packages will be
 *            added.
 * \param cf  The straightforward CAP file from which the imported packages
 *            will be fetched.
 * 
 * \return Return -1 if an error occurred, 0 else.
 */
static int analyze_imported_packages(analyzed_cap_file* acf, cap_file* cf) {

    u1 u1Index = 0;

    acf->imported_packages_count = cf->import.count;
    acf->imported_packages = (imported_package_info**)malloc(sizeof(imported_package_info*) * acf->imported_packages_count);
    if(acf->imported_packages == NULL) {
        perror("analyze_imported_packages");
        return -1;
    }

    for(u1Index = 0; u1Index < cf->import.count; ++u1Index) {
        acf->imported_packages[u1Index] = (imported_package_info*)malloc(sizeof(imported_package_info));
        if(acf->imported_packages[u1Index] == NULL) {
            perror("analyze_imported_packages");
            return -1;
        }

        acf->imported_packages[u1Index]->my_index = u1Index;
        acf->imported_packages[u1Index]->count = 0;
        acf->imported_packages[u1Index]->minor_version = cf->import.packages[u1Index].minor_version;
        acf->imported_packages[u1Index]->major_version = cf->import.packages[u1Index].major_version;
        acf->imported_packages[u1Index]->aid_length = cf->import.packages[u1Index].AID_length;
        acf->imported_packages[u1Index]->aid = (u1*)malloc(sizeof(u1) * acf->imported_packages[u1Index]->aid_length);
        if(acf->imported_packages[u1Index]->aid == NULL) {
            perror("analyze_imported_packages");
            return -1;
        }
        memcpy(acf->imported_packages[u1Index]->aid, cf->import.packages[u1Index].AID, sizeof(u1) * acf->imported_packages[u1Index]->aid_length);
    }

    return 0;

}


/**
 * \brief Analyze the signature pool of the descriptor component.
 *
 * This function mostly calls the analyze_nibbles function on each nibble from
 * the descriptor component. Class reference in nibbles will be linked later.
 *
 * \param acf The analyzed CAP file to which the signature pool will be
 *            added.
 * \param cf  The straightforward CAP file from which the signature pool will
 *            be fetched.
 * 
 * \return Return -1 if an error occurred, 0 else.
 */
static int analyze_signature_pool(analyzed_cap_file* acf, cap_file* cf) {

    u2 u2Index = 0;

    acf->signature_pool_count = cf->descriptor.types.type_desc_count;
    acf->signature_pool = (type_descriptor_info**)malloc(sizeof(type_descriptor_info*) * acf->signature_pool_count);
    if(acf->signature_pool == NULL) {
        perror("analyze_signature_pool");
        return -1;
    }

    for(; u2Index < cf->descriptor.types.type_desc_count; ++u2Index) {
        acf->signature_pool[u2Index] = (type_descriptor_info*)malloc(sizeof(type_descriptor_info));
        if(acf->signature_pool[u2Index] == NULL) {
            perror("analyze_signature_pool");
            return -1;
        }

        acf->signature_pool[u2Index]->count = 0;
        /* We keep the offset for later linking. */
        acf->signature_pool[u2Index]->offset = cf->descriptor.types.type_desc[u2Index].offset;

        if(analyze_nibbles(cf->descriptor.types.type_desc + u2Index, acf->signature_pool[u2Index]) == -1)
            return -1;
    }

    return 0;

}


/**
 * \brief Analyze the constant pool.
 *
 * Each constant pool entry is linked to its type descriptor (except class
 * references) from previously analyzed signature pool. Internal fields, methods
 * and classes reference linking is done later.
 * 
 * \param acf The analyzed CAP file to which the constant pool will be added.
 * \param cf  The straightforward CAP file from which the constant pool will be
 *            fetched.
 * 
 * \return Return -1 if an error occurred, 0 else.
 */
static int analyze_constant_pool(analyzed_cap_file* acf, cap_file* cf) {

    u2 u2Index1 = 0;

    acf->constant_pool_count = cf->constant_pool.count;
    acf->constant_pool = (constant_pool_entry_info**)malloc(sizeof(constant_pool_entry_info*) * acf->constant_pool_count);
    if(acf->constant_pool == NULL) {
        perror("analyze_constant_pool");
        return -1;
    }

    for(u2Index1 = 0; u2Index1 < cf->constant_pool.count; ++u2Index1) {
        u2 u2Index2 = 0;
        acf->constant_pool[u2Index1] = (constant_pool_entry_info*)malloc(sizeof(constant_pool_entry_info));
        if(acf->constant_pool[u2Index1] == NULL) {
            perror("analyze_constant_pool");
            return -1;
        }

        acf->constant_pool[u2Index1]->my_index = u2Index1;
        acf->constant_pool[u2Index1]->count = 0;

        switch(cf->constant_pool.constant_pool[u2Index1].tag) {
            case CF_CONSTANT_CLASSREF:
                acf->constant_pool[u2Index1]->flags = CONSTANT_POOL_CLASSREF;

                if(cf->constant_pool.constant_pool[u2Index1].CONSTANT_Classref.class_ref.isExternal) {
                    acf->constant_pool[u2Index1]->flags |= CONSTANT_POOL_IS_EXTERNAL;
                    /* We link the imported package to the class reference. */
                    acf->constant_pool[u2Index1]->external_package = acf->imported_packages[cf->constant_pool.constant_pool[u2Index1].CONSTANT_Classref.class_ref.ref.external_class_ref.package_token];
                    acf->constant_pool[u2Index1]->external_class_token = cf->constant_pool.constant_pool[u2Index1].CONSTANT_Classref.class_ref.ref.external_class_ref.class_token;
                } else {
                    /* Linking will be done later when classes or interface are analyzed. */
                    acf->constant_pool[u2Index1]->internal_class = NULL;
                    acf->constant_pool[u2Index1]->internal_interface = NULL;
                }
                break;

            case CF_CONSTANT_INSTANCEFIELDREF:
                acf->constant_pool[u2Index1]->flags = CONSTANT_POOL_INSTANCEFIELDREF;

                /* We fetch the field type from the analyzed signature pool using information from the Descriptor component. */
                for(;u2Index2 < cf->descriptor.types.type_desc_count; ++u2Index2)
                    if(cf->descriptor.types.constant_pool_types[u2Index1] == cf->descriptor.types.type_desc[u2Index2].offset) {
                        acf->constant_pool[u2Index1]->type = acf->signature_pool[u2Index2];
                        break;
                    }

                if(cf->constant_pool.constant_pool[u2Index1].CONSTANT_InstanceFieldref.class.isExternal) {
                    acf->constant_pool[u2Index1]->flags |= CONSTANT_POOL_IS_EXTERNAL;
                    /* We link the imported package to the instance field reference. */
                    acf->constant_pool[u2Index1]->external_package = acf->imported_packages[cf->constant_pool.constant_pool[u2Index1].CONSTANT_InstanceFieldref.class.ref.external_class_ref.package_token];
                    acf->constant_pool[u2Index1]->external_class_token = cf->constant_pool.constant_pool[u2Index1].CONSTANT_InstanceFieldref.class.ref.external_class_ref.class_token;
                    acf->constant_pool[u2Index1]->external_field_token = cf->constant_pool.constant_pool[u2Index1].CONSTANT_InstanceFieldref.token;
                } else {
                    /* Linking will be done later as part of the classes analysis. */ 
                    acf->constant_pool[u2Index1]->internal_class = NULL;
                    acf->constant_pool[u2Index1]->internal_field = NULL;
                }
                break;

            case CF_CONSTANT_VIRTUALMETHODREF:
                acf->constant_pool[u2Index1]->flags = CONSTANT_POOL_VIRTUALMETHODREF;

                /* We fetch the virtual method signature from the analyzed signature pool using information from the Descriptor component. */
                for(;u2Index2 < cf->descriptor.types.type_desc_count; ++u2Index2)
                    if(cf->descriptor.types.constant_pool_types[u2Index1] == cf->descriptor.types.type_desc[u2Index2].offset) {
                        acf->constant_pool[u2Index1]->type = acf->signature_pool[u2Index2];
                        break;
                    }

                if(cf->constant_pool.constant_pool[u2Index1].CONSTANT_VirtualMethodref.class.isExternal) {
                    acf->constant_pool[u2Index1]->flags |= CONSTANT_POOL_IS_EXTERNAL;
                    /* We link the imported package to the virtual method reference. */
                    acf->constant_pool[u2Index1]->external_package = acf->imported_packages[cf->constant_pool.constant_pool[u2Index1].CONSTANT_VirtualMethodref.class.ref.external_class_ref.package_token];
                    acf->constant_pool[u2Index1]->external_class_token = cf->constant_pool.constant_pool[u2Index1].CONSTANT_VirtualMethodref.class.ref.external_class_ref.class_token;
                    acf->constant_pool[u2Index1]->method_token = cf->constant_pool.constant_pool[u2Index1].CONSTANT_VirtualMethodref.token;
                } else {
                    /* Linking will be done later as part of the classes analysis. */
                    acf->constant_pool[u2Index1]->internal_class = NULL;
                    acf->constant_pool[u2Index1]->internal_method = NULL;
                }
                break;

            case CF_CONSTANT_SUPERMETHODREF:
                acf->constant_pool[u2Index1]->flags = CONSTANT_POOL_SUPERMETHODREF;

                /* We fetch the super method signature from the analyzed signature pool using information from the Descriptor component. */
                for(;u2Index2 < cf->descriptor.types.type_desc_count; ++u2Index2)
                    if(cf->descriptor.types.constant_pool_types[u2Index1] == cf->descriptor.types.type_desc[u2Index2].offset) {
                        acf->constant_pool[u2Index1]->type = acf->signature_pool[u2Index2];
                        break;
                    }

                /* External class reference for super method reference is in practice not possible */
                if(cf->constant_pool.constant_pool[u2Index1].CONSTANT_SuperMethodref.class.isExternal) {
                    acf->constant_pool[u2Index1]->flags |= CONSTANT_POOL_IS_EXTERNAL;
                    /* We link the imported package to the super method reference. */
                    acf->constant_pool[u2Index1]->external_package = acf->imported_packages[cf->constant_pool.constant_pool[u2Index1].CONSTANT_SuperMethodref.class.ref.external_class_ref.package_token];
                    acf->constant_pool[u2Index1]->external_class_token = cf->constant_pool.constant_pool[u2Index1].CONSTANT_SuperMethodref.class.ref.external_class_ref.class_token;
                } else {
                    /* Linking will be done later as part of classes analysis */
                    acf->constant_pool[u2Index1]->internal_class = NULL;
                    acf->constant_pool[u2Index1]->internal_method = NULL;
                }

                /* We keep the token but it might change anyway. */
                acf->constant_pool[u2Index1]->method_token = cf->constant_pool.constant_pool[u2Index1].CONSTANT_SuperMethodref.token;
                break;

            case CF_CONSTANT_STATICFIELDREF:
                acf->constant_pool[u2Index1]->flags = CONSTANT_POOL_STATICFIELDREF;

                /* We fetch the static field type from the analyzed signature pool using information from the Descriptor component. */
                for(;u2Index2 < cf->descriptor.types.type_desc_count; ++u2Index2)
                    if(cf->descriptor.types.constant_pool_types[u2Index1] == cf->descriptor.types.type_desc[u2Index2].offset) {
                        acf->constant_pool[u2Index1]->type = acf->signature_pool[u2Index2];
                        break;
                    }

                if(cf->constant_pool.constant_pool[u2Index1].CONSTANT_StaticFieldref.static_field_ref.isExternal) {
                    acf->constant_pool[u2Index1]->flags |= CONSTANT_POOL_IS_EXTERNAL;
                    /* We link the imported package to the static field reference. */
                    acf->constant_pool[u2Index1]->external_package = acf->imported_packages[cf->constant_pool.constant_pool[u2Index1].CONSTANT_StaticFieldref.static_field_ref.ref.external_ref.package_token];
                    acf->constant_pool[u2Index1]->external_class_token = cf->constant_pool.constant_pool[u2Index1].CONSTANT_StaticFieldref.static_field_ref.ref.external_ref.class_token;
                    acf->constant_pool[u2Index1]->external_field_token = cf->constant_pool.constant_pool[u2Index1].CONSTANT_StaticFieldref.static_field_ref.ref.external_ref.token;
                } else {
                    /* Linking will be done later as part of classes analysis. */
                    acf->constant_pool[u2Index1]->internal_class = NULL;
                    acf->constant_pool[u2Index1]->internal_field = NULL;
                }
                break;

            case CF_CONSTANT_STATICMETHODREF:
                acf->constant_pool[u2Index1]->flags = CONSTANT_POOL_STATICMETHODREF;

                /* We fetch the static method signature from the analyzed signature pool using information from the Descriptor component. */
                for(;u2Index2 < cf->descriptor.types.type_desc_count; ++u2Index2)
                    if(cf->descriptor.types.constant_pool_types[u2Index1] == cf->descriptor.types.type_desc[u2Index2].offset) {
                        acf->constant_pool[u2Index1]->type = acf->signature_pool[u2Index2];
                        break;
                    }

                if(cf->constant_pool.constant_pool[u2Index1].CONSTANT_StaticMethodref.static_method_ref.isExternal) {
                    acf->constant_pool[u2Index1]->flags |= CONSTANT_POOL_IS_EXTERNAL;
                    /* We link the imported package to the static method reference. */
                    acf->constant_pool[u2Index1]->external_package = acf->imported_packages[cf->constant_pool.constant_pool[u2Index1].CONSTANT_StaticMethodref.static_method_ref.ref.external_ref.package_token];
                    acf->constant_pool[u2Index1]->external_class_token = cf->constant_pool.constant_pool[u2Index1].CONSTANT_StaticMethodref.static_method_ref.ref.external_ref.class_token;
                    acf->constant_pool[u2Index1]->method_token = cf->constant_pool.constant_pool[u2Index1].CONSTANT_StaticMethodref.static_method_ref.ref.external_ref.token;
                } else {
                    /* Linking will be done later as part of classes analysis. */
                    acf->constant_pool[u2Index1]->internal_class = NULL;
                    acf->constant_pool[u2Index1]->internal_method = NULL;
                }
                break;
        }
    }

    return 0;

}


/**
 * \brief Analyze the superinterfaces of a particular interface.
 *
 * Each superinterfaces is linked to its class reference constant pool entries.
 * Missing constant pool entries are added.
 *
 * \param acf           The analyzed CAP file to which the constant pool will
 *                      be added.
 * \param new_interface The analyzed interface to which the analyzed
 *                      superinterfaces belong to.
 * \param old_interface The straightforward representation of the interface
 *                      containing superinterfaces offset.
 * 
 * \return Return -1 if an error occurred, 0 else.
 */
static int analyze_superinterfaces(analyzed_cap_file* acf, interface_info* new_interface, cf_interface_info* old_interface) {

    new_interface->superinterfaces_count = old_interface->interface_count;
    if(new_interface->superinterfaces_count == 0) {
        new_interface->superinterfaces = NULL;
    } else {
        u1 u1Index = 0;
        new_interface->superinterfaces = (constant_pool_entry_info**)malloc(sizeof(constant_pool_entry_info*) * new_interface->superinterfaces_count);
        if(new_interface->superinterfaces == NULL) {
            perror("analyze_superinterfaces");
            return -1;
        }

        for(; u1Index < new_interface->superinterfaces_count; ++u1Index) {
            new_interface->superinterfaces[u1Index] = NULL;
            if(old_interface->superinterfaces[u1Index].isExternal) {
                u2 u2Index = 0;
                for(; u2Index < acf->constant_pool_count; ++u2Index)
                    if((acf->constant_pool[u2Index]->flags & CONSTANT_POOL_CLASSREF) && (acf->constant_pool[u2Index]->flags & CONSTANT_POOL_IS_EXTERNAL) && (acf->constant_pool[u2Index]->external_package->my_index == old_interface->superinterfaces[u1Index].ref.external_class_ref.package_token) && (acf->constant_pool[u2Index]->external_class_token == old_interface->superinterfaces[u1Index].ref.external_class_ref.class_token)) {
                        new_interface->superinterfaces[u1Index] = acf->constant_pool[u2Index];
                        break;
                    }

                /* If the external superinterface reference is not in the constant pool then we add it. */
                if(new_interface->superinterfaces[u1Index] == NULL) {
                    new_interface->superinterfaces[u1Index] = add_new_external_class_ref_to_constant_pool(acf, acf->imported_packages[old_interface->superinterfaces[u1Index].ref.external_class_ref.package_token], old_interface->superinterfaces[u1Index].ref.external_class_ref.class_token);
                    if(new_interface->superinterfaces[u1Index] == NULL)
                        return -1;
                }
            } else {
                /* Since internal supertinterfaces are before extending interfaces, their class reference constant pool entries are assured to be present. */
                u2 u2Index = 0;
                for(; u2Index < acf->constant_pool_count; ++u2Index)
                    if(((acf->constant_pool[u2Index]->flags & (CONSTANT_POOL_CLASSREF|CONSTANT_POOL_IS_EXTERNAL)) == CONSTANT_POOL_CLASSREF) && acf->constant_pool[u2Index]->internal_interface && (acf->constant_pool[u2Index]->internal_interface->offset == old_interface->superinterfaces[u1Index].ref.internal_class_ref)) {
                        new_interface->superinterfaces[u1Index] = acf->constant_pool[u2Index];
                        break;
                    }
            }
        }
    }

    return 0;

}


/**
 * \brief Analyze each method defined by the given interface.
 * 
 * Methods signature linking is done.
 *
 * \param acf        The analyzed CAP file.
 * \param interface  The analyzed interface to which the analyzed methods
 *                   belong to.
 * \param descriptor The descriptor of the interface from the straightforward
 *                   representation of the CAP file.
 * \param types      The type descriptors used for matching a method with its
 *                   analyzed signature.
 *
 * \return Return -1 if an error occurred, 0 else.
 */
static int analyze_interface_methods(analyzed_cap_file* acf, interface_info* interface, cf_class_descriptor_info* descriptor, cf_type_descriptor_info* types) {

    u2 u2Index1 = 0;

    interface->methods_count = descriptor->method_count;
    interface->methods = (method_info**)malloc(sizeof(method_info*) * interface->methods_count);
    if(interface->methods == NULL) {
        perror("analyze_interface_methods");
        return -1;
    }

    for(; u2Index1 < descriptor->method_count; ++u2Index1) {
        u2 u2Index2 = 0;
        interface->methods[u2Index1] = (method_info*)malloc(sizeof(method_info));
        if(interface->methods[u2Index1] == NULL) {
            perror("analyze_interface_methods");
            return -1;
        }

        interface->methods[u2Index1]->token = descriptor->methods[u2Index1].token;
        interface->methods[u2Index1]->offset = 0;

        interface->methods[u2Index1]->is_overriding = 0;
        interface->methods[u2Index1]->internal_overrided_method = NULL;

        interface->methods[u2Index1]->flags = METHOD_PUBLIC | METHOD_ABSTRACT;

        for(u2Index2 = 0; u2Index2 < types->type_desc_count; ++u2Index2) {
            if(types->type_desc[u2Index2].offset == descriptor->methods[u2Index1].type_offset)
                break;
        }
        
        interface->methods[u2Index1]->signature = acf->signature_pool[u2Index2];

        interface->methods[u2Index1]->bytecodes_count = 0;
        interface->methods[u2Index1]->bytecodes = NULL;
    }

    return 0;

}


/**
 * \brief Analyze each interface of the CAP file including methods and
 *        superinterfaces.
 *
 * Class reference constant pool entries and signature pool entries linking are
 * done and missing entries are added.
 *
 * \param acf The analyzed CAP file to which the analyzed interfaces will be
 *            added.
 * \param cf  The straightforward CAP file from which the interfaces will be
 *            fetched.
 * 
 * \return Return -1 if an error occurred, 0 else.
 */
static int analyze_interfaces(analyzed_cap_file* acf, cap_file* cf) {

    u2 u2Index1 = 0;

    acf->interfaces_count = cf->class.interfaces_count;
    acf->interfaces = (interface_info**)malloc(sizeof(interface_info*) * acf->interfaces_count);
    if(acf->interfaces == NULL) {
        perror("analyze_cap_file");
        return -1;
    }

    for(; u2Index1 < acf->interfaces_count; ++u2Index1) {
        u1 descriptorIndex = 0;
        u2 u2Index2 = 0;

        acf->interfaces[u2Index1] = (interface_info*)malloc(sizeof(interface_info));
        if(acf->interfaces[u2Index1] == NULL) {
            perror("analyze_cap_file");
            return -1;
        }

        /* We fetch the descriptor for the analyzed interface. */
        for(descriptorIndex = 0; descriptorIndex < cf->descriptor.class_count; ++descriptorIndex)
            if((cf->descriptor.classes[descriptorIndex].access_flags & DESCRIPTOR_ACC_INTERFACE) && (cf->descriptor.classes[descriptorIndex].this_class_ref.ref.internal_class_ref == cf->class.interfaces[u2Index1].offset))
                break;

        acf->interfaces[u2Index1]->token = cf->descriptor.classes[descriptorIndex].token;

        acf->interfaces[u2Index1]->offset = cf->class.interfaces[u2Index1].offset;

        for(; u2Index2 < cf->constant_pool.count; ++u2Index2) {
            if((cf->constant_pool.constant_pool[u2Index2].tag == CF_CONSTANT_CLASSREF) && !cf->constant_pool.constant_pool[u2Index2].CONSTANT_Classref.class_ref.isExternal && (cf->constant_pool.constant_pool[u2Index2].CONSTANT_Classref.class_ref.ref.internal_class_ref == cf->class.interfaces[u2Index1].offset)) {
                acf->interfaces[u2Index1]->this_interface = acf->constant_pool[u2Index2];
                acf->constant_pool[u2Index2]->internal_interface = acf->interfaces[u2Index1];
                break;
            }
        }

        if(u2Index2 == cf->constant_pool.count) {
            acf->interfaces[u2Index1]->this_interface = add_new_internal_class_ref_to_constant_pool(acf, NULL, acf->interfaces[u2Index1]);
            if(acf->interfaces[u2Index1]->this_interface == NULL)
                return -1;
        }

        acf->interfaces[u2Index1]->flags = INTERFACE_ABSTRACT;

        /* Later it should be generated from extended interfaces. */
        if(cf->class.interfaces[u2Index1].flags & CLASS_ACC_SHAREABLE)
            acf->interfaces[u2Index1]->flags |= INTERFACE_SHAREABLE;

        if(cf->class.interfaces[u2Index1].flags & CLASS_ACC_REMOTE)
            acf->interfaces[u2Index1]->flags |= INTERFACE_REMOTE;

        if(cf->descriptor.classes[descriptorIndex].access_flags & DESCRIPTOR_ACC_PUBLIC)
            acf->interfaces[u2Index1]->flags |= INTERFACE_PUBLIC;

        if(analyze_superinterfaces(acf, acf->interfaces[u2Index1], cf->class.interfaces + u2Index1) == -1)
            return -1;

        if(analyze_interface_methods(acf, acf->interfaces[u2Index1], cf->descriptor.classes + descriptorIndex, &(cf->descriptor.types)) == -1)
            return -1;
    }

    return 0;

}


/**
 * \brief Add an internal static field reference constant pool entry given its
 *        type, class and field to the analyzed constant pool.
 *
 * \param acf            The analyzed CAP file to which the constant pool entry
 *                       is added.
 * \param type           The type of the field from the analyzed signature pool
 *                       entry.
 * \param internal_class The analyzed internal class.
 * \param internal_field The analyzed internal field.
 *
 * \return Return the newly added internal static field reference constant pool
 *         entry or NULL if an error occurred.
 */
static constant_pool_entry_info* add_new_internal_static_field_to_constant_pool(analyzed_cap_file* acf, type_descriptor_info* type, class_info* internal_class, field_info* internal_field) {

    constant_pool_entry_info** tmp = (constant_pool_entry_info**)realloc(acf->constant_pool, sizeof(constant_pool_entry_info*) * (acf->constant_pool_count + 1));
    if(tmp == NULL) {
        perror("add_new_internal_static_field_to_constant_pool");
        return NULL;
    }
    acf->constant_pool = tmp;

    acf->constant_pool[acf->constant_pool_count] = (constant_pool_entry_info*)malloc(sizeof(constant_pool_entry_info));
    if(acf->constant_pool[acf->constant_pool_count] == NULL) {
        perror("add_new_internal_static_field_to_constant_pool");
        return NULL;
    }

    acf->constant_pool[acf->constant_pool_count]->flags = CONSTANT_POOL_STATICFIELDREF;
    acf->constant_pool[acf->constant_pool_count]->my_index = acf->constant_pool_count;
    acf->constant_pool[acf->constant_pool_count]->count = 0;
    acf->constant_pool[acf->constant_pool_count]->type = type;
    acf->constant_pool[acf->constant_pool_count]->internal_class = internal_class;
    acf->constant_pool[acf->constant_pool_count]->internal_field = internal_field;

    return acf->constant_pool[acf->constant_pool_count++];

}


/**
 * \brief Add an internal instance field reference constant pool entry given its
 *        type, class and field to the analyzed constant pool.
 *
 * \param acf            The analyzed CAP file to which the constant pool entry
 *                       is added.
 * \param type           The type of the field from the analyzed signature pool
 *                       entry.
 * \param internal_class The analyzed internal class.
 * \param internal_field The analyzed internal field.
 *
 * \return Return the newly added internal instance field reference constant
 *         pool entry or NULL if an error occurred.
 */
static constant_pool_entry_info* add_new_internal_instance_field_to_constant_pool(analyzed_cap_file* acf, type_descriptor_info* type, class_info* internal_class, field_info* internal_field) {

    constant_pool_entry_info** tmp = (constant_pool_entry_info**)realloc(acf->constant_pool, sizeof(constant_pool_entry_info*) * (acf->constant_pool_count + 1));
    if(tmp == NULL) {
        perror("add_new_internal_instance_field_to_constant_pool");
        return NULL;
    }
    acf->constant_pool = tmp;

    acf->constant_pool[acf->constant_pool_count] = (constant_pool_entry_info*)malloc(sizeof(constant_pool_entry_info));
    if(acf->constant_pool[acf->constant_pool_count] == NULL) {
        perror("add_new_internal_instance_field_to_constant_pool");
        return NULL;
    }

    acf->constant_pool[acf->constant_pool_count]->flags = CONSTANT_POOL_INSTANCEFIELDREF;
    acf->constant_pool[acf->constant_pool_count]->my_index = acf->constant_pool_count;
    acf->constant_pool[acf->constant_pool_count]->count = 0;
    acf->constant_pool[acf->constant_pool_count]->type = type;
    acf->constant_pool[acf->constant_pool_count]->internal_class = internal_class;
    acf->constant_pool[acf->constant_pool_count]->internal_field = internal_field;

    return acf->constant_pool[acf->constant_pool_count++];

}


/**
 * \brief Get an array representing a static field initialization value from the
 *        static field component.
 * \param cf         The straighforward representation of the CAP file from
 *                   which we get the initial value.
 * \param offset     The offset of the initial value within the static image.
 * \param has_value  Is put to 1 if the initial value is different from the
 *                   default, 0 else.
 * \param value_size The size of the initial value in byte.
 *
 * \return Return the initial value as an allocated array or NULL if there is
 *         none. If NULL but has_value is 1 then it is an error.
 */
static u1* get_static_field_values(cap_file* cf, u2 offset, u1* has_value, u2* value_size) {

    u2 crt_offset = 0;
    u2 u2Index1 = 0;
    u1* values = NULL;

    if(offset < (cf->static_field.array_init_count * 2)) {
        u2 field_index = offset / 2;
        *has_value = 1;
        *value_size = cf->static_field.array_init[field_index].count;
        values = (u1*)malloc(cf->static_field.array_init[field_index].count);
        if(values == NULL) {
            perror("get_static_field_values");
            return NULL;
        }
        memcpy(values, cf->static_field.array_init[field_index].values, *value_size);

        return values;
    }

    crt_offset += (cf->static_field.reference_count * 2) + (cf->static_field.default_value_count);

    if(offset < crt_offset) {
        *has_value = 0;
        *value_size = 0;
        return NULL;
    } else {
        u2 u2Index2 = 0;

        *has_value = 1;
        values = (u1*)malloc(*value_size);
        for(u2Index1 = (crt_offset - offset); u2Index1 < *value_size; ++u2Index1)
            values[u2Index2++] = cf->static_field.non_default_values[u2Index1];
    }

    return values;

}


/**
 * \brief Find a primitive type descriptor from the analyzed signature pool. If
 *        not found, it is created.
 * 
 * \param acf        The analyzed CAP file from which the signature pool entry
 *                   is taken.
 * \param is_boolean Is equal to 1 if the searched for type is boolean, 0 else.
 * \param is_byte    Is equal to 1 if the searched for type is byte, 0 else.
 * \param is_short   Is equal to 1 if the searched for type is short, 0 else.
 * \param is_int     Is equal to 1 if the searched for type is int, 0 else.
 *
 * \return Return the analyzed signature pool entry (created if needed), NULL if
 *         an error occurred.
 */
static type_descriptor_info* find_type_descriptor(analyzed_cap_file* acf, u1 is_boolean, u1 is_byte, u1 is_short, u1 is_int) {

    type_descriptor_info** tmp = NULL;
    u2 u2Index = 0;

    for(; u2Index < acf->signature_pool_count; ++u2Index)
        if(acf->signature_pool[u2Index]->types_count == 1) {
            if(!(acf->signature_pool[u2Index]->types->type & TYPE_DESCRIPTOR_ARRAY) &&
               ((is_boolean && (acf->signature_pool[u2Index]->types->type & TYPE_DESCRIPTOR_BOOLEAN)) ||
               (is_byte && (acf->signature_pool[u2Index]->types->type & TYPE_DESCRIPTOR_BYTE)) ||
               (is_short && (acf->signature_pool[u2Index]->types->type & TYPE_DESCRIPTOR_SHORT)) ||
               (is_int && (acf->signature_pool[u2Index]->types->type & TYPE_DESCRIPTOR_INT))))
                return acf->signature_pool[u2Index];
        }

    tmp = (type_descriptor_info**)realloc(acf->signature_pool, sizeof(type_descriptor_info*) * (acf->signature_pool_count + 1));
    if(tmp == NULL) {
        perror("find_type_descriptor");
        return NULL;
    }
    acf->signature_pool = tmp;

    acf->signature_pool[acf->signature_pool_count] = (type_descriptor_info*)malloc(sizeof(type_descriptor_info));
    if(acf->signature_pool[acf->signature_pool_count] == NULL) {
        perror("find_type_descriptor");
        return NULL;
    }
    memset(acf->signature_pool[acf->signature_pool_count], 0, sizeof(type_descriptor_info));
    acf->signature_pool[acf->signature_pool_count]->types_count = 1;
    acf->signature_pool[acf->signature_pool_count]->types = (one_type_descriptor_info*)malloc(sizeof(one_type_descriptor_info) * acf->signature_pool[acf->signature_pool_count]->types_count);
    if(acf->signature_pool[acf->signature_pool_count]->types == NULL) {
        perror("find_type_descriptor");
        return NULL;
    }
    memset(acf->signature_pool[acf->signature_pool_count]->types, 0, sizeof(one_type_descriptor_info) * acf->signature_pool[acf->signature_pool_count]->types_count);
    if(is_boolean)
        acf->signature_pool[acf->signature_pool_count]->types->type = TYPE_DESCRIPTOR_BOOLEAN;
    else if(is_byte)
        acf->signature_pool[acf->signature_pool_count]->types->type = TYPE_DESCRIPTOR_BYTE;
    else if(is_short)
        acf->signature_pool[acf->signature_pool_count]->types->type = TYPE_DESCRIPTOR_SHORT;
    else
        acf->signature_pool[acf->signature_pool_count]->types->type = TYPE_DESCRIPTOR_INT;

    return acf->signature_pool[acf->signature_pool_count];

}


/**
 * \brief Analyze each field of a class.
 * 
 * Signature and constant pool entries linking is done. Missing signature or
 * constant pool entries are added. Static fields values are fetched from the
 * static field component.
 *
 * \param acf        The analyzed CAP file.
 * \param class      The analyzed class to which analyzed fields are added.
 * \param cf         The straightforward representation of the CAP file.
 * \param descriptor The descriptor of the class owning the fields.
 *
 * \return Return -1 if an error occurred, 0 else.
 */
static int analyze_class_fields(analyzed_cap_file* acf, class_info* class, cap_file* cf, cf_class_descriptor_info* descriptor) {

    u2 u2Index1 = 0;

    class->fields_count = descriptor->field_count;
    class->fields = (field_info**)malloc(sizeof(field_info*) * descriptor->field_count);
    if(class->fields == NULL) {
        perror("analyze_classes");
        return -1;
    }

    for(; u2Index1 < descriptor->field_count; ++u2Index1) {
        u2 u2Index2 = 0;

        class->fields[u2Index1] = (field_info*)malloc(sizeof(field_info));
        if(class->fields[u2Index1] == NULL) {
            perror("analyze_class_fields");
            return -1;
        }

        class->fields[u2Index1]->token = descriptor->fields[u2Index1].token;

        /* If neither of the three then it is package visibility. */

        if(descriptor->fields[u2Index1].access_flags & DESCRIPTOR_ACC_PUBLIC)
            class->fields[u2Index1]->flags = FIELD_PUBLIC;
        else if(descriptor->fields[u2Index1].access_flags & DESCRIPTOR_ACC_PRIVATE)
            class->fields[u2Index1]->flags = FIELD_PRIVATE;
        else if(descriptor->fields[u2Index1].access_flags & DESCRIPTOR_ACC_PROTECTED)
            class->fields[u2Index1]->flags = FIELD_PROTECTED;
        else
            class->fields[u2Index1]->flags = FIELD_PACKAGE;

        if(descriptor->fields[u2Index1].access_flags & DESCRIPTOR_ACC_STATIC)
            class->fields[u2Index1]->flags |= FIELD_STATIC;

        if(descriptor->fields[u2Index1].access_flags & DESCRIPTOR_ACC_FINAL)
            class->fields[u2Index1]->flags |= FIELD_FINAL;

        if(descriptor->fields[u2Index1].type.primitive_type & 0x8000) {
            /* Fetching the type descriptor from the analyzed signature pool (adding it neccessary). */
            if((class->fields[u2Index1]->type = find_type_descriptor(acf,
                                                                     descriptor->fields[u2Index1].type.primitive_type == 0x8002,
                                                                     descriptor->fields[u2Index1].type.primitive_type == 0x8003,
                                                                     descriptor->fields[u2Index1].type.primitive_type == 0x8004,
                                                                     descriptor->fields[u2Index1].type.primitive_type == 0x8005)) == NULL)
                return -1;
        } else {
            /* Fetching the type descriptor from the analyzed signature pool. */
            for(u2Index2 = 0; u2Index2 < cf->descriptor.types.type_desc_count; ++u2Index2) {
                if(acf->signature_pool[u2Index2]->offset == descriptor->fields[u2Index1].type.reference_type) {
                    class->fields[u2Index1]->type = acf->signature_pool[u2Index2];
                    break;
                }
            }
        }


        /* We go through the analyzed constant pool and link the matching instance or static field reference with the analyzed field. */
        for(u2Index2 = 0; u2Index2 < cf->constant_pool.count; ++u2Index2)
            if(class->fields[u2Index1]->flags & FIELD_STATIC) {
                if((cf->constant_pool.constant_pool[u2Index2].tag == CF_CONSTANT_STATICFIELDREF) && !cf->constant_pool.constant_pool[u2Index2].CONSTANT_StaticFieldref.static_field_ref.isExternal && (cf->constant_pool.constant_pool[u2Index2].CONSTANT_StaticFieldref.static_field_ref.ref.internal_ref.offset == descriptor->fields[u2Index1].field_ref.static_field.ref.internal_ref.offset)) {
                    acf->constant_pool[u2Index2]->internal_class = class;
                    acf->constant_pool[u2Index2]->internal_field = class->fields[u2Index1];
                    class->fields[u2Index1]->this_field = acf->constant_pool[u2Index2];
                    break;
                }
            } else {
                if((cf->constant_pool.constant_pool[u2Index2].tag == CF_CONSTANT_INSTANCEFIELDREF) && !cf->constant_pool.constant_pool[u2Index2].CONSTANT_InstanceFieldref.class.isExternal && (cf->constant_pool.constant_pool[u2Index2].CONSTANT_InstanceFieldref.class.ref.internal_class_ref == class->offset) && (cf->constant_pool.constant_pool[u2Index2].CONSTANT_InstanceFieldref.token == descriptor->fields[u2Index1].field_ref.instance_field.token)) {
                    acf->constant_pool[u2Index2]->internal_class = class;
                    acf->constant_pool[u2Index2]->internal_field = class->fields[u2Index1];
                    class->fields[u2Index1]->this_field = acf->constant_pool[u2Index2];
                    break;
                }
            }

        /* If we do not find it, we add it to the analyzed constant pool. */
        if(u2Index2 == cf->constant_pool.count) {
            if(class->fields[u2Index1]->flags & FIELD_STATIC) {
                if((class->fields[u2Index1]->this_field = add_new_internal_static_field_to_constant_pool(acf, class->fields[u2Index1]->type, class, class->fields[u2Index1])) == NULL)
                    return -1;
            } else {
                if((class->fields[u2Index1]->this_field = add_new_internal_instance_field_to_constant_pool(acf, class->fields[u2Index1]->type, class, class->fields[u2Index1])) == NULL)
                    return -1;
            }
        }

        /* If relevant, we try to fetch the initial value of the analyzed field. */
        if((class->fields[u2Index1]->flags & FIELD_STATIC) && (!(class->fields[u2Index1]->flags & FIELD_FINAL) || (class->fields[u2Index1]->type->types->type & (TYPE_DESCRIPTOR_ARRAY|TYPE_DESCRIPTOR_REF)))) {
            u1 has_value = 0;
            if(class->fields[u2Index1]->type->types->type & (TYPE_DESCRIPTOR_SHORT|TYPE_DESCRIPTOR_ARRAY|TYPE_DESCRIPTOR_REF))
                class->fields[u2Index1]->value_size = 2;
            else if(class->fields[u2Index1]->type->types->type & (TYPE_DESCRIPTOR_BOOLEAN|TYPE_DESCRIPTOR_BYTE))
                class->fields[u2Index1]->value_size = 1;
            else
                class->fields[u2Index1]->value_size = 4;
            class->fields[u2Index1]->value = get_static_field_values(cf, descriptor->fields[u2Index1].field_ref.static_field.ref.internal_ref.offset, &has_value, &(class->fields[u2Index1]->value_size));
            if((class->fields[u2Index1]->value == NULL) && has_value)
                return -1;
            if(has_value)
                class->fields[u2Index1]->flags |= FIELD_HAS_VALUE;
        }

    }

    return 0;

}


/**
 * \brief Get an analyzed bytecode given an absolute offset from a starting
 *        analyzed bytecode.
 * 
 * \param bytecodes       The analyzed bytecodes.
 * \param bytecodes_count The number of analyzed bytecodes.
 * \param start           The index of the starting analyzed bytecode (the
 *                        branching one).
 * \param offset          The offset of the targeted analyzed bytecode.
 * 
 * \return Return the found bytecode or NULL if it was not found.
 */
static bytecode_info* get_bytecode_from_offset(bytecode_info** bytecodes, u2 bytecodes_count, u2 start, u2 offset) {

    u2 u2Index = start;

    if(offset > bytecodes[start]->offset) {
        for(; u2Index < bytecodes_count; ++u2Index)
            if(bytecodes[u2Index]->offset == offset)
                return bytecodes[u2Index];
    } else {
        do {
            if(bytecodes[u2Index]->offset == offset)
                return bytecodes[u2Index];
        } while(u2Index-- != 0);
    }

    fprintf(stderr, "Could not find the bytecode: start_offset %u target offset %u\n", bytecodes[start]->offset, offset);
    return NULL;

}


/**
 * \brief Analyze the bytecodes of the given method.
 * 
 * Branch linking is done. Exception and constant pool reference linking is done
 * later.
 *
 * \param acf             The analyzed CAP file to which the analyzed bytecode
 *                        will be added.
 * \param method          The straightforward representation of the method from
 *                        which the bytecodes are taken.
 * \param bytecodes_count The number of bytecodes in the method.
 * \param crt_info_offset The current offset in info[] of the Method component.
 *
 * \return Return the analyzed bytecodes or NULL if an error occurred.
 */
static bytecode_info** analyze_bytecodes(analyzed_cap_file* acf, cf_method_info* method, u2* bytecodes_count, u2* crt_info_offset) {

    u2 u2Index1 = 0;
    *bytecodes_count = 0;
    bytecode_info** bytecodes = NULL;

    while(u2Index1 < method->bytecode_count) {
        bytecode_info** tmp = realloc(bytecodes, sizeof(bytecode_info*) * (*bytecodes_count + 1));
        if(tmp == NULL) {
            perror("analyze_bytecodes");
            return NULL;
        }
        bytecodes = tmp;
        bytecodes[*bytecodes_count] = (bytecode_info*)malloc(sizeof(bytecode_info));
        if(bytecodes[*bytecodes_count] == NULL) {
            perror("analyze_bytecodes");
            return NULL;
        }

        bytecodes[*bytecodes_count]->opcode = method->bytecodes[u2Index1];
        bytecodes[*bytecodes_count]->offset = u2Index1;
        bytecodes[*bytecodes_count]->info_offset = *crt_info_offset;
        bytecodes[*bytecodes_count]->nb_args = 0;
        bytecodes[*bytecodes_count]->nb_byte_args = 0;
        bytecodes[*bytecodes_count]->has_ref = 0;
        bytecodes[*bytecodes_count]->has_branch = 0;

        switch(method->bytecodes[u2Index1]) {
/* No operand */
            case 0:     /* nop */
            case 1:     /* aconst_null */
            case 2:     /* sconst_m1 */
            case 3:     /* sconst_0 */
            case 4:     /* sconst_1 */
            case 5:     /* sconst_2 */
            case 6:     /* sconst_3 */
            case 7:     /* sconst_4 */
            case 8:     /* sconst_5 */
            case 9:     /* iconst_m1 */
            case 10:    /* iconst_0 */
            case 11:    /* iconst_1 */
            case 12:    /* iconst_2 */
            case 13:    /* iconst_3 */
            case 14:    /* iconst_4 */
            case 15:    /* iconst_5 */
            case 24:    /* aload_0 */
            case 25:    /* aload_1 */
            case 26:    /* aload_2 */
            case 27:    /* aload_3 */
            case 28:    /* sload_0 */
            case 29:    /* sload_1 */
            case 30:    /* sload_2 */
            case 31:    /* sload_3 */
            case 32:    /* iload_0 */
            case 33:    /* iload_1 */
            case 34:    /* iload_2 */
            case 35:    /* iload_3 */
            case 36:    /* aaload */
            case 37:    /* baload */
            case 38:    /* saload */
            case 39:    /* iaload */
            case 43:    /* astore_0 */
            case 44:    /* astore_1 */
            case 45:    /* astore_2 */
            case 46:    /* astore_3 */
            case 47:    /* sstore_0 */
            case 48:    /* sstore_1 */
            case 49:    /* sstore_2 */
            case 50:    /* sstore_3 */
            case 51:    /* istore_0 */
            case 52:    /* istore_1 */
            case 53:    /* istore_2 */
            case 54:    /* istore_3 */
            case 55:    /* aastore */
            case 56:    /* bastore */
            case 57:    /* sastore */
            case 58:    /* iastore */
            case 59:    /* pop */
            case 60:    /* pop2 */
            case 61:    /* dup */
            case 62:    /* dup2 */
            case 65:    /* sadd */
            case 66:    /* iadd */
            case 67:    /* ssub */
            case 68:    /* isub */
            case 69:    /* smul */
            case 70:    /* imul */
            case 71:    /* sdiv */
            case 72:    /* idiv */
            case 73:    /* srem */
            case 74:    /* irem */
            case 75:    /* sneg */
            case 76:    /* ineg */
            case 77:    /* sshl */
            case 78:    /* ishl */
            case 79:    /* sshr */
            case 80:    /* ishr */
            case 81:    /* sushr */
            case 82:    /* iushr */
            case 83:    /* sand */
            case 84:    /* iand */
            case 85:    /* sor */
            case 86:    /* ior */
            case 87:    /* sxor */
            case 88:    /* ixor */
            case 91:    /* s2b */
            case 92:    /* s2i */
            case 93:    /* i2b */
            case 94:    /* i2s */
            case 95:    /* icmp */
            case 119:   /* areturn */
            case 120:   /* sreturn */
            case 121:   /* ireturn */
            case 122:   /* return */
            case 146:   /* arraylength */
            case 147:   /* athrow */
                *crt_info_offset += 1;
                break;

/* One operand but not a ref nor a branch*/
            case 16:    /* bspush */
            case 18:    /* bipush */
            case 21:    /* aload */
            case 22:    /* sload */
            case 23:    /* iload */
            case 40:    /* astore */
            case 41:    /* sstore */
            case 42:    /* istore */
            case 63:    /* dup_x */
            case 64:    /* swap_x */
            case 114:   /* ret */
            case 144:   /* newarray */
                bytecodes[*bytecodes_count]->nb_args = 1;
                bytecodes[*bytecodes_count]->nb_byte_args = 1;
                ++u2Index1;
                *crt_info_offset += 2;
                bytecodes[*bytecodes_count]->args[0] = method->bytecodes[u2Index1];
                break;

/* One operand but a branch */
            case 96:    /* ifeq */
            case 97:    /* ifne */
            case 98:    /* iflt */
            case 99:    /* ifge */
            case 100:   /* ifgt */
            case 101:   /* ifle */
            case 102:   /* ifnull */
            case 103:   /* ifnonnull */
            case 104:   /* if_acmpeq */
            case 105:   /* if_acmpne */
            case 106:   /* if_scmpeq */
            case 107:   /* if_scmpne */
            case 108:   /* if_scmplt */
            case 109:   /* if_scmpge */
            case 110:   /* if_scmpgt */
            case 111:   /* if_scmple */
            case 112:   /* goto */
                bytecodes[*bytecodes_count]->nb_args = 1;
                bytecodes[*bytecodes_count]->has_branch = 1;
                ++u2Index1;
                *crt_info_offset += 2;
                break;

/* One operand but a ref */
            case 131:   /* getfield_a */
            case 132:   /* getfield_b */
            case 133:   /* getfield_s */
            case 134:   /* getfield_i */
            case 135:   /* putfield_a */
            case 136:   /* putfield_b */
            case 137:   /* putfield_s */
            case 138:   /* putfield_i */
            case 173:   /* getfield_a_this */
            case 174:   /* getfield_b_this */
            case 175:   /* getfield_s_this */
            case 176:   /* getfield_i_this */
            case 181:   /* putfield_a_this */
            case 182:   /* putfield_b_this */
            case 183:   /* putfield_s_this */
            case 184:   /* putfiled_i_this */
                bytecodes[*bytecodes_count]->nb_args = 1;
                bytecodes[*bytecodes_count]->has_ref = 1;
                ++u2Index1;
                *crt_info_offset += 2;
                bytecodes[*bytecodes_count]->ref = acf->constant_pool[method->bytecodes[u2Index1]];
                break;

/* Two operands but not a wide ref */
            case 17:    /* sspush */
            case 19:    /* sipush */
            case 89:    /* sinc */
            case 90:    /* iinc */
                bytecodes[*bytecodes_count]->nb_args = 2;
                bytecodes[*bytecodes_count]->nb_byte_args = 2;
                ++u2Index1;
                bytecodes[*bytecodes_count]->args[0] = method->bytecodes[u2Index1];
                ++u2Index1;
                bytecodes[*bytecodes_count]->args[1] = method->bytecodes[u2Index1];
                *crt_info_offset += 3;
                break;

/* Two operands but a wide branch */
            case 113:   /* jsr */
            case 152:   /* ifeq_w */
            case 153:   /* ifne_w */
            case 154:   /* iflt_w */
            case 155:   /* ifge_w */
            case 156:   /* ifgt_w */
            case 157:   /* ifle_w */
            case 158:   /* ifnull_w */
            case 159:   /* ifnonnull_w */
            case 160:   /* if_acmpeq_w */
            case 161:   /* if_acmpne_w */
            case 162:   /* if_scmpeq_w */
            case 163:   /* if_scmpne_w */
            case 164:   /* if_scmplt_w */
            case 165:   /* if_scmpge_w */
            case 166:   /* if_scmpgt_w */
            case 167:   /* if_scmple_w */
            case 168:   /* goto_w */
                bytecodes[*bytecodes_count]->nb_args = 2;
                bytecodes[*bytecodes_count]->has_branch = 1;
                u2Index1 += 2;
                *crt_info_offset += 3;
                break;

/* Two operands but a wide ref */
            case 123:   /* getstatic_a */
            case 124:   /* getstatic_b */
            case 125:   /* getstatic_s */
            case 126:   /* getstatic_i */
            case 127:   /* putstatic_a */
            case 128:   /* putstatic_b */
            case 129:   /* putstatic_s */
            case 130:   /* putstatic_i */
            case 139:   /* invokevirtual */
            case 140:   /* invokespecial */
            case 141:   /* invokestatic */
            case 143:   /* new */
            case 145:   /* anewarray */
            case 169:   /* getfield_a_w */
            case 170:   /* getfield_b_w */
            case 171:   /* getfield_s_w */
            case 172:   /* getfield_i_w */
            case 177:   /* putfield_a_w */
            case 178:   /* putfield_b_w */
            case 179:   /* putfield_s_w */
            case 180:   /* putfield_i_w */
                bytecodes[*bytecodes_count]->nb_args = 2;
                bytecodes[*bytecodes_count]->has_ref = 1;
                bytecodes[*bytecodes_count]->ref = acf->constant_pool[(method->bytecodes[u2Index1 + 1] << 8) | method->bytecodes[u2Index1 + 2]];
                u2Index1 += 2;
                *crt_info_offset += 3;
                break;

/* Three operands */
            case 150:   /* sinc_w */
            case 151:   /* iinc_w */
                bytecodes[*bytecodes_count]->nb_args = 3;
                bytecodes[*bytecodes_count]->nb_byte_args = 3;
                ++u2Index1;
                bytecodes[*bytecodes_count]->args[0] = method->bytecodes[u2Index1];
                ++u2Index1;
                bytecodes[*bytecodes_count]->args[1] = method->bytecodes[u2Index1];
                ++u2Index1;
                bytecodes[*bytecodes_count]->args[2] = method->bytecodes[u2Index1];
                *crt_info_offset += 4;
                break;

/* Three operands with a wide ref */
            case 148:   /* checkcast */
            case 149:   /* instanceof */
                bytecodes[*bytecodes_count]->nb_args = 3;
                bytecodes[*bytecodes_count]->nb_byte_args = 1;
                ++u2Index1;
                bytecodes[*bytecodes_count]->args[0] = method->bytecodes[u2Index1];
                if(bytecodes[*bytecodes_count]->args[0] == 14 || bytecodes[*bytecodes_count]->args[0] == 0) {
                    bytecodes[*bytecodes_count]->has_ref = 1;
                    bytecodes[*bytecodes_count]->ref = acf->constant_pool[(method->bytecodes[u2Index1 + 1] << 8) | method->bytecodes[u2Index1 + 2]];
                } else {
                    bytecodes[*bytecodes_count]->nb_byte_args = 3;
                    bytecodes[*bytecodes_count]->args[1] = 0;
                    bytecodes[*bytecodes_count]->args[2] = 0;
                }
                u2Index1 += 2;
                *crt_info_offset += 4;
                break;

/* Four operands */
            case 20:    /* iipush */
                bytecodes[*bytecodes_count]->nb_args = 4;
                bytecodes[*bytecodes_count]->nb_byte_args = 4;
                ++u2Index1;
                bytecodes[*bytecodes_count]->args[0] = method->bytecodes[u2Index1];
                ++u2Index1;
                bytecodes[*bytecodes_count]->args[1] = method->bytecodes[u2Index1];
                ++u2Index1;
                bytecodes[*bytecodes_count]->args[2] = method->bytecodes[u2Index1];
                ++u2Index1;
                bytecodes[*bytecodes_count]->args[3] = method->bytecodes[u2Index1];
                *crt_info_offset += 5;
                break;

/* Four operands with refs */
            case 142:   /* invokeinterface */
                bytecodes[*bytecodes_count]->nb_args = 4;
                bytecodes[*bytecodes_count]->nb_byte_args = 2;
                ++u2Index1;
                bytecodes[*bytecodes_count]->args[0] = method->bytecodes[u2Index1];
                bytecodes[*bytecodes_count]->has_ref = 1;
                bytecodes[*bytecodes_count]->ref = acf->constant_pool[(method->bytecodes[u2Index1 + 1] << 8) | method->bytecodes[u2Index1 + 2]];
                u2Index1 += 3;
                bytecodes[*bytecodes_count]->args[1] = method->bytecodes[u2Index1];
                *crt_info_offset += 5;
                break;

/* LookupSwitch */
            case 117:   /* slookupswitch */
                bytecodes[*bytecodes_count]->nb_args = 4 + (((method->bytecodes[u2Index1 + 3] << 8) | method->bytecodes[u2Index1 + 4]) * 4);
                bytecodes[*bytecodes_count]->slookupswitch.nb_cases = (method->bytecodes[u2Index1 + 3] << 8) | method->bytecodes[u2Index1 + 4];
                u2Index1 += bytecodes[*bytecodes_count]->nb_args;
                *crt_info_offset += bytecodes[*bytecodes_count]->nb_args + 1;
                break;

            case 118:   /* ilookupswitch */
                bytecodes[*bytecodes_count]->nb_args = 4 + (((method->bytecodes[u2Index1 + 3] << 8) | method->bytecodes[u2Index1 + 4]) * 6);
                bytecodes[*bytecodes_count]->ilookupswitch.nb_cases = (method->bytecodes[u2Index1 + 3] << 8) | method->bytecodes[u2Index1 + 4];
                u2Index1 += bytecodes[*bytecodes_count]->nb_args;
                *crt_info_offset += bytecodes[*bytecodes_count]->nb_args + 1;
                break;

/* TableSwitch */
            case 115:   /* stableswitch */
                bytecodes[*bytecodes_count]->nb_args = 6 + ((((method->bytecodes[u2Index1 + 5] << 8) | method->bytecodes[u2Index1 + 6]) - ((method->bytecodes[u2Index1 + 3] << 8) | method->bytecodes[u2Index1 + 4]) + 1) * 2);
                bytecodes[*bytecodes_count]->stableswitch.low = (method->bytecodes[u2Index1 + 3] << 8) | method->bytecodes[u2Index1 + 4];
                bytecodes[*bytecodes_count]->stableswitch.high = (method->bytecodes[u2Index1 + 5] << 8) | method->bytecodes[u2Index1 + 6];
                bytecodes[*bytecodes_count]->stableswitch.nb_cases = bytecodes[*bytecodes_count]->stableswitch.high - bytecodes[*bytecodes_count]->stableswitch.low + 1;
                u2Index1 += bytecodes[*bytecodes_count]->nb_args;
                *crt_info_offset += bytecodes[*bytecodes_count]->nb_args + 1;
                break;

            case 116:   /* itableswitch */
                bytecodes[*bytecodes_count]->nb_args = 6 + ((((method->bytecodes[u2Index1 + 7] << 24) | (method->bytecodes[u2Index1 + 8] << 16) | (method->bytecodes[u2Index1 + 9] << 8) | method->bytecodes[u2Index1 + 10]) - ((method->bytecodes[u2Index1 + 3] << 24) | (method->bytecodes[u2Index1 + 4] << 16) | (method->bytecodes[u2Index1 + 5] << 8) | method->bytecodes[u2Index1 + 6]) + 1) * 2);
                bytecodes[*bytecodes_count]->itableswitch.low = (method->bytecodes[u2Index1 + 3] << 24) | (method->bytecodes[u2Index1 + 4] << 16) | (method->bytecodes[u2Index1 + 5] << 8) | method->bytecodes[u2Index1 + 6];
                bytecodes[*bytecodes_count]->itableswitch.high = (method->bytecodes[u2Index1 + 7] << 24) | (method->bytecodes[u2Index1 + 8] << 16) | (method->bytecodes[u2Index1 + 9] << 8) | method->bytecodes[u2Index1 + 10];
                bytecodes[*bytecodes_count]->itableswitch.nb_cases = bytecodes[*bytecodes_count]->itableswitch.high - bytecodes[*bytecodes_count]->itableswitch.low + 1;
                u2Index1 += bytecodes[*bytecodes_count]->nb_args;
                *crt_info_offset += bytecodes[*bytecodes_count]->nb_args + 1;
                break;

        }

        ++u2Index1;
        *bytecodes_count += 1;
    }

    /* Since every bytecode was analyzed, we can link branching ones to its target since it should be within the method. */
    for(u2Index1 = 0; u2Index1 < *bytecodes_count; ++u2Index1) {
        u2 u2Index2 = 0;

        if(bytecodes[u2Index1]->has_branch) {   /* Every bytecode with a branch except switch like bytecodes. */
            u2 offset_to_find = 0;

            if(bytecodes[u2Index1]->nb_args == 1)
                offset_to_find = bytecodes[u2Index1]->offset + (char)method->bytecodes[bytecodes[u2Index1]->offset + 1];
            else 
                offset_to_find = bytecodes[u2Index1]->offset + (int16_t)((method->bytecodes[bytecodes[u2Index1]->offset + 1] << 8) | method->bytecodes[bytecodes[u2Index1]->offset + 2]);

            bytecodes[u2Index1]->branch = get_bytecode_from_offset(bytecodes, *bytecodes_count, u2Index1, offset_to_find);
        } else if(bytecodes[u2Index1]->opcode == 115) { /* stableswitch */
            bytecodes[u2Index1]->stableswitch.branches = (bytecode_info**)malloc(sizeof(bytecode_info*) * (bytecodes[u2Index1]->stableswitch.nb_cases));
            if(bytecodes[u2Index1]->stableswitch.branches == NULL) {
                perror("analyze_bytecodes");
                return NULL;
            }

            bytecodes[u2Index1]->stableswitch.default_branch = get_bytecode_from_offset(bytecodes, *bytecodes_count, u2Index1, bytecodes[u2Index1]->offset + ((method->bytecodes[bytecodes[u2Index1]->offset + 1] << 8) | method->bytecodes[bytecodes[u2Index1]->offset + 2]));
            for(; u2Index2 < bytecodes[u2Index1]->stableswitch.nb_cases; ++u2Index2) {
                bytecodes[u2Index1]->stableswitch.branches[u2Index2] = get_bytecode_from_offset(bytecodes, *bytecodes_count, u2Index1, bytecodes[u2Index1]->offset + ((method->bytecodes[bytecodes[u2Index1]->offset + 6 + (u2Index2 * 2) + 1] << 8) | method->bytecodes[bytecodes[u2Index1]->offset + 6 + (u2Index2 * 2) + 2]));
            }
        } else if(bytecodes[u2Index1]->opcode == 116) { /* itableswitch */
            bytecodes[u2Index1]->itableswitch.branches = (bytecode_info**)malloc(sizeof(bytecode_info*) * (bytecodes[u2Index1]->itableswitch.nb_cases));
            if(bytecodes[u2Index1]->itableswitch.branches == NULL) {
                perror("analyze_bytecodes");
                return NULL;
            }

            bytecodes[u2Index1]->itableswitch.default_branch = get_bytecode_from_offset(bytecodes, *bytecodes_count, u2Index1, bytecodes[u2Index1]->offset + ((method->bytecodes[bytecodes[u2Index1]->offset + 1] << 8) | method->bytecodes[bytecodes[u2Index1]->offset + 2]));
            for(; u2Index2 < bytecodes[u2Index1]->itableswitch.nb_cases; ++u2Index2) {
                bytecodes[u2Index1]->itableswitch.branches[u2Index2] = get_bytecode_from_offset(bytecodes, *bytecodes_count, u2Index1, bytecodes[u2Index1]->offset + ((method->bytecodes[bytecodes[u2Index1]->offset + 10 + (u2Index2 * 2) + 1] << 8) | method->bytecodes[bytecodes[u2Index1]->offset + 10 + (u2Index2 * 2) + 2]));
            }
        } else if(bytecodes[u2Index1]->opcode == 117){  /* slookupswitch */
            bytecodes[u2Index1]->slookupswitch.cases = (slookupswitch_pair_info*)malloc(sizeof(slookupswitch_pair_info) * bytecodes[u2Index1]->slookupswitch.nb_cases);
            if(bytecodes[u2Index1]->slookupswitch.cases == NULL) {
                perror("analyze_bytecodes");
                return NULL;
            }

            bytecodes[u2Index1]->slookupswitch.default_branch = get_bytecode_from_offset(bytecodes, *bytecodes_count, u2Index1, bytecodes[u2Index1]->offset + (int16_t)((method->bytecodes[bytecodes[u2Index1]->offset + 1] << 8) | method->bytecodes[bytecodes[u2Index1]->offset + 2]));
            for(; u2Index2 < bytecodes[u2Index1]->slookupswitch.nb_cases; ++u2Index2) {
                bytecodes[u2Index1]->slookupswitch.cases[u2Index2].match = (method->bytecodes[bytecodes[u2Index1]->offset + 4 + (u2Index2 * 4) + 1] << 8) | method->bytecodes[bytecodes[u2Index1]->offset + 4 + (u2Index2 * 4) + 2];
                bytecodes[u2Index1]->slookupswitch.cases[u2Index2].branch = get_bytecode_from_offset(bytecodes, *bytecodes_count, u2Index1, bytecodes[u2Index1]->offset + (int16_t)((method->bytecodes[bytecodes[u2Index1]->offset + 4 + (u2Index2 * 4) + 3] << 8) | method->bytecodes[bytecodes[u2Index1]->offset + 4 + (u2Index2 * 4) + 4]));
            }
        } else if(bytecodes[u2Index1]->opcode == 118) { /* ilookupswitch */
            bytecodes[u2Index1]->ilookupswitch.cases = (ilookupswitch_pair_info*)malloc(sizeof(ilookupswitch_pair_info) * bytecodes[u2Index1]->ilookupswitch.nb_cases);
            if(bytecodes[u2Index1]->ilookupswitch.cases == NULL) {
                perror("analyze_bytecodes");
                return NULL;
            }

            bytecodes[u2Index1]->ilookupswitch.default_branch = get_bytecode_from_offset(bytecodes, *bytecodes_count, u2Index1, bytecodes[u2Index1]->offset + (int16_t)((method->bytecodes[bytecodes[u2Index1]->offset + 1] << 8) | method->bytecodes[bytecodes[u2Index1]->offset + 2]));
            for(; u2Index2 < bytecodes[u2Index1]->ilookupswitch.nb_cases; ++u2Index2) {
                bytecodes[u2Index1]->ilookupswitch.cases[u2Index2].match = (method->bytecodes[bytecodes[u2Index1]->offset + 4 + (u2Index2 * 6) + 1] << 24) | (method->bytecodes[bytecodes[u2Index1]->offset + 4 + (u2Index2 * 6) + 2] << 16) | (method->bytecodes[bytecodes[u2Index1]->offset + 4 + (u2Index2 * 6) + 3] << 8) | method->bytecodes[bytecodes[u2Index1]->offset + 4 + (u2Index2 * 6) + 4];
                bytecodes[u2Index1]->ilookupswitch.cases[u2Index2].branch = get_bytecode_from_offset(bytecodes, *bytecodes_count, u2Index1, bytecodes[u2Index1]->offset + (int16_t)((method->bytecodes[bytecodes[u2Index1]->offset + 4 + (u2Index2 * 6) + 5] << 8) | method->bytecodes[bytecodes[u2Index1]->offset + 4 + (u2Index2 * 6) + 6]));
            }
        }
    }

    return bytecodes;

}


/**
 * \brief Add an internal static method reference constant pool entry given its
 *        signature, class and method.
 * 
 * No check is done for redundancy.
 *
 * \param acf             The analyzed CAP file to which the constant pool
 *                        entry is added.  
 * \param signature       The analyzed signature of the method.
 * \param internal_class  The analyzed class to which the method pertains to.
 * \param internal_method The analyzed internal static method for which a
 *                        constant pool entry is added.
 *
 * \return Return the newly added internal static method constant pool entry or
 *         NULL if an error occurred.
 */
static constant_pool_entry_info* add_new_internal_static_method_to_constant_pool(analyzed_cap_file* acf, type_descriptor_info* signature, class_info* internal_class, method_info* internal_method) {

    constant_pool_entry_info** tmp = (constant_pool_entry_info**)realloc(acf->constant_pool, sizeof(constant_pool_entry_info*) * (acf->constant_pool_count + 1));
    if(tmp == NULL) {
        perror("add_new_internal_static_method_to_constant_pool");
        return NULL;
    }
    acf->constant_pool = tmp;

    acf->constant_pool[acf->constant_pool_count] = (constant_pool_entry_info*)malloc(sizeof(constant_pool_entry_info));
    if(acf->constant_pool[acf->constant_pool_count] == NULL) {
        perror("add_new_internal_static_method_to_constant_pool");
        return NULL;
    }

    acf->constant_pool[acf->constant_pool_count]->flags = CONSTANT_POOL_STATICMETHODREF;
    acf->constant_pool[acf->constant_pool_count]->my_index = acf->constant_pool_count;
    acf->constant_pool[acf->constant_pool_count]->count = 0;
    acf->constant_pool[acf->constant_pool_count]->type = signature;
    acf->constant_pool[acf->constant_pool_count]->internal_class = internal_class;
    acf->constant_pool[acf->constant_pool_count]->internal_method = internal_method;

    return acf->constant_pool[acf->constant_pool_count++];

}


/**
 * \brief Add an internal virtual method reference constant pool entry given its
 *        signature, class and method.
 *
 * No check is done for redundancy.
 * 
 * \param acf             The analyzed CAP file to which the constant pool
 *                        entry is added.  
 * \param signature       The analyzed signature of the method.
 * \param internal_class  The analyzed class to which the method pertains to.
 * \param internal_method The analyzed internal virtual method for which a
 *                        constant pool entry is added.
 *
 * \return Return the newly added internal virtual method constant pool entry or
 *         NULL if an error occurred.
 */
static constant_pool_entry_info* add_new_internal_virtual_method_to_constant_pool(analyzed_cap_file* acf, type_descriptor_info* signature, class_info* internal_class, method_info* internal_method) {

    constant_pool_entry_info** tmp = (constant_pool_entry_info**)realloc(acf->constant_pool, sizeof(constant_pool_entry_info*) * (acf->constant_pool_count + 1));
    if(tmp == NULL) {
        perror("add_new_internal_static_method_to_constant_pool");
        return NULL;
    }
    acf->constant_pool = tmp;

    acf->constant_pool[acf->constant_pool_count] = (constant_pool_entry_info*)malloc(sizeof(constant_pool_entry_info));
    if(acf->constant_pool[acf->constant_pool_count] == NULL) {
        perror("add_new_internal_static_method_to_constant_pool");
        return NULL;
    }

    acf->constant_pool[acf->constant_pool_count]->flags = CONSTANT_POOL_VIRTUALMETHODREF;
    acf->constant_pool[acf->constant_pool_count]->my_index = acf->constant_pool_count;
    acf->constant_pool[acf->constant_pool_count]->count = 0;
    acf->constant_pool[acf->constant_pool_count]->type = signature;
    acf->constant_pool[acf->constant_pool_count]->internal_class = internal_class;
    acf->constant_pool[acf->constant_pool_count]->internal_method = internal_method;

    return acf->constant_pool[acf->constant_pool_count++];

}


/**
 * \brief Analyze each method defined by a class.
 *
 * Bytecodes are analyzed. Signature and constant pool references linking is done
 * and missing ones are added. Install method linking is also done if applicable.
 *
 * \param acf         The analyzed CAP file.
 * \param class       The analyzed class to which the analyzed methods are
 *                    added.
 * \param cf          The straightforward representation of the CAP file.
 * \param descriptor  The descriptor from the Descriptor component of the class.
 * \param info_offset The current offset within info[] of the Method component.
 *
 * \return Return -1 if an error occurred, 0 else.
 */
static int analyze_class_methods(analyzed_cap_file* acf, class_info* class, cap_file* cf, cf_class_descriptor_info* descriptor, u2* info_offset) {

    u2 u2Index1 = 0;

    class->methods_count = descriptor->method_count;
    class->methods = (method_info**)malloc(sizeof(method_info*) * class->methods_count);
    if(class->methods == NULL) {
        perror("analyze_class_methods");
        return -1;
    }

    for(; u2Index1 < class->methods_count; ++u2Index1) {
        u2 u2Index2 = 0;

        class->methods[u2Index1] = (method_info*)malloc(sizeof(method_info));
        if(class->methods[u2Index1] == NULL) {
            perror("analyze_class_methods");
            return -1;
        }

        class->methods[u2Index1]->token = descriptor->methods[u2Index1].token;
        class->methods[u2Index1]->offset = descriptor->methods[u2Index1].method_offset;

        if(descriptor->methods[u2Index1].access_flags & DESCRIPTOR_ACC_PUBLIC)
            class->methods[u2Index1]->flags = METHOD_PUBLIC;
        else if(descriptor->methods[u2Index1].access_flags & DESCRIPTOR_ACC_PRIVATE)
            class->methods[u2Index1]->flags = METHOD_PRIVATE;
        else if(descriptor->methods[u2Index1].access_flags & DESCRIPTOR_ACC_PROTECTED)
            class->methods[u2Index1]->flags = METHOD_PROTECTED;
        else
            class->methods[u2Index1]->flags = METHOD_PACKAGE;

        if(descriptor->methods[u2Index1].access_flags & DESCRIPTOR_ACC_STATIC)
            class->methods[u2Index1]->flags |= METHOD_STATIC;

        if(descriptor->methods[u2Index1].access_flags & DESCRIPTOR_ACC_FINAL)
            class->methods[u2Index1]->flags |= METHOD_FINAL; 

        if(descriptor->methods[u2Index1].access_flags & DESCRIPTOR_ACC_ABSTRACT2)
            class->methods[u2Index1]->flags |= METHOD_ABSTRACT;

        if(descriptor->methods[u2Index1].access_flags & DESCRIPTOR_ACC_INIT)
            class->methods[u2Index1]->flags |= METHOD_INIT;

        /* Overriding will be determined later. */
        class->methods[u2Index1]->is_overriding = 0;
        class->methods[u2Index1]->internal_overrided_method = NULL;

        /* Exception handlers will be determined later. */
        class->methods[u2Index1]->exception_handlers_count = 0;
        class->methods[u2Index1]->exception_handlers = NULL;

        /* We should support more than one applet per class definition which is not the case for now .*/
        if(!(class->flags & CLASS_APPLET)) {
            u1 u1Index = 0;
            for(; u1Index < cf->applet.count; ++u1Index)
                if(descriptor->methods[u2Index1].method_offset == cf->applet.applets[u1Index].install_method_offset) {
                    class->flags |= CLASS_APPLET;
                    class->aid_length = cf->applet.applets[u1Index].AID_length;
                    class->aid = (u1*)malloc(class->aid_length);
                    if(class->aid == NULL) {
                        perror("analyze_class_methods");
                        return -1;
                    }
                    memcpy(class->aid, cf->applet.applets[u1Index].AID, class->aid_length);
                    class->install_method = class->methods[u2Index1];
                    break;
                }
        }

        /* We fetch the signature of the method. */
        for(u2Index2 = 0; u2Index2 < cf->descriptor.types.type_desc_count; ++u2Index2)
            if(descriptor->methods[u2Index1].type_offset == acf->signature_pool[u2Index2]->offset) {
                class->methods[u2Index1]->signature = acf->signature_pool[u2Index2];
                break;
            }

        /* We fetch the constant pool entry if any. */
        for(u2Index2 = 0; u2Index2 < cf->constant_pool.count; ++u2Index2) {
            if(class->methods[u2Index1]->flags & (METHOD_STATIC|METHOD_INIT|METHOD_PRIVATE)) {
                if((cf->constant_pool.constant_pool[u2Index2].tag == CF_CONSTANT_STATICMETHODREF) && !cf->constant_pool.constant_pool[u2Index2].CONSTANT_StaticMethodref.static_method_ref.isExternal && (cf->constant_pool.constant_pool[u2Index2].CONSTANT_StaticMethodref.static_method_ref.ref.internal_ref.offset == descriptor->methods[u2Index1].method_offset)) {
                    class->methods[u2Index1]->this_method = acf->constant_pool[u2Index2];
                    acf->constant_pool[u2Index2]->type = class->methods[u2Index1]->signature;
                    acf->constant_pool[u2Index2]->internal_class = class;
                    acf->constant_pool[u2Index2]->internal_method = class->methods[u2Index1];
                    break;
                }
            } else {
                if((cf->constant_pool.constant_pool[u2Index2].tag == CF_CONSTANT_VIRTUALMETHODREF) && !cf->constant_pool.constant_pool[u2Index2].CONSTANT_VirtualMethodref.class.isExternal && (cf->constant_pool.constant_pool[u2Index2].CONSTANT_VirtualMethodref.class.ref.internal_class_ref == class->offset) && (cf->constant_pool.constant_pool[u2Index2].CONSTANT_VirtualMethodref.token == class->methods[u2Index1]->token)) {
                    class->methods[u2Index1]->this_method = acf->constant_pool[u2Index2];
                    acf->constant_pool[u2Index2]->type = class->methods[u2Index1]->signature;
                    acf->constant_pool[u2Index2]->internal_class = class;
                    acf->constant_pool[u2Index2]->internal_method = class->methods[u2Index1];
                    break;
                }
            }
        }

        /* If the constant pool entry was not found then we add it. */
        if(u2Index2 == cf->constant_pool.count) {
            if(class->methods[u2Index1]->flags & (METHOD_STATIC|METHOD_INIT|METHOD_PRIVATE)) {
                if((class->methods[u2Index1]->this_method = add_new_internal_static_method_to_constant_pool(acf, class->methods[u2Index1]->signature, class, class->methods[u2Index1])) == NULL)
                    return -1;
            } else {
                if((class->methods[u2Index1]->this_method = add_new_internal_virtual_method_to_constant_pool(acf, class->methods[u2Index1]->signature, class, class->methods[u2Index1])) == NULL)
                    return -1;
            }
        }

        /* We fetch the descriptor of the method from the Descriptor component. */
        for(u2Index2 = 0; u2Index2 < cf->method.method_count; ++u2Index2)
            if(cf->method.methods[u2Index2].offset == descriptor->methods[u2Index1].method_offset)
                break;

        if(cf->method.methods[u2Index2].method_header.flags & METHOD_ACC_EXTENDED) {
            class->methods[u2Index1]->flags |= METHOD_EXTENDED;
            class->methods[u2Index1]->max_stack = cf->method.methods[u2Index2].method_header.extended_method_header.max_stack;
            class->methods[u2Index1]->nargs = cf->method.methods[u2Index2].method_header.extended_method_header.nargs;
            class->methods[u2Index1]->max_locals =cf->method.methods[u2Index2].method_header.extended_method_header.max_locals;
            *info_offset += 4;
        } else {
            class->methods[u2Index1]->max_stack = cf->method.methods[u2Index2].method_header.standard_method_header.max_stack;
            class->methods[u2Index1]->nargs = cf->method.methods[u2Index2].method_header.standard_method_header.nargs;
            class->methods[u2Index1]->max_locals =cf->method.methods[u2Index2].method_header.standard_method_header.max_locals;
            *info_offset += 2;
        }

        /* Mainly if the method is not abstract then we analyze its bytecodes. */
        if(cf->method.methods[u2Index2].bytecode_count != 0) {
            if((class->methods[u2Index1]->bytecodes = analyze_bytecodes(acf, cf->method.methods + u2Index2, &(class->methods[u2Index1]->bytecodes_count), info_offset)) == NULL)
                return -1;
        } else {
            class->methods[u2Index1]->bytecodes_count = 0;
            class->methods[u2Index1]->bytecodes = NULL;
        }

    }

    return 0;

}


/**
 * \brief Analyze each implemented interface.
 * 
 * Interfaces constant pool reference linking is done and missing ones are
 * added. Analyzed method linking is done if applicable. Implementing method
 * token is fetched if applicable. The class is not flagged as abstract if a
 * method is not implemented.
 *
 * \param acf      The analyzed CAP file.
 * \param class    The analyzed class implementing the interfaces.
 * \param cf_class The straightforward representation of the class.
 * 
 * \return Return -1 if an occurred, 0 else.
 */
static int analyze_implemented_interfaces(analyzed_cap_file* acf, class_info* class, cf_class_info* cf_class) {

    u1 u1Index1 = 0;

    class->interfaces_count = cf_class->interface_count;
    class->interfaces = (implemented_interface_info*)malloc(sizeof(implemented_interface_info) * cf_class->interface_count);
    if(class->interfaces == NULL) {
        perror("analyze_classes");
        return -1;
    }

    for(; u1Index1 < cf_class->interface_count; ++u1Index1) {
        u2 u2Index = 0;
        u1 u1Index2 = 0;

        if(cf_class->interfaces[u1Index1].interface.isExternal) {
            /* We fetch the constant pool entry for the external interface. */
            for(; u2Index < acf->constant_pool_count; ++u2Index)
                if((acf->constant_pool[u2Index]->flags & CONSTANT_POOL_CLASSREF) && (acf->constant_pool[u2Index]->flags & CONSTANT_POOL_IS_EXTERNAL) && (acf->constant_pool[u2Index]->external_package->my_index == cf_class->interfaces[u1Index1].interface.ref.external_class_ref.package_token) && (acf->constant_pool[u2Index]->external_class_token == cf_class->interfaces[u1Index1].interface.ref.external_class_ref.class_token)) {
                    class->interfaces[u1Index1].ref = acf->constant_pool[u2Index];
                    break;
                }

            /* If the constant pool entry was not found, we add it. */
            if(u2Index == acf->constant_pool_count) {
                class->interfaces[u1Index1].ref = add_new_external_class_ref_to_constant_pool(acf, acf->imported_packages[cf_class->interfaces[u1Index1].interface.ref.external_class_ref.package_token], cf_class->interfaces[u1Index1].interface.ref.external_class_ref.class_token);
                if(class->interfaces[u1Index1].ref == NULL)
                    return -1;
            }

            class->interfaces[u1Index1].count = cf_class->interfaces[u1Index1].count;
            class->interfaces[u1Index1].index = (implemented_method_info*)malloc(sizeof(implemented_method_info) * class->interfaces[u1Index1].count);
            if(class->interfaces[u1Index1].index == NULL) {
                perror("analyze_implemented_interfaces");
                return -1;
            }

            for(; u1Index2 < class->interfaces[u1Index1].count; ++u1Index2) {
                class_info* crt_class = class;

                /* The interface is external so the declaration is too. */
                class->interfaces[u1Index1].index[u1Index2].declaration = NULL;
                class->interfaces[u1Index1].index[u1Index2].method_token = cf_class->interfaces[u1Index1].index[u1Index2];
                class->interfaces[u1Index1].index[u1Index2].implementation = NULL;

                /* We search through the class hierarchy for the method implementation. */
                while(1) {
                    for(u2Index = 0; u2Index < crt_class->methods_count; ++u2Index)
                        if(crt_class->methods[u2Index]->token == cf_class->interfaces[u1Index1].index[u1Index2]) {
                            class->interfaces[u1Index1].index[u1Index2].implementation = crt_class->methods[u2Index];
                            break;
                        }

                    if(u2Index != crt_class->methods_count)
                        break;

                    /* If we do not find it in this class, we see in the superclass if not external. */
                    if(crt_class->superclass && !(crt_class->superclass->flags & CONSTANT_POOL_IS_EXTERNAL)) {
                        crt_class = crt_class->superclass->internal_class;
                    } else
                        break;
                }
            }
        } else {
            /* Since the interface is internal, it was already analyzed and its constant pool entry exists for sure. */
            for(; u2Index < acf->constant_pool_count; ++u2Index)
                if((acf->constant_pool[u2Index]->flags & CONSTANT_POOL_CLASSREF) && !(acf->constant_pool[u2Index]->flags & CONSTANT_POOL_IS_EXTERNAL) && acf->constant_pool[u2Index]->internal_interface && (acf->constant_pool[u2Index]->internal_interface->offset == cf_class->interfaces[u1Index1].interface.ref.internal_class_ref)) {
                    class->interfaces[u1Index1].ref = acf->constant_pool[u2Index];
                    break;
                }

            class->interfaces[u1Index1].count = cf_class->interfaces[u1Index1].count;
            class->interfaces[u1Index1].index = (implemented_method_info*)malloc(sizeof(implemented_method_info) * class->interfaces[u1Index1].count);
            if(class->interfaces[u1Index1].index == NULL) {
                perror("analyze_implemented_interfaces");
                return -1;
            }

            for(; u1Index2 < class->interfaces[u1Index1].count; ++u1Index2) {
                class_info* crt_class = class;

                class->interfaces[u1Index1].index[u1Index2].declaration = NULL;
                class->interfaces[u1Index1].index[u1Index2].method_token = cf_class->interfaces[u1Index1].index[u1Index2];
                class->interfaces[u1Index1].index[u1Index2].implementation = NULL;

                /* We fetch the declaration of the method within the analyzed method. */
                for(u2Index = 0; u2Index < class->interfaces[u1Index1].ref->internal_interface->methods_count; ++u2Index)
                    if(class->interfaces[u1Index1].ref->internal_interface->methods[u2Index]->token == u1Index2) {
                        class->interfaces[u1Index1].index[u1Index2].declaration = class->interfaces[u1Index1].ref->internal_interface->methods[u2Index];
                        break;
                    }

                /* We search through the class hierarchy for the method implementation. */
                while(1) {
                    for(u2Index = 0; u2Index < crt_class->methods_count; ++u2Index)
                        if(crt_class->methods[u2Index]->token == cf_class->interfaces[u1Index1].index[u1Index2]) {
                            class->interfaces[u1Index1].index[u1Index2].implementation = crt_class->methods[u2Index];
                            break;
                        }

                    if(u2Index != crt_class->methods_count)
                        break;

                    /* If we do not find it in this class, we see in the superclass if not external. */
                    if(crt_class->superclass && !(crt_class->superclass->flags & CONSTANT_POOL_IS_EXTERNAL)) {
                        crt_class = crt_class->superclass->internal_class;
                    } else
                        break;
                }
            }
        }

    }

    return 0;

}


/**
 * \brief Analyze each class of the CAP file.
 *
 * Fields, methods and implemented interfaces are analyzed for each class.
 * Superclass constant pool reference linking is done and missing ones are
 * added. Class constant pool reference linking is done and missing ones are
 * added.
 *
 * \param acf The analyzed CAP file to which the analyzed classes are added.
 * \param cf  The straightforward representation of the CAP file.
 *
 * \return Return -1 if an error occurred, 0 else.
 */
static int analyze_classes(analyzed_cap_file* acf, cap_file* cf) {

    u2 u2Index1 = 0;
    u2 info_offset = 1 + (cf->method.handler_count * 8);

    acf->classes_count = cf->class.classes_count;
    acf->classes = (class_info**)malloc(sizeof(class_info) * acf->classes_count);
    if(acf->classes == NULL) {
        perror("analyze_cap_file");
        return -1;
    }

    for(; u2Index1 < cf->class.classes_count; ++u2Index1) {
        u1 descriptorIndex = 0;
        u2 u2Index2 = 0;

        acf->classes[u2Index1] = (class_info*)malloc(sizeof(class_info));
        if(acf->classes[u2Index1] == NULL) {
            perror("analyze_classes");
            return -1;
        }

        /* We fetch the descriptor of the class from the Descriptor component. */
        for(descriptorIndex = 0; descriptorIndex < cf->descriptor.class_count; ++descriptorIndex)
            if(!(cf->descriptor.classes[descriptorIndex].access_flags & DESCRIPTOR_ACC_INTERFACE) && (cf->descriptor.classes[descriptorIndex].this_class_ref.ref.internal_class_ref == cf->class.classes[u2Index1].offset))
                break;

        acf->classes[u2Index1]->token = cf->descriptor.classes[descriptorIndex].token;
        acf->classes[u2Index1]->offset = cf->class.classes[u2Index1].offset;
        acf->classes[u2Index1]->name = NULL;
        acf->classes[u2Index1]->superclass = NULL;
        acf->classes[u2Index1]->interfaces_count = 0;
        acf->classes[u2Index1]->fields_count = 0;
        acf->classes[u2Index1]->methods_count = 0;

        for(; u2Index2 < cf->constant_pool.count; ++u2Index2)
            if((cf->constant_pool.constant_pool[u2Index2].tag == CF_CONSTANT_CLASSREF) && !cf->constant_pool.constant_pool[u2Index2].CONSTANT_Classref.class_ref.isExternal && (cf->constant_pool.constant_pool[u2Index2].CONSTANT_Classref.class_ref.ref.internal_class_ref == acf->classes[u2Index1]->offset)) {
                acf->classes[u2Index1]->this_class = acf->constant_pool[u2Index2];
                acf->constant_pool[u2Index2]->internal_class = acf->classes[u2Index1];
                break;
            }

        if(u2Index2 == cf->constant_pool.count) {
            acf->classes[u2Index1]->this_class = add_new_internal_class_ref_to_constant_pool(acf, acf->classes[u2Index1], NULL);
            if(acf->classes[u2Index1]->this_class == NULL)
                return -1;
        }

        if(cf->descriptor.classes[descriptorIndex].access_flags & DESCRIPTOR_ACC_PUBLIC)
            acf->classes[u2Index1]->flags = CLASS_PUBLIC;
        else
            acf->classes[u2Index1]->flags = CLASS_PACKAGE;

        if(cf->descriptor.classes[descriptorIndex].access_flags & DESCRIPTOR_ACC_FINAL)
            acf->classes[u2Index1]->flags |= CLASS_FINAL;

        if(cf->descriptor.classes[descriptorIndex].access_flags & DESCRIPTOR_ACC_ABSTRACT1)
            acf->classes[u2Index1]->flags |= CLASS_ABSTRACT;

        if(cf->class.classes[u2Index1].flags & CLASS_ACC_SHAREABLE)
            acf->classes[u2Index1]->flags |= CLASS_SHAREABLE;

        if(cf->class.classes[u2Index1].flags & CLASS_ACC_REMOTE)
            acf->classes[u2Index1]->flags |= CLASS_REMOTE;

        if(analyze_class_fields(acf, acf->classes[u2Index1], cf, cf->descriptor.classes + descriptorIndex) == -1)
            return -1;

        if(analyze_class_methods(acf, acf->classes[u2Index1], cf, cf->descriptor.classes + descriptorIndex, &info_offset) == -1)
            return -1;

        /* We use those for overriding determination later. */
        acf->classes[u2Index1]->has_largest_public_method_token = 0;
        acf->classes[u2Index1]->largest_public_method_token = 0;
        acf->classes[u2Index1]->has_largest_package_method_token = 0;
        acf->classes[u2Index1]->largest_package_method_token = 0;
    }

    /* Since every class was analyzed, we can link each class with its superclass constant pool entry and analyze implemented interfaces. */
    for(u2Index1 = 0; u2Index1 < cf->class.classes_count; ++u2Index1) {
        u2 u2Index2 = 0;
        if(cf->class.classes[u2Index1].has_superclass) {
            if(cf->class.classes[u2Index1].super_class_ref.isExternal) {
                for(u2Index2 = 0; u2Index2 < acf->constant_pool_count; ++u2Index2)
                    if(((acf->constant_pool[u2Index2]->flags & (CONSTANT_POOL_CLASSREF|CONSTANT_POOL_IS_EXTERNAL)) == (CONSTANT_POOL_CLASSREF|CONSTANT_POOL_IS_EXTERNAL)) && (acf->constant_pool[u2Index2]->external_package->my_index == cf->class.classes[u2Index1].super_class_ref.ref.external_class_ref.package_token) && (acf->constant_pool[u2Index2]->external_class_token == cf->class.classes[u2Index1].super_class_ref.ref.external_class_ref.class_token)) {
                        acf->classes[u2Index1]->superclass = acf->constant_pool[u2Index2];
                        break;
                    }
                if(u2Index2 == acf->constant_pool_count) {
                    acf->classes[u2Index1]->superclass = add_new_external_class_ref_to_constant_pool(acf, acf->imported_packages[cf->class.classes[u2Index1].super_class_ref.ref.external_class_ref.package_token], cf->class.classes[u2Index1].super_class_ref.ref.external_class_ref.class_token);
                    if(acf->classes[u2Index1]->superclass == NULL)
                        return -1;
                }
            } else {
                for(u2Index2 = 0; u2Index2 < acf->constant_pool_count; ++u2Index2)
                    if(((acf->constant_pool[u2Index2]->flags & (CONSTANT_POOL_CLASSREF|CONSTANT_POOL_IS_EXTERNAL)) == CONSTANT_POOL_CLASSREF) && acf->constant_pool[u2Index2]->internal_class && (acf->constant_pool[u2Index2]->internal_class->offset == cf->class.classes[u2Index1].super_class_ref.ref.internal_class_ref)) {
                        acf->classes[u2Index1]->superclass = acf->constant_pool[u2Index2];
                        break;
                    }
            }
        }

        if(analyze_implemented_interfaces(acf, acf->classes[u2Index1], cf->class.classes + u2Index1) == -1)
            return -1;
    }

    return 0;

}


/**
 * \brief Analyze exception handlers of the CAP file.
 *
 * Bytecodes linking is done for try-catch block. Catch type constant pool
 * reference linking is done.
 *
 * \param acf The analyzed CAP file to which the exception handlers are added.
 * \param cf  The straightforward representation of the CAP file.
 * 
 * \return Return -1 if an error occurred, 0 else.
 */
static int analyze_exception_handlers(analyzed_cap_file* acf, cap_file* cf) {

    u1 u1Index = 0;

    acf->exception_handlers_count = cf->method.handler_count;
    acf->exception_handlers = (exception_handler_info**)malloc(sizeof(exception_handler_info*) * acf->exception_handlers_count);
    if(acf->exception_handlers == NULL) {
        perror("analyze_exception_handlers");
        return -1;
    }

    for(;u1Index < cf->method.handler_count; ++u1Index) {
        u2 u2Index1 = 0;

        acf->exception_handlers[u1Index] = (exception_handler_info*)malloc(sizeof(exception_handler_info));
        if(acf->exception_handlers[u1Index] == NULL) {
            perror("analyze_exception_handlers");
            return -1;
        }

        acf->exception_handlers[u1Index]->stop_bit = cf->method.exception_handlers[u1Index].stop_bit;
        acf->exception_handlers[u1Index]->my_index = u1Index;
        acf->exception_handlers[u1Index]->try_in = NULL;
        acf->exception_handlers[u1Index]->start = NULL;
        acf->exception_handlers[u1Index]->end = NULL;
        acf->exception_handlers[u1Index]->handler = NULL;

        for(;u2Index1 < acf->classes_count; ++u2Index1) {
            u2 u2Index2 = 0;
            for(; u2Index2 < acf->classes[u2Index1]->methods_count; ++u2Index2) {
                u2 u2Index3 = 0;
                for(;u2Index3 < acf->classes[u2Index1]->methods[u2Index2]->bytecodes_count; ++u2Index3)
                    if(cf->method.exception_handlers[u1Index].start_offset == acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3]->info_offset) {
                        acf->exception_handlers[u1Index]->try_in = acf->classes[u2Index1]->methods[u2Index2];
                        acf->exception_handlers[u1Index]->start = acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3];
                        break;
                    }

                if(u2Index3 != acf->classes[u2Index1]->methods[u2Index2]->bytecodes_count) {
                    acf->exception_handlers[u1Index]->end = NULL;
                    for(;u2Index3 < acf->classes[u2Index1]->methods[u2Index2]->bytecodes_count; ++u2Index3)
                        if((cf->method.exception_handlers[u1Index].start_offset + cf->method.exception_handlers[u1Index].active_length) == acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3]->info_offset) {
                            acf->exception_handlers[u1Index]->end = acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3];
                            break;
                        }
                    break;
                }
            }
            if(u2Index2 != acf->classes[u2Index1]->methods_count)
                break;
        }

        for(u2Index1 = 0; u2Index1 < acf->classes_count; ++u2Index1) {
            u2 u2Index2 = 0;
            for(; u2Index2 < acf->classes[u2Index1]->methods_count; ++u2Index2) {
                u2 u2Index3 = 0;
                for(;u2Index3 < acf->classes[u2Index1]->methods[u2Index2]->bytecodes_count; ++u2Index3)
                    if(cf->method.exception_handlers[u1Index].handler_offset == acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3]->info_offset) {
                        exception_handler_info** tmp = (exception_handler_info**)realloc(acf->classes[u2Index1]->methods[u2Index2]->exception_handlers, sizeof(exception_handler_info*) * (acf->classes[u2Index1]->methods[u2Index2]->exception_handlers_count + 1));
                        if(tmp == NULL) {
                            perror("analyze_exception_handlers");
                            return -1;
                        }
                        acf->classes[u2Index1]->methods[u2Index2]->exception_handlers = tmp;

                        acf->classes[u2Index1]->methods[u2Index2]->exception_handlers[acf->classes[u2Index1]->methods[u2Index2]->exception_handlers_count] = acf->exception_handlers[u1Index];
                        ++acf->classes[u2Index1]->methods[u2Index2]->exception_handlers_count;

                        acf->exception_handlers[u1Index]->handler = acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3];
                        break;
                    }
                if(u2Index3 != acf->classes[u2Index1]->methods[u2Index2]->bytecodes_count)
                    break;
            }
            if(u2Index2 != acf->classes[u2Index1]->methods_count)
                break;
        }

        /* Is it a finally block or not ? */
        if(cf->method.exception_handlers[u1Index].catch_type_index != 0)
            acf->exception_handlers[u1Index]->catch_type = acf->constant_pool[cf->method.exception_handlers[u1Index].catch_type_index];
        else
            acf->exception_handlers[u1Index]->catch_type = NULL;

    }

    return 0;

}


/**
 * \brief Update the analyzed signature pool to link each reference type to a
 *        class reference constant pool entry.
 *
 * \param acf The analyzed CAP file owning the analyzed signature pool.
 * \param cf  The straightforward representation of the CAP file.
 *
 * \return Return -1 if an error occurred, 0 else.
 */
static int signature_pool_second_pass(analyzed_cap_file* acf, cap_file* cf) {

    u2 u2Index1 = 0;

    for(; u2Index1 < cf->descriptor.types.type_desc_count; ++u2Index1) {
        u1 u1Index = 0;
        for(; u1Index < acf->signature_pool[u2Index1]->types_count; ++u1Index)
            if(acf->signature_pool[u2Index1]->types[u1Index].type & TYPE_DESCRIPTOR_REF) {
                u2 u2Index2 = 0;
                if(acf->signature_pool[u2Index1]->types[u1Index].is_external) {
                    /* We fetch the constant pool entry for the external class reference. */ 
                    for(; u2Index2 < acf->constant_pool_count; ++u2Index2)
                        if(((acf->constant_pool[u2Index2]->flags & (CONSTANT_POOL_CLASSREF|CONSTANT_POOL_IS_EXTERNAL)) == (CONSTANT_POOL_CLASSREF|CONSTANT_POOL_IS_EXTERNAL)) && (acf->constant_pool[u2Index2]->external_package->my_index == acf->signature_pool[u2Index1]->types[u1Index].p1) && (acf->constant_pool[u2Index2]->external_class_token == acf->signature_pool[u2Index1]->types[u1Index].c1)) {
                            acf->signature_pool[u2Index1]->types[u1Index].ref = acf->constant_pool[u2Index2];
                            break;
                        }

                    /* If not found, we add it to the constant pool. */
                    if(u2Index2 == acf->constant_pool_count) {
                        if((acf->signature_pool[u2Index1]->types[u1Index].ref = add_new_external_class_ref_to_constant_pool(acf, acf->imported_packages[acf->signature_pool[u2Index1]->types[u1Index].p1], acf->signature_pool[u2Index1]->types[u1Index].c1)) == NULL)
                            return -1;
                    }
                } else {
                    /* Every class was analyzed so the constant pool entry exists for sure. */
                    for(; u2Index2 < acf->constant_pool_count; ++u2Index2)
                        if(((acf->constant_pool[u2Index2]->flags & (CONSTANT_POOL_CLASSREF|CONSTANT_POOL_IS_EXTERNAL)) == CONSTANT_POOL_CLASSREF) && ((acf->constant_pool[u2Index2]->internal_class && (acf->constant_pool[u2Index2]->internal_class->offset == acf->signature_pool[u2Index1]->types[u1Index].offset)) || (acf->constant_pool[u2Index2]->internal_interface && (acf->constant_pool[u2Index2]->internal_interface->offset == acf->signature_pool[u2Index1]->types[u1Index].offset)))) {
                            acf->signature_pool[u2Index1]->types[u1Index].ref = acf->constant_pool[u2Index2];
                            break;
                        }
                    if(u2Index2 == acf->constant_pool_count) {
                        fprintf(stderr, "Cannot find ref for signature pool entry\n");
                        return -1;
                    }
                }
            }
    }

    return 0;

}


/** \brief Update each supermethod reference constant pool entry with its
 *         analyzed class and methods.
 *
 * \param acf The analyzed CAP file owning the constant pool entries.
 * \param cf  The straightfoward representation of the CAP file.
 * 
 * \return Return -1 if an error occurred, 0 else.
 */
static int super_method_ref_second_pass(analyzed_cap_file* acf, cap_file* cf) {

    u2 u2Index1 = 0;

    for(; u2Index1 < cf->constant_pool.count; ++u2Index1) {
        if((acf->constant_pool[u2Index1]->flags & (CONSTANT_POOL_SUPERMETHODREF|CONSTANT_POOL_IS_EXTERNAL)) == CONSTANT_POOL_SUPERMETHODREF) {
                u2 u2Index2 = 0;
                /* We fetch the analyzed class. */
                for(; u2Index2 < acf->classes_count; ++u2Index2)
                    if(acf->classes[u2Index2]->offset == cf->constant_pool.constant_pool[u2Index1].CONSTANT_SuperMethodref.class.ref.internal_class_ref) {
                        acf->constant_pool[u2Index1]->internal_class = acf->classes[u2Index2];
                        break;
                    }

                /* We fetch the analyzed method. */
                for(u2Index2 = 0; u2Index2 < acf->constant_pool[u2Index1]->internal_class->methods_count; ++u2Index2) {
                    if(!(acf->constant_pool[u2Index1]->internal_class->methods[u2Index2]->flags & METHOD_STATIC) && !(acf->constant_pool[u2Index1]->internal_class->methods[u2Index2]->flags & METHOD_INIT) && (acf->constant_pool[u2Index1]->internal_class->methods[u2Index2]->token == acf->constant_pool[u2Index1]->method_token)) {
                        acf->constant_pool[u2Index1]->internal_method = acf->constant_pool[u2Index1]->internal_class->methods[u2Index2];
                        break;
                    }
                }
        }
    }

    return 0;

}


/**
 * \brief Print the given AID to the standard error output.
 * 
 * \param aid The AID to print.
 * \param length The length of the AID to print.
 */
static void print_AID(u1* aid, u1 length) {

    u1 u1Index = 0;

    for(;u1Index < length; ++u1Index) {
        if(u1Index != 0)
            fprintf(stderr, ":");
        fprintf(stderr, "0x%.2X", aid[u1Index]);
    }

}


/**
 * \brief Recursively search in the given directory for export files and build
 *        an array of parsed export files.
 *
 * \param directory       The root directory in which the search starts.
 * \param export_files    The built array of parsed export files (might be not
 *                        empty).
 * \param nb_export_files The number of found and parsed export files (might be
 *                        not 0).
 *
 * \return Return -1 if an error occurred, 0 else.
 */
static int get_export_files_from_directory(const char* directory, export_file*** export_files, int* nb_export_files) {

    char* path = NULL;

    struct dirent* crt_entry = NULL;
    size_t directory_length = strlen(directory);
    DIR* crt_dir = opendir(directory);

    if(crt_dir == NULL) {
        perror("get_export_files_from_directory");
        return -1;
    }

    if(directory[directory_length - 1] != '/')
        ++directory_length;

    path = (char*)malloc(directory_length + NAME_MAX + 1);
    if(path == NULL) {
        perror("get_export_files_from_directory");
        return -1;
    }

    strcpy(path, directory);
    path[directory_length - 1] = '/';

    while((crt_entry = readdir(crt_dir)) != NULL) {
        struct stat stat_buf;

        if((strcmp(crt_entry->d_name, ".") == 0) || (strcmp(crt_entry->d_name, "..") == 0))
            continue;

        path[directory_length] = '\0';
        strcat(path, crt_entry->d_name);

        if(stat(path, &stat_buf) != -1) {
            if(S_ISREG(stat_buf.st_mode)) {
                int path_length = strlen(path);
                if((path_length > 2) && (path[path_length - 3] == 'e') && (path[path_length - 2] == 'x') && (path[path_length - 1] == 'p')) {
                    export_file* ef = read_export_file(path);
                    if(ef != NULL) {
                        export_file** tmp = (export_file**)realloc(*export_files, sizeof(export_file*) * (*nb_export_files + 1));
                        if(tmp == NULL) {
                            perror("get_export_files_from_directory");
                            return -1;
                        }
                        *export_files = tmp;
    
                        (*export_files)[*nb_export_files] = ef;
                        ++(*nb_export_files);
                    }
                }
            } else if(S_ISDIR(stat_buf.st_mode)) {
                if(get_export_files_from_directory(path, export_files, nb_export_files) != 0)
                    return -1;
            }
        }
    }

    return 0;

}


/**
 * \brief Search for export files in an array of directories and build an array
 *        of parsed export files.
 *
 * \param directories     The directories to search in.
 * \param nb_directories  The number of directories in the array.
 * \param nb_export_files The number of found and parsed export files.
 *
 * \return Return an array of found and parsed export files or NULL if an error
 *         occurred.
 */
export_file** get_export_files_from_directories(char* const* directories, int nb_directories, int* nb_export_files) {

    int i = 0;
    export_file** export_files = NULL;
    *nb_export_files = 0;

    for(; i < nb_directories; ++i)
        if(get_export_files_from_directory(directories[i], &export_files, nb_export_files) != 0)
            return NULL;

    return export_files;

}


/**
 * \brief Linking each imported package to a parsed export file. If one is not
 *        found, prompt for a path to it.
 * 
 * \param acf             The analyzed CAP file to which parsed export files
 *                        are added.
 * \param export_files    The array of parsed export files.
 * \param nb_export_files The number of parsed export files in the array.
 *
 * \return Return -1 if an error occurred, 0 else.
 */
static int get_export_files(analyzed_cap_file* acf, export_file** export_files, int nb_export_files) {

    u1 u1Index1 = 0;

    for(; u1Index1 < acf->imported_packages_count; ++u1Index1) {
        u1 u1Index2 = 0;

        for(; u1Index2 < nb_export_files; ++u1Index2) {
            if(export_files[u1Index2]->constant_pool[export_files[u1Index2]->this_package].CONSTANT_Package.aid_length == acf->imported_packages[u1Index1]->aid_length) {
                u1 u1Index3 = 0;
                for(; u1Index3 < acf->imported_packages[u1Index1]->aid_length; ++u1Index3)
                    if(export_files[u1Index2]->constant_pool[export_files[u1Index2]->this_package].CONSTANT_Package.aid[u1Index3] != acf->imported_packages[u1Index1]->aid[u1Index3])
                        break;

                if(u1Index3 == acf->imported_packages[u1Index1]->aid_length) {
                    acf->imported_packages[u1Index1]->ef = export_files[u1Index2];
                    break;
                }
            }
        }

        if(u1Index2 == nb_export_files) {
            fprintf(stderr, "Could not find an export file: ");
            print_AID(acf->imported_packages[u1Index1]->aid, acf->imported_packages[u1Index1]->aid_length);
            fprintf(stderr, "\n");
            return -1;
        }
    }

    return 0;

}


/**
 * \brief Get the largest public method token given an external class token and
 *        parsed export file representing the package.
 *
 * \param ef                   The parsed export file representing the package
 *                             defining the class
 * \param external_class_token The token of the external class.
 *
 * \return Return the largest public method token.
 */
static int16_t get_external_class_largest_public_method_token(export_file* ef, u1 external_class_token) {

    u1 u1Index = 0;
    int16_t crt_largest_method_token = -1;

    for(; u1Index < ef->export_class_count; ++u1Index) {
        if(ef->classes[u1Index].token == external_class_token) {
            u2 u2Index = 0;
            for(; u2Index < ef->classes[u1Index].export_methods_count; ++u2Index)
                if(!(ef->classes[u1Index].methods[u2Index].access_flags & EF_ACC_STATIC) && (ef->classes[u1Index].methods[u2Index].token > crt_largest_method_token))
                    crt_largest_method_token = ef->classes[u1Index].methods[u2Index].token;

            return crt_largest_method_token;
        }
    }

    return crt_largest_method_token;

}


/**
 * \brief Compute for each class its largest public and package method tokens.
 *
 * We suppose that classes are well ordered (i.e. superclasses before extending
 * class) .
 *
 * \param acf The analyzed CAP file.
 */
static void find_largest_method_tokens(analyzed_cap_file* acf) {

    u2 u2Index1 = 0;

    for(; u2Index1 < acf->classes_count; ++u2Index1) {
        u2 u2Index2 = 0;

        /* If the class has a superclass, the initial largest token are the same than its superclass. */
        if(acf->classes[u2Index1]->superclass) {
            if(acf->classes[u2Index1]->superclass->flags & CONSTANT_POOL_IS_EXTERNAL) {
                /* If the superclass is external then the initial largest package token is the lowest one. */
                int16_t largest_public_method_token = get_external_class_largest_public_method_token(acf->classes[u2Index1]->superclass->external_package->ef, acf->classes[u2Index1]->superclass->external_class_token);
                if(largest_public_method_token != -1) {
                    acf->classes[u2Index1]->has_largest_public_method_token = 1;
                    acf->classes[u2Index1]->largest_public_method_token = largest_public_method_token;
                }
            } else {
                if(acf->classes[u2Index1]->superclass->internal_class->has_largest_public_method_token) {
                    acf->classes[u2Index1]->has_largest_public_method_token = 1;
                    acf->classes[u2Index1]->largest_public_method_token = acf->classes[u2Index1]->superclass->internal_class->largest_public_method_token;
                }

                if(acf->classes[u2Index1]->superclass->internal_class->has_largest_package_method_token) {
                    acf->classes[u2Index1]->has_largest_package_method_token = 1;
                    acf->classes[u2Index1]->largest_package_method_token = acf->classes[u2Index1]->superclass->internal_class->largest_package_method_token;
                }
            }
        }

        /* We go through the method defined by the class and check tokens. */
        for(; u2Index2 < acf->classes[u2Index1]->methods_count; ++u2Index2) {
            if((acf->classes[u2Index1]->methods[u2Index2]->flags & (METHOD_STATIC|METHOD_INIT)) == 0) {
                if(acf->classes[u2Index1]->methods[u2Index2]->flags & (METHOD_PUBLIC|METHOD_PROTECTED)) {
                    if(acf->classes[u2Index1]->has_largest_public_method_token) {
                        if(acf->classes[u2Index1]->largest_public_method_token < acf->classes[u2Index1]->methods[u2Index2]->token)
                            acf->classes[u2Index1]->largest_public_method_token = acf->classes[u2Index1]->methods[u2Index2]->token;
                    } else {
                        acf->classes[u2Index1]->has_largest_public_method_token = 1;
                        acf->classes[u2Index1]->largest_public_method_token = acf->classes[u2Index1]->methods[u2Index2]->token;
                    }
                } else if(acf->classes[u2Index1]->methods[u2Index2]->flags & METHOD_PACKAGE) {
                    if(acf->classes[u2Index1]->has_largest_package_method_token) {
                        if(acf->classes[u2Index1]->largest_package_method_token < acf->classes[u2Index1]->methods[u2Index2]->token)
                            acf->classes[u2Index1]->largest_package_method_token = acf->classes[u2Index1]->methods[u2Index2]->token;
                    } else {
                        acf->classes[u2Index1]->has_largest_package_method_token = 1;
                        acf->classes[u2Index1]->largest_package_method_token = acf->classes[u2Index1]->methods[u2Index2]->token;
                    }
                }
            }
        }
    }
}


/**
 * \brief Analyze each method of each class for overriding methods.
 *
 * Overrided analyzed method linking is done if applicable.
 *
 * \param acf The analyzed CAP file.
 *
 * \return Return -1 if an error occurred, 0 else.
 */
static int analyze_overriding_methods(analyzed_cap_file* acf) {

    u2 u2Index1 = 0;

    find_largest_method_tokens(acf);

    for(; u2Index1 < acf->classes_count; ++u2Index1) {
        if(acf->classes[u2Index1]->superclass) {
            if(acf->classes[u2Index1]->superclass->flags & CONSTANT_POOL_IS_EXTERNAL) {
                u2 u2Index2 = 0;
                int16_t largest_superclass_public_method_token = get_external_class_largest_public_method_token(acf->classes[u2Index1]->superclass->external_package->ef, acf->classes[u2Index1]->superclass->external_class_token);

                for(; u2Index2 < acf->classes[u2Index1]->methods_count; ++u2Index2) {
                    /* If a defined method token is less than the largest of its superclass then it is overriding. */
                    if(!(acf->classes[u2Index1]->methods[u2Index2]->flags & (METHOD_STATIC|METHOD_INIT)) && (acf->classes[u2Index1]->methods[u2Index2]->flags & (METHOD_PUBLIC|METHOD_PROTECTED)) && (acf->classes[u2Index1]->methods[u2Index2]->token <= largest_superclass_public_method_token)) {
                        acf->classes[u2Index1]->methods[u2Index2]->is_overriding = 1;
                    }
                }

            } else {
                u2 u2Index2 = 0;

                for(; u2Index2 < acf->classes[u2Index1]->methods_count; ++u2Index2) {
                    if(!(acf->classes[u2Index1]->methods[u2Index2]->flags & (METHOD_STATIC|METHOD_INIT))) {
                        if(acf->classes[u2Index1]->methods[u2Index2]->flags & (METHOD_PUBLIC|METHOD_PROTECTED)) {
                            /* If a defined method token is less than the largest of its superclass then it is overriding. */
                            if(acf->classes[u2Index1]->methods[u2Index2]->token <= acf->classes[u2Index1]->superclass->internal_class->largest_public_method_token) {
                                u2 u2Index3 = 0;

                                acf->classes[u2Index1]->methods[u2Index2]->is_overriding = 1;

                                /* We fetch the overriden method */
                                for(; u2Index3 < acf->classes[u2Index1]->superclass->internal_class->methods_count; ++u2Index3) {
                                    if(acf->classes[u2Index1]->methods[u2Index2]->token == acf->classes[u2Index1]->superclass->internal_class->methods[u2Index3]->token) {
                                        acf->classes[u2Index1]->methods[u2Index2]->internal_overrided_method = acf->classes[u2Index1]->superclass->internal_class->methods[u2Index3];
                                        break;
                                    }
                                }
                            }
                        } else if(acf->classes[u2Index1]->methods[u2Index2]->flags & METHOD_PACKAGE) {
                            /* If a defined method token is less than the largest of its superclass then it is overriding. */
                            if(acf->classes[u2Index1]->methods[u2Index2]->token <= acf->classes[u2Index1]->superclass->internal_class->largest_package_method_token) {
                                u2 u2Index3 = 0;

                                acf->classes[u2Index1]->methods[u2Index2]->is_overriding = 1;

                                /* We fetch the overriden method */
                                for(; u2Index3 < acf->classes[u2Index1]->superclass->internal_class->methods_count; ++u2Index3) {
                                    if(acf->classes[u2Index1]->methods[u2Index2]->token == acf->classes[u2Index1]->superclass->internal_class->methods[u2Index3]->token) {
                                        acf->classes[u2Index1]->methods[u2Index2]->internal_overrided_method = acf->classes[u2Index1]->superclass->internal_class->methods[u2Index3];
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return 0;

}


/**
 * \brief Convert an AID in string format to a byte array.
 *
 * \param aid_str The AID in string format.
 * \param aid_str_length The length of the AID in string format.
 * \param aid_length The length of the AID returned.
 *
 * \return Return the converted AID or NULL if an error occurred.
 */
static u1* parseAID(char* aid_str, u1 aid_str_length, u1* aid_length) {

    u1 u1Index = 0;
    char one_byte[] = "0x00";
    u1* result = NULL;

    *aid_length = ((aid_str_length - 4) / 5) + 1;

    result = (u1*)malloc(*aid_length);
    if(result == NULL) {
        perror("parseAID");
        return NULL;
    }

    for(; u1Index < *aid_length; ++u1Index) {
        one_byte[2] = aid_str[(u1Index * 5) + 2];
        one_byte[3] = aid_str[(u1Index * 5) + 3];

        result[u1Index] = (u1)strtoul(one_byte, NULL, 16);
    }

    return result;

}


/**
 * \brief Skip a value in the read manifest.
 *
 * \param manifest  The read manifest.
 * \param crt_index The current position in the manifest.
 * \param length    The length of the read manifest.
 */
static void skip_manifest_value(char* manifest, int* crt_index, int length) {

    while(*crt_index < length) {
        char* end_index = strchr(manifest + *crt_index, '\n');
        if(end_index == NULL) {
            *crt_index = length;
            return;
        }

        *crt_index = (end_index - manifest) + 1;
        if((*crt_index < length) && (manifest[*crt_index] == ' '))
            ++(*crt_index);
        else
            return;
    }

}


/**
 * \brief Read a value from the manifest.
 * 
 * \param manifest  The read manifest.
 * \param crt_index The current position in the manifest.
 * \param length    The length of the read manifest.
 *
 * \return Return the value as an allocated string or NULL if an error occurred.
 */
static char* read_manifest_value(char* manifest, int* crt_index, int length) {

    int result_length = 0;
    char* result = NULL;

    while(*crt_index < length) {
        char* tmp = NULL;

        if(manifest[*crt_index] == '\r') { /*\r\n*/
            ++(*crt_index);

            if((*crt_index < length) && (manifest[*crt_index] == '\n'))
                ++(*crt_index);

            if((*crt_index < length) && (manifest[*crt_index] == ' ')) {
                ++(*crt_index);
                continue;
            } else {
                result[result_length] = '\0';
                return result;
            }
        }

        tmp = (char*)realloc(result, result_length + 2);
        if(tmp == NULL) {
            perror("read_manifest_value");
            return NULL;
        }
        result = tmp;

        result[result_length] = manifest[*crt_index];
        ++result_length;
        ++(*crt_index);
    }

    result[result_length] = '\0';

    return result;

}


/**
 * \brief Skip an empty line of the manifest.
 *
 * \param manifest  The read manifest.
 * \param crt_index The current position in the manifest.
 * \param length    The length of the read manifest.
 */
static void skip_empty_lines(char* manifest, int* crt_index, int length) {

    while(*crt_index < length)
        if((manifest[*crt_index] == '\r') || (manifest[*crt_index] == '\n') || (manifest[*crt_index] == ' '))
            ++(*crt_index);
        else
            break;

}


/**
 * \brief Parse the manifest to get information for later rebuilding and
 *        writing.
 *
 * \param acf The analyzed CAP file to which the analyzed manifest is added.
 * \param cf  The straightforward representation of the CAP file.
 *
 * \return Return -1 if an error occurred, 0 else.
 */
static int analyze_manifest(analyzed_cap_file* acf, cap_file* cf) {

    int length = strlen(cf->manifest);
    int crt_index = 0;
    u1 u1Index = 0;

    u1 applets_count = 0;
    u1* applet_aid_lengths = NULL;
    u1** applet_aids = NULL;
    char** applet_names = NULL;

    acf->manifest.version = NULL;
    acf->manifest.created_by = NULL;
    acf->manifest.name = NULL;
    acf->manifest.package_name = NULL;
    acf->manifest.converter_provider = NULL;
    acf->manifest.converter_version = NULL;
    acf->manifest.creation_time = NULL;

    while(crt_index < length) {
        int substring_length = 0;
        char* end_index = strchr(cf->manifest + crt_index, ':');
        if(end_index == NULL)
            return 0;

        substring_length = end_index - (cf->manifest + crt_index);
        if(strncmp(cf->manifest + crt_index, "Manifest-Version", substring_length) == 0) {
            crt_index += 18; /*Manifest-Version: */

            if((acf->manifest.version = read_manifest_value(cf->manifest, &crt_index, length)) == NULL)
                return -1;
        } else if(strncmp(cf->manifest + crt_index, "Created-By", substring_length) == 0) {
            crt_index += 12;  /*Created-By: */

            if((acf->manifest.created_by = read_manifest_value(cf->manifest, &crt_index, length)) == NULL)
                return -1;
        } else if(strncmp(cf->manifest + crt_index, "Name", substring_length) == 0) {
            crt_index += 6;  /*Name: */

            if((acf->manifest.name = read_manifest_value(cf->manifest, &crt_index, length)) == NULL)
                return -1;
        } else if(strncmp(cf->manifest + crt_index, "Java-Card-Converter-Provider", substring_length) == 0) {
            crt_index += 30;  /*Java-Card-Converter-Provider: */

            if((acf->manifest.converter_provider = read_manifest_value(cf->manifest, &crt_index, length)) == NULL)
                return -1;
        } else if(strncmp(cf->manifest + crt_index, "Java-Card-Converter-Version", substring_length) == 0) {
            crt_index += 29;  /*Java-Card-Converter-Version: */

            if((acf->manifest.converter_version = read_manifest_value(cf->manifest, &crt_index, length)) == NULL)
                return -1;
        } else if(strncmp(cf->manifest + crt_index, "Java-Card-CAP-File-Version", substring_length) == 0) {
            crt_index += 28;  /*Java-Card-CAP-File-Version: */

            skip_manifest_value(cf->manifest, &crt_index, length);
        } else if(strncmp(cf->manifest + crt_index, "Java-Card-CAP-Creation-Time", substring_length) == 0) {
            crt_index += 29;  /*Java-Card-CAP-Creation-Time: */

            if((acf->manifest.creation_time = read_manifest_value(cf->manifest, &crt_index, length)) == NULL)
                return -1;
        } else if(strncmp(cf->manifest + crt_index, "Java-Card-Integer-Support-Required", substring_length) == 0) {
            crt_index += 36;  /*Java-Card-Integer-Support-Required: */

            skip_manifest_value(cf->manifest, &crt_index, length);
        } else if(strncmp(cf->manifest + crt_index, "Java-Card-Package-Name", substring_length) == 0) {
            crt_index += 24;  /*Java-Card-Package-Name: */

            if((acf->manifest.package_name = read_manifest_value(cf->manifest, &crt_index, length)) == NULL)
                return -1;

        } else if(strncmp(cf->manifest + crt_index, "Java-Card-Package-AID", substring_length) == 0) {
            crt_index += 23;  /*Java-Card-Package-AID: */

            skip_manifest_value(cf->manifest, &crt_index, length);
        } else if(strncmp(cf->manifest + crt_index, "Java-Card-Package-Version", substring_length) == 0) {
            crt_index += 27;  /*Java-Card-Package-Version: */

            skip_manifest_value(cf->manifest, &crt_index, length);
        } else {
            /*Java-*/
            end_index = strchr(cf->manifest + crt_index, '-');
            if(end_index == NULL)
                break;

            /*Card-*/
            end_index = strchr(end_index + 1, '-');
            if(end_index == NULL)
                return -1;

            crt_index = (end_index - cf->manifest) + 1;

            /*Applet- or Imported-*/
            end_index = strchr(end_index + 1, '-');
            if(end_index == NULL)
                return -1;

            substring_length = end_index - (cf->manifest + crt_index);

            if(strncmp(cf->manifest + crt_index, "Applet", substring_length) == 0) {
                char number[4];

                crt_index = (end_index - cf->manifest) + 1;
                end_index = strchr(end_index + 1, '-');
                if(end_index == NULL)
                    return -1;

                substring_length = end_index - (cf->manifest + crt_index);

                for(u1Index = 0; u1Index < substring_length; ++u1Index)
                    number[u1Index] = (cf->manifest + crt_index)[u1Index];

                number[u1Index] = '\0';
                u1Index = atoi(number) - 1;

                if(u1Index >= applets_count) {
                    char** tmp1 = NULL;
                    u1** tmp2 = NULL;
                    u1* tmp3 = NULL;
                    applets_count = u1Index + 1;

                    tmp1 = (char**)realloc(applet_names, sizeof(char*) * applets_count);
                    if(tmp1 == NULL) {
                        perror("analyze_manifest");
                        return -1;
                    }
                    applet_names = tmp1;

                    tmp2 = (u1**)realloc(applet_aids, sizeof(u1*) * applets_count);
                    if(tmp2 == NULL) {
                        perror("analyze_manifest");
                        return -1;
                    }
                    applet_aids = tmp2;

                    tmp3 = (u1*)realloc(applet_aid_lengths, applets_count);
                    if(tmp3 == NULL) {
                        perror("analyze_manifest");
                        return -1;
                    }
                    applet_aid_lengths = tmp3;
                }
                    
                crt_index = (end_index - cf->manifest) + 1;

                end_index = strchr(end_index + 1, ':');
                if(end_index == NULL)
                    return -1;

                substring_length = end_index - (cf->manifest + crt_index);

                if(strncmp(cf->manifest + crt_index, "Name", substring_length) == 0) {
                    applet_names[u1Index] = NULL;

                    crt_index += 6; /*Name: */

                    if((applet_names[u1Index] = read_manifest_value(cf->manifest, &crt_index, length)) == NULL)
                        return -1;
                } else if(strncmp(cf->manifest + crt_index, "AID", substring_length) == 0) {
                    char* aid_str = NULL;

                    crt_index += 5; /*AID: */

                    if((aid_str = read_manifest_value(cf->manifest, &crt_index, length)) == NULL)
                        return -1;

                    applet_aids[u1Index] = parseAID(aid_str, strlen(aid_str), applet_aid_lengths + u1Index);
                    if(applet_aids[u1Index] == NULL)
                        return -1;

                    free(aid_str);
                } else
                    return -1;

            } else if(strncmp(cf->manifest + crt_index, "Imported", substring_length) == 0) {
                crt_index += 9; /*Imported-*/
                skip_manifest_value(cf->manifest, &crt_index, length); 
            } else
                return -1;
        }

        skip_empty_lines(cf->manifest, &crt_index, length);

    }

    for(u1Index = 0; u1Index < applets_count; ++u1Index) {
        u2 u2Index = 0;

        for(; u2Index < acf->classes_count; ++u2Index)
            if((acf->classes[u2Index]->flags & CLASS_APPLET) && (acf->classes[u2Index]->name == NULL) && (acf->classes[u2Index]->aid_length == applet_aid_lengths[u1Index])) {
                u1 u1Index2 = 0;

                for(; u1Index2 < applet_aid_lengths[u1Index]; ++u1Index2)
                    if(applet_aids[u1Index][u1Index2] != acf->classes[u2Index]->aid[u1Index2])
                        break;

                if(u1Index2 == applet_aid_lengths[u1Index]) {
                    acf->classes[u2Index]->name = applet_names[u1Index];
                    break;
                }
            }
    }

    free(applet_aids);
    free(applet_aid_lengths);

    return 0;

}


/**
 * \brief Analyze a straightforward representation of a CAP file into a more
 *        useful format.
 *
 * \param cf              The straightforward representation of the CAP file.
 * \param export_files    An array of parsed export files.
 * \param nb_export_files The number of parsed export files in the array.
 *
 * \return Return the analyzed CAP file or NULL if an error occurred.
 */
analyzed_cap_file* analyze_cap_file(cap_file* cf, export_file** export_files, int nb_export_files) {

    analyzed_cap_file* acf = (analyzed_cap_file*)malloc(sizeof(analyzed_cap_file));
    if(acf == NULL) {
        perror("analyze_cap_file");
        return NULL;
    }

    if(analyze_constant_info(acf, cf) == -1) {
        fprintf(stderr, "Constant info analyze failed\n");
        return NULL;
    }

    if(analyze_imported_packages(acf, cf) == -1) {
        fprintf(stderr,"Imported packages analyze failed\n");
        return NULL;
    }

    if(analyze_signature_pool(acf, cf) == -1) {
        fprintf(stderr,"Signature pool analyze failed\n");
        return NULL;
    }

    if(analyze_constant_pool(acf, cf) == -1) {
        fprintf(stderr,"Constant pool analyze failed\n");
        return NULL;
    }

    if(analyze_interfaces(acf, cf) == -1) {
        fprintf(stderr,"Interfaces analyze failed\n");
        return NULL;
    }

    if(analyze_classes(acf, cf) == -1) {
        fprintf(stderr,"Classes analyze failed\n");
        return NULL;
    }

    if(analyze_exception_handlers(acf, cf) == -1) {
        fprintf(stderr,"Exception handlers analyze failed\n");
        return NULL;
    }

    if(signature_pool_second_pass(acf, cf) == -1) {
        fprintf(stderr,"Signature pool second pass failed\n");
        return NULL;
    }

    if(super_method_ref_second_pass(acf, cf) == -1) {
        fprintf(stderr,"Super method ref second pass failed\n");
        return NULL;
    }

    if(get_export_files(acf, export_files, nb_export_files) == -1)
        return NULL;

    if(analyze_overriding_methods(acf) == -1)
        return NULL;

    if(analyze_manifest(acf, cf) == -1)
        return NULL;

    return acf;

}
