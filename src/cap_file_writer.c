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
 * \file cap_file_writer.c
 * \brief Write a CAP file from its straightforward representation.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <zip.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "cap_file.h"


static void U2ToBigIndian(char* buffer, u2 value) {

    buffer[0] = value >> 8;
    buffer[1] = value & 0xFF;

}

static void U4ToBigIndian(char* buffer, u4 value) {

    buffer[0] = value >> 24;
    buffer[1] = (value >> 16) & 0xFF;
    buffer[2] = (value >> 8) & 0xFF;
    buffer[3] = value & 0xFF;

}


static int addToZip(struct zip* z, const char* name, char* buffer, zip_uint64_t len) {

    zip_int64_t index = -1;
    struct zip_source* source = zip_source_buffer(z, buffer, len, 0);
    if(source == NULL) {
        fprintf(stderr, "addToZip: %s | %s\n", name, zip_strerror(z));
        return -1;
    }

    index = zip_add(z, name, source);

    if(index == -1) {
        int errorCode = 0;

        zip_error_get(z, &errorCode, NULL);
        if(errorCode == ZIP_ER_EXISTS)
            if((index = zip_name_locate(z, name, 0) != -1))
                if(zip_replace(z, index, source) != -1)
                    return 0;

        fprintf(stderr, "addToZip: %s | %s\n", name, zip_strerror(z));
        return -1;
    }

    return 0;

}


static int writeManifest(struct zip* z, cap_file* cf) {

    return addToZip(z, "META-INF/MANIFEST.MF", cf->manifest, strlen(cf->manifest));

}


static int writeHeader(struct zip* z, cap_file* cf) {

    char name[1024];

    unsigned int position = 0;
    char* buffer = (char*)malloc(sizeof(char) * (cf->header.size + 3u));
    if(buffer == NULL) {
        perror("writeHeader");
        return -1;
    }

    buffer[position++] = cf->header.tag;
    U2ToBigIndian(buffer + position, cf->header.size);
    position += 2;
    U4ToBigIndian(buffer + position, cf->header.magic);
    position += 4;
    buffer[position++] = cf->header.minor_version;
    buffer[position++] = cf->header.major_version;
    buffer[position++] = cf->header.flags;
    buffer[position++] = cf->header.package.minor_version;
    buffer[position++] = cf->header.package.major_version;
    buffer[position++] = cf->header.package.AID_length;

    memcpy(buffer + position, cf->header.package.AID, sizeof(u1) * cf->header.package.AID_length);
    position += cf->header.package.AID_length;

    if((cf->header.major_version == 2) && (cf->header.minor_version > 1)) {
        buffer[position++] = cf->header.package_name.name_length;
        memcpy(buffer + position, cf->header.package_name.name, sizeof(u1) * cf->header.package_name.name_length);
        position += cf->header.package_name.name_length;
    }

    snprintf(name, 1024, "%sHeader.cap", cf->path);

    return addToZip(z, name, buffer, cf->header.size + 3u);

}


static int writeDirectory(struct zip* z, cap_file* cf) {

    char name[1024];
    u1 u1Index = 0;

    unsigned int position = 0;
    char* buffer = (char*)malloc(sizeof(char) * (cf->directory.size + 3u));
    if(buffer == NULL) {
        perror("writeDirectory");
        return -1;
    }

    buffer[position++] = cf->directory.tag;
    U2ToBigIndian(buffer + position, cf->directory.size);
    position += 2;

    for(; u1Index < 11; ++u1Index) {
        U2ToBigIndian(buffer + position, cf->directory.component_sizes[u1Index]);
        position += 2;
    }

    if((cf->header.major_version == 2) && (cf->header.minor_version > 1)) {
        U2ToBigIndian(buffer + position, cf->directory.component_sizes[11]);
        position += 2;
    }

    U2ToBigIndian(buffer + position, cf->directory.static_field_size.image_size);
    position += 2;
    U2ToBigIndian(buffer + position, cf->directory.static_field_size.array_init_count);
    position += 2;
    U2ToBigIndian(buffer + position, cf->directory.static_field_size.array_init_size);
    position += 2;

    buffer[position++] = cf->directory.import_count;
    buffer[position++] = cf->directory.applet_count;
    buffer[position++] = cf->directory.custom_count;

    for(u1Index = 0; u1Index < cf->directory.custom_count; ++u1Index) {
        buffer[position++] = cf->directory.custom_components[u1Index].component_tag;
        U2ToBigIndian(buffer + position, cf->directory.custom_components[u1Index].size);
        position += 2;
        buffer[position++] = cf->directory.custom_components[u1Index].AID_length;
        memcpy(buffer + position, cf->directory.custom_components[u1Index].AID, sizeof(u1) * cf->directory.custom_components[u1Index].AID_length);
        position += cf->directory.custom_components[u1Index].AID_length;
    }

    snprintf(name, 1024, "%sDirectory.cap", cf->path);
    return addToZip(z, name, buffer, cf->directory.size + 3u);

}


int writeApplet(struct zip* z, cap_file* cf) {

    char name[1024];
    u1 u1Index = 0;

    unsigned int position = 0;
    char* buffer = (char*)malloc(sizeof(char) * (cf->applet.size + 3u));
    if(buffer == NULL) {
        perror("writeApplet");
        return -1;
    }

    buffer[position++] = cf->applet.tag;
    U2ToBigIndian(buffer + position, cf->applet.size);
    position += 2;
    buffer[position++] = cf->applet.count;

    for(; u1Index < cf->applet.count; ++u1Index) {
        buffer[position++] = cf->applet.applets[u1Index].AID_length;
        memcpy(buffer + position, cf->applet.applets[u1Index].AID, sizeof(u1) * cf->applet.applets[u1Index].AID_length);
        position += cf->applet.applets[u1Index].AID_length;
        U2ToBigIndian(buffer + position, cf->applet.applets[u1Index].install_method_offset);
        position += 2;
    }

    snprintf(name, 1024, "%sApplet.cap", cf->path);
    return addToZip(z, name, buffer, cf->applet.size + 3u);

}


