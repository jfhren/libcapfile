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
 * \file cap_file_generate.c
 * \brief Generate from an analyzed CAP file a new CAP file in a straightfoward
 * representation ready to be written.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "cap_file.h"
#include "analyzed_cap_file.h"

/**
 * Searching for a parameter, a field or a bytecode using int type.
 * We suppose that an int local variable will use at least one bytecode using int type
 */
static char is_using_int(analyzed_cap_file* acf) {

    u2 u2Index1 = 0;

    for(; u2Index1 < acf->classes_count; ++u2Index1) {
        u2 u2Index2 = 0;

        for(; u2Index2 < acf->classes[u2Index1]->fields_count; ++u2Index2)
            if(acf->classes[u2Index1]->fields[u2Index2]->type->types->type & TYPE_DESCRIPTOR_INT)
                return 1;

        for(u2Index2 = 0; u2Index2 < acf->classes[u2Index1]->methods_count; ++u2Index2) {
            u1 u1Index = 0;
            u2 u2Index3 = 0;

            for(; u1Index < acf->classes[u2Index1]->methods[u2Index2]->signature->types_count; ++u1Index)
                if(acf->classes[u2Index1]->methods[u2Index2]->signature->types[u1Index].type & TYPE_DESCRIPTOR_INT)
                    return 1;

            for(; u2Index3 < acf->classes[u2Index1]->methods[u2Index2]->bytecodes_count; ++u2Index3)
                if((acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3]->opcode == 10) ||
                   (acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3]->opcode == 11) ||
                   (acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3]->opcode == 12) ||
                   (acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3]->opcode == 13) ||
                   (acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3]->opcode == 14) ||
                   (acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3]->opcode == 15) ||
                   (acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3]->opcode == 18) ||
                   (acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3]->opcode == 19) ||
                   (acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3]->opcode == 20) ||
                   (acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3]->opcode == 23) ||
                   (acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3]->opcode == 32) ||
                   (acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3]->opcode == 33) ||
                   (acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3]->opcode == 34) ||
                   (acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3]->opcode == 35) ||
                   (acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3]->opcode == 39) ||
                   (acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3]->opcode == 42) ||
                   (acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3]->opcode == 51) ||
                   (acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3]->opcode == 52) ||
                   (acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3]->opcode == 53) ||
                   (acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3]->opcode == 54) ||
                   (acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3]->opcode == 58) ||
                   (acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3]->opcode == 66) ||
                   (acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3]->opcode == 68) ||
                   (acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3]->opcode == 70) ||
                   (acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3]->opcode == 72) ||
                   (acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3]->opcode == 74) ||
                   (acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3]->opcode == 76) ||
                   (acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3]->opcode == 78) ||
                   (acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3]->opcode == 80) ||
                   (acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3]->opcode == 82) ||
                   (acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3]->opcode == 84) ||
                   (acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3]->opcode == 86) ||
                   (acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3]->opcode == 88) ||
                   (acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3]->opcode == 90) ||
                   (acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3]->opcode == 92) ||
                   (acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3]->opcode == 93) ||
                   (acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3]->opcode == 94) ||
                   (acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3]->opcode == 95) ||
                   (acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3]->opcode == 116) ||
                   (acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3]->opcode == 118) ||
                   (acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3]->opcode == 121) ||
                   (acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3]->opcode == 126) ||
                   (acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3]->opcode == 130) ||
                   (acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3]->opcode == 134) ||
                   (acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3]->opcode == 138) ||
                   (acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3]->opcode == 151) ||
                   (acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3]->opcode == 172) ||
                   (acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3]->opcode == 176) ||
                   (acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3]->opcode == 180) ||
                   (acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3]->opcode == 184) ||
                   ((acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3]->opcode == 144) &&
                   (acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3]->args[0] == 13)) ||
                   ((acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3]->opcode == 148) &&
                   (acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3]->args[0] == 13)) ||
                   ((acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3]->opcode == 149) &&
                   (acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3]->args[0] == 13)))
                    return 1;
        }
    }

    return 0;

}


/**
 * Return 1 if the given analyzed CAP file should produce an applet component, else 0.
 */
static char has_applet_component(analyzed_cap_file* acf) {

    u2 u2Index = 0;

    for(; u2Index < acf->classes_count; ++u2Index)
        if(acf->classes[u2Index]->flags & CLASS_APPLET)
            return 1;

    return 0;

}


/**
 * Return 1 of the given analyzed CAP file should produce an export component, else 0. 
 */
static char has_export_component(analyzed_cap_file* acf) {
    u2 u2Index = 0;

    if(has_applet_component(acf)) {
        for(; u2Index < acf->interfaces_count; ++u2Index)
            if(acf->interfaces[u2Index]->flags & INTERFACE_SHAREABLE)
                return 1;
    } else {
        for(; u2Index < acf->interfaces_count; ++u2Index)
            if(acf->interfaces[u2Index]->flags & INTERFACE_PUBLIC)
                return 1;

        for(u2Index = 0; u2Index < acf->classes_count; ++u2Index)
            if(acf->classes[u2Index]->flags & CLASS_PUBLIC)
                return 1;
    }

    return 0;

}


/**
 * Generate the header component in the given cap_file structure.
 */
static int generate_header_component(analyzed_cap_file* acf, cap_file* new) {

    new->header.tag = COMPONENT_HEADER;
    new->header.size = 10 + acf->info.package_aid_length;
    new->header.magic = 0xDECAFFED;
    new->header.minor_version = acf->info.javacard_minor_version;
    new->header.major_version = acf->info.javacard_major_version;
    new->header.flags = 0;
    if(is_using_int(acf))
        new->header.flags = HEADER_ACC_INT;

    if(has_export_component(acf))
        new->header.flags |= HEADER_ACC_EXPORT;

    if(has_applet_component(acf))
        new->header.flags |= HEADER_ACC_APPLET;

    new->header.package.minor_version = acf->info.package_minor_version;
    new->header.package.major_version = acf->info.package_major_version;

    new->header.package.AID_length = acf->info.package_aid_length;
    new->header.package.AID = (u1*)malloc(new->header.package.AID_length);
    if(new->header.package.AID == NULL) {
        perror("generate_header_component");
        return -1;
    }
    memcpy(new->header.package.AID, acf->info.package_aid, new->header.package.AID_length);

    if(acf->info.has_package_name) {
        new->header.size += 1 + strlen(acf->info.package_name);

        new->header.has_package_name = 1;
        new->header.package_name.name_length = strlen(acf->info.package_name);
        new->header.package_name.name = (u1*)malloc(new->header.package_name.name_length);
        if(new->header.package_name.name == NULL) {
            perror("generate_header_component");
            return -1;
        }
        memcpy(new->header.package_name.name, acf->info.package_name, new->header.package_name.name_length);
    } else {
        new->header.has_package_name = 0;
        new->header.package_name.name_length = 0;
        new->header.package_name.name = NULL;
    }

    return 0;

}


/**
 * Generate the directory component in the given cap_file structure.
 */
static int generate_directory_component(analyzed_cap_file* acf, cap_file* new) {

    u1 u1Index = 0;
    u2 u2Index = 0;

    new->directory.tag = COMPONENT_DIRECTORY;
    new->directory.size = (11 * 2) + (3 * 2) + (3 * 1);

    if((new->header.major_version == 2) && (new->header.minor_version > 1)) {
        new->directory.can_have_debug_component = 1;
        new->directory.size += 2;
   } else {
        new->directory.can_have_debug_component = 0;
    }

    new->directory.custom_count = acf->info.custom_count;
    new->directory.custom_components = (cf_custom_component_info*)malloc(sizeof(cf_custom_component_info) * new->directory.custom_count);
    if(new->directory.custom_components == NULL) {
        perror("generate_directory_component");
        return -1;
    }

    for(; u1Index < new->directory.custom_count; ++u1Index) {
        new->directory.size += 4 + acf->info.custom_components[u1Index].aid_length;

        new->directory.custom_components[u1Index].component_tag = acf->info.custom_components[u1Index].tag;
        new->directory.custom_components[u1Index].size = acf->info.custom_components[u1Index].size;

        new->directory.custom_components[u1Index].AID_length = acf->info.custom_components[u1Index].aid_length;
        new->directory.custom_components[u1Index].AID = (u1*)malloc(new->directory.custom_components[u1Index].AID_length);
        if(new->directory.custom_components[u1Index].AID == NULL) {
            perror("generate_directory_component");
            return -1;
        }
        memcpy(new->directory.custom_components[u1Index].AID, acf->info.custom_components[u1Index].aid, new->directory.custom_components[u1Index].AID_length);
    }

    new->directory.component_sizes[COMPONENT_HEADER - 1] = new->header.size;
    new->directory.component_sizes[COMPONENT_DIRECTORY - 1] = new->directory.size;
    new->directory.component_sizes[COMPONENT_APPLET - 1] = new->applet.size;
    new->directory.component_sizes[COMPONENT_IMPORT - 1] = new->import.size;
    new->directory.component_sizes[COMPONENT_CONSTANTPOOL - 1] = new->constant_pool.size;
    new->directory.component_sizes[COMPONENT_CLASS - 1] = new->class.size;
    new->directory.component_sizes[COMPONENT_METHOD - 1] = new->method.size;
    new->directory.component_sizes[COMPONENT_STATICFIELD - 1] = new->static_field.size;
    new->directory.component_sizes[COMPONENT_REFERENCELOCATION - 1] = new->reference_location.size;
    new->directory.component_sizes[COMPONENT_EXPORT - 1] = new->export.size;
    new->directory.component_sizes[COMPONENT_DESCRIPTOR - 1] = new->descriptor.size;
    new->directory.component_sizes[COMPONENT_DEBUG - 1] = 0;

    new->directory.static_field_size.image_size = new->static_field.image_size;
    new->directory.static_field_size.array_init_count = new->static_field.array_init_count;
    new->directory.static_field_size.array_init_size = 0;

    for(; u2Index < new->static_field.array_init_count; ++u2Index)
        if((new->static_field.array_init[u2Index].type == 2) || (new->static_field.array_init[u2Index].type == 3))
            new->directory.static_field_size.array_init_size += new->static_field.array_init[u2Index].count;
        else if(new->static_field.array_init[u2Index].type == 4)
            new->directory.static_field_size.array_init_size += (new->static_field.array_init[u2Index].count * 2);
        else
            new->directory.static_field_size.array_init_size += (new->static_field.array_init[u2Index].count * 4);

    new->directory.import_count = new->import.count;
    new->directory.applet_count = new->applet.count;

    return 0;

}


/**
 * Generate the applet component in the given cap_file structure.
 */ 
static int generate_applet_component(analyzed_cap_file* acf, cap_file* new) {

    u2 u2Index = 0;

    if(!(new->header.flags & HEADER_ACC_APPLET)) {
        new->applet.tag = 0;
        new->applet.size = 0;
        new->applet.count = 0;
        new->applet.applets = NULL;
        return 0;
    }

    new->applet.tag = COMPONENT_APPLET;
    new->applet.size = 1;
    new->applet.count = 0;
    new->applet.applets = NULL;

    for(; u2Index < acf->classes_count; ++u2Index)
        if(acf->classes[u2Index]->flags & CLASS_APPLET) {
            cf_applet_info* tmp = (cf_applet_info*)realloc(new->applet.applets, sizeof(cf_applet_info) * (new->applet.count + 1));
            if(tmp == NULL) {
                perror("generate_applet_component");
                return -1;
            }
            new->applet.applets = tmp;

            new->applet.applets[new->applet.count].AID_length = acf->classes[u2Index]->aid_length;
            new->applet.applets[new->applet.count].AID = (u1*)malloc(new->applet.applets[new->applet.count].AID_length);
            if(new->applet.applets[new->applet.count].AID == NULL) {
                perror("generate_applet_component");
                return -1;
            }
            memcpy(new->applet.applets[new->applet.count].AID, acf->classes[u2Index]->aid, new->applet.applets[new->applet.count].AID_length);
            new->applet.applets[new->applet.count].install_method_offset = acf->classes[u2Index]->install_method->offset;
            new->applet.size += 1 + new->applet.applets[new->applet.count].AID_length + 2;
            ++new->applet.count;
        }

    return 0;

}


/**
 * Update count field of each imported package.
 */
static void count_imported_package(analyzed_cap_file* acf) {

    u2 u2Index = 0;

    for(; u2Index < acf->constant_pool_count; ++u2Index)
        if(acf->constant_pool[u2Index]->flags & CONSTANT_POOL_IS_EXTERNAL && (acf->constant_pool[u2Index]->count !=0))
            ++acf->constant_pool[u2Index]->external_package->count;

    for(u2Index = 0; u2Index < acf->interfaces_count; ++u2Index) {
        u1 u1Index = 0;

        for(; u1Index < acf->interfaces[u2Index]->superinterfaces_count; ++u1Index)
            if(acf->interfaces[u2Index]->superinterfaces[u1Index]->flags & CONSTANT_POOL_IS_EXTERNAL)
                ++acf->interfaces[u2Index]->superinterfaces[u1Index]->external_package->count;
    }

    for(u2Index = 0; u2Index < acf->classes_count; ++u2Index) {
        u1 u1Index = 0;

        for(; u1Index < acf->classes[u2Index]->interfaces_count; ++u1Index)
            if(acf->classes[u2Index]->interfaces[u1Index].ref->flags & CONSTANT_POOL_IS_EXTERNAL)
                ++acf->classes[u2Index]->interfaces[u1Index].ref->external_package->count;

        if(acf->classes[u2Index]->superclass && acf->classes[u2Index]->superclass->flags & CONSTANT_POOL_IS_EXTERNAL)
            ++acf->classes[u2Index]->superclass->external_package->count;
    }

}


/**
 * Generate the import component in the given cap_file structure.
 * Only imported packages with at least one reference are added.
 */
static int generate_import_component(analyzed_cap_file* acf, cap_file* new) {

    u1 u1Index = 0;

    new->import.tag = COMPONENT_IMPORT;
    new->import.size = 1;
    new->import.count = 0;
    new->import.packages = NULL;

    for(; u1Index < acf->imported_packages_count; ++u1Index)
        if(acf->imported_packages[u1Index]->count != 0) {
            cf_package_info* tmp = (cf_package_info*)realloc(new->import.packages, sizeof(cf_package_info) * (new->import.count + 1));
            if(tmp == NULL) {
                perror("generate_import_component");
                return -1;
            }
            new->import.packages = tmp;

            new->import.packages[new->import.count].minor_version = acf->imported_packages[u1Index]->minor_version;
            new->import.packages[new->import.count].major_version = acf->imported_packages[u1Index]->major_version;
            new->import.packages[new->import.count].AID_length = acf->imported_packages[u1Index]->aid_length;
            new->import.packages[new->import.count].AID = (u1*)malloc(new->import.packages[new->import.count].AID_length);
            if(new->import.packages[new->import.count].AID == NULL) {
                perror("generate_import_component");
                return -1;
            }
            memcpy(new->import.packages[new->import.count].AID, acf->imported_packages[u1Index]->aid, new->import.packages[new->import.count].AID_length);
            new->import.size += 3 + new->import.packages[new->import.count].AID_length;
            acf->imported_packages[u1Index]->my_index = new->import.count;
            ++new->import.count;
        }

    return 0;

}


/**
 * Update the count field of each constant pool entry.
 */
static void count_constant_pool_references(analyzed_cap_file* acf) {

    u1 u1Index = 0;
    u2 u2Index1 = 0;

    for(; u2Index1 < acf->classes_count; ++u2Index1) {
        u2 u2Index2 = 0;

        for(; u2Index2 < acf->classes[u2Index1]->methods_count; ++u2Index2) {
            u2 u2Index3 = 0;

            for(; u2Index3 < acf->classes[u2Index1]->methods[u2Index2]->bytecodes_count; ++u2Index3) {
                if(acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3]->has_ref) {
                    ++acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3]->ref->count;
                }
            }
        }
    }

    for(; u1Index < acf->exception_handlers_count; ++u1Index)
        if(acf->exception_handlers[u1Index]->catch_type)
            ++acf->exception_handlers[u1Index]->catch_type->count;

}


/**
 * Update the index of each constant pool entry while respecting this order: instance_fieldref, classref, virtual_methodref, super_methodref, static_fieldref, static_methodref.
 */