int writeImport(struct zip* z, cap_file* cf) {

    char name[1024];
    u1 u1Index = 0;

    unsigned int position = 0;
    char* buffer = (char*)malloc(sizeof(char) * (cf->import.size + 3u));
    if(buffer == NULL) {
        perror("writeImport");
        return -1;
    }

    buffer[position++] = cf->import.tag;
    U2ToBigIndian(buffer + position, cf->import.size);
    position += 2;
    buffer[position++] = cf->import.count;

    for(; u1Index < cf->import.count; ++u1Index) {
        buffer[position++] = cf->import.packages[u1Index].minor_version;
        buffer[position++] = cf->import.packages[u1Index].major_version;
        buffer[position++] = cf->import.packages[u1Index].AID_length;
        memcpy(buffer + position, cf->import.packages[u1Index].AID, sizeof(u1) * cf->import.packages[u1Index].AID_length);
        position += cf->import.packages[u1Index].AID_length;
    }

    snprintf(name, 1024, "%sImport.cap", cf->path);
    return addToZip(z, name, buffer, cf->import.size + 3u);

}


int writeConstantPool(struct zip* z, cap_file* cf) {

    char name[1024];
    u2 u2Index = 0;

    unsigned int position = 0;
    char* buffer = (char*)malloc(sizeof(char) * (cf->constant_pool.size + 3u));
    if(buffer == NULL) {
        perror("writeConstantPool");
        return -1;
    }

    buffer[position++] = cf->constant_pool.tag;
    U2ToBigIndian(buffer + position, cf->constant_pool.size);
    position += 2;
    U2ToBigIndian(buffer + position, cf->constant_pool.count);
    position += 2;

    for(; u2Index < cf->constant_pool.count; ++u2Index) {
        buffer[position++] = cf->constant_pool.constant_pool[u2Index].tag;
        switch(cf->constant_pool.constant_pool[u2Index].tag) {
            case CF_CONSTANT_CLASSREF:
                if(cf->constant_pool.constant_pool[u2Index].CONSTANT_Classref.class_ref.isExternal) {
                    buffer[position++] = cf->constant_pool.constant_pool[u2Index].CONSTANT_Classref.class_ref.ref.external_class_ref.package_token | 0x80;
                    buffer[position++] = cf->constant_pool.constant_pool[u2Index].CONSTANT_Classref.class_ref.ref.external_class_ref.class_token;
                } else {
                    U2ToBigIndian(buffer + position, cf->constant_pool.constant_pool[u2Index].CONSTANT_Classref.class_ref.ref.internal_class_ref);
                    position += 2;
                }
                buffer[position++] = cf->constant_pool.constant_pool[u2Index].CONSTANT_Classref.padding;
                break;

            case CF_CONSTANT_INSTANCEFIELDREF:
                if(cf->constant_pool.constant_pool[u2Index].CONSTANT_InstanceFieldref.class.isExternal) {
                    buffer[position++] = cf->constant_pool.constant_pool[u2Index].CONSTANT_InstanceFieldref.class.ref.external_class_ref.package_token | 0x80;
                    buffer[position++] = cf->constant_pool.constant_pool[u2Index].CONSTANT_InstanceFieldref.class.ref.external_class_ref.class_token;
                } else {
                    U2ToBigIndian(buffer + position, cf->constant_pool.constant_pool[u2Index].CONSTANT_InstanceFieldref.class.ref.internal_class_ref);
                    position += 2;
                }
                buffer[position++] = cf->constant_pool.constant_pool[u2Index].CONSTANT_InstanceFieldref.token;
                break;

            case CF_CONSTANT_VIRTUALMETHODREF:
                if(cf->constant_pool.constant_pool[u2Index].CONSTANT_VirtualMethodref.class.isExternal) {
                    buffer[position++] = cf->constant_pool.constant_pool[u2Index].CONSTANT_VirtualMethodref.class.ref.external_class_ref.package_token | 0x80;
                    buffer[position++] = cf->constant_pool.constant_pool[u2Index].CONSTANT_VirtualMethodref.class.ref.external_class_ref.class_token;
                } else {
                    U2ToBigIndian(buffer + position, cf->constant_pool.constant_pool[u2Index].CONSTANT_VirtualMethodref.class.ref.internal_class_ref);
                    position += 2;
                }
                buffer[position++] = cf->constant_pool.constant_pool[u2Index].CONSTANT_VirtualMethodref.token;
                break;

            case CF_CONSTANT_SUPERMETHODREF:
                if(cf->constant_pool.constant_pool[u2Index].CONSTANT_SuperMethodref.class.isExternal) {
                    buffer[position++] = cf->constant_pool.constant_pool[u2Index].CONSTANT_SuperMethodref.class.ref.external_class_ref.package_token | 0x80;
                    buffer[position++] = cf->constant_pool.constant_pool[u2Index].CONSTANT_SuperMethodref.class.ref.external_class_ref.class_token;
                } else {
                    U2ToBigIndian(buffer + position, cf->constant_pool.constant_pool[u2Index].CONSTANT_SuperMethodref.class.ref.internal_class_ref);
                    position += 2;
                }
                buffer[position++] = cf->constant_pool.constant_pool[u2Index].CONSTANT_SuperMethodref.token;
                break;

            case CF_CONSTANT_STATICFIELDREF:
                if(cf->constant_pool.constant_pool[u2Index].CONSTANT_StaticFieldref.static_field_ref.isExternal) {
                    buffer[position++] = cf->constant_pool.constant_pool[u2Index].CONSTANT_StaticFieldref.static_field_ref.ref.external_ref.package_token | 0x80;
                    buffer[position++] = cf->constant_pool.constant_pool[u2Index].CONSTANT_StaticFieldref.static_field_ref.ref.external_ref.class_token;
                    buffer[position++] = cf->constant_pool.constant_pool[u2Index].CONSTANT_StaticFieldref.static_field_ref.ref.external_ref.token;
                } else {
                    buffer[position++] = cf->constant_pool.constant_pool[u2Index].CONSTANT_StaticFieldref.static_field_ref.ref.internal_ref.padding;
                    U2ToBigIndian(buffer + position, cf->constant_pool.constant_pool[u2Index].CONSTANT_StaticFieldref.static_field_ref.ref.internal_ref.offset);
                    position += 2;
                }
                break;

            case CF_CONSTANT_STATICMETHODREF:
                if(cf->constant_pool.constant_pool[u2Index].CONSTANT_StaticMethodref.static_method_ref.isExternal) {
                    buffer[position++] = cf->constant_pool.constant_pool[u2Index].CONSTANT_StaticMethodref.static_method_ref.ref.external_ref.package_token | 0x80;
                    buffer[position++] = cf->constant_pool.constant_pool[u2Index].CONSTANT_StaticMethodref.static_method_ref.ref.external_ref.class_token;
                    buffer[position++] = cf->constant_pool.constant_pool[u2Index].CONSTANT_StaticMethodref.static_method_ref.ref.external_ref.token;
                } else {
                    buffer[position++] = cf->constant_pool.constant_pool[u2Index].CONSTANT_StaticMethodref.static_method_ref.ref.internal_ref.padding;
                    U2ToBigIndian(buffer + position, cf->constant_pool.constant_pool[u2Index].CONSTANT_StaticMethodref.static_method_ref.ref.internal_ref.offset);
                    position += 2;
                }
                break;

            default:
                fprintf(stderr, "Constant pool tag %u not supported\n", cf->constant_pool.constant_pool[u2Index].tag);
                return -1;
        }

    }

    snprintf(name, 1024, "%sConstantPool.cap", cf->path);
    return addToZip(z, name, buffer, cf->constant_pool.size + 3u);

}


int writeClass(struct zip* z, cap_file* cf) {

    char name[1024];
    u2 u2Index = 0;

    unsigned int position = 0;
    char* buffer = (char*)malloc(sizeof(char) * (cf->class.size + 3u));
    if(buffer == NULL) {
        perror("writeClass");
        return -1;
    }

    buffer[position++] = cf->class.tag;
    U2ToBigIndian(buffer + position, cf->class.size);
    position += 2;

    if((cf->header.major_version == 2) && (cf->header.minor_version > 1)) {
        U2ToBigIndian(buffer + position, cf->class.signature_pool_length);
        position += 2;

        for(; u2Index < cf->class.signature_pool_count; ++u2Index) {
            u1 count = (cf->class.signature_pool[u2Index].nibble_count + 1) / 2;
            buffer[position++] = cf->class.signature_pool[u2Index].nibble_count;
            memcpy(buffer + position, cf->class.signature_pool[u2Index].type, sizeof(u1) * count);
            position += count;
        }
    }

    for(u2Index = 0; u2Index < cf->class.interfaces_count; ++u2Index) {
        u1 u1Index = 0;
        buffer[position++] = (cf->class.interfaces[u2Index].flags << 4) | (cf->class.interfaces[u2Index].interface_count & 0x0F);
        for(; u1Index < cf->class.interfaces[u2Index].interface_count; ++u1Index) {
            if(cf->class.interfaces[u2Index].superinterfaces[u1Index].isExternal) {
                buffer[position++] = cf->class.interfaces[u2Index].superinterfaces[u1Index].ref.external_class_ref.package_token | 0x80;
                buffer[position++] = cf->class.interfaces[u2Index].superinterfaces[u1Index].ref.external_class_ref.class_token;
            } else {
                U2ToBigIndian(buffer + position, cf->class.interfaces[u2Index].superinterfaces[u1Index].ref.internal_class_ref);
                position += 2;
            }
        }
        if(cf->class.interfaces[u2Index].flags & CLASS_ACC_REMOTE) {
            buffer[position++] = cf->class.interfaces[u2Index].interface_name.interface_name_length;
            memcpy(buffer + position, cf->class.interfaces[u2Index].interface_name.interface_name, sizeof(u1) * cf->class.interfaces[u2Index].interface_name.interface_name_length);
            position += cf->class.interfaces[u2Index].interface_name.interface_name_length;
        }
    }

    for(u2Index = 0; u2Index < cf->class.classes_count; ++u2Index) {
        u1 u1Index = 0;
        buffer[position++] = (cf->class.classes[u2Index].flags << 4) | (cf->class.classes[u2Index].interface_count & 0x0F);

        if(cf->class.classes[u2Index].has_superclass) {
            if(cf->class.classes[u2Index].super_class_ref.isExternal) {
                buffer[position++] = cf->class.classes[u2Index].super_class_ref.ref.external_class_ref.package_token | 0x80;
                buffer[position++] = cf->class.classes[u2Index].super_class_ref.ref.external_class_ref.class_token;
            } else {
                U2ToBigIndian(buffer + position, cf->class.classes[u2Index].super_class_ref.ref.internal_class_ref);
                position += 2;
            }
        } else {
            buffer[position++] |= 0xFF;
            buffer[position++] |= 0xFF;
        }

        buffer[position++] = cf->class.classes[u2Index].declared_instance_size;
        buffer[position++] = cf->class.classes[u2Index].first_reference_token;
        buffer[position++] = cf->class.classes[u2Index].reference_count;
        buffer[position++] = cf->class.classes[u2Index].public_method_table_base;
        buffer[position++] = cf->class.classes[u2Index].public_method_table_count;
        buffer[position++] = cf->class.classes[u2Index].package_method_table_base;
        buffer[position++] = cf->class.classes[u2Index].package_method_table_count;

        for(; u1Index < cf->class.classes[u2Index].public_method_table_count; ++u1Index) {
            U2ToBigIndian(buffer + position, cf->class.classes[u2Index].public_virtual_method_table[u1Index]);
            position += 2;
        }

        for(u1Index = 0; u1Index < cf->class.classes[u2Index].package_method_table_count; ++u1Index) {
            U2ToBigIndian(buffer + position, cf->class.classes[u2Index].package_virtual_method_table[u1Index]);
            position += 2;
        }

        for(u1Index = 0; u1Index < cf->class.classes[u2Index].interface_count; ++u1Index) {
            if(cf->class.classes[u2Index].interfaces[u1Index].interface.isExternal) {
                buffer[position++] = cf->class.classes[u2Index].interfaces[u1Index].interface.ref.external_class_ref.package_token | 0x80;
                buffer[position++] = cf->class.classes[u2Index].interfaces[u1Index].interface.ref.external_class_ref.class_token;
            } else {
                U2ToBigIndian(buffer + position, cf->class.classes[u2Index].interfaces[u1Index].interface.ref.internal_class_ref);
                position += 2;
            }
            buffer[position++] = cf->class.classes[u2Index].interfaces[u1Index].count;
            memcpy(buffer + position, cf->class.classes[u2Index].interfaces[u1Index].index, sizeof(u1) * cf->class.classes[u2Index].interfaces[u1Index].count);
            position += cf->class.classes[u2Index].interfaces[u1Index].count;
        }

        if(cf->class.classes[u2Index].flags & CLASS_ACC_REMOTE) {
            buffer[position++] = cf->class.classes[u2Index].remote_interfaces.remote_methods_count;

            for(u1Index = 0; u1Index < cf->class.classes[u2Index].remote_interfaces.remote_methods_count; ++u1Index) {
                U2ToBigIndian(buffer + position, cf->class.classes[u2Index].remote_interfaces.remote_methods[u1Index].remote_method_hash);
                position += 2;
                U2ToBigIndian(buffer + position, cf->class.classes[u2Index].remote_interfaces.remote_methods[u1Index].signature_offset);
                position += 2;
                buffer[position++] = cf->class.classes[u2Index].remote_interfaces.remote_methods[u1Index].virtual_method_token;
            }

            buffer[position++] = cf->class.classes[u2Index].remote_interfaces.hash_modifier_length;
            memcpy(buffer + position, cf->class.classes[u2Index].remote_interfaces.hash_modifier, sizeof(u1) * cf->class.classes[u2Index].remote_interfaces.hash_modifier_length);
            position += cf->class.classes[u2Index].remote_interfaces.hash_modifier_length;

            buffer[position++] = cf->class.classes[u2Index].remote_interfaces.class_name_length;
            memcpy(buffer + position, cf->class.classes[u2Index].remote_interfaces.class_name, sizeof(u1) * cf->class.classes[u2Index].remote_interfaces.class_name_length);
            position += cf->class.classes[u2Index].remote_interfaces.class_name_length;

            buffer[position++] = cf->class.classes[u2Index].remote_interfaces.remote_interfaces_count;
            for(u1Index = 0; u1Index < cf->class.classes[u2Index].remote_interfaces.remote_interfaces_count; ++u1Index) {
                if(cf->class.classes[u2Index].remote_interfaces.remote_interfaces[u1Index].isExternal) {
                    buffer[position++] = cf->class.classes[u2Index].remote_interfaces.remote_interfaces[u1Index].ref.external_class_ref.package_token | 0x80;
                    buffer[position++] = cf->class.classes[u2Index].remote_interfaces.remote_interfaces[u1Index].ref.external_class_ref.class_token;
                } else {
                    U2ToBigIndian(buffer + position, cf->class.classes[u2Index].remote_interfaces.remote_interfaces[u1Index].ref.internal_class_ref);
                    position += 2;
                }
            }
        }
    }

    snprintf(name, 1024, "%sClass.cap", cf->path);
    return addToZip(z, name, buffer, cf->class.size + 3u);

}