static void update_constant_pool_entry_index(analyzed_cap_file* acf) {

    u2 u2Index = 0;
    u2 crtIndex = 0;

    for(; u2Index < acf->constant_pool_count; ++u2Index)
        if((acf->constant_pool[u2Index]->flags & CONSTANT_POOL_INSTANCEFIELDREF) && (acf->constant_pool[u2Index]->count != 0)) {
            acf->constant_pool[u2Index]->my_index = crtIndex;
            ++crtIndex;
        }

    for(u2Index = 0; u2Index < acf->constant_pool_count; ++u2Index)
        if((acf->constant_pool[u2Index]->flags & CONSTANT_POOL_CLASSREF) && (acf->constant_pool[u2Index]->count != 0)) {
            acf->constant_pool[u2Index]->my_index = crtIndex;
            ++crtIndex;
        }

    for(u2Index = 0; u2Index < acf->constant_pool_count; ++u2Index)
        if((acf->constant_pool[u2Index]->flags & CONSTANT_POOL_VIRTUALMETHODREF) && (acf->constant_pool[u2Index]->count != 0)) {
            acf->constant_pool[u2Index]->my_index = crtIndex;
            ++crtIndex;
        }

    for(u2Index = 0; u2Index < acf->constant_pool_count; ++u2Index)
        if((acf->constant_pool[u2Index]->flags & CONSTANT_POOL_SUPERMETHODREF) && (acf->constant_pool[u2Index]->count != 0)) {
            acf->constant_pool[u2Index]->my_index = crtIndex;
            ++crtIndex;
        }

    for(u2Index = 0; u2Index < acf->constant_pool_count; ++u2Index)
        if((acf->constant_pool[u2Index]->flags & CONSTANT_POOL_STATICFIELDREF) && (acf->constant_pool[u2Index]->count != 0)) {
            acf->constant_pool[u2Index]->my_index = crtIndex;
            ++crtIndex;
        }

    for(u2Index = 0; u2Index < acf->constant_pool_count; ++u2Index)
        if((acf->constant_pool[u2Index]->flags & CONSTANT_POOL_STATICMETHODREF) && (acf->constant_pool[u2Index]->count != 0)) {
            acf->constant_pool[u2Index]->my_index = crtIndex;
            ++crtIndex;
        }

}


/**
 * Update bytecodes opcode with respect to constant pool entry index value.
 * Some bytecode can only handle index of one byte width while other two bytes width.
 * We compact or expand accordingly.
 */
static int compact_bytecodes(analyzed_cap_file* acf) {

    u2 u2Index1 = 0;

    for(; u2Index1 < acf->classes_count; ++u2Index1) {
        u2 u2Index2 = 0;

        for(; u2Index2 < acf->classes[u2Index1]->methods_count; ++u2Index2) {
            u2 u2Index3 = 0;

            while(u2Index3 < acf->classes[u2Index1]->methods[u2Index2]->bytecodes_count) {
                switch(acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3]->opcode) {
                    case 131:       /* getfield_a */
                    case 132:       /* getfield_b */
                    case 133:       /* getfield_s */
                    case 134:       /* getfield_i */
                        if(acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3]->ref->my_index > 255) {
                            acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3]->opcode += 38;
                            acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3]->nb_args = 2;
                        }
                        break;

                    case 173:       /* getfield_a_this */
                    case 174:       /* getfield_b_this */
                    case 175:       /* getfield_s_this */
                    case 176:       /* getfield_i_this */
                        if(acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3]->ref->my_index > 255) {
                            u2 u2Index4 = 0;
                            bytecode_info** tmp = (bytecode_info**)realloc(acf->classes[u2Index1]->methods[u2Index2]->bytecodes, sizeof(bytecode_info*) * (acf->classes[u2Index1]->methods[u2Index2]->bytecodes_count + 1));
                            if(tmp == NULL) {
                                perror("compact_bytecodes");
                                return -1;
                            }
                            acf->classes[u2Index1]->methods[u2Index2]->bytecodes = tmp;

                            for(u2Index4 = acf->classes[u2Index1]->methods[u2Index2]->bytecodes_count; u2Index4 > u2Index3; --u2Index4)
                                acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index4] = acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index4 - 1];

                            ++acf->classes[u2Index1]->methods[u2Index2]->bytecodes_count;

                            acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3 + 1] = (bytecode_info*)malloc(sizeof(bytecode_info));
                            if(acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3 + 1] == NULL) {
                                perror("compact_bytecodes");
                                return -1;
                            }

                            acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3 + 1]->opcode = acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3]->opcode - 4;
                            acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3 + 1]->offset = 0;
                            acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3 + 1]->nb_byte_args = 0;
                            acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3 + 1]->has_ref = 1;
                            acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3 + 1]->ref = acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3]->ref;
                            acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3 + 1]->has_branch = 0;

                            acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3]->opcode = 24;    /* aload_0 */
                            acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3]->nb_args = 0;
                            acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3]->has_ref = 0;
                            acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3]->ref = NULL;

                            ++u2Index3;
                        }
                        break;

                    case 169:       /* getfield_a_w */
                    case 170:       /* getfield_b_w */
                    case 171:       /* getfield_s_w */
                    case 172:       /* getfield_i_w */
                        if(acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3]->ref->my_index < 256) {
                            acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3]->opcode -= 38;
                            acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3]->nb_args = 1;
                        }
                        break;

                    case 135:       /* putfield_a */
                    case 136:       /* putfield_b */
                    case 137:       /* putfield_s */
                    case 138:       /* putfield_i */
                        if(acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3]->ref->my_index > 255) {
                            acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3]->opcode += 42;
                            acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3]->nb_args = 2;
                        }
                        break;

                    case 181:       /* putfield_a_this */
                    case 182:       /* putfield_b_this */
                    case 183:       /* putfield_s_this */
                    case 184:       /* putfield_i_this */
                        if(acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3]->ref->my_index > 255) {
                            u2 u2Index4 = 0;
                            bytecode_info** tmp = (bytecode_info**)realloc(acf->classes[u2Index1]->methods[u2Index2]->bytecodes, sizeof(bytecode_info*) * (acf->classes[u2Index1]->methods[u2Index2]->bytecodes_count + 2));
                            if(tmp == NULL) {
                                perror("compact_bytecodes");
                                return -1;
                            }
                            acf->classes[u2Index1]->methods[u2Index2]->bytecodes = tmp;

                            for(u2Index4 = acf->classes[u2Index1]->methods[u2Index2]->bytecodes_count + 1; u2Index4 > u2Index3; --u2Index4)
                                acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index4] = acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index4 - 1];

                            acf->classes[u2Index1]->methods[u2Index2]->bytecodes_count += 2;

                            acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3 + 1] = (bytecode_info*)malloc(sizeof(bytecode_info));
                            if(acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3 + 1] == NULL) {
                                perror("compact_bytecodes");
                                return -1;
                            }


                            /* putfield_<t>_w */
                            acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3 + 2]->opcode = acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3]->opcode - 4;
                            acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3 + 2]->offset = 0;
                            acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3 + 2]->nb_byte_args = 0;
                            acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3 + 2]->has_ref = 1;
                            acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3 + 2]->ref = acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3]->ref;
                            acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3 + 2]->has_branch = 0;

                            acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3 + 2] = (bytecode_info*)malloc(sizeof(bytecode_info));
                            if(acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3 + 2] == NULL) {
                                perror("compact_bytecodes");
                                return -1;
                            }

                            /* swap_x */
                            acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3 + 1]->opcode = 64;
                            acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3 + 1]->offset = 0;
                            acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3 + 1]->nb_byte_args = 1;
                            if(acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3 + 2]->opcode == 180)   /* putfield_i_w */
                                acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3 + 1]->args[0] = 0x12;
                            else
                                acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3 + 1]->args[0] = 0x11;
                            acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3 + 1]->has_ref = 0;
                            acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3 + 1]->ref = NULL;

                            /* aload_0 */
                            acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3 + 1]->has_branch = 0;
                            acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3]->opcode = 24;
                            acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3]->nb_args = 0;
                            acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3]->has_ref = 0;
                            acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3]->ref = NULL;

                            ++u2Index3;
                        }
                        break;

                    case 177:       /* putfield_a_w */
                    case 178:       /* putfield_b_w */
                    case 179:       /* putfield_s_w */
                    case 180:       /* putfield_i_w */
                        if(acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3]->ref->my_index < 256) {
                            acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3]->opcode -= 42;
                            acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3]->nb_args = 1;
                        }
                        break;

                }

                ++u2Index3;

            }
        }
    }

    return 0;

}


/**
 * Compute bytecodes offsets with respect to each opcode number of arguments.
 */
static void compute_bytecodes_offsets(analyzed_cap_file* acf) {

    u2 u2Index1 = 0;
    u2 crt_info_offset = 1 + (acf->exception_handlers_count * 8);

    for(; u2Index1 < acf->classes_count; ++u2Index1) {
        u2 u2Index2 = 0;

        for(; u2Index2 < acf->classes[u2Index1]->methods_count; ++u2Index2) {
            u2 u2Index3 = 0;
            u2 crt_offset = 0;

            if(acf->classes[u2Index1]->methods[u2Index2]->flags & METHOD_EXTENDED)
                crt_info_offset += 4;
            else
                crt_info_offset += 2;

            for(; u2Index3 < acf->classes[u2Index1]->methods[u2Index2]->bytecodes_count; ++u2Index3) {
                acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3]->offset = crt_offset;
                acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3]->info_offset = crt_info_offset;
                crt_offset += acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3]->nb_args + 1;
                crt_info_offset += acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3]->nb_args + 1;
            }
        }
    }

}


/**
 * Compute the bytecode size in byte for each methods of the analyzed CAP file.
 */
static void compute_bytecodes_sizes(analyzed_cap_file* acf) {

    u2 u2Index1 = 0;

    for(; u2Index1 < acf->classes_count; ++u2Index1) {
        u2 u2Index2 = 0;

        for(; u2Index2 < acf->classes[u2Index1]->methods_count; ++u2Index2) {
            u2 u2Index3 = 0;
            u2 crt_bytecodes_size = 0;

            for(; u2Index3 < acf->classes[u2Index1]->methods[u2Index2]->bytecodes_count; ++u2Index3) {
                crt_bytecodes_size += (1 + acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3]->nb_args);
            }

            acf->classes[u2Index1]->methods[u2Index2]->bytecodes_size = crt_bytecodes_size;
        }
    }

}


/**
 * Generate bytecodes given the analyzed bytecodes.
 */
static u1* generate_bytecodes(bytecode_info** bytecodes, u2 bytecodes_count, u2 bytecodes_size) {

    u2 u2Index = 0;
    u2 crt_bytecode = 0;
    u1* new_bytecodes = (u1*)malloc(sizeof(u1) * bytecodes_size);
    if(new_bytecodes == NULL) {
        perror("generate_method_component");
        return NULL;
    }

    for(; u2Index < bytecodes_count; ++u2Index) {
        if(bytecodes[u2Index]->opcode == 115) {         /* stableswitch */
            u2 crt_case = 0;

            new_bytecodes[crt_bytecode] = 115;

            new_bytecodes[crt_bytecode + 1] = ((int16_t)(bytecodes[u2Index]->stableswitch.default_branch->offset - bytecodes[u2Index]->offset)) >> 8;
            new_bytecodes[crt_bytecode + 2] = ((int16_t)(bytecodes[u2Index]->stableswitch.default_branch->offset - bytecodes[u2Index]->offset)) & 0xFF;

            new_bytecodes[crt_bytecode + 3] = bytecodes[u2Index]->stableswitch.low >> 8;
            new_bytecodes[crt_bytecode + 4] = bytecodes[u2Index]->stableswitch.low & 0xFF;

            new_bytecodes[crt_bytecode + 5] = bytecodes[u2Index]->stableswitch.high >> 8;
            new_bytecodes[crt_bytecode + 6] = bytecodes[u2Index]->stableswitch.high & 0xFF;

            crt_bytecode += 7;

            for(; crt_case < bytecodes[u2Index]->stableswitch.nb_cases; ++crt_case) {
                new_bytecodes[crt_bytecode + (crt_case * 2)] = ((int16_t)(bytecodes[u2Index]->stableswitch.branches[crt_case]->offset - bytecodes[u2Index]->offset)) >> 8;
                new_bytecodes[crt_bytecode + (crt_case * 2) + 1] = ((int16_t)(bytecodes[u2Index]->stableswitch.branches[crt_case]->offset - bytecodes[u2Index]->offset)) & 0xFF;
            }

            crt_bytecode += (bytecodes[u2Index]->stableswitch.nb_cases * 2);
        } else if(bytecodes[u2Index]->opcode == 116) {  /* itableswitch */
            u2 crt_case = 0;

            new_bytecodes[crt_bytecode] = 116;

            new_bytecodes[crt_bytecode + 1] = ((int16_t)(bytecodes[u2Index]->itableswitch.default_branch->offset - bytecodes[u2Index]->offset)) >> 8;
            new_bytecodes[crt_bytecode + 2] = ((int16_t)(bytecodes[u2Index]->itableswitch.default_branch->offset - bytecodes[u2Index]->offset)) & 0xFF;

            new_bytecodes[crt_bytecode + 3] = bytecodes[u2Index]->itableswitch.low >> 24;
            new_bytecodes[crt_bytecode + 4] = bytecodes[u2Index]->itableswitch.low >> 16;
            new_bytecodes[crt_bytecode + 5] = bytecodes[u2Index]->itableswitch.low >> 8;
            new_bytecodes[crt_bytecode + 6] = bytecodes[u2Index]->itableswitch.low & 0xFF;

            new_bytecodes[crt_bytecode + 7] = bytecodes[u2Index]->itableswitch.high >> 24;
            new_bytecodes[crt_bytecode + 8] = bytecodes[u2Index]->itableswitch.high >> 16;
            new_bytecodes[crt_bytecode + 9] = bytecodes[u2Index]->itableswitch.high >> 8;
            new_bytecodes[crt_bytecode + 10] = bytecodes[u2Index]->itableswitch.high & 0xFF;

            crt_bytecode += 11;

            for(; crt_case < bytecodes[u2Index]->itableswitch.nb_cases; ++crt_case) {
                new_bytecodes[crt_bytecode + (crt_case * 2)] = ((int16_t)(bytecodes[u2Index]->itableswitch.branches[crt_case]->offset - bytecodes[u2Index]->offset)) >> 8;
                new_bytecodes[crt_bytecode + (crt_case * 2) + 1] = ((int16_t)(bytecodes[u2Index]->itableswitch.branches[crt_case]->offset - bytecodes[u2Index]->offset)) & 0xFF;
            }

            crt_bytecode += (bytecodes[u2Index]->itableswitch.nb_cases * 2);
        } else if(bytecodes[u2Index]->opcode == 117) {  /* slookupswitch */
            u2 crt_case = 0;

            new_bytecodes[crt_bytecode] = 117;

            new_bytecodes[crt_bytecode + 1] = ((int16_t)(bytecodes[u2Index]->slookupswitch.default_branch->offset - bytecodes[u2Index]->offset)) >> 8;
            new_bytecodes[crt_bytecode + 2] = ((int16_t)(bytecodes[u2Index]->slookupswitch.default_branch->offset - bytecodes[u2Index]->offset)) & 0xFF;

            new_bytecodes[crt_bytecode + 3] = bytecodes[u2Index]->slookupswitch.nb_cases >> 8;
            new_bytecodes[crt_bytecode + 4] = bytecodes[u2Index]->slookupswitch.nb_cases & 0xFF;

            crt_bytecode += 5;

            for(; crt_case < bytecodes[u2Index]->slookupswitch.nb_cases; ++crt_case) {
                new_bytecodes[crt_bytecode + (crt_case * 4)] = bytecodes[u2Index]->slookupswitch.cases[crt_case].match >> 8;
                new_bytecodes[crt_bytecode + (crt_case * 4) + 1] = bytecodes[u2Index]->slookupswitch.cases[crt_case].match & 0xFF;

                new_bytecodes[crt_bytecode + (crt_case * 4) + 2] = ((int16_t)(bytecodes[u2Index]->slookupswitch.cases[crt_case].branch->offset - bytecodes[u2Index]->offset)) >> 8;
                new_bytecodes[crt_bytecode + (crt_case * 4) + 3] = ((int16_t)(bytecodes[u2Index]->slookupswitch.cases[crt_case].branch->offset - bytecodes[u2Index]->offset)) & 0xFF;
            }

            crt_bytecode += (bytecodes[u2Index]->slookupswitch.nb_cases * 4);
        } else if(bytecodes[u2Index]->opcode == 118) {  /* ilookupswitch */
            u2 crt_case = 0;

            new_bytecodes[crt_bytecode] = 118;

            new_bytecodes[crt_bytecode + 1] = ((int16_t)(bytecodes[u2Index]->ilookupswitch.default_branch->offset - bytecodes[u2Index]->offset)) >> 8;
            new_bytecodes[crt_bytecode + 2] = ((int16_t)(bytecodes[u2Index]->ilookupswitch.default_branch->offset - bytecodes[u2Index]->offset)) & 0xFF;

            new_bytecodes[crt_bytecode + 3] = bytecodes[u2Index]->ilookupswitch.nb_cases >> 8;
            new_bytecodes[crt_bytecode + 4] = bytecodes[u2Index]->ilookupswitch.nb_cases & 0xFF;

            crt_bytecode += 5;

            for(; crt_case < bytecodes[u2Index]->ilookupswitch.nb_cases; ++crt_case) {
                new_bytecodes[crt_bytecode + (crt_case * 4)] = bytecodes[u2Index]->ilookupswitch.cases[crt_case].match >> 24;
                new_bytecodes[crt_bytecode + (crt_case * 4) + 1] = bytecodes[u2Index]->ilookupswitch.cases[crt_case].match >> 16;
                new_bytecodes[crt_bytecode + (crt_case * 4) + 2] = bytecodes[u2Index]->ilookupswitch.cases[crt_case].match >> 8;
                new_bytecodes[crt_bytecode + (crt_case * 4) + 3] = bytecodes[u2Index]->ilookupswitch.cases[crt_case].match & 0xFF;

                new_bytecodes[crt_bytecode + (crt_case * 4) + 2] = ((int16_t)(bytecodes[u2Index]->ilookupswitch.cases[crt_case].branch->offset - bytecodes[u2Index]->offset)) >> 8;
                new_bytecodes[crt_bytecode + (crt_case * 4) + 3] = ((int16_t)(bytecodes[u2Index]->ilookupswitch.cases[crt_case].branch->offset - bytecodes[u2Index]->offset)) & 0xFF;
            }

            crt_bytecode += (bytecodes[u2Index]->ilookupswitch.nb_cases * 6);
        } else if(bytecodes[u2Index]->nb_args) {    /* Have at least one arg */
            new_bytecodes[crt_bytecode] = bytecodes[u2Index]->opcode;
            ++crt_bytecode;
            if(bytecodes[u2Index]->has_branch) {    /* If there is a branch, it makes use of all the args */
                if(bytecodes[u2Index]->nb_args == 1) {
                    new_bytecodes[crt_bytecode] = ((char)(bytecodes[u2Index]->branch->offset - bytecodes[u2Index]->offset));
                    ++crt_bytecode;
                } else {
                    new_bytecodes[crt_bytecode] = ((int16_t)(bytecodes[u2Index]->branch->offset - bytecodes[u2Index]->offset)) >> 8;
                    new_bytecodes[crt_bytecode + 1] = ((int16_t)(bytecodes[u2Index]->branch->offset - bytecodes[u2Index]->offset)) & 0xFF;
                    crt_bytecode += 2;
                }
            } else if(bytecodes[u2Index]->has_ref) {    /* If there is a ref */
                if(bytecodes[u2Index]->nb_args == 1) {
                    new_bytecodes[crt_bytecode] = bytecodes[u2Index]->ref->my_index & 0xFF;
                    ++crt_bytecode;
                } else if(bytecodes[u2Index]->nb_args == 2) {
                    new_bytecodes[crt_bytecode] = bytecodes[u2Index]->ref->my_index >> 8;
                    new_bytecodes[crt_bytecode + 1] = bytecodes[u2Index]->ref->my_index & 0xFF;
                    crt_bytecode += 2;
                } else if(bytecodes[u2Index]->nb_args == 3) {   /* checkcast & instanceof */
                    new_bytecodes[crt_bytecode] = bytecodes[u2Index]->args[0];
                    if(bytecodes[u2Index]->has_ref) {
                        new_bytecodes[crt_bytecode + 1] = bytecodes[u2Index]->ref->my_index >> 8;
                        new_bytecodes[crt_bytecode + 2] = bytecodes[u2Index]->ref->my_index & 0xFF;
                    } else {
                        new_bytecodes[crt_bytecode + 1] = bytecodes[u2Index]->args[1];
                        new_bytecodes[crt_bytecode + 2] = bytecodes[u2Index]->args[2];
                    }
                    crt_bytecode += 3;
                } else {
                    new_bytecodes[crt_bytecode] = bytecodes[u2Index]->args[0];
                    new_bytecodes[crt_bytecode + 1] = bytecodes[u2Index]->ref->my_index >> 8;
                    new_bytecodes[crt_bytecode + 2] = bytecodes[u2Index]->ref->my_index & 0xFF;
                    new_bytecodes[crt_bytecode + 3] = bytecodes[u2Index]->args[1];
                    crt_bytecode += 4;
                }
            } else {    /* It's only byte args */
                u1 crt_arg = 0;

                for(; crt_arg < bytecodes[u2Index]->nb_byte_args; ++crt_arg)
                    new_bytecodes[crt_bytecode + crt_arg] = bytecodes[u2Index]->args[crt_arg];

                crt_bytecode += bytecodes[u2Index]->nb_byte_args;
            }
        } else {    /* There is no arg */
            new_bytecodes[crt_bytecode] = bytecodes[u2Index]->opcode;
            ++crt_bytecode;
        }
    }

    return new_bytecodes;

}