int writeMethod(struct zip* z, cap_file* cf) {

    char name[1024];
    u1 u1Index = 0;
    u2 u2Index = 0;

    unsigned int position = 0;
    char* buffer = (char*)malloc(sizeof(char) * (cf->method.size + 3u));
    if(buffer == NULL) {
        perror("writeMethod");
        return -1;
    }

    buffer[position++] = cf->method.tag;
    U2ToBigIndian(buffer + position, cf->method.size);
    position += 2;
    buffer[position++] = cf->method.handler_count;

    for(; u1Index < cf->method.handler_count; ++u1Index) {
        U2ToBigIndian(buffer + position, cf->method.exception_handlers[u1Index].start_offset);
        position += 2;
        U2ToBigIndian(buffer + position, (cf->method.exception_handlers[u1Index].stop_bit << 15) | (cf->method.exception_handlers[u1Index].active_length & 0x7F));
        position += 2;
        U2ToBigIndian(buffer + position, cf->method.exception_handlers[u1Index].handler_offset);
        position += 2;
        U2ToBigIndian(buffer + position, cf->method.exception_handlers[u1Index].catch_type_index);
        position += 2;
    }

    for(; u2Index < cf->method.method_count; ++u2Index) {
        if(cf->method.methods[u2Index].method_header.flags & METHOD_ACC_EXTENDED) {
            buffer[position++] = (cf->method.methods[u2Index].method_header.extended_method_header.flags << 4) | (cf->method.methods[u2Index].method_header.extended_method_header.padding & 0x0F);
            buffer[position++] = cf->method.methods[u2Index].method_header.extended_method_header.max_stack;
            buffer[position++] = cf->method.methods[u2Index].method_header.extended_method_header.nargs;
            buffer[position++] = cf->method.methods[u2Index].method_header.extended_method_header.max_locals;
        } else {
            buffer[position++] = (cf->method.methods[u2Index].method_header.standard_method_header.flags << 4) | (cf->method.methods[u2Index].method_header.standard_method_header.max_stack & 0x0F);
            buffer[position++] = (cf->method.methods[u2Index].method_header.standard_method_header.nargs << 4) | (cf->method.methods[u2Index].method_header.standard_method_header.max_locals & 0x0F);
        }

        memcpy(buffer + position, cf->method.methods[u2Index].bytecodes, sizeof(u1) * cf->method.methods[u2Index].bytecode_count);
        position += cf->method.methods[u2Index].bytecode_count;
    }

    snprintf(name, 1024, "%sMethod.cap", cf->path);
    return addToZip(z, name, buffer, cf->method.size + 3u);

}