/**
 * Sort the exception handlers by increasing offset within the method component info.
 */
static void sort_exception_handlers(analyzed_cap_file* acf) {

    u1 u1Index1 = 0;

    for(; u1Index1 < (acf->exception_handlers_count - 1); ++u1Index1) {
        u1 u1Index2 = u1Index1;

        for(; u1Index2 < acf->exception_handlers_count; ++u1Index2)
            if(acf->exception_handlers[u1Index1]->handler->info_offset > acf->exception_handlers[u1Index2]->handler->info_offset) {
                exception_handler_info* tmp = acf->exception_handlers[u1Index2];
                acf->exception_handlers[u1Index2] = acf->exception_handlers[u1Index1];
                acf->exception_handlers[u1Index1] = tmp;
            }
    }

}


/**
 * Generate the method component in the given cap_file structure.
 */
static int generate_method_component(analyzed_cap_file* acf, cap_file* new) {

    u1 u1Index = 0;
    u2 u2Index1 = 0;

    new->method.tag = COMPONENT_METHOD;
    new->method.size = 1;

    new->method.handler_count = acf->exception_handlers_count;
    new->method.size += (new->method.handler_count * 8);

    new->method.exception_handlers = (cf_exception_handler_info*)malloc(sizeof(cf_exception_handler_info) * new->method.handler_count);
    if(new->method.exception_handlers == NULL) {
        perror("generate_method_component");
        return -1;
    }

    for(; u1Index < new->method.handler_count; ++u1Index) {
        acf->exception_handlers[u1Index]->my_index = u1Index;
        new->method.exception_handlers[u1Index].start_offset = acf->exception_handlers[u1Index]->start->info_offset;
        new->method.exception_handlers[u1Index].stop_bit = acf->exception_handlers[u1Index]->stop_bit;
        if(acf->exception_handlers[u1Index]->end)
            new->method.exception_handlers[u1Index].active_length = acf->exception_handlers[u1Index]->end->info_offset - new->method.exception_handlers[u1Index].start_offset;
        else
            new->method.exception_handlers[u1Index].active_length = (acf->exception_handlers[u1Index]->try_in->bytecodes[0]->info_offset + acf->exception_handlers[u1Index]->try_in->bytecodes_size) - new->method.exception_handlers[u1Index].start_offset;
        new->method.exception_handlers[u1Index].handler_offset = acf->exception_handlers[u1Index]->handler->info_offset;
        if(acf->exception_handlers[u1Index]->catch_type)
            new->method.exception_handlers[u1Index].catch_type_index = acf->exception_handlers[u1Index]->catch_type->my_index;
        else
            new->method.exception_handlers[u1Index].catch_type_index = 0;
    }

    new->method.method_count = 0;
    new->method.methods = NULL;

    for(;u2Index1 < acf->classes_count; ++u2Index1) {
        u2 u2Index2 = 0;
        cf_method_info* tmp = NULL;
        tmp = (cf_method_info*)realloc(new->method.methods, sizeof(cf_method_info) * (new->method.method_count + acf->classes[u2Index1]->methods_count));
        if(tmp == NULL) {
            perror("generate_method_component");
            return -1;
        }
        new->method.methods = tmp;

        for(; u2Index2 < acf->classes[u2Index1]->methods_count; ++u2Index2) {
            new->method.methods[new->method.method_count + u2Index2].method_header.flags = 0;

            new->method.methods[new->method.method_count + u2Index2].offset = new->method.size;
            acf->classes[u2Index1]->methods[u2Index2]->offset = new->method.size;

            if(acf->classes[u2Index1]->methods[u2Index2]->flags & METHOD_EXTENDED) {
                new->method.methods[new->method.method_count + u2Index2].method_header.flags |= METHOD_ACC_EXTENDED;
                new->method.methods[new->method.method_count + u2Index2].method_header.extended_method_header.padding = 0;
                new->method.methods[new->method.method_count + u2Index2].method_header.extended_method_header.max_stack = acf->classes[u2Index1]->methods[u2Index2]->max_stack;
                new->method.methods[new->method.method_count + u2Index2].method_header.extended_method_header.nargs = acf->classes[u2Index1]->methods[u2Index2]->nargs;
                new->method.methods[new->method.method_count + u2Index2].method_header.extended_method_header.max_locals = acf->classes[u2Index1]->methods[u2Index2]->max_locals;
                new->method.size += 4;
            } else {
                new->method.methods[new->method.method_count + u2Index2].method_header.standard_method_header.max_stack = acf->classes[u2Index1]->methods[u2Index2]->max_stack;
                new->method.methods[new->method.method_count + u2Index2].method_header.standard_method_header.nargs = acf->classes[u2Index1]->methods[u2Index2]->nargs;
                new->method.methods[new->method.method_count + u2Index2].method_header.standard_method_header.max_locals = acf->classes[u2Index1]->methods[u2Index2]->max_locals;
                new->method.size += 2;
            }

            if(acf->classes[u2Index1]->methods[u2Index2]->flags & METHOD_ABSTRACT) {
                new->method.methods[new->method.method_count + u2Index2].method_header.flags |= METHOD_ACC_ABSTRACT;
                new->method.methods[new->method.method_count + u2Index2].bytecode_count = 0;
                new->method.methods[new->method.method_count + u2Index2].bytecodes = NULL;
            } else {
                new->method.methods[new->method.method_count + u2Index2].bytecode_count = acf->classes[u2Index1]->methods[u2Index2]->bytecodes_size;
                new->method.methods[new->method.method_count + u2Index2].bytecodes = generate_bytecodes(acf->classes[u2Index1]->methods[u2Index2]->bytecodes, acf->classes[u2Index1]->methods[u2Index2]->bytecodes_count, acf->classes[u2Index1]->methods[u2Index2]->bytecodes_size);
                if(new->method.methods[new->method.method_count + u2Index2].bytecodes == NULL)
                    return -1;
                new->method.size += acf->classes[u2Index1]->methods[u2Index2]->bytecodes_size;
            }

        }
        
        new->method.method_count += acf->classes[u2Index1]->methods_count;
    }

    return 0;

}


/**
 * Generate the constant pool component in the given cap_file structure.
 * Constant pool entries are sorted by previously updated indexes.
 */
static int generate_constant_pool_component(analyzed_cap_file* acf, cap_file* new) {

    u2 crt_index = 0;
    u2 starting_index = 0;
    u2 u2Index = 0;

    new->constant_pool.tag = COMPONENT_CONSTANTPOOL;
    new->constant_pool.size = 2;
    new->constant_pool.count = 0;
    new->constant_pool.constant_pool = NULL;

    do {
        if((acf->constant_pool[u2Index]->count != 0) && (acf->constant_pool[u2Index]->my_index == crt_index)) {
            cf_cp_info* tmp = (cf_cp_info*)realloc(new->constant_pool.constant_pool, sizeof(cf_cp_info) * (new->constant_pool.count + 1));
            if(tmp == NULL) {
                perror("generate_constant_pool");
                return -1;
            }
            new->constant_pool.constant_pool = tmp;

            if(acf->constant_pool[u2Index]->flags & CONSTANT_POOL_CLASSREF) {
                new->constant_pool.constant_pool[new->constant_pool.count].tag = CF_CONSTANT_CLASSREF;
                if(acf->constant_pool[u2Index]->flags & CONSTANT_POOL_IS_EXTERNAL) {
                    new->constant_pool.constant_pool[new->constant_pool.count].CONSTANT_Classref.class_ref.isExternal = 1;
                    new->constant_pool.constant_pool[new->constant_pool.count].CONSTANT_Classref.class_ref.ref.external_class_ref.package_token = acf->constant_pool[u2Index]->external_package->my_index;
                    new->constant_pool.constant_pool[new->constant_pool.count].CONSTANT_Classref.class_ref.ref.external_class_ref.class_token = acf->constant_pool[u2Index]->external_class_token;
                } else {
                    new->constant_pool.constant_pool[new->constant_pool.count].CONSTANT_Classref.class_ref.isExternal = 0;
                    if(acf->constant_pool[u2Index]->internal_class != NULL)
                        new->constant_pool.constant_pool[new->constant_pool.count].CONSTANT_Classref.class_ref.ref.internal_class_ref = acf->constant_pool[u2Index]->internal_class->offset;
                    else
                        new->constant_pool.constant_pool[new->constant_pool.count].CONSTANT_Classref.class_ref.ref.internal_class_ref = acf->constant_pool[u2Index]->internal_interface->offset;
                }
                new->constant_pool.constant_pool[new->constant_pool.count].CONSTANT_Classref.padding = 0;
            } else if(acf->constant_pool[u2Index]->flags & CONSTANT_POOL_INSTANCEFIELDREF) {
                new->constant_pool.constant_pool[new->constant_pool.count].tag = CF_CONSTANT_INSTANCEFIELDREF;
                if(acf->constant_pool[u2Index]->flags & CONSTANT_POOL_IS_EXTERNAL) {
                    new->constant_pool.constant_pool[new->constant_pool.count].CONSTANT_InstanceFieldref.class.isExternal = 1;
                    new->constant_pool.constant_pool[new->constant_pool.count].CONSTANT_InstanceFieldref.class.ref.external_class_ref.package_token = acf->constant_pool[u2Index]->external_package->my_index;
                    new->constant_pool.constant_pool[new->constant_pool.count].CONSTANT_InstanceFieldref.class.ref.external_class_ref.class_token = acf->constant_pool[u2Index]->external_class_token;
                    new->constant_pool.constant_pool[new->constant_pool.count].CONSTANT_InstanceFieldref.token = acf->constant_pool[u2Index]->external_field_token;
                } else {
                    new->constant_pool.constant_pool[new->constant_pool.count].CONSTANT_InstanceFieldref.class.isExternal = 0;
                    new->constant_pool.constant_pool[new->constant_pool.count].CONSTANT_InstanceFieldref.class.ref.internal_class_ref = acf->constant_pool[u2Index]->internal_class->offset;
                    new->constant_pool.constant_pool[new->constant_pool.count].CONSTANT_InstanceFieldref.token = acf->constant_pool[u2Index]->internal_field->token;
                }
            } else if(acf->constant_pool[u2Index]->flags & CONSTANT_POOL_VIRTUALMETHODREF) {
                new->constant_pool.constant_pool[new->constant_pool.count].tag = CF_CONSTANT_VIRTUALMETHODREF;
                if(acf->constant_pool[u2Index]->flags & CONSTANT_POOL_IS_EXTERNAL) {
                    new->constant_pool.constant_pool[new->constant_pool.count].CONSTANT_VirtualMethodref.class.isExternal = 1;
                    new->constant_pool.constant_pool[new->constant_pool.count].CONSTANT_VirtualMethodref.class.ref.external_class_ref.package_token = acf->constant_pool[u2Index]->external_package->my_index;
                    new->constant_pool.constant_pool[new->constant_pool.count].CONSTANT_VirtualMethodref.class.ref.external_class_ref.class_token = acf->constant_pool[u2Index]->external_class_token;
                    new->constant_pool.constant_pool[new->constant_pool.count].CONSTANT_VirtualMethodref.token = acf->constant_pool[u2Index]->method_token;
                } else {
                    new->constant_pool.constant_pool[new->constant_pool.count].CONSTANT_VirtualMethodref.class.isExternal = 0;
                    new->constant_pool.constant_pool[new->constant_pool.count].CONSTANT_VirtualMethodref.class.ref.internal_class_ref = acf->constant_pool[u2Index]->internal_class->offset;
                    new->constant_pool.constant_pool[new->constant_pool.count].CONSTANT_VirtualMethodref.token = acf->constant_pool[u2Index]->internal_method->token;
                }

            } else if(acf->constant_pool[u2Index]->flags & CONSTANT_POOL_SUPERMETHODREF) {
                new->constant_pool.constant_pool[new->constant_pool.count].tag = CF_CONSTANT_SUPERMETHODREF;
                if(acf->constant_pool[u2Index]->flags & CONSTANT_POOL_IS_EXTERNAL) {
                    new->constant_pool.constant_pool[new->constant_pool.count].CONSTANT_SuperMethodref.class.isExternal = 1;
                    new->constant_pool.constant_pool[new->constant_pool.count].CONSTANT_SuperMethodref.class.ref.external_class_ref.package_token = acf->constant_pool[u2Index]->external_package->my_index;
                    new->constant_pool.constant_pool[new->constant_pool.count].CONSTANT_SuperMethodref.class.ref.external_class_ref.class_token = acf->constant_pool[u2Index]->external_class_token;
                    new->constant_pool.constant_pool[new->constant_pool.count].CONSTANT_SuperMethodref.token = acf->constant_pool[u2Index]->method_token;
                } else {
                    new->constant_pool.constant_pool[new->constant_pool.count].CONSTANT_SuperMethodref.class.isExternal = 0;
                    new->constant_pool.constant_pool[new->constant_pool.count].CONSTANT_SuperMethodref.class.ref.internal_class_ref = acf->constant_pool[u2Index]->internal_class->offset;
                    if(acf->constant_pool[u2Index]->internal_method == NULL)
                        new->constant_pool.constant_pool[new->constant_pool.count].CONSTANT_SuperMethodref.token = acf->constant_pool[u2Index]->method_token;
                    else
                        new->constant_pool.constant_pool[new->constant_pool.count].CONSTANT_SuperMethodref.token = acf->constant_pool[u2Index]->internal_method->token;
                }
            } else if(acf->constant_pool[u2Index]->flags & CONSTANT_POOL_STATICFIELDREF) {
                new->constant_pool.constant_pool[new->constant_pool.count].tag = CF_CONSTANT_STATICFIELDREF;
                if(acf->constant_pool[u2Index]->flags & CONSTANT_POOL_IS_EXTERNAL) {
                    new->constant_pool.constant_pool[new->constant_pool.count].CONSTANT_StaticFieldref.static_field_ref.isExternal = 1;
                    new->constant_pool.constant_pool[new->constant_pool.count].CONSTANT_StaticFieldref.static_field_ref.ref.external_ref.package_token = acf->constant_pool[u2Index]->external_package->my_index;
                    new->constant_pool.constant_pool[new->constant_pool.count].CONSTANT_StaticFieldref.static_field_ref.ref.external_ref.class_token = acf->constant_pool[u2Index]->external_class_token;
                    new->constant_pool.constant_pool[new->constant_pool.count].CONSTANT_StaticFieldref.static_field_ref.ref.external_ref.token = acf->constant_pool[u2Index]->external_field_token;
                } else {
                    new->constant_pool.constant_pool[new->constant_pool.count].CONSTANT_StaticFieldref.static_field_ref.isExternal = 0;
                    new->constant_pool.constant_pool[new->constant_pool.count].CONSTANT_StaticFieldref.static_field_ref.ref.internal_ref.padding = 0;
                    new->constant_pool.constant_pool[new->constant_pool.count].CONSTANT_StaticFieldref.static_field_ref.ref.internal_ref.offset = acf->constant_pool[u2Index]->internal_field->offset;
                }
            } else if(acf->constant_pool[u2Index]->flags & CONSTANT_POOL_STATICMETHODREF) {
                new->constant_pool.constant_pool[new->constant_pool.count].tag = CF_CONSTANT_STATICMETHODREF;
                if(acf->constant_pool[u2Index]->flags & CONSTANT_POOL_IS_EXTERNAL) {
                    new->constant_pool.constant_pool[new->constant_pool.count].CONSTANT_StaticMethodref.static_method_ref.isExternal = 1;
                    new->constant_pool.constant_pool[new->constant_pool.count].CONSTANT_StaticMethodref.static_method_ref.ref.external_ref.package_token = acf->constant_pool[u2Index]->external_package->my_index;
                    new->constant_pool.constant_pool[new->constant_pool.count].CONSTANT_StaticMethodref.static_method_ref.ref.external_ref.class_token = acf->constant_pool[u2Index]->external_class_token;
                    new->constant_pool.constant_pool[new->constant_pool.count].CONSTANT_StaticMethodref.static_method_ref.ref.external_ref.token = acf->constant_pool[u2Index]->method_token;
                } else {
                    new->constant_pool.constant_pool[new->constant_pool.count].CONSTANT_StaticMethodref.static_method_ref.isExternal = 0;
                    new->constant_pool.constant_pool[new->constant_pool.count].CONSTANT_StaticMethodref.static_method_ref.ref.internal_ref.padding = 0;
                    new->constant_pool.constant_pool[new->constant_pool.count].CONSTANT_StaticMethodref.static_method_ref.ref.internal_ref.offset = acf->constant_pool[u2Index]->internal_method->offset;
                }
            }

            new->constant_pool.size += 4;

            ++new->constant_pool.count;
            ++crt_index;
            starting_index = u2Index;
        }

        ++u2Index;

        if(u2Index == acf->constant_pool_count)
            u2Index = 0;

    } while(u2Index != starting_index);

    return 0;

}


/**
 * Compute the token for each fields of the analyzed CAP file.
 * Token computation is described in section 4.3,Token-base linking, page 4-5, the CAP file format, virtual machine Specification, Java Card platform.
 */
static void compute_fields_token(analyzed_cap_file* acf) {

    u2 u2Index1 = 0;
    u1 crt_static_token = 0;

    for(; u2Index1 < acf->classes_count; ++u2Index1) {
        u2 u2Index2 = 0;
        u1 crt_token = 0;

        /* Token for public & protected non-reference fields & for public & protected static fields */
        for(; u2Index2 < acf->classes[u2Index1]->fields_count; ++u2Index2) {
            acf->classes[u2Index1]->fields[u2Index2]->token = 0xFF;
            if(acf->classes[u2Index1]->fields[u2Index2]->flags & FIELD_STATIC) {
                if(acf->classes[u2Index1]->fields[u2Index2]->flags & (FIELD_PUBLIC|FIELD_PROTECTED)) {
                    acf->classes[u2Index1]->fields[u2Index2]->token = crt_static_token;
                    ++crt_static_token;
                }
            } else if(acf->classes[u2Index1]->fields[u2Index2]->flags & (FIELD_PUBLIC|FIELD_PROTECTED) && !(acf->classes[u2Index1]->fields[u2Index2]->type->types->type & (TYPE_DESCRIPTOR_REF|TYPE_DESCRIPTOR_ARRAY))) {
                acf->classes[u2Index1]->fields[u2Index2]->token = crt_token;
                if(acf->classes[u2Index1]->fields[u2Index2]->type->types->type & TYPE_DESCRIPTOR_INT)
                    crt_token += 2;
                else
                    ++crt_token;
            }
        }

        /* Token for public & protected reference fields */
        for(u2Index2 = 0; u2Index2 < acf->classes[u2Index1]->fields_count; ++u2Index2)
            if(!(acf->classes[u2Index1]->fields[u2Index2]->flags & FIELD_STATIC) && (acf->classes[u2Index1]->fields[u2Index2]->flags & (FIELD_PUBLIC|FIELD_PROTECTED)) && (acf->classes[u2Index1]->fields[u2Index2]->type->types->type & (TYPE_DESCRIPTOR_REF|TYPE_DESCRIPTOR_ARRAY))) {
                acf->classes[u2Index1]->fields[u2Index2]->token = crt_token;
                ++crt_token;
            }

        /* Token for package & private reference fields */
        for(u2Index2 = 0; u2Index2 < acf->classes[u2Index1]->fields_count; ++u2Index2)
            if(!(acf->classes[u2Index1]->fields[u2Index2]->flags & FIELD_STATIC) && (acf->classes[u2Index1]->fields[u2Index2]->flags & (FIELD_PACKAGE|FIELD_PRIVATE)) && (acf->classes[u2Index1]->fields[u2Index2]->type->types->type & (TYPE_DESCRIPTOR_REF|TYPE_DESCRIPTOR_ARRAY))) {
                acf->classes[u2Index1]->fields[u2Index2]->token = crt_token;
                ++crt_token;
            }

        /* Token for package & private non-reference fields */
        for(u2Index2 = 0; u2Index2 < acf->classes[u2Index1]->fields_count; ++u2Index2)
            if(!(acf->classes[u2Index1]->fields[u2Index2]->flags & FIELD_STATIC) && (acf->classes[u2Index1]->fields[u2Index2]->flags & (FIELD_PACKAGE|FIELD_PRIVATE)) && !(acf->classes[u2Index1]->fields[u2Index2]->type->types->type & (TYPE_DESCRIPTOR_REF|TYPE_DESCRIPTOR_ARRAY))) {
                acf->classes[u2Index1]->fields[u2Index2]->token = crt_token;
                if(acf->classes[u2Index1]->fields[u2Index2]->type->types->type & TYPE_DESCRIPTOR_INT)
                    crt_token += 2;
                else
                    ++crt_token;
            }
    }

}


/**
 * Get the largest public method token in a given class within the given parsed export file.
 */
static int16_t get_largest_public_method_token(export_file* ef, u1 class_token) {

    u1 u1Index = 0;
    int16_t largest_token = -1;

    for(; u1Index < ef->export_class_count; ++u1Index) {
        if(ef->classes[u1Index].token == class_token) {
            u2 u2Index = 0;

            for(; u2Index < ef->classes[u1Index].export_methods_count; ++u2Index)
                if(!(ef->classes[u1Index].methods[u2Index].access_flags & EF_ACC_STATIC) && !((ef->constant_pool[ef->classes[u1Index].methods[u2Index].name_index].CONSTANT_Utf8.length == 6) && (strncmp((char*)(ef->constant_pool[ef->classes[u1Index].methods[u2Index].name_index].CONSTANT_Utf8.bytes), "<init>",6) == 0)) && (ef->classes[u1Index].methods[u2Index].token > largest_token))
                    largest_token = ef->classes[u1Index].methods[u2Index].token;;

            break;
        }
    }

    return largest_token;

}


/**
 * Compute the token for each methods of the analyzed CAP file.
 * Token computation is described in section 4.3,Token-base linking, page 4-5, the CAP file format, virtual machine Specification, Java Card platform.
 */
static void compute_virtual_methods_token(analyzed_cap_file* acf) {

    u2 u2Index1 = 0;

    for(; u2Index1 < acf->classes_count; ++u2Index1) {
        u2 u2Index2 = 0;
        u1 crt_public_token = 0;
        u1 crt_package_token = 0x80;

        acf->classes[u2Index1]->has_largest_public_method_token = 0;
        acf->classes[u2Index1]->has_largest_package_method_token = 0;

        if(acf->classes[u2Index1]->superclass) {
            if(acf->classes[u2Index1]->superclass->flags & CONSTANT_POOL_IS_EXTERNAL) {
                int16_t largest_public_method_token = get_largest_public_method_token(acf->classes[u2Index1]->superclass->external_package->ef, acf->classes[u2Index1]->superclass->external_class_token);
                if(largest_public_method_token != -1) {
                    crt_public_token = largest_public_method_token + 1;
                    acf->classes[u2Index1]->has_largest_public_method_token = 1;
                    acf->classes[u2Index1]->largest_public_method_token  = largest_public_method_token;
                }
            } else {
                if(acf->classes[u2Index1]->superclass->internal_class->has_largest_public_method_token) {
                    crt_public_token = acf->classes[u2Index1]->superclass->internal_class->largest_public_method_token + 1;
                    acf->classes[u2Index1]->has_largest_public_method_token = 1;
                    acf->classes[u2Index1]->largest_public_method_token = acf->classes[u2Index1]->superclass->internal_class->largest_public_method_token;
                }
                if(acf->classes[u2Index1]->superclass->internal_class->has_largest_package_method_token) {
                    crt_package_token = acf->classes[u2Index1]->superclass->internal_class->largest_package_method_token + 1;
                    acf->classes[u2Index1]->has_largest_package_method_token = 1;
                    acf->classes[u2Index1]->largest_package_method_token = acf->classes[u2Index1]->superclass->internal_class->largest_public_method_token;
                }
            }
        }

        for(; u2Index2 < acf->classes[u2Index1]->methods_count; ++u2Index2) {
            if(!(acf->classes[u2Index1]->methods[u2Index2]->flags & (METHOD_STATIC|METHOD_INIT))) {
                if(acf->classes[u2Index1]->methods[u2Index2]->flags & (METHOD_PUBLIC|METHOD_PROTECTED)) {
                    if(acf->classes[u2Index1]->methods[u2Index2]->is_overriding) {
                        if(acf->classes[u2Index1]->methods[u2Index2]->internal_overrided_method)
                            acf->classes[u2Index1]->methods[u2Index2]->token = acf->classes[u2Index1]->methods[u2Index2]->internal_overrided_method->token;
                    } else {
                        acf->classes[u2Index1]->methods[u2Index2]->token = crt_public_token;
                        acf->classes[u2Index1]->largest_public_method_token = crt_public_token;
                        ++crt_public_token;
                    }

                    acf->classes[u2Index1]->has_largest_public_method_token = 1;
                } else if(acf->classes[u2Index1]->methods[u2Index2]->flags & METHOD_PRIVATE) {
                    acf->classes[u2Index1]->methods[u2Index2]->token = 0xFF;
                } else {
                    if(acf->classes[u2Index1]->methods[u2Index2]->is_overriding) {
                        if(acf->classes[u2Index1]->methods[u2Index2]->internal_overrided_method)
                            acf->classes[u2Index1]->methods[u2Index2]->token = acf->classes[u2Index1]->methods[u2Index2]->internal_overrided_method->token;
                    } else {
                        acf->classes[u2Index1]->methods[u2Index2]->token = crt_package_token;
                        acf->classes[u2Index1]->largest_package_method_token = crt_package_token;
                        ++crt_package_token;
                    }

                    acf->classes[u2Index1]->has_largest_package_method_token = 1;
                }
            }
        }
    }

}


/**
 * Compute tokens for each interface, class, field and method of the analyzed CAP file.
 * Token computation is described in section 4.3,Token-base linking, page 4-5, the CAP file format, virtual machine Specification, Java Card platform.
 */
static void compute_tokens(analyzed_cap_file* acf) {

    u2 u2Index1 = 0;
    u1 crt_token = 0;

    /* Interface token */

    for(u2Index1 = 0; u2Index1 < acf->interfaces_count; ++u2Index1) {
        if(acf->interfaces[u2Index1]->flags & INTERFACE_PUBLIC) {
            acf->interfaces[u2Index1]->token = crt_token;
            ++crt_token;
        } else {
            acf->interfaces[u2Index1]->token = 0xFF;
        }
    }

    /* Class token */

    for(; u2Index1 < acf->classes_count; ++u2Index1) {
        if(acf->classes[u2Index1]->flags & CLASS_PUBLIC) {
            acf->classes[u2Index1]->token = crt_token;
            ++crt_token;
        } else {
            acf->classes[u2Index1]->token = 0xFF;
        }
    }

    /* Static field token */

    for(u2Index1 = 0; u2Index1 < acf->classes_count; ++u2Index1) {
        u2 u2Index2 = 0;
        crt_token = 0;

        for(; u2Index2 < acf->classes[u2Index1]->fields_count; ++u2Index2) {
            if((acf->classes[u2Index1]->fields[u2Index2]->flags & FIELD_STATIC) && ((acf->classes[u2Index1]->fields[u2Index2]->flags & (FIELD_PUBLIC|FIELD_PROTECTED|FIELD_FINAL)) == (FIELD_PUBLIC|FIELD_PROTECTED))) {
                acf->classes[u2Index1]->fields[u2Index2]->token = crt_token;
                ++crt_token;
            }
        }
    }

    /* Static method and constructor token */

    for(u2Index1 = 0; u2Index1 < acf->classes_count; ++u2Index1) {
        u2 u2Index2 = 0;
        crt_token = 0;

        for(; u2Index2 < acf->classes[u2Index1]->methods_count; ++u2Index2) {
            if((acf->classes[u2Index1]->methods[u2Index2]->flags & (METHOD_STATIC|METHOD_INIT)) && (acf->classes[u2Index1]->methods[u2Index2]->flags & (METHOD_PUBLIC|METHOD_PROTECTED))) {
                acf->classes[u2Index1]->methods[u2Index2]->token = crt_token;
                ++crt_token;
            }
        }
    }

    compute_fields_token(acf);
    compute_virtual_methods_token(acf);

    /* Interface method token */

    for(u2Index1 = 0; u2Index1 < acf->interfaces_count; ++u2Index1) {
        u2 u2Index2 = 0;

        for(; u2Index2 < acf->interfaces[u2Index1]->methods_count; ++u2Index2)
            acf->interfaces[u2Index1]->methods[u2Index2]->token = u2Index2;
    }

}


/**
 * Get an instance method offset given its token.
 */
static u2 get_method_offset_from_token(class_info* class, u1 token) {

    u2 u2Index = 0;

    for(; u2Index < class->methods_count; ++u2Index)
        if(!(class->methods[u2Index]->flags & (METHOD_STATIC|METHOD_INIT)) && (class->methods[u2Index]->token == token))
            return class->methods[u2Index]->offset;

    return 0xFFFF;

}