int writeStaticField(struct zip* z, cap_file* cf) {

    char name[1024];
    u2 u2Index = 0;

    unsigned int position = 0;
    char* buffer = (char*)malloc(sizeof(char) * (cf->static_field.size + 3u));
    if(buffer == NULL) {
        perror("writeStaticField");
        return -1;
    }

    buffer[position++] = cf->static_field.tag;
    U2ToBigIndian(buffer + position, cf->static_field.size);
    position += 2;
    U2ToBigIndian(buffer + position, cf->static_field.image_size);
    position += 2;
    U2ToBigIndian(buffer + position, cf->static_field.reference_count);
    position += 2;
    U2ToBigIndian(buffer + position, cf->static_field.array_init_count);
    position += 2;

    for(; u2Index < cf->static_field.array_init_count; ++u2Index) {
        buffer[position++] = cf->static_field.array_init[u2Index].type;
        U2ToBigIndian(buffer + position, cf->static_field.array_init[u2Index].count);
        position += 2;
        memcpy(buffer + position, cf->static_field.array_init[u2Index].values, sizeof(u1) * cf->static_field.array_init[u2Index].count);
        position += cf->static_field.array_init[u2Index].count;
    }

    U2ToBigIndian(buffer + position, cf->static_field.default_value_count);
    position += 2;
    U2ToBigIndian(buffer + position, cf->static_field.non_default_value_count);
    position += 2;
    memcpy(buffer + position, cf->static_field.non_default_values, sizeof(u1) * cf->static_field.non_default_value_count);
    position += cf->static_field.non_default_value_count;

    snprintf(name, 1024, "%sStaticField.cap", cf->path);
    return addToZip(z, name, buffer, cf->static_field.size + 3u);

}


int writeReferenceLocation(struct zip* z, cap_file* cf) {

    char name[1024];

    unsigned int position = 0;
    char* buffer = (char*)malloc(sizeof(char) * (cf->reference_location.size + 3u));
    if(buffer == NULL) {
        perror("writeReferenceLocation");
        return -1;
    }

    buffer[position++] = cf->reference_location.tag;
    U2ToBigIndian(buffer + position, cf->reference_location.size);
    position += 2;
    U2ToBigIndian(buffer + position, cf->reference_location.byte_index_count);
    position += 2;
    memcpy(buffer + position, cf->reference_location.offset_to_byte_indices, sizeof(u1) * cf->reference_location.byte_index_count);
    position += cf->reference_location.byte_index_count;
    U2ToBigIndian(buffer + position, cf->reference_location.byte2_index_count);
    position += 2;
    memcpy(buffer + position, cf->reference_location.offset_to_byte2_indices, sizeof(u1) * cf->reference_location.byte2_index_count);
    position += cf->reference_location.byte2_index_count;

    snprintf(name, 1024, "%sRefLocation.cap", cf->path);
    return addToZip(z, name, buffer, cf->reference_location.size + 3u);

}


int writeExport(struct zip* z, cap_file* cf) {

    char name[1024];
    u1 u1Index = 0;

    unsigned int position = 0;
    char* buffer = (char*)malloc(sizeof(char) * (cf->export.size + 3u));
    if(buffer == NULL) {
        perror("writeExport");
        return -1;
    }

    buffer[position++] = cf->export.tag;
    U2ToBigIndian(buffer + position, cf->export.size);
    position += 2;
    buffer[position++] = cf->export.class_count;

    for(; u1Index < cf->export.class_count; ++u1Index) {
        u1 i = 0;
        U2ToBigIndian(buffer + position, cf->export.class_exports[u1Index].class_offset);
        position += 2;
        buffer[position++] = cf->export.class_exports[u1Index].static_field_count;
        buffer[position++] = cf->export.class_exports[u1Index].static_method_count;

        for(; i < cf->export.class_exports[u1Index].static_field_count; ++i) {
            U2ToBigIndian(buffer + position, cf->export.class_exports[u1Index].static_field_offsets[i]);
            position += 2;
        }

        for(i = 0; i < cf->export.class_exports[u1Index].static_method_count; ++i) {
            U2ToBigIndian(buffer + position, cf->export.class_exports[u1Index].static_method_offsets[i]);
            position += 2;
        }
    }

    snprintf(name, 1024, "%sExport.cap", cf->path);
    return addToZip(z, name, buffer, cf->export.size + 3u);

}


int writeDescriptor(struct zip* z, cap_file* cf) {

    char name[1024];
    u1 u1Index = 0;
    u2 u2Index = 0;

    unsigned int position = 0;
    char* buffer = (char*)malloc(sizeof(char) * (cf->descriptor.size + 3u));
    if(buffer == NULL) {
        perror("writeDescriptor");
        return -1;
    }

    buffer[position++] = cf->descriptor.tag;
    U2ToBigIndian(buffer + position, cf->descriptor.size);
    position += 2;
    buffer[position++] = cf->descriptor.class_count;

    for(; u1Index < cf->descriptor.class_count; ++u1Index) {
        u1 i = 0;
        buffer[position++] = cf->descriptor.classes[u1Index].token;
        buffer[position++] = cf->descriptor.classes[u1Index].access_flags;
        if(cf->descriptor.classes[u1Index].this_class_ref.isExternal) {
            buffer[position++] = cf->descriptor.classes[u1Index].this_class_ref.ref.external_class_ref.package_token | 0x80;
            buffer[position++] = cf->descriptor.classes[u1Index].this_class_ref.ref.external_class_ref.class_token;
        } else {
            U2ToBigIndian(buffer + position, cf->descriptor.classes[u1Index].this_class_ref.ref.internal_class_ref);
            position += 2;
        }
        buffer[position++] = cf->descriptor.classes[u1Index].interface_count;
        U2ToBigIndian(buffer + position, cf->descriptor.classes[u1Index].field_count);
        position += 2;
        U2ToBigIndian(buffer + position, cf->descriptor.classes[u1Index].method_count);
        position += 2;

        for(; i < cf->descriptor.classes[u1Index].interface_count; ++i) {
            if(cf->descriptor.classes[u1Index].interfaces[i].isExternal) {
                buffer[position++] = cf->descriptor.classes[u1Index].interfaces[i].ref.external_class_ref.package_token | 0x80;
                buffer[position++] = cf->descriptor.classes[u1Index].interfaces[i].ref.external_class_ref.class_token;
            } else {
                U2ToBigIndian(buffer + position, cf->descriptor.classes[u1Index].interfaces[i].ref.internal_class_ref);
                position += 2;
            }
        }

        for(u2Index = 0; u2Index < cf->descriptor.classes[u1Index].field_count; ++u2Index) {
            buffer[position++] = cf->descriptor.classes[u1Index].fields[u2Index].token;
            buffer[position++] = cf->descriptor.classes[u1Index].fields[u2Index].access_flags;
            if(cf->descriptor.classes[u1Index].fields[u2Index].access_flags & DESCRIPTOR_ACC_STATIC) {
                if(cf->descriptor.classes[u1Index].fields[u2Index].field_ref.static_field.isExternal) {
                    buffer[position++] = cf->descriptor.classes[u1Index].fields[u2Index].field_ref.static_field.ref.external_ref.package_token | 0x80;
                    buffer[position++] = cf->descriptor.classes[u1Index].fields[u2Index].field_ref.static_field.ref.external_ref.class_token;
                    buffer[position++] = cf->descriptor.classes[u1Index].fields[u2Index].field_ref.static_field.ref.external_ref.token;
                } else {
                    buffer[position++] = cf->descriptor.classes[u1Index].fields[u2Index].field_ref.static_field.ref.internal_ref.padding; 
                    U2ToBigIndian(buffer + position, cf->descriptor.classes[u1Index].fields[u2Index].field_ref.static_field.ref.internal_ref.offset);
                    position += 2;
                }
            } else {
                if(cf->descriptor.classes[u1Index].fields[u2Index].field_ref.instance_field.class_ref.isExternal) {
                    buffer[position++] = cf->descriptor.classes[u1Index].fields[u2Index].field_ref.instance_field.class_ref.ref.external_class_ref.package_token | 0x80;
                    buffer[position++] = cf->descriptor.classes[u1Index].fields[u2Index].field_ref.instance_field.class_ref.ref.external_class_ref.class_token;
                } else {
                    U2ToBigIndian(buffer + position, cf->descriptor.classes[u1Index].fields[u2Index].field_ref.instance_field.class_ref.ref.internal_class_ref);
                    position += 2;
                }
                buffer[position++] = cf->descriptor.classes[u1Index].fields[u2Index].field_ref.instance_field.token;
            }
            if(cf->descriptor.classes[u1Index].fields[u2Index].type.primitive_type & 0x8000) {
                U2ToBigIndian(buffer + position, cf->descriptor.classes[u1Index].fields[u2Index].type.primitive_type);
                position += 2;
            } else {
                U2ToBigIndian(buffer + position, cf->descriptor.classes[u1Index].fields[u2Index].type.reference_type);
                position += 2;
            }
        }

        for(u2Index = 0; u2Index < cf->descriptor.classes[u1Index].method_count; ++u2Index) {
            buffer[position++] = cf->descriptor.classes[u1Index].methods[u2Index].token;
            buffer[position++] = cf->descriptor.classes[u1Index].methods[u2Index].access_flags;
            U2ToBigIndian(buffer + position, cf->descriptor.classes[u1Index].methods[u2Index].method_offset);
            position += 2;
            U2ToBigIndian(buffer + position, cf->descriptor.classes[u1Index].methods[u2Index].type_offset);
            position += 2;
            U2ToBigIndian(buffer + position, cf->descriptor.classes[u1Index].methods[u2Index].bytecode_count);
            position += 2;
            U2ToBigIndian(buffer + position, cf->descriptor.classes[u1Index].methods[u2Index].exception_handler_count);
            position += 2;
            U2ToBigIndian(buffer + position, cf->descriptor.classes[u1Index].methods[u2Index].exception_handler_index);
            position += 2;
        }

    }

    U2ToBigIndian(buffer + position, cf->descriptor.types.constant_pool_count);
    position += 2;

    for(u2Index = 0; u2Index < cf->descriptor.types.constant_pool_count; ++u2Index) {
        U2ToBigIndian(buffer + position, cf->descriptor.types.constant_pool_types[u2Index]);
        position += 2;
    }

    for(u2Index = 0; u2Index < cf->descriptor.types.type_desc_count; ++u2Index) {
        u1 count = (cf->descriptor.types.type_desc[u2Index].nibble_count + 1) / 2;
        buffer[position++] = cf->descriptor.types.type_desc[u2Index].nibble_count;
        memcpy(buffer + position, cf->descriptor.types.type_desc[u2Index].type, sizeof(u1) * count);
        position += count;
    }

    snprintf(name, 1024, "%sDescriptor.cap", cf->path);
    return addToZip(z, name, buffer, cf->descriptor.size + 3u);

}