/**
 * Get the method token implementing an interface method given its token.
 */
static u1 get_implemented_method_token(implemented_interface_info* interface, u1 token) {

    u1 u1Index = 0;

    if(interface->ref->flags & CONSTANT_POOL_IS_EXTERNAL)
        return interface->index[token].implementation ? interface->index[token].implementation->token : interface->index[token].method_token;
    else
        for(; u1Index < interface->count; ++u1Index)
            if(interface->index[u1Index].declaration->token == token)
                return interface->index[u1Index].implementation ? interface->index[u1Index].implementation->token : interface->index[u1Index].method_token;

    return 0;

}


/**
 * Generate the class component in the given cap_file structure.
 * Only work when there is no remote nor signature pool.
 * We supposed that the interfaces and classes are in the right order
 * (i.e. superinterfaces/superclasses before subinterfaces/subclasses)
 */
static int generate_class_component(analyzed_cap_file* acf, cap_file* new) {

    u2 u2Index1 = 0;

    new->class.tag = COMPONENT_CLASS;
    new->class.size = 0;

    if((acf->info.javacard_major_version == 2) && (acf->info.javacard_minor_version > 1)) {
        new->class.can_have_signature_pool = 1;
        new->class.size += 2;
    } else {
        new->class.can_have_signature_pool = 0;
    }

    new->class.signature_pool_count = 0;
    new->class.signature_pool_length = 0;
    new->class.signature_pool = NULL;        

    new->class.interfaces_count = acf->interfaces_count;
    new->class.interfaces = (cf_interface_info*)malloc(sizeof(cf_interface_info) * new->class.interfaces_count);
    if(new->class.interfaces == NULL) {
        perror("generate_class_component");
        return -1;
    }

    for(; u2Index1 < acf->interfaces_count; ++u2Index1) {
        u1 u1Index = 0;

        new->class.interfaces[u2Index1].offset = new->class.size;
        acf->interfaces[u2Index1]->offset = new->class.size;

        new->class.interfaces[u2Index1].flags = CLASS_ACC_INTERFACE;
        if(acf->interfaces[u2Index1]->flags & INTERFACE_SHAREABLE)
            new->class.interfaces[u2Index1].flags |= CLASS_ACC_SHAREABLE;

        new->class.interfaces[u2Index1].interface_count = acf->interfaces[u2Index1]->superinterfaces_count;
        new->class.interfaces[u2Index1].superinterfaces = (cf_class_ref_info*)malloc(sizeof(cf_class_ref_info) * new->class.interfaces[u2Index1].interface_count);
        if(new->class.interfaces[u2Index1].superinterfaces == NULL) {
            perror("generate_class_component");
            return -1;
        }

        for(; u1Index < new->class.interfaces[u2Index1].interface_count; ++u1Index) {
            if(acf->interfaces[u2Index1]->superinterfaces[u1Index]->flags & CONSTANT_POOL_IS_EXTERNAL) {
                new->class.interfaces[u2Index1].superinterfaces[u1Index].isExternal = 1;
                new->class.interfaces[u2Index1].superinterfaces[u1Index].ref.external_class_ref.package_token = acf->interfaces[u2Index1]->superinterfaces[u1Index]->external_package->my_index;
                new->class.interfaces[u2Index1].superinterfaces[u1Index].ref.external_class_ref.class_token = acf->interfaces[u2Index1]->superinterfaces[u1Index]->external_class_token;
            } else {
                new->class.interfaces[u2Index1].superinterfaces[u1Index].isExternal = 0;
                new->class.interfaces[u2Index1].superinterfaces[u1Index].ref.internal_class_ref = acf->interfaces[u2Index1]->superinterfaces[u1Index]->internal_interface->offset;
            }
        }

        new->class.interfaces[u2Index1].has_interface_name = 0;

        new->class.size += 1 + (new->class.interfaces[u2Index1].interface_count * 2);
    }

    new->class.classes_count = acf->classes_count;
    new->class.classes = (cf_class_info*)malloc(sizeof(cf_class_info) * new->class.classes_count);
    if(new->class.classes == NULL) {
        perror("generate_class_component");
        return -1;
    }

    for(u2Index1 = 0; u2Index1 < acf->classes_count; ++u2Index1) {
        u2 u2Index2 = 0;
        u1 u1Index1 = 0;

        new->class.classes[u2Index1].offset = new->class.size;
        acf->classes[u2Index1]->offset = new->class.size;

        new->class.classes[u2Index1].flags = 0;
        if(acf->classes[u2Index1]->flags & CLASS_SHAREABLE)
            new->class.classes[u2Index1].flags = CLASS_ACC_SHAREABLE;

        new->class.classes[u2Index1].interface_count = acf->classes[u2Index1]->interfaces_count;

        if(acf->classes[u2Index1]->superclass) {
            new->class.classes[u2Index1].has_superclass = 1;
            if(acf->classes[u2Index1]->superclass->flags & CONSTANT_POOL_IS_EXTERNAL) {
                new->class.classes[u2Index1].super_class_ref.isExternal = 1;
                new->class.classes[u2Index1].super_class_ref.ref.external_class_ref.package_token = acf->classes[u2Index1]->superclass->external_package->my_index;
                new->class.classes[u2Index1].super_class_ref.ref.external_class_ref.class_token = acf->classes[u2Index1]->superclass->external_class_token;
            } else {
                new->class.classes[u2Index1].super_class_ref.isExternal = 0;
                new->class.classes[u2Index1].super_class_ref.ref.internal_class_ref = acf->classes[u2Index1]->superclass->internal_class->offset;
            }
        } else {
            new->class.classes[u2Index1].has_superclass = 0;
        }

        new->class.classes[u2Index1].declared_instance_size = 0;
        for(; u2Index2 < acf->classes[u2Index1]->fields_count; ++u2Index2)
            if(!(acf->classes[u2Index1]->fields[u2Index2]->flags & FIELD_STATIC)) {
                if((acf->classes[u2Index1]->fields[u2Index2]->type->types->type & (TYPE_DESCRIPTOR_INT|TYPE_DESCRIPTOR_ARRAY)) == TYPE_DESCRIPTOR_INT)
                    new->class.classes[u2Index1].declared_instance_size += 2;
                else
                    ++new->class.classes[u2Index1].declared_instance_size;
            }

        new->class.classes[u2Index1].first_reference_token = 0xFF;
        new->class.classes[u2Index1].reference_count = 0;
        for(u2Index2 = 0; u2Index2 < acf->classes[u2Index1]->fields_count; ++u2Index2)
            if(!(acf->classes[u2Index1]->fields[u2Index2]->flags & FIELD_STATIC) && (acf->classes[u2Index1]->fields[u2Index2]->type->types->type & (TYPE_DESCRIPTOR_REF|TYPE_DESCRIPTOR_ARRAY))) {
                if(acf->classes[u2Index1]->fields[u2Index2]->token < new->class.classes[u2Index1].first_reference_token)
                    new->class.classes[u2Index1].first_reference_token = acf->classes[u2Index1]->fields[u2Index2]->token;

                ++new->class.classes[u2Index1].reference_count;
            }

        if(acf->classes[u2Index1]->superclass) {
            u1 is_empty = 1;
            u1 smallest_token = 0xFF;

            /* public method (same when superclass is external or internal */

            for(u2Index2 = 0; u2Index2 < acf->classes[u2Index1]->methods_count; ++u2Index2) {
                if(!(acf->classes[u2Index1]->methods[u2Index2]->flags & (METHOD_STATIC|METHOD_INIT)) && (acf->classes[u2Index1]->methods[u2Index2]->flags & (METHOD_PUBLIC|METHOD_PROTECTED)) && (smallest_token > acf->classes[u2Index1]->methods[u2Index2]->token)) {
                    smallest_token = acf->classes[u2Index1]->methods[u2Index2]->token;
                    is_empty = 0;
                }
            }

            if(is_empty) {
                if(acf->classes[u2Index1]->has_largest_public_method_token)
                    new->class.classes[u2Index1].public_method_table_base = acf->classes[u2Index1]->largest_public_method_token + 1;
                else
                    new->class.classes[u2Index1].public_method_table_base = 0;
                new->class.classes[u2Index1].public_method_table_count = 0;
                new->class.classes[u2Index1].public_virtual_method_table = NULL;
            } else {
                new->class.classes[u2Index1].public_method_table_base = smallest_token;
                new->class.classes[u2Index1].public_method_table_count = (u1)((acf->classes[u2Index1]->largest_public_method_token - smallest_token) + 1);

                new->class.classes[u2Index1].public_virtual_method_table = (u2*)malloc(sizeof(u2) * new->class.classes[u2Index1].public_method_table_count);
                if(new->class.classes[u2Index1].public_virtual_method_table == NULL) {
                    perror("generate_class_component");
                    return -1;
                }

                for(; u1Index1 < new->class.classes[u2Index1].public_method_table_count; ++u1Index1)
                    new->class.classes[u2Index1].public_virtual_method_table[u1Index1] = get_method_offset_from_token(acf->classes[u2Index1], u1Index1 + new->class.classes[u2Index1].public_method_table_base);
            }

            /* Package method */

            is_empty = 1;
            smallest_token = 0xFF;

            for(u2Index2 = 0; u2Index2 < acf->classes[u2Index1]->methods_count; ++u2Index2)
                if(!(acf->classes[u2Index1]->methods[u2Index2]->flags & (METHOD_STATIC|METHOD_INIT)) && (acf->classes[u2Index1]->methods[u2Index2]->flags & METHOD_PACKAGE) && (smallest_token > acf->classes[u2Index1]->methods[u2Index2]->token)) {
                    smallest_token = acf->classes[u2Index1]->methods[u2Index2]->token;
                    is_empty = 0;
                }

            if(is_empty) {
                if(acf->classes[u2Index1]->has_largest_package_method_token)
                    new->class.classes[u2Index1].package_method_table_base = (acf->classes[u2Index1]->largest_package_method_token & 0x7F) + 1;
                else
                    new->class.classes[u2Index1].package_method_table_base = 0;
                new->class.classes[u2Index1].package_method_table_count = 0;
                new->class.classes[u2Index1].package_virtual_method_table = NULL;
            } else {
                new->class.classes[u2Index1].package_method_table_base = smallest_token & 0x7F;
                new->class.classes[u2Index1].package_method_table_count = (u1)((acf->classes[u2Index1]->largest_package_method_token - smallest_token) + 1);

                new->class.classes[u2Index1].package_virtual_method_table = (u2*)malloc(sizeof(u2) * new->class.classes[u2Index1].package_method_table_count);
                if(new->class.classes[u2Index1].package_virtual_method_table == NULL) {
                    perror("generate_class_component");
                    return -1;
                }

                for(u1Index1 = 0; u1Index1 < new->class.classes[u2Index1].package_method_table_count; ++u1Index1)
                    new->class.classes[u2Index1].package_virtual_method_table[u1Index1] = get_method_offset_from_token(acf->classes[u2Index1], (u1Index1 + new->class.classes[u2Index1].package_method_table_base) | 0x80);
            }
        } else {
            new->class.classes[u2Index1].public_method_table_base = 0;
            new->class.classes[u2Index1].public_method_table_count = acf->classes[u2Index1]->has_largest_public_method_token ? acf->classes[u2Index1]->largest_public_method_token + 1 : 0;

            if(new->class.classes[u2Index1].public_method_table_count != 0) {
                new->class.classes[u2Index1].public_virtual_method_table = (u2*)malloc(sizeof(u2) * new->class.classes[u2Index1].public_method_table_count);
                if(new->class.classes[u2Index1].public_virtual_method_table == NULL) {
                    perror("generate_class_component");
                    return -1;
                }

                for(; u1Index1 < new->class.classes[u2Index1].public_method_table_count; ++u1Index1)
                    new->class.classes[u2Index1].public_virtual_method_table[u1Index1] = get_method_offset_from_token(acf->classes[u2Index1], u1Index1);
            } else {
                new->class.classes[u2Index1].public_virtual_method_table = NULL;
            }

            new->class.classes[u2Index1].package_method_table_base = 0;
            new->class.classes[u2Index1].package_method_table_count = acf->classes[u2Index1]->has_largest_package_method_token ? (acf->classes[u2Index1]->largest_package_method_token & 0x7F) + 1 : 0;

            if(new->class.classes[u2Index1].package_method_table_count != 0) {
                new->class.classes[u2Index1].package_virtual_method_table = (u2*)malloc(sizeof(u2) * new->class.classes[u2Index1].package_method_table_count);
                if(new->class.classes[u2Index1].package_virtual_method_table == NULL) {
                    perror("generate_class_component");
                    return -1;
                }

                for(u1Index1 = 0; u1Index1 < new->class.classes[u2Index1].package_method_table_count; ++u1Index1)
                    new->class.classes[u2Index1].package_virtual_method_table[u1Index1] = get_method_offset_from_token(acf->classes[u2Index1], u1Index1 | 0x80);
            } else {
                new->class.classes[u2Index1].package_virtual_method_table = NULL;
            }
        }

        new->class.size += 10 + (new->class.classes[u2Index1].public_method_table_count * 2) + (new->class.classes[u2Index1].package_method_table_count * 2);

        new->class.classes[u2Index1].interfaces = (cf_implemented_interface_info*)malloc(sizeof(cf_implemented_interface_info) * new->class.classes[u2Index1].interface_count);
        if(new->class.classes[u2Index1].interfaces == NULL) {
            perror("generate_class_component");
            return -1;
        }

        for(u1Index1 = 0; u1Index1 < new->class.classes[u2Index1].interface_count; ++u1Index1) {
            u1 u1Index2 = 0;

            new->class.classes[u2Index1].interfaces[u1Index1].interface.isExternal = (acf->classes[u2Index1]->interfaces[u1Index1].ref->flags & CONSTANT_POOL_IS_EXTERNAL) != 0;
            if(new->class.classes[u2Index1].interfaces[u1Index1].interface.isExternal) {
                new->class.classes[u2Index1].interfaces[u1Index1].interface.ref.external_class_ref.package_token = acf->classes[u2Index1]->interfaces[u1Index1].ref->external_package->my_index;
                new->class.classes[u2Index1].interfaces[u1Index1].interface.ref.external_class_ref.class_token = acf->classes[u2Index1]->interfaces[u1Index1].ref->external_class_token;
            } else {
                new->class.classes[u2Index1].interfaces[u1Index1].interface.ref.internal_class_ref = acf->classes[u2Index1]->interfaces[u1Index1].ref->internal_interface->offset;
            }

            new->class.classes[u2Index1].interfaces[u1Index1].count = acf->classes[u2Index1]->interfaces[u1Index1].count;

            if(new->class.classes[u2Index1].interfaces[u1Index1].count != 0) {
                new->class.classes[u2Index1].interfaces[u1Index1].index = (u1*)malloc(new->class.classes[u2Index1].interfaces[u1Index1].count);
                if(new->class.classes[u2Index1].interfaces[u1Index1].index == NULL) {
                    perror("generate_class_component");
                    return -1;
                }

                for(; u1Index2 < acf->classes[u2Index1]->interfaces[u1Index1].count; ++u1Index2) {
                    new->class.classes[u2Index1].interfaces[u1Index1].index[u1Index2] = get_implemented_method_token(acf->classes[u2Index1]->interfaces + u1Index1, u1Index2);
                }
            } else {
                new->class.classes[u2Index1].interfaces[u1Index1].index = NULL;
            }

            new->class.size += 3 + new->class.classes[u2Index1].interfaces[u1Index1].count;
        }

        new->class.classes[u2Index1].has_remote_interfaces = 0;   /* TODO */

    }

    return 0;

}


/**
 * Generate the static field component in the given cap_file structure.
 */