int writeDebug(struct zip* z, cap_file* cf) {

    char name[1024];
    u1 u1Index = 0;
    u2 u2Index = 0;

    unsigned int position = 0;
    char* buffer = (char*)malloc(sizeof(char) * (cf->debug.size + 3u));
    if(buffer == NULL) {
        perror("writeDebug");
        return -1;
    }

    buffer[position++] = cf->debug.tag;
    U2ToBigIndian(buffer + position, cf->debug.size);
    position += 2;
    U2ToBigIndian(buffer + position, cf->debug.string_count);
    position += 2;

    for(; u2Index < cf->debug.string_count; ++u2Index) {
        U2ToBigIndian(buffer + position, cf->debug.strings_table[u2Index].length);
        position += 2;
        memcpy(buffer + position, cf->debug.strings_table[u2Index].bytes, sizeof(u1) * cf->debug.strings_table[u2Index].length);
        position += cf->debug.strings_table[u2Index].length;
    }

    U2ToBigIndian(buffer + position, cf->debug.package_name_index);
    position += 2;
    U2ToBigIndian(buffer + position, cf->debug.class_count);
    position += 2;

    for(u2Index = 0; u2Index < cf->debug.class_count; ++u2Index) {
        u2 i = 0;
        U2ToBigIndian(buffer + position, cf->debug.classes[u2Index].name_index);
        position += 2;
        U2ToBigIndian(buffer + position, cf->debug.classes[u2Index].access_flags);
        position += 2;
        U2ToBigIndian(buffer + position, cf->debug.classes[u2Index].location);
        position += 2;
        U2ToBigIndian(buffer + position, cf->debug.classes[u2Index].superclass_name_index);
        position += 2;
        U2ToBigIndian(buffer + position, cf->debug.classes[u2Index].source_file_index);
        position += 2;
        buffer[position++] = cf->debug.classes[u2Index].interface_count;
        U2ToBigIndian(buffer + position, cf->debug.classes[u2Index].field_count);
        position += 2;
        U2ToBigIndian(buffer + position, cf->debug.classes[u2Index].method_count);
        position += 2;

        for(u1Index = 0; u1Index < cf->debug.classes[u2Index].interface_count; ++u1Index) {
            U2ToBigIndian(buffer + position, cf->debug.classes[u2Index].interface_names_indexes[u1Index]);
            position += 2;
        }

        for(; i < cf->debug.classes[u2Index].field_count; ++i) {
            U2ToBigIndian(buffer + position, cf->debug.classes[u2Index].fields[i].name_index);
            position += 2;
            U2ToBigIndian(buffer + position, cf->debug.classes[u2Index].fields[i].descriptor_index);
            position += 2;
            U2ToBigIndian(buffer + position, cf->debug.classes[u2Index].fields[i].access_flags);
            position += 2;
            if((cf->debug.classes[u2Index].fields[i].access_flags & DEBUG_ACC_STATIC) &&
               (cf->debug.classes[u2Index].fields[i].access_flags & DEBUG_ACC_FINAL) &&
               ((cf->debug.strings_table[cf->debug.classes[u2Index].fields[i].descriptor_index].bytes[0] == 'B') ||
                (cf->debug.strings_table[cf->debug.classes[u2Index].fields[i].descriptor_index].bytes[0] == 'Z') ||
                (cf->debug.strings_table[cf->debug.classes[u2Index].fields[i].descriptor_index].bytes[0] == 'S') ||
                (cf->debug.strings_table[cf->debug.classes[u2Index].fields[i].descriptor_index].bytes[0] == 'I'))) {
                    U4ToBigIndian(buffer + position, cf->debug.classes[u2Index].fields[i].contents.const_value);
                    position += 4;
            } else if((cf->debug.classes[u2Index].fields[i].access_flags & DEBUG_ACC_STATIC) &&
                          (!(cf->debug.classes[u2Index].fields[i].access_flags & DEBUG_ACC_FINAL) ||
                          (cf->debug.strings_table[cf->debug.classes[u2Index].fields[i].descriptor_index].bytes[0] == '['))) {
                U2ToBigIndian(buffer + position, cf->debug.classes[u2Index].fields[i].contents.location_var.pad);
                position += 2;
                U2ToBigIndian(buffer + position, cf->debug.classes[u2Index].fields[i].contents.location_var.location);
                position += 2;
            } else {
                buffer[position++] = cf->debug.classes[u2Index].fields[i].contents.token_var.pad1;
                buffer[position++] = cf->debug.classes[u2Index].fields[i].contents.token_var.pad2;
                buffer[position++] = cf->debug.classes[u2Index].fields[i].contents.token_var.pad3;
                buffer[position++] = cf->debug.classes[u2Index].fields[i].contents.token_var.token;
            }
        }

        for(i = 0; i < cf->debug.classes[u2Index].method_count; ++i) {
            u2 j = 0;
            U2ToBigIndian(buffer + position, cf->debug.classes[u2Index].methods[i].name_index);
            position += 2;
            U2ToBigIndian(buffer + position, cf->debug.classes[u2Index].methods[i].descriptor_index);
            position += 2;
            U2ToBigIndian(buffer + position, cf->debug.classes[u2Index].methods[i].access_flags);
            position += 2;
            U2ToBigIndian(buffer + position, cf->debug.classes[u2Index].methods[i].location);
            position += 2;
            buffer[position++] = cf->debug.classes[u2Index].methods[i].header_size;
            U2ToBigIndian(buffer + position, cf->debug.classes[u2Index].methods[i].body_size);
            position += 2;
            U2ToBigIndian(buffer + position, cf->debug.classes[u2Index].methods[i].variable_count);
            position += 2;
            U2ToBigIndian(buffer + position, cf->debug.classes[u2Index].methods[i].line_count);
            position += 2;
            for(; j < cf->debug.classes[u2Index].methods[i].variable_count; ++j) {
                buffer[position++] = cf->debug.classes[u2Index].methods[i].variable_table[j].index;
                U2ToBigIndian(buffer + position, cf->debug.classes[u2Index].methods[i].variable_table[j].name_index);
                position += 2;
                U2ToBigIndian(buffer + position, cf->debug.classes[u2Index].methods[i].variable_table[j].descriptor_index);
                position += 2;
                U2ToBigIndian(buffer + position, cf->debug.classes[u2Index].methods[i].variable_table[j].start_pc);
                position += 2;
                U2ToBigIndian(buffer + position, cf->debug.classes[u2Index].methods[i].variable_table[j].length);
                position += 2;
            }
            for(j = 0; j < cf->debug.classes[u2Index].methods[i].line_count; ++j) {
                U2ToBigIndian(buffer + position, cf->debug.classes[u2Index].methods[i].line_table[j].start_pc);
                position += 2;
                U2ToBigIndian(buffer + position, cf->debug.classes[u2Index].methods[i].line_table[j].end_pc);
                position += 2;
                U2ToBigIndian(buffer + position, cf->debug.classes[u2Index].methods[i].line_table[j].source_line);
                position += 2;
            }
        }
    }

    snprintf(name, 1024, "%sDebug.cap", cf->path);
    return addToZip(z, name, buffer, cf->debug.size + 3u);

}

/**
 * \brief Write a straightforward CAP file representation into a file.
 * 
 * \param The straightforward representation of a CAP file.
 * \param filename The path to the file to write in.
 *
 * \return Return -1 if an error occurred, 0 else.
 */
int write_cap_file(cap_file* cf, const char* filename) {

    int error = 0;
    struct zip* z = NULL;

    open(filename, O_CREAT | O_TRUNC, S_IWUSR | S_IRUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);

    z = zip_open(filename, ZIP_CREATE, &error); 
    if(error != 0) {
        char buf[1024];
        zip_error_to_str(buf, 1024, error, errno);
        fprintf(stderr, "writeCapFile: %s\n", buf);
        return -1;
    }

    if(writeManifest(z, cf) == -1) {
        zip_close(z);
        return -1;
    } else
        printf("Manifest written\n");

    if(cf->header.tag != 0) {
        if(writeHeader(z, cf) == -1) {
            zip_close(z);
            return -1;
        } else
            printf("Header written\n");
    }

    if(cf->directory.tag != 0) {
        if(writeDirectory(z, cf) == -1) {
            zip_close(z);
            return -1;
        } else
            printf("Directory written\n");
    }

    if(cf->applet.tag != 0) {
        if(writeApplet(z, cf) == -1) {
            zip_close(z);
            return -1;
        } else
            printf("Applet written\n");
    }

    if(cf->import.tag != 0) {
        if(writeImport(z, cf) == -1) {
            zip_close(z);
            return -1;
        } else
            printf("Import written\n");
    }

    if(cf->constant_pool.tag != 0) {
        if(writeConstantPool(z, cf) == -1) {
            zip_close(z);
            return -1;
        } else
            printf("ConstantPool written\n");
    }

    if(cf->class.tag != 0) {
        if(writeClass(z, cf) == -1) {
            zip_close(z);
            return -1;
        } else
            printf("Class written\n");
    }

    if(cf->method.tag != 0) {
        if(writeMethod(z, cf) == -1) {
            zip_close(z);
            return -1;
        } else
            printf("Method written\n");
    }

    if(cf->static_field.tag != 0) {
        if(writeStaticField(z, cf) == -1) {
            zip_close(z);
            return -1;
        } else
            printf("StaticField written\n");
    }

    if(cf->reference_location.tag != 0) {
        if(writeReferenceLocation(z, cf) == -1) {
            zip_close(z);
            return -1;
        } else
            printf("RefLocation written\n");
    }

    if(cf->export.tag != 0) {
        if(writeExport(z, cf) == -1) {
            zip_close(z);
            return -1;
        } else
            printf("Export written\n");
    }

    if(cf->descriptor.tag != 0) {
        if(writeDescriptor(z, cf) == -1) {
            zip_close(z);
            return -1;
        } else
            printf("Descriptor written\n");
    }

    if(cf->debug.tag != 0) {
        if(writeDebug(z, cf) == -1) {
            zip_close(z);
            return -1;
        } else
            printf("Debug written\n");
    }

    zip_close(z);

    return 0;

}