static int generate_static_field_component(analyzed_cap_file* acf, cap_file* new) {

    u2 u2Index1 = 0;
    u2 crt_offset = 0;

    new->static_field.tag = COMPONENT_STATICFIELD;
    new->static_field.size = 10;
    new->static_field.image_size = 0;
    new->static_field.reference_count = 0;
    new->static_field.array_init_count = 0;
    new->static_field.array_init = NULL;
    new->static_field.default_value_count = 0;
    new->static_field.non_default_value_count = 0;
    new->static_field.non_default_values = NULL;

    /* Segment 1 */

    for(; u2Index1 < acf->classes_count; ++u2Index1) {
        u2 u2Index2 = 0;

        for(; u2Index2 < acf->classes[u2Index1]->fields_count; ++u2Index2)
            if(((acf->classes[u2Index1]->fields[u2Index2]->flags & (FIELD_STATIC|FIELD_HAS_VALUE)) == (FIELD_STATIC|FIELD_HAS_VALUE)) && (acf->classes[u2Index1]->fields[u2Index2]->type->types->type & TYPE_DESCRIPTOR_ARRAY)) {
                cf_array_init_info* tmp = NULL;

                acf->classes[u2Index1]->fields[u2Index2]->offset = crt_offset;
                ++new->static_field.reference_count;

                tmp = (cf_array_init_info*)realloc(new->static_field.array_init, sizeof(cf_array_init_info) * (new->static_field.array_init_count + 1));
                if(tmp == NULL) {
                    perror("generate_static_field_component");
                    return -1;
                }
                new->static_field.array_init = tmp;

                if(acf->classes[u2Index1]->fields[u2Index2]->type->types->type & TYPE_DESCRIPTOR_BOOLEAN)
                    new->static_field.array_init[new->static_field.array_init_count].type = 2;
                else if(acf->classes[u2Index1]->fields[u2Index2]->type->types->type & TYPE_DESCRIPTOR_BYTE)
                    new->static_field.array_init[new->static_field.array_init_count].type = 3;
                else if(acf->classes[u2Index1]->fields[u2Index2]->type->types->type & TYPE_DESCRIPTOR_SHORT)
                    new->static_field.array_init[new->static_field.array_init_count].type = 4;
                else
                    new->static_field.array_init[new->static_field.array_init_count].type = 5;

                new->static_field.array_init[new->static_field.array_init_count].count = acf->classes[u2Index1]->fields[u2Index2]->value_size;

                new->static_field.array_init[new->static_field.array_init_count].values = (u1*)malloc(acf->classes[u2Index1]->fields[u2Index2]->value_size);
                if(new->static_field.array_init[new->static_field.array_init_count].values == NULL) {
                    perror("generate_static_field_component");
                    return -1;
                }
                memcpy(new->static_field.array_init[new->static_field.array_init_count].values, acf->classes[u2Index1]->fields[u2Index2]->value, acf->classes[u2Index1]->fields[u2Index2]->value_size);

                ++new->static_field.array_init_count;

                crt_offset += 2;
                new->static_field.size += 3 + acf->classes[u2Index1]->fields[u2Index2]->value_size;
            }
    }

    /* Segment 2 */

    for(u2Index1 = 0; u2Index1 < acf->classes_count; ++u2Index1) {
        u2 u2Index2 = 0;

        for(; u2Index2 < acf->classes[u2Index1]->fields_count; ++u2Index2)
            if(((acf->classes[u2Index1]->fields[u2Index2]->flags & (FIELD_STATIC|FIELD_HAS_VALUE)) == FIELD_STATIC) && (acf->classes[u2Index1]->fields[u2Index2]->type->types->type & TYPE_DESCRIPTOR_ARRAY)) {
                acf->classes[u2Index1]->fields[u2Index2]->offset = crt_offset;
                ++new->static_field.reference_count;
                crt_offset += 2;
            }
    }

    /* Segment 3 */

    for(u2Index1 = 0; u2Index1 < acf->classes_count; ++u2Index1) {
        u2 u2Index2 = 0;

        for(; u2Index2 < acf->classes[u2Index1]->fields_count; ++u2Index2)
            if(((acf->classes[u2Index1]->fields[u2Index2]->flags & (FIELD_STATIC|FIELD_HAS_VALUE)) == FIELD_STATIC) && !(acf->classes[u2Index1]->fields[u2Index2]->type->types->type & TYPE_DESCRIPTOR_ARRAY)) {
                acf->classes[u2Index1]->fields[u2Index2]->offset = crt_offset;

                if(acf->classes[u2Index1]->fields[u2Index2]->type->types->type & (TYPE_DESCRIPTOR_BOOLEAN|TYPE_DESCRIPTOR_BYTE)) {
                    ++new->static_field.default_value_count;
                    ++crt_offset;
                } else if(acf->classes[u2Index1]->fields[u2Index2]->type->types->type & TYPE_DESCRIPTOR_SHORT) {
                    new->static_field.default_value_count += 2;
                    crt_offset += 2;
                } else {
                    new->static_field.default_value_count += 4;
                    crt_offset += 4;
                }
            }
    }

    /* Segment 4 */

    for(u2Index1 = 0; u2Index1 < acf->classes_count; ++u2Index1) {
        u2 u2Index2 = 0;

        for(; u2Index2 < acf->classes[u2Index1]->fields_count; ++u2Index2)
            if(((acf->classes[u2Index1]->fields[u2Index2]->flags & (FIELD_STATIC|FIELD_HAS_VALUE)) == (FIELD_STATIC|FIELD_HAS_VALUE)) && !(acf->classes[u2Index1]->fields[u2Index2]->type->types->type & TYPE_DESCRIPTOR_ARRAY)) {
                u1* tmp = NULL;

                acf->classes[u2Index1]->fields[u2Index2]->offset = crt_offset;

                tmp = (u1*)realloc(new->static_field.non_default_values, new->static_field.non_default_value_count + acf->classes[u2Index1]->fields[u2Index2]->value_size);
                if(tmp == NULL) {
                    perror("generate_static_field_component");
                    return -1;
                }
                new->static_field.non_default_values = tmp;

                memcpy(new->static_field.non_default_values + new->static_field.non_default_value_count, acf->classes[u2Index1]->fields[u2Index2]->value, acf->classes[u2Index1]->fields[u2Index2]->value_size);

                new->static_field.non_default_value_count += acf->classes[u2Index1]->fields[u2Index2]->value_size;
                crt_offset += acf->classes[u2Index1]->fields[u2Index2]->value_size;
            }
    }

    new->static_field.image_size = (new->static_field.reference_count * 2) + new->static_field.default_value_count + new->static_field.non_default_value_count;
    new->static_field.size += new->static_field.non_default_value_count;

    return 0;

}


/**
 * Add reference location entries in the index array given the previous and current offset.
 */
static int add_reference_location(u1** index, u2* index_count, u2 crt_offset, u2* prev_offset) {

    u2 u2Index = 0;
    u1* tmp = NULL;
    u2 crt_jump = crt_offset - *prev_offset;
    u2 nb_full_jump = crt_jump / 255;
    u2 remainder_jump = crt_jump % 255;

    tmp = (u1*)realloc(*index, *index_count + nb_full_jump + 1);
    if(tmp == NULL) {
        perror("add_reference_location");
        return -1;
    }
    *index = tmp;

    for(; u2Index < nb_full_jump; ++u2Index)
        (*index)[*index_count + u2Index] = 255;

    (*index)[*index_count + nb_full_jump] = remainder_jump;

    *index_count += nb_full_jump + 1;
    *prev_offset = crt_offset;

    return 0;

}


/**
 * Generate the reference location component in the given cap_file structure.
 */
static int generate_reference_location_component(analyzed_cap_file* acf, cap_file* new) {

    u1 u1Index = 0;
    u2 u2Index1 = 0;

    u2 prev_offset = 0;
    u2 prev_offset2 = 0;

    new->reference_location.tag = COMPONENT_REFERENCELOCATION;
    new->reference_location.size = 4;
    new->reference_location.byte_index_count = 0;
    new->reference_location.offset_to_byte_indices = NULL;
    new->reference_location.byte2_index_count = 0;
    new->reference_location.offset_to_byte2_indices = NULL;

    for(; u1Index < acf->exception_handlers_count; ++u1Index) {
        if(acf->exception_handlers[u1Index]->catch_type != NULL)
            if(add_reference_location(&(new->reference_location.offset_to_byte2_indices), &(new->reference_location.byte2_index_count), (1 + (u1Index * 8) + 6), &prev_offset2) == -1)
                return -1;
    }

    for(u2Index1 = 0; u2Index1 < acf->classes_count; ++u2Index1) {
        u2 u2Index2 = 0;

        for(; u2Index2 < acf->classes[u2Index1]->methods_count; ++u2Index2) {
            u2 u2Index3 = 0;

            for(; u2Index3 < acf->classes[u2Index1]->methods[u2Index2]->bytecodes_count; ++u2Index3)
                if(acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3]->has_ref)
                    switch(acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3]->opcode) {
                        case 131:
                        case 132:
                        case 133:
                        case 134:
                        case 135:
                        case 136:
                        case 137:
                        case 138:
                        case 173:
                        case 174:
                        case 175:
                        case 176:
                        case 181:
                        case 182:
                        case 183:
                        case 184:
                            if(add_reference_location(&(new->reference_location.offset_to_byte_indices), &(new->reference_location.byte_index_count), acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3]->info_offset + 1, &prev_offset) == -1)
                                return -1;
                            break;

                        case 142:
                        case 148:
                        case 149:
                            if(add_reference_location(&(new->reference_location.offset_to_byte2_indices), &(new->reference_location.byte2_index_count), acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3]->info_offset + 2, &prev_offset2) == -1)
                                return -1;
                            break;

                        default:
                            if(add_reference_location(&(new->reference_location.offset_to_byte2_indices), &(new->reference_location.byte2_index_count), acf->classes[u2Index1]->methods[u2Index2]->bytecodes[u2Index3]->info_offset + 1, &prev_offset2) == -1)
                                return -1;
                    }
        }
    }

    new->reference_location.size += new->reference_location.byte_index_count + new->reference_location.byte2_index_count;

    return 0;

}


/**
 * Generate the export component in the given cap_file structure.
 */
static int generate_export_component(analyzed_cap_file* acf, cap_file* new) {

    u2 u2Index1 = 0; 

    new->export.tag = COMPONENT_EXPORT;
    new->export.size = 1;
    new->export.class_count = 0;
    new->export.class_exports = NULL;

    if(has_applet_component(acf)) {
        for(; u2Index1 < acf->interfaces_count; ++u2Index1)
            if((acf->interfaces[u2Index1]->flags & (INTERFACE_SHAREABLE|INTERFACE_PUBLIC)) == (INTERFACE_SHAREABLE|INTERFACE_PUBLIC)) {
                cf_class_export_info* tmp = (cf_class_export_info*)realloc(new->export.class_exports, sizeof(cf_class_export_info) * (new->export.class_count + 1));
                if(tmp == NULL) {
                    perror("generate_export_component");
                    return -1;
                }
                new->export.class_exports = tmp;

                new->export.class_exports[new->export.class_count].class_offset = acf->interfaces[u2Index1]->offset;
                new->export.class_exports[new->export.class_count].static_field_count = 0;
                new->export.class_exports[new->export.class_count].static_method_count = 0;
                new->export.class_exports[new->export.class_count].static_field_offsets = NULL;
                new->export.class_exports[new->export.class_count].static_method_offsets = NULL;

                ++new->export.class_count;
            }
        new->export.size += (new->export.class_count * 4);
    } else {
        for(; u2Index1 < acf->interfaces_count; ++u2Index1)
            if(acf->interfaces[u2Index1]->flags & INTERFACE_PUBLIC) {
                cf_class_export_info* tmp = (cf_class_export_info*)realloc(new->export.class_exports, sizeof(cf_class_export_info) * (new->export.class_count + 1));
                if(tmp == NULL) {
                    perror("generate_export_component");
                    return -1;
                }
                new->export.class_exports = tmp;

                new->export.class_exports[new->export.class_count].class_offset = acf->interfaces[u2Index1]->offset;
                new->export.class_exports[new->export.class_count].static_field_count = 0;
                new->export.class_exports[new->export.class_count].static_method_count = 0;
                new->export.class_exports[new->export.class_count].static_field_offsets = NULL;
                new->export.class_exports[new->export.class_count].static_method_offsets = NULL;

                ++new->export.class_count;
            }
        new->export.size += (new->export.class_count * 4);

        for(u2Index1 = 0; u2Index1 < acf->classes_count; ++u2Index1)
            if(acf->classes[u2Index1]->flags & CLASS_PUBLIC) {
                u2 u2Index2 = 0;
                cf_class_export_info* tmp1 = (cf_class_export_info*)realloc(new->export.class_exports, sizeof(cf_class_export_info) * (new->export.class_count + 1));
                if(tmp1 == NULL) {
                    perror("generate_export_component");
                    return -1;
                }
                new->export.class_exports = tmp1;

                new->export.class_exports[new->export.class_count].class_offset = acf->classes[u2Index1]->offset;
                new->export.class_exports[new->export.class_count].static_field_count = 0;
                new->export.class_exports[new->export.class_count].static_method_count = 0;
                new->export.class_exports[new->export.class_count].static_field_offsets = NULL;
                new->export.class_exports[new->export.class_count].static_method_offsets = NULL;

                for(; u2Index2 < acf->classes[u2Index1]->fields_count; ++u2Index2)
                    if(((acf->classes[u2Index1]->fields[u2Index2]->flags & (FIELD_STATIC|FIELD_FINAL)) == FIELD_STATIC) && (acf->classes[u2Index1]->fields[u2Index2]->flags & (FIELD_PUBLIC|FIELD_PROTECTED))) {
                        u2* tmp2 = (u2*)realloc(new->export.class_exports[new->export.class_count].static_field_offsets, sizeof(u2) * (new->export.class_exports[new->export.class_count].static_field_count + 1));
                        if(tmp2 == NULL) {
                            perror("generate_export_component");
                            return -1;
                        }
                        new->export.class_exports[new->export.class_count].static_field_offsets = tmp2;

                        new->export.class_exports[new->export.class_count].static_field_offsets[new->export.class_exports[new->export.class_count].static_field_count] = acf->classes[u2Index1]->fields[u2Index2]->offset;
                        ++new->export.class_exports[new->export.class_count].static_field_count;
                    }

                for(u2Index2 = 0; u2Index2 < acf->classes[u2Index1]->methods_count; ++u2Index2)
                    if((acf->classes[u2Index1]->methods[u2Index2]->flags & (METHOD_STATIC|METHOD_INIT)) && (acf->classes[u2Index1]->methods[u2Index2]->flags & (METHOD_PUBLIC|METHOD_PROTECTED))) {
                        u2* tmp2 = (u2*)realloc(new->export.class_exports[new->export.class_count].static_method_offsets, sizeof(u2) * (new->export.class_exports[new->export.class_count].static_method_count + 1));
                        if(tmp2 == NULL) {
                            perror("generate_export_component");
                            return -1;
                        }
                        new->export.class_exports[new->export.class_count].static_method_offsets = tmp2;

                        new->export.class_exports[new->export.class_count].static_method_offsets[new->export.class_exports[new->export.class_count].static_method_count] = acf->classes[u2Index1]->methods[u2Index2]->offset;
                        ++new->export.class_exports[new->export.class_count].static_method_count;
                    }

                new->export.size += 4 + (new->export.class_exports[new->export.class_count].static_field_count * 2) + (new->export.class_exports[new->export.class_count].static_method_count * 2);
                ++new->export.class_count;
            }
    }

    if(new->export.size == 1) {
        new->export.tag = 0;
        new->export.size = 0;
    }

    return 0;

}


/**
 * Update the count field for each type descriptor entry of the signature pool.
 */
static void count_type_descriptor_references(analyzed_cap_file* acf) {

    u2 u2Index1 = 0;

    for(; u2Index1 < acf->constant_pool_count; ++u2Index1)
        if(!(acf->constant_pool[u2Index1]->flags & CONSTANT_POOL_CLASSREF) && acf->constant_pool[u2Index1]->count != 0)
            ++acf->constant_pool[u2Index1]->type->count;

    for(u2Index1 = 0; u2Index1 < acf->classes_count; ++u2Index1) {
        u2 u2Index2 = 0;

        for(; u2Index2 < acf->classes[u2Index1]->fields_count; ++u2Index2)
            if(acf->classes[u2Index1]->fields[u2Index2]->type->types->type & (TYPE_DESCRIPTOR_REF|TYPE_DESCRIPTOR_ARRAY))
                ++acf->classes[u2Index1]->fields[u2Index2]->type->count;

        for(u2Index2 = 0; u2Index2 < acf->classes[u2Index1]->methods_count; ++u2Index2)
            ++acf->classes[u2Index1]->methods[u2Index2]->signature->count;
    }

    for(u2Index1 = 0; u2Index1 < acf->interfaces_count; ++u2Index1) {
        u2 u2Index2 = 0;

        for(; u2Index2 < acf->interfaces[u2Index1]->methods_count; ++u2Index2)
            ++acf->interfaces[u2Index1]->methods[u2Index2]->signature->count;
    }

}


/**
 * Compute nibbles from the human readable type descriptor of the analyzed CAP file.
 */
static int compute_nibbles(type_descriptor_info* type, u1** nibbles, u1* nibbles_count) {

    u1 u1Index = 0;
    u1 crt_nibble = 0;

    for(; u1Index < type->types_count; ++u1Index) {
        if((crt_nibble % 2) == 0) {
            if(type->types[u1Index].type & TYPE_DESCRIPTOR_REF) {
                u1* tmp = (u1*)realloc(*nibbles, (crt_nibble / 2) + 3);
                if(tmp == NULL) {
                    perror("compute_nibbles");
                    return -1;
                }
                *nibbles = tmp;

                if(type->types[u1Index].type & TYPE_DESCRIPTOR_ARRAY)
                    (*nibbles)[crt_nibble / 2] = 0xE << 4;
                else
                    (*nibbles)[crt_nibble / 2] = 0x6 << 4;

                if(type->types[u1Index].ref->flags & CONSTANT_POOL_IS_EXTERNAL) {
                    (*nibbles)[crt_nibble / 2] |= ((0x80 | type->types[u1Index].ref->external_package->my_index) >> 4);
                    (*nibbles)[(crt_nibble / 2) + 1] = (type->types[u1Index].ref->external_package->my_index << 4) | (type->types[u1Index].ref->external_class_token >> 4);
                    (*nibbles)[(crt_nibble / 2) + 2] = type->types[u1Index].ref->external_class_token << 4;
                } else {
                    if(type->types[u1Index].ref->internal_class) {
                        (*nibbles)[crt_nibble / 2] |= ((0x7FFF & type->types[u1Index].ref->internal_class->offset) >> 12);
                        (*nibbles)[(crt_nibble / 2) + 1] = (0x0FF0 & type->types[u1Index].ref->internal_class->offset) >> 4;
                        (*nibbles)[(crt_nibble / 2) + 2] = type->types[u1Index].ref->internal_class->offset << 4;
                    } else {
                        (*nibbles)[crt_nibble / 2] |= ((0x7FFF & type->types[u1Index].ref->internal_interface->offset) >> 12);
                        (*nibbles)[(crt_nibble / 2) + 1] = (0x0FF0 & type->types[u1Index].ref->internal_interface->offset) >> 4;
                        (*nibbles)[(crt_nibble / 2) + 2] = type->types[u1Index].ref->internal_interface->offset << 4;
                    }
                }

                crt_nibble += 5;
            } else {
                u1* tmp = (u1*)realloc(*nibbles, (crt_nibble / 2) + 1);
                if(tmp == NULL) {
                    perror("compute_nibbles");
                    return -1;
                }
                *nibbles = tmp;

                if(type->types[u1Index].type & TYPE_DESCRIPTOR_VOID) {
                    (*nibbles)[crt_nibble / 2] = 0x1 << 4;
                } else if(type->types[u1Index].type & TYPE_DESCRIPTOR_ARRAY) {
                    if(type->types[u1Index].type & TYPE_DESCRIPTOR_BOOLEAN)
                        (*nibbles)[crt_nibble / 2] = 0xA << 4;
                    else if(type->types[u1Index].type & TYPE_DESCRIPTOR_BYTE)
                        (*nibbles)[crt_nibble / 2] = 0xB << 4;
                    else if(type->types[u1Index].type & TYPE_DESCRIPTOR_SHORT)
                        (*nibbles)[crt_nibble / 2] = 0xC << 4;
                    else if(type->types[u1Index].type & TYPE_DESCRIPTOR_INT)
                        (*nibbles)[crt_nibble / 2] = 0xD << 4;
                } else {
                    if(type->types[u1Index].type & TYPE_DESCRIPTOR_BOOLEAN)
                        (*nibbles)[crt_nibble / 2] = 0x2 << 4;
                    else if(type->types[u1Index].type & TYPE_DESCRIPTOR_BYTE)
                        (*nibbles)[crt_nibble / 2] = 0x3 << 4;
                    else if(type->types[u1Index].type & TYPE_DESCRIPTOR_SHORT)
                        (*nibbles)[crt_nibble / 2] = 0x4 << 4;
                    else if(type->types[u1Index].type & TYPE_DESCRIPTOR_INT)
                        (*nibbles)[crt_nibble / 2] = 0x5 << 4;
                }

                ++crt_nibble;
            }
        } else {
            if(type->types[u1Index].type & TYPE_DESCRIPTOR_REF) {
                u1* tmp = (u1*)realloc(*nibbles, (crt_nibble / 2) + 3);
                if(tmp == NULL) {
                    perror("compute_nibbles");
                    return -1;
                }
                *nibbles = tmp;

                if(type->types[u1Index].type & TYPE_DESCRIPTOR_ARRAY)
                    (*nibbles)[crt_nibble / 2] |= 0xE;
                else
                    (*nibbles)[crt_nibble / 2] |= 0x6;

                if(type->types[u1Index].ref->flags & CONSTANT_POOL_IS_EXTERNAL) {
                    (*nibbles)[(crt_nibble / 2) + 1] = 0x80 | type->types[u1Index].ref->external_package->my_index;
                    (*nibbles)[(crt_nibble / 2) + 2] = type->types[u1Index].ref->external_class_token;
                } else {
                    if(type->types[u1Index].ref->internal_class) {
                        (*nibbles)[(crt_nibble / 2) + 1] = (0x7F00 & type->types[u1Index].ref->internal_class->offset) >> 8;
                        (*nibbles)[(crt_nibble / 2) + 2] = 0x00FF & type->types[u1Index].ref->internal_class->offset;
                    } else {
                        (*nibbles)[(crt_nibble / 2) + 1] = (0x7F00 & type->types[u1Index].ref->internal_interface->offset) >> 8;
                        (*nibbles)[(crt_nibble / 2) + 2] = 0x00FF & type->types[u1Index].ref->internal_interface->offset;
                    }
                }

                crt_nibble += 5;
            } else {
                if(type->types[u1Index].type & TYPE_DESCRIPTOR_VOID) {
                    (*nibbles)[crt_nibble / 2] |= 0x1;
                } else if(type->types[u1Index].type & TYPE_DESCRIPTOR_ARRAY) {
                    if(type->types[u1Index].type & TYPE_DESCRIPTOR_BOOLEAN)
                        (*nibbles)[crt_nibble / 2] |= 0xA;
                    else if(type->types[u1Index].type & TYPE_DESCRIPTOR_BYTE)
                        (*nibbles)[crt_nibble / 2] |= 0xB;
                    else if(type->types[u1Index].type & TYPE_DESCRIPTOR_SHORT)
                        (*nibbles)[crt_nibble / 2] |= 0xC;
                    else if(type->types[u1Index].type & TYPE_DESCRIPTOR_INT)
                        (*nibbles)[crt_nibble / 2] |= 0xD;
                } else {
                    if(type->types[u1Index].type & TYPE_DESCRIPTOR_BOOLEAN)
                        (*nibbles)[crt_nibble / 2] |= 0x2;
                    else if(type->types[u1Index].type & TYPE_DESCRIPTOR_BYTE)
                        (*nibbles)[crt_nibble / 2] |= 0x3;
                    else if(type->types[u1Index].type & TYPE_DESCRIPTOR_SHORT)
                        (*nibbles)[crt_nibble / 2] |= 0x4;
                    else if(type->types[u1Index].type & TYPE_DESCRIPTOR_INT)
                        (*nibbles)[crt_nibble / 2] |= 0x5;
                }

                ++crt_nibble;
            }
        }
    }

    if((crt_nibble % 2) != 0)
        (*nibbles)[crt_nibble / 2] &= 0xF0;

    *nibbles_count = crt_nibble;

    return 0;

}


/**
 * Generate the descriptor component in the given cap_file structure.
 */
static int generate_descriptor_component(analyzed_cap_file* acf, cap_file* new) {

    u2 u2Index1 = 0;
    u2 crt_offset = 0;

    new->descriptor.tag = COMPONENT_DESCRIPTOR;
    new->descriptor.size = 1;

    new->descriptor.types.constant_pool_count = new->constant_pool.count;
    new->descriptor.types.constant_pool_types = (u2*)malloc(sizeof(u2) * new->descriptor.types.constant_pool_count);
    if(new->descriptor.types.constant_pool_types == NULL) {
        perror("generate_descriptor_component");
        return -1;
    }

    new->descriptor.types.type_desc_count = 0;
    new->descriptor.types.type_desc = NULL;

    crt_offset = 2 + (2 * new->descriptor.types.constant_pool_count);

    for(; u2Index1 < acf->signature_pool_count; ++u2Index1)
        if(acf->signature_pool[u2Index1]->count != 0) {
            cf_type_descriptor* tmp = (cf_type_descriptor*)realloc(new->descriptor.types.type_desc, sizeof(cf_type_descriptor) * (new->descriptor.types.type_desc_count + 1));
            if(tmp == NULL) {
                perror("generate_descriptor_component");
                return -1;
            }
            new->descriptor.types.type_desc = tmp;

            new->descriptor.types.type_desc[new->descriptor.types.type_desc_count].offset = crt_offset;
            acf->signature_pool[u2Index1]->offset = crt_offset;
            new->descriptor.types.type_desc[new->descriptor.types.type_desc_count].nibble_count = 0;
            new->descriptor.types.type_desc[new->descriptor.types.type_desc_count].type = NULL;

            if(compute_nibbles(acf->signature_pool[u2Index1], &(new->descriptor.types.type_desc[new->descriptor.types.type_desc_count].type), &(new->descriptor.types.type_desc[new->descriptor.types.type_desc_count].nibble_count)) == -1)
                return -1;

            crt_offset += 1 + ((new->descriptor.types.type_desc[new->descriptor.types.type_desc_count].nibble_count + 1) / 2);
            ++new->descriptor.types.type_desc_count;
        }

    for(u2Index1 = 0; u2Index1 < acf->constant_pool_count; ++u2Index1)
        if(acf->constant_pool[u2Index1]->flags & CONSTANT_POOL_CLASSREF) {
            if(acf->constant_pool[u2Index1]->count != 0)
                new->descriptor.types.constant_pool_types[acf->constant_pool[u2Index1]->my_index] = 0xFFFF;
        } else {
            if(acf->constant_pool[u2Index1]->count != 0)
                new->descriptor.types.constant_pool_types[acf->constant_pool[u2Index1]->my_index] = acf->constant_pool[u2Index1]->type->offset;
        }

    new->descriptor.size += crt_offset;

    new->descriptor.class_count = acf->classes_count + acf->interfaces_count;
    new->descriptor.classes = (cf_class_descriptor_info*)malloc(sizeof(cf_class_descriptor_info) * new->descriptor.class_count);
    if(new->descriptor.classes == NULL) {
        perror("generate_descriptor_component");
        return -1;
    }

    for(u2Index1 = 0; u2Index1 < acf->interfaces_count; ++u2Index1) {
        u2 u2Index2 = 0;

        new->descriptor.classes[u2Index1].token = acf->interfaces[u2Index1]->token;

        new->descriptor.classes[u2Index1].access_flags = DESCRIPTOR_ACC_INTERFACE | DESCRIPTOR_ACC_ABSTRACT1;
        if(acf->interfaces[u2Index1]->flags & INTERFACE_PUBLIC)
            new->descriptor.classes[u2Index1].access_flags |= DESCRIPTOR_ACC_PUBLIC;

        new->descriptor.classes[u2Index1].this_class_ref.isExternal = 0;
        new->descriptor.classes[u2Index1].this_class_ref.ref.internal_class_ref = acf->interfaces[u2Index1]->offset;

        new->descriptor.classes[u2Index1].interface_count = 0;
        new->descriptor.classes[u2Index1].field_count = 0;
        new->descriptor.classes[u2Index1].method_count = acf->interfaces[u2Index1]->methods_count;

        new->descriptor.classes[u2Index1].interfaces = NULL;
        new->descriptor.classes[u2Index1].fields = NULL;
        new->descriptor.classes[u2Index1].methods = (cf_method_descriptor_info*)malloc(sizeof(cf_method_descriptor_info) * new->descriptor.classes[u2Index1].method_count);
        if(new->descriptor.classes[u2Index1].methods == NULL) {
            perror("generate_descriptor_component");
            return -1;
        }

        for(; u2Index2 < acf->interfaces[u2Index1]->methods_count; ++u2Index2) {
            new->descriptor.classes[u2Index1].methods[u2Index2].token = acf->interfaces[u2Index1]->methods[u2Index2]->token;
            new->descriptor.classes[u2Index1].methods[u2Index2].access_flags = DESCRIPTOR_ACC_PUBLIC | DESCRIPTOR_ACC_ABSTRACT2;
            new->descriptor.classes[u2Index1].methods[u2Index2].method_offset = 0;
            new->descriptor.classes[u2Index1].methods[u2Index2].type_offset = acf->interfaces[u2Index1]->methods[u2Index2]->signature->offset;
            new->descriptor.classes[u2Index1].methods[u2Index2].bytecode_count = 0;
            new->descriptor.classes[u2Index1].methods[u2Index2].exception_handler_count = 0;
            new->descriptor.classes[u2Index1].methods[u2Index2].exception_handler_index = 0;
        }

        new->descriptor.size += 9 + (12 * acf->interfaces[u2Index1]->methods_count);
    }

    for(u2Index1 = 0; u2Index1 < acf->classes_count; ++u2Index1) {
        u1 u1Index = 0;
        u2 u2Index2 = 0;

        new->descriptor.classes[acf->interfaces_count + u2Index1].token = acf->classes[u2Index1]->token;

        new->descriptor.classes[acf->interfaces_count + u2Index1].access_flags = 0;
        if(acf->classes[u2Index1]->flags & CLASS_PUBLIC)
            new->descriptor.classes[acf->interfaces_count + u2Index1].access_flags = DESCRIPTOR_ACC_PUBLIC;
        if(acf->classes[u2Index1]->flags & CLASS_FINAL)
            new->descriptor.classes[acf->interfaces_count + u2Index1].access_flags |= DESCRIPTOR_ACC_FINAL;
        if(acf->classes[u2Index1]->flags & CLASS_ABSTRACT)
            new->descriptor.classes[acf->interfaces_count + u2Index1].access_flags |= DESCRIPTOR_ACC_ABSTRACT1;

        new->descriptor.classes[acf->interfaces_count + u2Index1].this_class_ref.isExternal = 0;
        new->descriptor.classes[acf->interfaces_count + u2Index1].this_class_ref.ref.internal_class_ref = acf->classes[u2Index1]->offset;

        new->descriptor.classes[acf->interfaces_count + u2Index1].interface_count = acf->classes[u2Index1]->interfaces_count;
        new->descriptor.classes[acf->interfaces_count + u2Index1].field_count = acf->classes[u2Index1]->fields_count;
        new->descriptor.classes[acf->interfaces_count + u2Index1].method_count = acf->classes[u2Index1]->methods_count;

        new->descriptor.classes[acf->interfaces_count + u2Index1].interfaces = (cf_class_ref_info*)malloc(sizeof(cf_class_ref_info) * new->descriptor.classes[acf->interfaces_count + u2Index1].interface_count);
        if(new->descriptor.classes[acf->interfaces_count + u2Index1].interfaces == NULL) {
            perror("generate_descriptor_component");
            return -1;
        }

        for(; u1Index < acf->classes[u2Index1]->interfaces_count; ++u1Index)
            if(acf->classes[u2Index1]->interfaces[u1Index].ref->flags & CONSTANT_POOL_IS_EXTERNAL) {
                new->descriptor.classes[acf->interfaces_count + u2Index1].interfaces[u1Index].isExternal = 1;
                new->descriptor.classes[acf->interfaces_count + u2Index1].interfaces[u1Index].ref.external_class_ref.package_token = acf->classes[u2Index1]->interfaces[u1Index].ref->external_package->my_index;
                new->descriptor.classes[acf->interfaces_count + u2Index1].interfaces[u1Index].ref.external_class_ref.class_token = acf->classes[u2Index1]->interfaces[u1Index].ref->external_class_token;
            } else {
                new->descriptor.classes[acf->interfaces_count + u2Index1].interfaces[u1Index].isExternal = 0;
                new->descriptor.classes[acf->interfaces_count + u2Index1].interfaces[u1Index].ref.internal_class_ref = acf->classes[u2Index1]->interfaces[u1Index].ref->internal_interface->offset;
            }

        new->descriptor.classes[acf->interfaces_count + u2Index1].fields = (cf_field_descriptor_info*)malloc(sizeof(cf_field_descriptor_info) * new->descriptor.classes[acf->interfaces_count + u2Index1].field_count);
        if(new->descriptor.classes[acf->interfaces_count + u2Index1].fields == NULL) {
            perror("generate_descriptor_component");
            return -1;
        }

        for(; u2Index2 < acf->classes[u2Index1]->fields_count; ++u2Index2) {
            new->descriptor.classes[acf->interfaces_count + u2Index1].fields[u2Index2].token = acf->classes[u2Index1]->fields[u2Index2]->token;

            new->descriptor.classes[acf->interfaces_count + u2Index1].fields[u2Index2].access_flags = 0;
            if(acf->classes[u2Index1]->fields[u2Index2]->flags & FIELD_PUBLIC)
                new->descriptor.classes[acf->interfaces_count + u2Index1].fields[u2Index2].access_flags = DESCRIPTOR_ACC_PUBLIC;
            if(acf->classes[u2Index1]->fields[u2Index2]->flags & FIELD_PRIVATE)
                new->descriptor.classes[acf->interfaces_count + u2Index1].fields[u2Index2].access_flags = DESCRIPTOR_ACC_PRIVATE;
            if(acf->classes[u2Index1]->fields[u2Index2]->flags & FIELD_PROTECTED)
                new->descriptor.classes[acf->interfaces_count + u2Index1].fields[u2Index2].access_flags = DESCRIPTOR_ACC_PROTECTED;
            if(acf->classes[u2Index1]->fields[u2Index2]->flags & FIELD_FINAL)
                new->descriptor.classes[acf->interfaces_count + u2Index1].fields[u2Index2].access_flags |= DESCRIPTOR_ACC_FINAL;
            if(acf->classes[u2Index1]->fields[u2Index2]->flags & FIELD_STATIC) {
                new->descriptor.classes[acf->interfaces_count + u2Index1].fields[u2Index2].access_flags |= DESCRIPTOR_ACC_STATIC;
                new->descriptor.classes[acf->interfaces_count + u2Index1].fields[u2Index2].field_ref.static_field.isExternal = 0;
                new->descriptor.classes[acf->interfaces_count + u2Index1].fields[u2Index2].field_ref.static_field.ref.internal_ref.padding = 0;
                new->descriptor.classes[acf->interfaces_count + u2Index1].fields[u2Index2].field_ref.static_field.ref.internal_ref.offset = acf->classes[u2Index1]->fields[u2Index2]->offset;
            } else {
                new->descriptor.classes[acf->interfaces_count + u2Index1].fields[u2Index2].field_ref.instance_field.class_ref.isExternal = 0;
                new->descriptor.classes[acf->interfaces_count + u2Index1].fields[u2Index2].field_ref.instance_field.class_ref.ref.internal_class_ref = acf->classes[u2Index1]->offset;
                new->descriptor.classes[acf->interfaces_count + u2Index1].fields[u2Index2].field_ref.instance_field.token = acf->classes[u2Index1]->fields[u2Index2]->token;
            }

            if((acf->classes[u2Index1]->fields[u2Index2]->type->types->type & (TYPE_DESCRIPTOR_BOOLEAN|TYPE_DESCRIPTOR_ARRAY)) == TYPE_DESCRIPTOR_BOOLEAN)
                new->descriptor.classes[acf->interfaces_count + u2Index1].fields[u2Index2].type.primitive_type = 0x8002;
            else if((acf->classes[u2Index1]->fields[u2Index2]->type->types->type & (TYPE_DESCRIPTOR_BYTE|TYPE_DESCRIPTOR_ARRAY)) == TYPE_DESCRIPTOR_BYTE)
                new->descriptor.classes[acf->interfaces_count + u2Index1].fields[u2Index2].type.primitive_type = 0x8003;
            else if((acf->classes[u2Index1]->fields[u2Index2]->type->types->type & (TYPE_DESCRIPTOR_SHORT|TYPE_DESCRIPTOR_ARRAY)) == TYPE_DESCRIPTOR_SHORT)
                new->descriptor.classes[acf->interfaces_count + u2Index1].fields[u2Index2].type.primitive_type = 0x8004;
            else if((acf->classes[u2Index1]->fields[u2Index2]->type->types->type & (TYPE_DESCRIPTOR_INT|TYPE_DESCRIPTOR_ARRAY)) == TYPE_DESCRIPTOR_INT)
                new->descriptor.classes[acf->interfaces_count + u2Index1].fields[u2Index2].type.primitive_type = 0x8005;
            else
                new->descriptor.classes[acf->interfaces_count + u2Index1].fields[u2Index2].type.reference_type = acf->classes[u2Index1]->fields[u2Index2]->type->offset;
        }

        new->descriptor.classes[acf->interfaces_count + u2Index1].methods = (cf_method_descriptor_info*)malloc(sizeof(cf_method_descriptor_info) * new->descriptor.classes[acf->interfaces_count + u2Index1].method_count);
        if(new->descriptor.classes[acf->interfaces_count + u2Index1].methods == NULL) {
            perror("generate_descriptor_component");
            return -1;
        }

        for(u2Index2 = 0; u2Index2 < acf->classes[u2Index1]->methods_count; ++u2Index2) {
            new->descriptor.classes[acf->interfaces_count + u2Index1].methods[u2Index2].token = acf->classes[u2Index1]->methods[u2Index2]->token;

            new->descriptor.classes[acf->interfaces_count + u2Index1].methods[u2Index2].access_flags = 0;
            if(acf->classes[u2Index1]->methods[u2Index2]->flags & METHOD_PUBLIC)
                new->descriptor.classes[acf->interfaces_count + u2Index1].methods[u2Index2].access_flags = DESCRIPTOR_ACC_PUBLIC;
            if(acf->classes[u2Index1]->methods[u2Index2]->flags & METHOD_PRIVATE)
                new->descriptor.classes[acf->interfaces_count + u2Index1].methods[u2Index2].access_flags = DESCRIPTOR_ACC_PRIVATE;
            if(acf->classes[u2Index1]->methods[u2Index2]->flags & METHOD_PROTECTED)
                new->descriptor.classes[acf->interfaces_count + u2Index1].methods[u2Index2].access_flags = DESCRIPTOR_ACC_PROTECTED;
            if(acf->classes[u2Index1]->methods[u2Index2]->flags & METHOD_STATIC)
                new->descriptor.classes[acf->interfaces_count + u2Index1].methods[u2Index2].access_flags |= DESCRIPTOR_ACC_STATIC;
            if(acf->classes[u2Index1]->methods[u2Index2]->flags & METHOD_FINAL)
                new->descriptor.classes[acf->interfaces_count + u2Index1].methods[u2Index2].access_flags |= DESCRIPTOR_ACC_FINAL;
            if(acf->classes[u2Index1]->methods[u2Index2]->flags & METHOD_ABSTRACT)
                new->descriptor.classes[acf->interfaces_count + u2Index1].methods[u2Index2].access_flags |= DESCRIPTOR_ACC_ABSTRACT2;
            if(acf->classes[u2Index1]->methods[u2Index2]->flags & METHOD_INIT)
                new->descriptor.classes[acf->interfaces_count + u2Index1].methods[u2Index2].access_flags |= DESCRIPTOR_ACC_INIT;

            new->descriptor.classes[acf->interfaces_count + u2Index1].methods[u2Index2].method_offset = acf->classes[u2Index1]->methods[u2Index2]->offset;
            new->descriptor.classes[acf->interfaces_count + u2Index1].methods[u2Index2].type_offset = acf->classes[u2Index1]->methods[u2Index2]->signature->offset;
            new->descriptor.classes[acf->interfaces_count + u2Index1].methods[u2Index2].bytecode_count = acf->classes[u2Index1]->methods[u2Index2]->bytecodes_size;

            new->descriptor.classes[acf->interfaces_count + u2Index1].methods[u2Index2].exception_handler_count = acf->classes[u2Index1]->methods[u2Index2]->exception_handlers_count;
            if(acf->classes[u2Index1]->methods[u2Index2]->exception_handlers_count != 0) {
                u1 u1Index = 1;
                new->descriptor.classes[acf->interfaces_count + u2Index1].methods[u2Index2].exception_handler_index = acf->classes[u2Index1]->methods[u2Index2]->exception_handlers[0]->my_index;
            for(; u1Index < acf->classes[u2Index1]->methods[u2Index2]->exception_handlers_count; ++u1Index)
                if(acf->classes[u2Index1]->methods[u2Index2]->exception_handlers[u1Index]->my_index < new->descriptor.classes[acf->interfaces_count + u2Index1].methods[u2Index2].exception_handler_index)
                    new->descriptor.classes[acf->interfaces_count + u2Index1].methods[u2Index2].exception_handler_index = acf->classes[u2Index1]->methods[u2Index2]->exception_handlers[u1Index]->my_index;
            } else {
                new->descriptor.classes[acf->interfaces_count + u2Index1].methods[u2Index2].exception_handler_index = 0;
            }
        }

        new->descriptor.size += 9 + (2 * new->descriptor.classes[acf->interfaces_count + u2Index1].interface_count) + (7 * new->descriptor.classes[acf->interfaces_count + u2Index1].field_count) + (12 * new->descriptor.classes[acf->interfaces_count + u2Index1].method_count);

    }

    return 0;

}


/**
 * Convert an AID in byte array format to a string.
 */
static void convert_aid(char* buffer, u1* aid, u1 aid_length) {

    u1 u1Index = 0;
    char one_byte[6];

    buffer[0] = '\0';
    for(; u1Index < (aid_length - 1); ++u1Index) {
        sprintf(one_byte, "0x%.2x:", aid[u1Index]);
        strcat(buffer, one_byte);
    }
    sprintf(one_byte, "0x%.2x", aid[u1Index]);
    strcat(buffer, one_byte);

}


/**
 * Add a manifest entry name and associated value.
 */
static int add_manifest_entry(char** manifest, int* crt_length, const char* entry_name, const char* entry_value) {

    int name_length = strlen(entry_name);
    int value_length = strlen(entry_value);
    int total_length = name_length + value_length;

    if(total_length > 70)  /* If the entry is on two lines */
        total_length += 3;


    char* tmp = (char*)realloc(*manifest, *crt_length + total_length + 2 + 1); /*\r\n\0*/
    if(tmp == NULL) {
        perror("add_manifest_entry");
        return -1;
    }
    *manifest = tmp;

    strcpy(*manifest + *crt_length, entry_name);
    *crt_length += name_length;

    if(total_length > 73) {
        strncpy(*manifest + *crt_length, entry_value, 70 - name_length);
        *crt_length += (70 - name_length);

        (*manifest)[*crt_length] = '\r';
        ++(*crt_length);
        (*manifest)[*crt_length] = '\n';
        ++(*crt_length);
        (*manifest)[*crt_length] = ' ';
        ++(*crt_length);

        strcpy(*manifest + *crt_length, entry_value + (70 - name_length));
        *crt_length += (value_length - (70 - name_length));
    } else {
        strcpy(*manifest + *crt_length, entry_value);
        *crt_length += value_length;
    }

    (*manifest)[*crt_length] = '\r';
    ++(*crt_length);
    (*manifest)[*crt_length] = '\n';
    ++(*crt_length);
    (*manifest)[*crt_length] = '\0';

    return 0;

}


/**
 * Generated the manifest in the given cap_file structure.
 */
static int generate_manifest(analyzed_cap_file* acf, cap_file* new) {

    int crt_length = 0;
    char name_buffer[41];
    char value_buffer[80];
    u1 applets_count = 1;
    u1 packages_count = 1;
    u2 u2Index = 0;
    u1 u1Index = 0;

    new->manifest = (char*)malloc(1);
    if(new->manifest == NULL) {
        perror("generate_manifest");
        return -1;
    }

    new->manifest[0] = '\0';

    if(acf->manifest.version && (add_manifest_entry(&(new->manifest), &crt_length, "Manifest-Version: ", acf->manifest.version) == -1))
        return -1;

    if(acf->manifest.created_by && (add_manifest_entry(&(new->manifest), &crt_length, "Created-By: ", acf->manifest.created_by) == -1))
        return -1;

    if(acf->manifest.name && (add_manifest_entry(&(new->manifest), &crt_length, "Name: ", acf->manifest.name) == -1))
        return -1;

    if(acf->manifest.converter_provider && (add_manifest_entry(&(new->manifest), &crt_length, "Java-Card-Converter-Provider: ", acf->manifest.converter_provider) == -1))
        return -1;

    if(acf->manifest.converter_version && (add_manifest_entry(&(new->manifest), &crt_length, "Java-Card-Converter-Version: ", acf->manifest.converter_version) == -1))
        return -1;

    sprintf(value_buffer, "%u.%u", acf->info.javacard_major_version, acf->info.javacard_minor_version);

    if(add_manifest_entry(&(new->manifest), &crt_length, "Java-Card-CAP-File-Version: ", value_buffer) == -1)
        return -1;

    if(acf->manifest.creation_time && (add_manifest_entry(&(new->manifest), &crt_length, "Java-Card-CAP-Creation-Time: ", acf->manifest.creation_time) == -1))
        return -1;

    if(new->header.flags & HEADER_ACC_INT) {
        if(add_manifest_entry(&(new->manifest), &crt_length, "Java-Card-Integer-Support-Required: ", "TRUE") == -1)
            return -1;
    } else {
        if(add_manifest_entry(&(new->manifest), &crt_length, "Java-Card-Integer-Support-Required: ", "FALSE") == -1)
            return -1;
    }

    if(acf->manifest.package_name && (add_manifest_entry(&(new->manifest), &crt_length, "Java-Card-Package-Name: ", acf->manifest.package_name) == -1))
        return -1;

    convert_aid(value_buffer, acf->info.package_aid, acf->info.package_aid_length);

    if(add_manifest_entry(&(new->manifest), &crt_length, "Java-Card-Package-AID: ", value_buffer) == -1)
        return -1;

    sprintf(value_buffer, "%u.%u", acf->info.package_major_version, acf->info.package_minor_version);

    if(add_manifest_entry(&(new->manifest), &crt_length, "Java-Card-Package-Version: ", value_buffer) == -1)
        return -1;

    for(; u2Index < acf->classes_count; ++u2Index)
        if(acf->classes[u2Index]->flags & CLASS_APPLET) {
            sprintf(name_buffer, "Java-Card-Applet-%u-Name: ", applets_count);

            if(add_manifest_entry(&(new->manifest), &crt_length, name_buffer, acf->classes[u2Index]->name) == -1)
                return -1;

            sprintf(name_buffer, "Java-Card-Applet-%u-AID: ", applets_count);

            convert_aid(value_buffer, acf->classes[u2Index]->aid, acf->classes[u2Index]->aid_length);

            if(add_manifest_entry(&(new->manifest), &crt_length, name_buffer, value_buffer) == -1)
                return -1;

            ++applets_count;
        }

    for(u1Index = 0; u1Index < acf->imported_packages_count; ++u1Index)
        if(acf->imported_packages[u1Index]->count != 0) {
            sprintf(name_buffer, "Java-Card-Imported-Package-%u-AID: ", packages_count);

            convert_aid(value_buffer, acf->imported_packages[u1Index]->aid, acf->imported_packages[u1Index]->aid_length);

            if(add_manifest_entry(&(new->manifest), &crt_length, name_buffer, value_buffer) == -1)
                return -1;

            sprintf(name_buffer, "Java-Card-Imported-Package-%u-Version: ", packages_count);
            sprintf(value_buffer, "%u.%u", acf->imported_packages[u1Index]->major_version, acf->imported_packages[u1Index]->minor_version);

            if(add_manifest_entry(&(new->manifest), &crt_length, name_buffer, value_buffer) == -1)
                return -1;

            ++packages_count;
        }

    return 0;

}


/**
 * Generate from the analyzed CAP file a cap_file structure and return it.
 */
cap_file* generate_cap_file(analyzed_cap_file* acf) {

    cap_file* new = (cap_file*)malloc(sizeof(cap_file));
    if(new == NULL) {
        perror("generate_cap_file");
        return NULL;
    }

    new->path = acf->info.path;
    new->path = (char*)malloc(strlen(acf->info.path) + 1);
    if(new->path == NULL) {
        perror("generate_cap_file");
        return NULL;
    }
    strcpy(new->path, acf->info.path);

    if(generate_header_component(acf, new) == -1)
        return NULL;

    /* We compute the count to know which constant pool entries will remain. */
    count_constant_pool_references(acf);
    /* From the remaining constant pool entries, we determine the remaining imported packages. */
    count_imported_package(acf);
    if(generate_import_component(acf, new) == -1)
        return NULL;

    /* We generate the constant pool entry indexes used by bytecodes. */
    update_constant_pool_entry_index(acf);

    /* If constant pool entry indexes are smaller or bigger in width than before,
       we compact or expend bytecodes. */
    if(compact_bytecodes(acf) == -1)
        return NULL;

    /* We compute offsets */
    compute_bytecodes_offsets(acf);
    /* TODO A second pass for compacting or expending bytecode should be done since offset might be smaller or bigger than before (i.e. ifeq should become a ifeq_w). */
    compute_bytecodes_sizes(acf);
    sort_exception_handlers(acf);

    if(generate_method_component(acf, new) == -1)
        return NULL;

    /* Compute token for everything. */
    compute_tokens(acf);

    if(generate_class_component(acf, new) == -1)
        return NULL;

    if(generate_static_field_component(acf, new) == -1)
        return NULL;

    if(generate_reference_location_component(acf, new) == -1)
        return NULL;

    if(generate_constant_pool_component(acf, new) == -1)
        return NULL;

    if(generate_export_component(acf, new) == -1)
        return NULL;

    if(generate_applet_component(acf, new) == -1)
        return NULL;

    /* From the remaining constant pool entries and other descriptor dependency,
       we sort out the remaining type descriptors. */
    count_type_descriptor_references(acf);

    if(generate_descriptor_component(acf, new) == -1)
        return NULL;

    /* Since we have all the component sizes and such, we can generate the directory component. */
    if(generate_directory_component(acf, new) == -1)
        return NULL;

    if(generate_manifest(acf, new) == -1)
        return NULL;

    /* We don't support the debug component. */
    new->debug.tag = 0;
    new->debug.size = 0;

    /* TODO custom components */

    return new;

}
